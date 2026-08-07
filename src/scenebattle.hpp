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
	EFFECTS,
	VICTORY,
	DEFEAT,
	RESULTS,
	GAMEOVER
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

	// Assets

	const Texture2D& background = gameEngine->getAssets().getTexture("BATTLEROOM");
	const Texture2D& menuBox = gameEngine->getAssets().getTexture("MENUBOX");
	const Texture2D& arrow = gameEngine->getAssets().getTexture("ARROW");
	const Texture2D& portrait = gameEngine->getAssets().getTexture("OLPORTRAIT");
	const Font& font = gameEngine->getAssets().getFont("alagard");
	
	// Battle system variables
	BattleState battleState = BattleState::INPUT; /* Current state of the battle */
	BattleState nextBattleState;
	DamageNumber damageNumber;
	std::string playerAction = "";
	std::string enemyAction = "";
	ItemSpec itemUsed;
	MagicSpec magicUsed;
	MagicSpec enemyMagic;
	bool playerDefended = false;
	std::string battleMessage = "";
	std::string nextMessage = "";
	bool enemyFaster = false;
	int battlespeed = 80;
	int waitTimer = 0;                       /* Buffer for waiting between actions so everything isnt instant */
	bool playerTurn = true;
	int itemDrop = 0;
	std::string itemDropName = "";
	bool levelUp = false;
	

	// Systems
	void init();
	void sRender() override;
	void sDoAction(const Action& action) override;
	void sAnimation();
	void sLifespan();
	void sMovement();
	void sBattle();
	void sStatusEffects();

	// State control
	void inputState();
	void actionState();
	void playerAct();
	void enemyAct();
	void victoryState();
	void resultsState();

	// Helpers
	void applyDamage(std::shared_ptr<Entity> attacker, std::shared_ptr<Entity> defender);
	void applyMagic(std::shared_ptr<Entity> caster, std::shared_ptr<Entity> target, MagicSpec spell);
	void applyEffect(std::shared_ptr<Entity> target, Effect effect);
	void useItem();
	void collectLoot(std::shared_ptr<Entity> looter, std::shared_ptr<Entity> looted);
	void queueMessage(const std::string& message, BattleState nextState);
	void drawDamageNumber(bool player, int damage, Color color);
	void renderUI();
	void renderBattleEntity(std::shared_ptr<Entity> entity);
	void spawnWeapon();
	void spawnEffect(std::shared_ptr<Entity> target, std::string effectId);
	void spawnSpell(std::shared_ptr<Entity> target, MagicSpec spell);
	void spawnItem(std::shared_ptr<Entity> target, ItemSpec item);
	void battleWeaponSwing(std::shared_ptr<Entity> e);

public:
	SceneBattle(GameEngine* gameEngine, std::shared_ptr<Entity> player, std::shared_ptr<Entity> enemy, std::shared_ptr<ScenePlay> previousScene);
	void update() override;

};