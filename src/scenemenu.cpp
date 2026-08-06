#include "scenemenu.hpp"
#include "sceneplay.hpp"
#include <iostream>

SceneMenu::SceneMenu(GameEngine* gameEngine) : Scene(gameEngine){
    init();
    
}

/**
 * Initializes actions and sets strings for level paths and menu items
 */
void SceneMenu::init(){
    title="USURPED!";
    menuStrings.push_back("START");
    menuStrings.push_back("TEST ROOM");
    menuStrings.push_back("QUIT");

    levelPaths.push_back("assets/TestRoom.tmj");
    levelPaths.push_back("level2.txt");
    levelPaths.push_back("level3.txt");

    //register input
    registerAction(KEY_S, "DOWN");
    registerAction(KEY_W, "UP");
    registerAction(KEY_SPACE, "PLAY");
    registerAction(KEY_ESCAPE, "QUIT");

    gameEngine->playMusic("TITLEMUSIC");
}

/**
 * Renders menu scene
 */
void SceneMenu::sRender(){
    BeginDrawing();

        ClearBackground(Color(252,216,168,255));

        DrawTexturePro(
            bg,
            Rectangle{ 0.0f, 0.0f, static_cast<float>(bg.width), static_cast<float>(bg.height) },
            Rectangle{ 0.0f, 0.0f, static_cast<float>(gameEngine->getWidth()), static_cast<float>(gameEngine->getHeight()) },
            Vector2{ 0.0f, 0.0f },
            0.0f,
            WHITE
        );

        //********** Raylib Drawing Content **********
        for (int i=0;i<menuStrings.size();i++) {
            Color textColor=BLACK;
            if(i==selectedMenuItem){
                textColor=RED;
            }
            DrawTextEx(font, menuStrings[i].c_str(), Vector2(600,120*(i + 2.5)), 50, 1, textColor);
        }
        DrawTextEx(font, title.c_str(), Vector2(500,60), 150, 1, GOLD);
        DrawTextEx(font, std::string("Demo").c_str(), Vector2(1100, 175), 30, 1, WHITE);

    EndDrawing();
}

/**
 * Performs PRESS and RELEASE actions that come from gameEngine
 * 
 * @param action Action that gets sent from gameEngine user input
 */
void SceneMenu::sDoAction(const Action& action){
    if((action.getType()=="PRESS")){
        if(action.getName()=="UP"){
            gameEngine->playSound("MENUSELECT");
            selectedMenuItem--;
            if(selectedMenuItem<0) selectedMenuItem=menuStrings.size()-1;
        }
        if(action.getName()=="DOWN"){
            gameEngine->playSound("MENUSELECT");
            selectedMenuItem++;
            if(selectedMenuItem>menuStrings.size()-1) selectedMenuItem=0;
        }
        if(action.getName()=="PLAY"){
            gameEngine->playSound("MENUSELECT");
            if (selectedMenuItem == 0) {
                // random map selection logic
                gameEngine->changeScene("PLAY", std::make_shared<ScenePlay>(gameEngine, levelPaths[selectedMenuItem], true, 1));
            }
            else if (selectedMenuItem == 1) {
                gameEngine->changeScene("PLAY", std::make_shared<ScenePlay>(gameEngine, levelPaths[selectedMenuItem], true, 1));
            }
            else if (selectedMenuItem == 2) {
                gameEngine->quit();
			}
        }
        if(action.getName()=="QUIT"){
            gameEngine->quit();
        }
    }

}

/**
 * Updates the scene (rendering and Music stream updating only for the Menu Scene)
 */
void SceneMenu::update(){
    sMusic();
    sRender();
}