#pragma once
#include <map>
#include <set>
#include <string>
#include <memory>
#include "scene.hpp"
#include "assets.hpp"

class Scene; //to avoid header circular dependency

class GameEngine{
    private:
        std::map<std::string,std::shared_ptr<Scene>> sceneMap; /* Map from scene name to shared pointer to the associate scene */
        Assets assets;                                         /* Holds all game assets, loaded in the GameEngine::init() function*/
        std::string currScene;                                 /* Currently running scene name */
        bool running=true;                                     /* If the game is currently running. When false, program will exit on next frame */
        int currentFrame=0;                                    /* Current game frame */
        std::set<int> keysPressed;                             /* Set of currently pressed keys */
        std::set<int> keysPressedThisFrame;                    /* Set of keys pressed on this frame */
        std::set<int> keysReleasedThisFrame;                   /* Set of keys released on this frame */
        int windowHeight;                                      /* Height of raylib window */
        int windowWidth;                                       /* Width of raylib window */
        int tilesX;                                            /* Number of tiles in the game in the x direction */
        int tilesY;                                            /* Number of tiles in the game in the y direction */
        int tileSizeX=64;                                      /* X pixel size of each tile */
        int tileSizeY=64;                                      /* Y pixel size of each tile */

        void init(const std::string path);
        std::shared_ptr<Scene> currentScene();
    public:
        GameEngine(const std::string path);
        ~GameEngine();
        void run();
        void update();
        void changeScene(const std::string sceneName, const std::shared_ptr<Scene> scene);
        const Assets& getAssets() const;
        void sUserInput();
        void quit();
        bool isRunning() const;
        bool keyInMap(int key);
        void updateKeys();
        int getHeight();
        int getWidth();
        int getTilesX();
        int getTilesY();
        int getTileSizeX();
        int getTileSizeY();
        int getCurrentFrame();
        void updateMusicStreams();
        void playSound(const std::string& name);
        void stopSound(const std::string& name);
        void playMusic(const std::string& name);
        void stopMusic(const std::string& name);

        
};