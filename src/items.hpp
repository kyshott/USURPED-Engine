#pragma once
#include <raylib.h>
#include <memory>
#include <string>

class GameEngine; // for circular dependency
class EntityManager;
class Entity;

/**
* Contains information about an item. Loaded into the gameEngine's item map.
* 
* This is to avoid having to repeatedly load item information from disk every time an item is created.
*/

struct StatusEffect {
	std::string id;
	std::string type;
	int duration = 0;
	int magnitude = 0;
};

struct ItemSpec {
	std::string id;
	std::string type;
	std::string name;
	std::string description;
	int damage = 0;
	int speed = 0;
	int rarity = 0;
	int cost = 0;
	int manacost = 0;
	int knockback = 0;
	int lifespan = 0;
};

struct WeaponSpec {
	std::string id;
	std::string type;
	std::string name;
	std::string description;
	int damage = 0;
	int cost = 0;
	StatusEffect effect;
};

struct MagicSpec {
	std::string id;
	std::string type;
	std::string name;
	std::string description;
	int damage = 0;
	int cost = 0;
	int manacost = 0;
	StatusEffect effect;
};

// Weapon/item usage functions
void usePrimaryWeapon(std::shared_ptr<Entity> player, EntityManager& manager, GameEngine* engine);
void useSecondaryWeapon(std::shared_ptr<Entity> e);
void useActiveItem(std::shared_ptr<Entity> e);



