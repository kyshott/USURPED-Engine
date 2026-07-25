#include <string>
#include "entity.hpp"
#include "scene.hpp"
#include <memory>

class ScenePlay; // circular dependency

class SceneBattle : public Scene {
	
	std::string title;                    /* Scene name */
	std::vector<std::string> menuStrings; /* List of menu strings that are drawn to the screen */
	std::vector<std::string> levelPaths;  /* List of relative paths to level definition files */
	std::vector<std::shared_ptr<Entity>> enemies; /* List of enemies in the battle scene */
	std::shared_ptr<Entity> player;        /* Pointer to the player entity */
	std::shared_ptr<ScenePlay> previousScene; /* Pointer to the previous scene */
	std::string menuText;                 /* Other menu text */
	int selectedMenuItem = 0;               /* Currently selected menu item */
	std::string playerAction = "";
	int waitTimer = 60;                       /* Buffer for waiting between actions so everything isnt instant */
	bool playerTurn = true;
	void init();
	void sRender() override;
	void sDoAction(const Action& action) override;
	void sAnimation();
	void sLifespan();
	void sMovement();
	void sBattle();

public:
	SceneBattle(GameEngine* gameEngine, std::shared_ptr<Entity> player, std::shared_ptr<Entity> enemy, std::shared_ptr<ScenePlay> previousScene);
	void update() override;

};