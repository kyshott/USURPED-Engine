#include <raylib.h>
#include "items.hpp"
#include "entityManager.hpp"
#include <fstream>
#include "gameengine.hpp"

/**
* Swings weapon entity in an arc based on the player's direction.
*
* Definitive behavior for all melee weapons. The longer the weapons lifespan, the longer the swing takes.
* 
* @param e shared pointer to the weapon entity
* @param player shared pointer to the player entity
* @param manager reference to the entity manager
* @param engine raw pointer to the game engine
*/
void weaponSwing(std::shared_ptr<Entity> e, std::shared_ptr<Entity> player, EntityManager& manager, GameEngine* engine) {
    CTransform& playerTransform = player->getComponent<CTransform>();
    CTransform& weaponTransform = e->getComponent<CTransform>();
    CLifespan& life = e->getComponent<CLifespan>();

    float progress = 1.0f - (static_cast<float>(life.remaining) / static_cast<float>(life.total));
    float radius = static_cast<float>(engine->getTileSizeX() + 15.0f);

    float startAngle = 0.0f;
    float endAngle = 0.0f;

    if (playerTransform.facing.y == 1) {
        startAngle = 225.0f;
        endAngle = 315.0f;
    }
    else if (playerTransform.facing.x == 1) {
        startAngle = -45.0f;
        endAngle = 45.0f;
    }
    else if (playerTransform.facing.y == -1) {
        startAngle = 45.0f;
        endAngle = 135.0f;
    }
    else if (playerTransform.facing.x == -1) {
        startAngle = 135.0f;
        endAngle = 225.0f;
    }

    float angle = startAngle + (endAngle - startAngle) * progress;
    float radians = angle * DEG2RAD;

    weaponTransform.prevPosition = weaponTransform.position;
    weaponTransform.position.x = playerTransform.position.x + std::cos(radians) * radius;
    weaponTransform.position.y = playerTransform.position.y + std::sin(radians) * radius;
    weaponTransform.angle = angle + 90.0f;
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
	std::string name = equipment.items["PRIMARY"];

	auto e = manager.addEntity("WEAPON", name);
	e->addComponent<CAnimation>(engine->getAssets().getAnimation(name), true);
	e->addComponent<CLifespan>(engine->getAssets().getItem(name).lifespan);
	e->getComponent<CLifespan>().remaining = engine->getAssets().getItem(name).lifespan;
    e->addComponent<CDamage>(engine->getAssets().getItem(name).damage);

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