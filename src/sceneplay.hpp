#pragma once
#include <string>
#include <imgui/imgui.h>
#include <imgui/rlImGui.h>
#include <imgui/imgui_stdlib.h>
#include "entity.hpp"
#include "scene.hpp"
#include <random>

/**
 * Contains the player data from the level definition file. Filled when a scene loads a level.
 */
struct PlayerConfig{float X=9,Y=6,BX=64,BY=64,SPEED=5,HEALTH=10; std::string WEAPON;};

/**
 * Scene that contains all logic and information for playing a loaded level
 */
class ScenePlay : public Scene, public std::enable_shared_from_this<ScenePlay>{
    private:
        PlayerConfig playerConfig;      /* Information about the properties of the player for this level */
        std::string levelPath;          /* Relative path to the level definition file from exe */
        std::shared_ptr<Entity> player; /* Shared pointer reference to the player entity */
        std::random_device rd;
        bool reload=false;              /* If the Scene should be reloaded at the end of the current frame */
        bool renderHealth=true;         /* If the entity health should be rendered */
        bool followCam=false;           /* If the locked follow camera should be active, false=room cam */
        Vec2 room = {0,0};              /* Current (x,y) room. (0,0) is the starting room */
        Vec2 spawnPoint = {0,0};
		int stage = 1;					 /* Current stage of the player, used for loot selection */
        std::string message;
        bool showMessage = false;

        RenderTexture2D mapTexture{};
        bool mapTextureReady = false;

        const Texture2D& menuBox = gameEngine->getAssets().getTexture("MENUBOX");
        const Font& font = gameEngine->getAssets().getFont("alagard");

        void init(const std::string& levelPath);
        void sAnimation();
        void sMovement();
        void sCollision();
        void sRender();
        void sLifespan();
        void sGUI();
        void sDoAction(const Action& action);
        void sCamera();
        void spawnPlayer();
        void spawnSword();
        void interact();
        void teleport(std::shared_ptr<Entity> e);
        void loadLevel(const std::string& levelPath);
		void loadMap(const std::string& levelPath);
        void renderTiledMap(const std::string& levelPath);
        void buildTiledMap(const std::string& levelPath);
        void reloadScene();
        std::string selectLoot(std::string type);
        Vec2 getPosition(int rx, int ry, int tx, int ty);
		Vec2 mouseToWorld(Vec2 windowPos);
        void renderAIDebug();
    public:
        ScenePlay(GameEngine* gameEngine,std::string levelPath);
        ScenePlay()=default;
        void battleReturn(std::shared_ptr<Entity> e);
        void update();
};