#pragma once
#include <raylib.h>
#include "entity.hpp"
#include "entitymanager.hpp"
#include <memory>

class GameEngine; // for circular dependency

/**
* Contains information about an item. Loaded into the gameEngine's item map.
* 
* This is to avoid having to repeatedly load item information from disk every time an item is created.
*/

struct ItemSpec {
	std::string type;
	std::string name;
	int damage = 0;
	int speed = 0;
	int rarity = 0;
	int cost = 0;
	int manacost = 0;
	int lifespan = 0;
};

// Item behavior
void weaponSwing(std::shared_ptr<Entity> e, std::shared_ptr<Entity> player, EntityManager& manager, GameEngine* engine);


// Weapon/item usage functions
void usePrimaryWeapon(std::shared_ptr<Entity> player, EntityManager& manager, GameEngine* engine);
void useSecondaryWeapon(std::shared_ptr<Entity> e);
void useActiveItem(std::shared_ptr<Entity> e);



