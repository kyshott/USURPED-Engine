#include <raylib.h>
#include "items.hpp"
#include "entityManager.hpp"
#include <fstream>
#include "gameengine.hpp"

/**
* Spawns/uses the entity's primary weapon
* 
* The primary weapon is typically some form of melee weapon.
*/
void usePrimaryWeapon(std::shared_ptr<Entity> e, EntityManager& manager, GameEngine* engine) {
	CEquipment& equipment = e->getComponent<CEquipment>();
	std::string name = equipment.items["PRIMARY"];

	e = manager.addEntity("WEAPON", name);
	e->addComponent<CAnimation>(engine->getAssets().getAnimation(name), true);
	e->addComponent<CLifespan>(engine->getAssets().getItem(name).lifespan);
	e->addComponent<CDamage>(engine->getAssets().getItem(name).damage);
	e->addComponent<CTransform>(e->getComponent<CTransform>().position, Vec2(0, 0), 0);

}

void useSecondaryWeapon(std::shared_ptr<Entity> e) {

}

void useActiveItem(std::shared_ptr<Entity> e) {

}