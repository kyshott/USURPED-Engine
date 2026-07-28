#pragma once
#include <raylib.h>
#include <memory>
#include <string>

class SceneBattle;
class Entity;
class GameEngine;

/**
* Contains information about an item. Loaded into the gameEngine's item map.
* 
* This is to avoid having to repeatedly load item information from disk every time an item is created.
*/

struct Effect {
	std::string id;
	std::string type;
	std::string element;
	int magnitude = 0;
};

struct RingSpec {
	std::string id;
	std::string type;
	std::string name;
	std::string description;
	int hp = 0;
	int mp = 0;
	int speed = 0;
	int defense = 0;
	int cost = 0;
	int rarity = 0;
};

struct ItemSpec {
	std::string id;
	std::string type;
	std::string name;
	std::string description;
	Effect effect;
	int rarity = 0;
	int cost = 0;
};

struct WeaponSpec {
	std::string id;
	std::string type;
	std::string name;
	std::string description;
	int damage = 0;
	int cost = 0;
	Effect effect;
};

struct MagicSpec {
	std::string id;
	std::string type;
	std::string name;
	std::string description;
	int cost = 0;
	int manacost = 0;
	Effect effect;
};

void useItem(ItemSpec& item, SceneBattle& scene, std::shared_ptr<Entity> e, GameEngine* engine);



