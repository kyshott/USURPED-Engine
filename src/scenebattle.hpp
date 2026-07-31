#include <string>
#include "entity.hpp"
#include "scene.hpp"
#include <memory>
#include <random>

class ScenePlay; // circular dependency

enum class BattleState {
	INPUT,
	ACTION,
	MESSAGE,
	VICTORY,
	DEFEAT,
	RESULTS,
};

struct DamageNumber {
	std::string text;
	Vec2 position;
	Vec2 velocity;
	int remaining = 0;
	int total = 0;
	Color color = WHITE;
};

class SceneBattle : public Scene {
	// Meta variables
	std::string playername;
	std::string enemyname;
	std::random_device rd;
	std::string title;                    /* Scene name */
	std::vector<std::string> menuStrings; /* List of menu strings that are drawn to the screen */
	std::vector<std::string> levelPaths;  /* List of relative paths to level definition files */
	std::shared_ptr<Entity> enemy;        /* Pointer to the enemy entity */
	std::shared_ptr<Entity> player;        /* Pointer to the player entity */
	std::shared_ptr<ScenePlay> previousScene; /* Pointer to the previous scene */
	std::string selectTip = "";
	int selectedMenuItem = 0;               /* Currently selected menu item */
	int menu = 0;
	
	// Battle system variables
	BattleState battleState = BattleState::INPUT; /* Current state of the battle */
	BattleState nextBattleState;
	DamageNumber damageNumber;
	std::string playerAction = "";
	std::string enemyAction = "";
	ItemSpec itemUsed;
	MagicSpec magicUsed;
	std::string battleMessage = "";
	std::string nextMessage = "";
	bool enemyFaster = false;
	int battlespeed = 80;
	int waitTimer = 0;                       /* Buffer for waiting between actions so everything isnt instant */
	bool playerTurn = true;

	// Systems
	void init();
	void sRender() override;
	void sDoAction(const Action& action) override;
	void sAnimation();
	void sLifespan();
	void sMovement();
	void sBattle();

	// State control
	void inputState();
	void actionState();
	void playerAct();
	void enemyAct();
	void victoryState();
	void defeatState();
	void resultsState();

	// Helpers
	void applyDamage(std::shared_ptr<Entity> attacker, std::shared_ptr<Entity> defender);
	void applyMagic(std::shared_ptr<Entity> caster, std::shared_ptr<Entity> target, MagicSpec spell);
	void useItem();
	void collectLoot(std::shared_ptr<Entity> looter, std::shared_ptr<Entity> looted);
	void queueMessage(const std::string& message, BattleState nextState);
	void drawDamageNumber(bool player, int damage, Color color);
	void renderUI();
	void renderBattleEntity(std::shared_ptr<Entity> entity);
	void spawnWeapon();
	void battleWeaponSwing(std::shared_ptr<Entity> e);

public:
	SceneBattle(GameEngine* gameEngine, std::shared_ptr<Entity> player, std::shared_ptr<Entity> enemy, std::shared_ptr<ScenePlay> previousScene);
	void update() override;

};