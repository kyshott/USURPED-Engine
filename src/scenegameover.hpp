#include <string>
#include "entity.hpp"
#include "scene.hpp"

/**
 * Contains all information needed to display the main menu and allow the user to choose a level to play.
 *
 * This is the first scene loaded by the gameEngine
 */
class SceneGameOver : public Scene {
    std::string title;                    /* Scene name */
    std::string subtitle;
    std::vector<std::string> menuStrings; /* List of menu strings that are drawn to the screen */
    std::vector<std::string> levelPaths;  /* List of relative paths to level definition files */
    int selectedMenuItem = 0;               /* Currently selected menu item */
	std::shared_ptr<Entity> player; /* Pointer to the player entity */
    int highlevel = 1;
    int highfloor = 1;
    std::string newlevel;

    const Texture2D& bg = gameEngine->getAssets().getTexture("GAMEOVER");
    const Font& font = gameEngine->getAssets().getFont("alagard");

    void init();
    void sRender() override;
    void sDoAction(const Action& action) override;
public:
    SceneGameOver(GameEngine* gameEngine, std::shared_ptr<Entity> player);
    void update() override;
};