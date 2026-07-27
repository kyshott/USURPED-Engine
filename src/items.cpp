#include <raylib.h>
#include "items.hpp"
#include "entityManager.hpp"
#include <fstream>
#include "gameengine.hpp"
#include "scenebattle.hpp"


void useItem(ItemSpec& item, SceneBattle& scene, std::shared_ptr<Entity> e) {
    if (item.effect.type == "RESTORE") {
        if (e->hasComponent<CHealth>()) {
            CHealth& health = e->getComponent<CHealth>();
            health.current += item.effect.magnitude;
            if (health.current > health.max) {
                health.current = health.max;
            }
			scene.drawDamageNumber(true, -item.effect.magnitude);
        }
    }
    else if (item.effect.type == "RESTOREM") {
        if (e->hasComponent<CHealth>()) {
            CHealth& health = e->getComponent<CHealth>();
            health.current -= item.effect.magnitude;
            if (health.current < 0) {
                health.current = 0;
            }
        }
	}
}

/**
* Spawns/uses the entity's primary weapon
* 
* The primary weapon is typically some form of melee weapon.
* 
* @param player shared pointer to the player entity
* @param manager reference to the entity manager
* @param engine raw pointer to the game engine
*/
void usePrimaryWeapon(std::shared_ptr<Entity> player, EntityManager& manager, GameEngine* engine) {
    engine->playSound("LINKSWING");
	CEquipment& equipment = player->getComponent<CEquipment>();
	std::string name = equipment.weapons[0].name;

	auto e = manager.addEntity("WEAPON", name);
	e->addComponent<CAnimation>(engine->getAssets().getAnimation(name), true);
    e->addComponent<CDamage>(equipment.weapons[0].damage);

    float bboxSizeX = engine->getAssets().getAnimation(name).getScaledSize().x;
    float bboxSizeY = engine->getAssets().getAnimation(name).getScaledSize().y;

    CTransform& ptsf = player->getComponent<CTransform>();
    float px = ptsf.position.x;
    float py = ptsf.position.y;
    e->addComponent<CTransform>();
    CTransform& transf = e->getComponent<CTransform>();
    if (ptsf.facing.y == 1) {
        e->addComponent<CBoundingBox>(Vec2(bboxSizeX, bboxSizeY));
        transf.position.x = px;
        transf.prevPosition.x = px;
        transf.position.y = py - engine->getTileSizeY();
        transf.prevPosition.y = py - engine->getTileSizeY();
        transf.angle = 0.0f;
    }
    else if (ptsf.facing.y == -1) {
        e->addComponent<CBoundingBox>(Vec2(bboxSizeX, bboxSizeY));
        transf.position.x = px;
        transf.prevPosition.x = px;
        transf.position.y = py + engine->getTileSizeY();
        transf.prevPosition.y = py + engine->getTileSizeY();
        transf.angle = 180.0f;
    }
    else if (ptsf.facing.x == 1) {
        e->addComponent<CBoundingBox>(Vec2(bboxSizeY, bboxSizeX));
        transf.position.x = px + engine->getTileSizeX();
        transf.prevPosition.x = px + engine->getTileSizeX();
        transf.position.y = py;
        transf.prevPosition.y = py;
        transf.angle = 90.0f;
    }
    else if (ptsf.facing.x == -1) {
        e->addComponent<CBoundingBox>(Vec2(bboxSizeY, bboxSizeX));
        transf.position.x = px - engine->getTileSizeX();
        transf.prevPosition.x = px - engine->getTileSizeX();
        transf.position.y = py;
        transf.prevPosition.y = py;
        transf.angle = 270.0f;
    }

    e->getComponent<CBoundingBox>().blocksVision = false;

}

void useSecondaryWeapon(std::shared_ptr<Entity> e) {

}

void useActiveItem(std::shared_ptr<Entity> e) {

}