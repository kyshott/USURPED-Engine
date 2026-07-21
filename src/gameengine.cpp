#include "gameengine.hpp"
#include <raylib.h>
#include <imgui/imgui.h>
#include <imgui/rlImGui.h>
#include <imgui/imgui_stdlib.h>
#include <iostream>
#include <fstream>
#include <math.h>
#include <memory>
#include <cstdio>
#include "components.hpp"
#include "vec2.hpp"
#include "scenemenu.hpp"
#include "sceneplay.hpp"


GameEngine::GameEngine(const std::string path){
    init(path);
}
GameEngine::~GameEngine(){
    rlImGuiShutdown();    // Shuts down the raylib ImGui backend
    CloseAudioDevice();
    CloseWindow();
}

/**
 * Initializes the game configuration including the raylib window, 
 * asset file location and world parameters
 * 
 * @param path Path to configuration file relative to exe
 */
void GameEngine::init(const std::string path){
    //read config
    std::ifstream file(path);
    if(!file.is_open()){
        std::cout<<"ERROR: config file not found: "+path<<"\n";
        return;
    }
    std::string str;
    std::string assetPath;
    std::string windowName;
    while (file.good()) {
        file >> str;
        if(str=="WINDOW"){
            file>>windowName>>windowWidth>>windowHeight;
        }
        if(str=="ASSETS"){
            file>>assetPath;
        }
        if(str=="WORLD"){
            file>>tilesX>>tilesY;
        }
    }
    file.close();
    //init raylib, must be done before loading any
    //other raylib assets
    SetConfigFlags(FLAG_WINDOW_HIGHDPI);
    InitWindow(windowWidth, windowHeight, windowName.c_str());
    rlImGuiSetup(true);
    InitAudioDevice();
    ImGui::GetStyle().ScaleAllSizes(2);
    SetTargetFPS(60);
    //init assets
    assets.load(assetPath);
    //change scene to starting scene
    changeScene("MENU",std::make_shared<SceneMenu>(this));
    
}

/**
 * Gets the current scene
 * 
 * @return shared_ptr to the current scene
 */
std::shared_ptr<Scene> GameEngine::currentScene(){
    return sceneMap.at(currScene);
}

/**
 * Updates the game while isRunning() is true
 */
void GameEngine::run(){
    while(isRunning()){
        update();
    }
}

/**
 * Gathers user input then updates the currently running scene
 */
void GameEngine::update(){
    sUserInput();
    sceneMap[currScene]->update();
    currentFrame++;
}

/**
 * Updates the current scene to a new scene
 * 
 * @param sceneName string that contains the scene name to switch to
 * @param scene shared_ptr to new scene to switch to
 */
void GameEngine::changeScene(const std::string sceneName, const std::shared_ptr<Scene> scene){
    if(scene){
        currScene=sceneName;
        sceneMap[currScene]=scene; 
    }
}

/**
 * Gets the assets class currently stored in gameEngine
 * 
 * @return reference to the Assets class
 */
const Assets& GameEngine::getAssets() const{
    return assets;
}

/**
 * Sends actions to the currently running scene based on keys pressed and released this frame
 */
void GameEngine::sUserInput(){
    updateKeys();

    for(int key : keysPressedThisFrame){
        if(!keyInMap(key)){
            continue;
        }
        Action a = Action(currentScene()->getActionMap().at(key),"PRESS");
        currentScene()->doAction(a);
    }
    for(int key : keysReleasedThisFrame){
        if(!keyInMap(key)){
            continue;
        }
        Action a = Action(currentScene()->getActionMap().at(key),"RELEASE");
        currentScene()->doAction(a);
    }
}

/**
 * Sets the running boolean to false, quitting the program on the next frame update
 */
void GameEngine::quit(){
    running=false;
}

/**
 * Returns the running state of the game
 * 
 * @return boolean that stores if the game is running
 */
bool GameEngine::isRunning() const{
    return (running);
}

/**
 * Returns if the key value is in the action map for the scene
 * 
 * @param key raylib key value
 * @return boolean if the key value is in the action map for the scene
 */
bool GameEngine::keyInMap(int key){
    if(currentScene()->getActionMap().find(key) == currentScene()->getActionMap().end()){
        return false;
    } else {
        return true;
    }
}
/**
 * Stores pressed and released keys this frame in their vectors
 */
void GameEngine::updateKeys(){
    std::vector<int> keysThisFrame;
    keysPressedThisFrame.clear();
    keysReleasedThisFrame.clear();
    int key=0;
    do{
        key=GetKeyPressed();
        if(key!=0) keysThisFrame.push_back(key);
    }while(key!=0);

    for(int theKey : keysThisFrame){
        keysPressed.insert(theKey);
        keysPressedThisFrame.insert(theKey);
    }

    std::vector<int> toErase;
    for(int theKey : keysPressed){
        if(IsKeyReleased(theKey)){
            toErase.push_back(theKey);
            keysReleasedThisFrame.insert(theKey);
        }
    }
    for(int erase : toErase){
        keysPressed.erase(erase);
    }
}

/**
 * Plays the named sound once.
 *
 * If the sound is currently playing, it stops and plays again
 * 
 * @param name Sound name
 */
void GameEngine::playSound(const std::string& name){
    Sound s = assets.getSound(name);
    if(IsSoundPlaying(s)) StopSound(s);
    if(IsSoundValid(s)) PlaySound(s);
}

/**
 * Stops the named sound.
 * 
 * @param name Sound name
 */
void GameEngine::stopSound(const std::string& name){
    Sound s = assets.getSound(name);
    if(IsSoundPlaying(s)) StopSound(s);
}

/**
 * Plays the named Music.
 *
 * If the Music is currently playing, it stops and plays again
 * 
 * @param name Music name
 */
void GameEngine::playMusic(const std::string& name){
    Music m = assets.getMusic(name);
    if(IsMusicStreamPlaying(m)) StopMusicStream(m);
    if(IsMusicValid(m)){
        SetMusicVolume(m, 1.0);
        PlayMusicStream(m);
    }
}

/**
 * Stops the named Music.
 * 
 * @param name Music name
 */
void GameEngine::stopMusic(const std::string& name){
    Music m = assets.getMusic(name);
    if(IsMusicStreamPlaying(m)) StopMusicStream(m);
}

/**
 * Loads more data into all Music streams. Must be called frequently.
 */
void GameEngine::updateMusicStreams(){
    for(auto& [name,m] : assets.musicMap){
        if(IsMusicStreamPlaying(m)) UpdateMusicStream(m);
    }
}

/**
 * 
 * @return raylib window height
 */
int GameEngine::getHeight(){
    return windowHeight;
}

/**
 * 
 * @return raylib window width
 */
int GameEngine::getWidth(){
    return windowWidth;
}

/**
 * 
 * @return number of world tiles in the x direction
 */
int GameEngine::getTilesX(){
    return tilesX;
}

/**
 * 
 * @return number of world tiles in the Y direction
 */
int GameEngine::getTilesY(){
    return tilesY;
}

/**
 * 
 * @return x size in pixels of world tiles
 */
int GameEngine::getTileSizeX(){
    return tileSizeX;
}

/**
 * 
 * @return y size in pixels of world tiles
 */
int GameEngine::getTileSizeY(){
    return tileSizeY;
}

/**
 * 
 * @return current game frame
 */
int GameEngine::getCurrentFrame(){
    return currentFrame;
}
