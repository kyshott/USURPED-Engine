#pragma once
#include <map>
#include <string>
#include "gameengine.hpp"
#include "entitymanager.hpp"
#include "action.hpp"

class GameEngine; //to avoid header circular dependency

/**
 * Base class that contains all information that every scene needs to access.
 * 
 * This includes, a reference to the gameEngine, the entityManager for this scene, a raylib Camera2D, etc.
 */
class Scene{
    protected:
        GameEngine* gameEngine;                 /* Raw pointer reference to the gameEngine */
        EntityManager entityManager;            /* EntityManager object that store all entities in this scene */
        int currFrame;                          /* Current frame since this scene started */
        int frameCounter;                       /* Counter used to count frames */
        std::map<int, std::string> actionMap;   /* Map of action keycodes to string names */
        bool paused;                            /* if the scene is paused*/
        bool ended;                             /* if the scene should end */
        Camera2D mainCamera;                    /* Raylib 2D camera struct */
        bool renderBoundingBox=false;           /* Should the entity bounding boxes be rendered */
        bool renderTextures=true;               /* Should the entity textures be rendered */
        bool renderGridLines=false;             /* Should the grid lines be rendered */
        bool renderVisionDebug=false;           /* Should the AI vision be rendered */

        void renderBB();
        void renderTex();
        void renderGrid();
        void renderType(std::string type);

    public:
        virtual void update() = 0;
        virtual void sDoAction(const Action& action)=0;
        virtual void sRender()=0;

        Scene(GameEngine* gameEngine);
        Scene();

        void doAction(const Action& action);
        void registerAction(int type, std::string name);
        bool hasEnded() const;
        int currentFrame() const;
        void setPaused(bool paused);
        void sMusic();
        std::map<int, std::string>& getActionMap();
        Vec2 gridToMidPixel(int gridX, int gridY, std::shared_ptr<Entity> entity);

};