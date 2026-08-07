#include "scenegameover.hpp"
#include "sceneplay.hpp"
#include "scenemenu.hpp"
#include <iostream>
#include <fstream>

SceneGameOver::SceneGameOver(GameEngine* gameEngine, std::shared_ptr<Entity> player) : Scene(gameEngine) {
    this->player = player;
    init();

}

/**
 * Initializes actions and sets strings for level paths and menu items
 */
void SceneGameOver::init() {
    title = "GAME OVER";
    newlevel = gameEngine->getAssets().getRandomMap();
    subtitle = "You have been defeated!";
    menuStrings.push_back("NEW RUN");
    menuStrings.push_back("RETURN TO TITLE");

    levelPaths.push_back("assets/TestRoom.tmj");

    std::fstream file("HIGHSCORES.txt");
    std::string str;
    int num;

    while (file.good()) {
        file >> str;
        if (str == "FLOOR") {
            file >> num;
            highfloor = num;
        }
        if (str == "LEVEL") {
            file >> num;
            highlevel = num;
        }
    }

    file.close();

    if (player->getComponent<CStats>().stage > highfloor) {
        highfloor = player->getComponent<CStats>().stage;
        highlevel = player->getComponent<CStats>().level;

        std::ofstream outFile("HIGHSCORES.txt", std::ios::trunc);
        if (outFile.is_open()) {
            outFile << "FLOOR " << highfloor << '\n';
            outFile << "LEVEL " << highlevel << '\n';
        }
    }

    //register input
    registerAction(KEY_S, "DOWN");
    registerAction(KEY_W, "UP");
    registerAction(KEY_SPACE, "PLAY");

    //gameEngine->playMusic("TITLEMUSIC");
}

/**
 * Renders game over scene
 */
void SceneGameOver::sRender() {
    BeginDrawing();

    ClearBackground(Color(252, 216, 168, 255));

    DrawTexturePro(
        bg,
        Rectangle{ 0.0f, 0.0f, static_cast<float>(bg.width), static_cast<float>(bg.height) },
        Rectangle{ 0.0f, 0.0f, static_cast<float>(gameEngine->getWidth()), static_cast<float>(gameEngine->getHeight()) },
        Vector2{ 0.0f, 0.0f },
        0.0f,
        WHITE
    );

    //********** Raylib Drawing Content **********
    for (int i = 0;i < menuStrings.size();i++) {
        Color textColor = WHITE;
        if (i == selectedMenuItem) {
            textColor = RED;
        }
        DrawTextEx(font, menuStrings[i].c_str(), Vector2(600, 120 * (i + 3)), 50, 1, textColor);
    }
    DrawTextEx(font, title.c_str(), Vector2(600, 60), 100, 1, RED);
    DrawTextEx(font, subtitle.c_str(), Vector2(600, 180), 50, 1, RED);
    DrawTextEx(font, ("Floor: " + std::to_string(player->getComponent<CStats>().stage)).c_str(), Vector2(750, 275), 30, 1, GOLD);
    DrawTextEx(font, ("Level: " + std::to_string(player->getComponent<CStats>().level)).c_str(), Vector2(950, 275), 30, 1, GOLD);

    EndDrawing();
}

/**
 * Performs PRESS and RELEASE actions that come from gameEngine
 *
 * @param action Action that gets sent from gameEngine user input
 */
void SceneGameOver::sDoAction(const Action& action) {
    if ((action.getType() == "PRESS")) {
        if (action.getName() == "UP") {
            gameEngine->playSound("MENUSELECT");
            selectedMenuItem--;
            if (selectedMenuItem < 0) selectedMenuItem = menuStrings.size() - 1;
        }
        if (action.getName() == "DOWN") {
            gameEngine->playSound("MENUSELECT");
            selectedMenuItem++;
            if (selectedMenuItem > menuStrings.size() - 1) selectedMenuItem = 0;
        }
        if (action.getName() == "PLAY") {
            gameEngine->playSound("MENUSELECT");
            if (selectedMenuItem == 0) {
                // random map selection logic
                gameEngine->changeScene("PLAY", std::make_shared<ScenePlay>(gameEngine, newlevel, true, 1));
            }
            else if (selectedMenuItem == 1) {
                gameEngine->changeScene("MENU", std::make_shared<SceneMenu>(gameEngine));
            }
        }
    }

}

/**
 * Updates the scene (rendering and Music stream updating only for the Menu Scene)
 */
void SceneGameOver::update() {
    sMusic();
    sRender();
}