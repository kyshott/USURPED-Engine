#include <string>
#include "entity.hpp"
#include "scene.hpp"
#include <memory>
#include <random>

class ScenePlay; // circular dependency

enum class BattleState {
	PLAYER_INPUT,
	PLAYER_TURN,
	ENEMY_INPUT,
	ENEMY_TURN,
	MESSAGE,
	EFFECTS,
	VICTORY,
	DEFEAT
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

	std::string playername;

	std::random_device rd;
	
	std::string title;                    /* Scene name */
	std::vector<std::string> menuStrings; /* List of menu strings that are drawn to the screen */
	std::vector<std::string> levelPaths;  /* List of relative paths to level definition files */
	std::shared_ptr<Entity> enemy;        /* Pointer to the enemy entity */
	std::shared_ptr<Entity> player;        /* Pointer to the player entity */
	std::shared_ptr<ScenePlay> previousScene; /* Pointer to the previous scene */
	std::string menuText;                 /* Other menu text */
	int selectedMenuItem = 0;               /* Currently selected menu item */
	int menu = 0;
	BattleState battleState = BattleState::PLAYER_INPUT; /* Current state of the battle */
	BattleState nextBattleState;
	DamageNumber damageNumber;
	std::string playerAction = "";
	ItemSpec itemUsed;
	MagicSpec magicUsed;
	std::string enemyAction = "";
	std::string battleMessage = "";
	int waitTimer = 0;                       /* Buffer for waiting between actions so everything isnt instant */
	bool playerTurn = true;
	void init();
	void sRender() override;
	void sDoAction(const Action& action) override;
	void sAnimation();
	void sLifespan();
	void sMovement();
	void sBattle();

	// State control
	void playerInputState();
	void enemyInputState();
	void damageState();
	void playerTurnState();
	void enemyTurnState();
	void victoryState();
	void defeatState();

	// Helpers
	void queueMessage(const std::string& message, BattleState nextState, int frames);
	void renderUI();
	void renderBattleEntity(std::shared_ptr<Entity> entity);
	void enemyDie(std::shared_ptr<Entity> e);
	void battleWeaponSwing(std::shared_ptr<Entity> e);

public:
	SceneBattle(GameEngine* gameEngine, std::shared_ptr<Entity> player, std::shared_ptr<Entity> enemy, std::shared_ptr<ScenePlay> previousScene);
	void drawDamageNumber(bool player, int damage);
	void update() override;

};