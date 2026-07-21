#pragma once
#include <raylib.h>
#include "entity.hpp"
#include "entitymanager.hpp"
#include "gameengine.hpp"
#include <memory>

/**
* Contains information about an item. Loaded into the gameEngine's item map at runtime and is dynamically appended to.
* 
* Items that do not exist and need to be used will be added to the item map. If the item is already in the item map and needs to be used, then the information will be pulled.
*/
struct itemSpec {
	std::string type;
	std::string name;
	int damage = 0;
	float speed = 0;
	int rarity = 0;
	int cost = 0;
	int manacost = 0;
	int lifespan = 0;
};

/**
 * Contains all weapon information for the game
 *
 * Weapons are defined in the weapons.txt file and loaded into the game at runtime. Each weapon has a name, damage, speed, and range.
 */

void usePrimaryWeapon(std::shared_ptr<Entity> e, EntityManager& manager, GameEngine* engine);
void useSecondaryWeapon(std::shared_ptr<Entity> e);
void useActiveItem(std::shared_ptr<Entity> e);



