#include "scene.hpp"
#include <iostream>

Scene::Scene(GameEngine* gameEngine) : gameEngine(gameEngine){}
Scene::Scene()=default;

/**
 * Sends action to the Do Action system in the currently running scene
 * 
 * @param action Action that gets sent from gameEngine user input
 */
void Scene::doAction(const Action& action){
    sDoAction(action);
}

/**
 * Sends action to the Do Action system in the currently running scene
 * 
 * @param type int that represents the type of action, e.g. raylib key code
 * @param name string that contains the name of the action
 */
void Scene::registerAction(int type, std::string name){
    actionMap[type]=name;
}

/**
 * Returns if the scene has ended
 * 
 * @return true/false if the scene has ended
 */
bool Scene::hasEnded() const{
    return ended;
}

/**
 * Gets the current frame of the game
 * 
 * @return int that contains the current frame of the game
 */
int Scene::currentFrame() const{
    return currFrame;
}

/**
 * Sets the paused state of the game
 * 
 * @param paused the state to change to
 */
void Scene::setPaused(bool paused){
    this->paused=paused;
}

/**
 * Draws the bounding boxes for each entity
 */
void Scene::renderBB(){
    for(auto& e : entityManager.getEntities()){
        if(!e->hasComponent<CBoundingBox>()) continue;
        auto& box=e->getComponent<CBoundingBox>();
        auto& transform=e->getComponent<CTransform>();
        Vec2 size=box.size;
        float x=transform.position.x-size.x/2;
        float y=transform.position.y-size.y/2;
        int lineThickness=2;
        Color c = RED;
        if(!box.blocksVision) c=GREEN;
        DrawRectangleLinesEx({x,y,size.x,size.y}, lineThickness, c);
    }
}

/**
 * Draws the textures for each entity in the game. Textures rendered last will be on top.
 */
void Scene::renderTex(){
    renderType("DEC");
    renderType("TILE");
    renderType("DYNAMIC");
    renderType("WEAPON");
}

/**
 * Draws the textures for each entity based on input type
 * 
 * @param type string that contains the type of object to draw
 */
void Scene::renderType(std::string type){
    for(auto& e : entityManager.getEntities(type)){
        auto& transform=e->getComponent<CTransform>();
        auto& anim = e->getComponent<CAnimation>();
        auto& state = e->getComponent<CState>();
        const Texture2D& tex = anim.animation.getTexture();
        
        float scaledWidth=anim.animation.getScaledSize().x;
        float scaledHeight=anim.animation.getScaledSize().y;

        float x=transform.position.x;
        float y=transform.position.y;

        Rectangle src = anim.animation.getFrameRect();
        if(transform.facing.x!=0){
            src.width*=transform.facing.x;
        } else {
            src.width*=1;
        }
        Rectangle dest = {x,y,scaledWidth,scaledHeight};
        Vector2 origin = {scaledWidth/2,scaledHeight/2};
        Color c = WHITE;
        if(e->hasComponent<CInvincibility>()) c.a=128;
        DrawTexturePro(tex,src,dest,origin,transform.angle,c);
    }
}

/**
 * Draws the game grid overlay, including (x,y) positions
 */
void Scene::renderGrid(){
    int gridSizeX=gameEngine->getTileSizeX();
    int gridSizeY=gameEngine->getTileSizeY();
    int width=gameEngine->getWidth();
    int height=gameEngine->getHeight();
    int tilesX=gameEngine->getTilesX();
    int tilesY=gameEngine->getTilesY();

    for(int rx=-2;rx<=2;rx++){
        for(int ry=-2;ry<=2;ry++){
            int rxShift=rx*tilesX*gridSizeX;
            int ryShift=ry*tilesY*gridSizeY;
            for(int x=gridSizeX;x<tilesX*gridSizeX;x+=gridSizeX){
                DrawLine(x+rxShift,0+ryShift,x+rxShift,height+ryShift,BLACK);
            }
            for(int y=gridSizeY;y<height;y+=gridSizeX){
                DrawLine(0+rxShift,height-y-ryShift,tilesX*gridSizeX+rxShift,height-y-ryShift,BLACK);
            }
            for(int i=0;i<tilesX*gridSizeX;i+=gridSizeX){
                for(int j=0;j<height;j+=gridSizeY){
                    int textSize=12;
                    std::string label="("+std::to_string(i/gridSizeX)+","+std::to_string(j/gridSizeY)+")";
                    DrawText(label.c_str(),i+2+rxShift,height-j-gridSizeY+2+ryShift,textSize,BLACK);
                }
            }
        }
    }
}

/**
 * Gets the registered action map
 * 
 * @return map of registered action: keycode->action name 
 */
std::map<int, std::string>& Scene::getActionMap(){
    return actionMap;
}

/**
 * Music System
 * 
 * Updates the raylib Music streams only
 */
void Scene::sMusic(){
    gameEngine->updateMusicStreams();
}

/**
 * Gets the position in world space based on game grid location
 * 
 * @param gridX X coordinate on the game grid
 * @param gridY Y coordinate on the game grid
 * @param entity Entity that should be placed on the grid
 * @return Vec2 that contains the center pixel in world space corresponding to the input grid coords
 */
Vec2 Scene::gridToMidPixel(int gridX, int gridY, std::shared_ptr<Entity> entity){
    float x, y;
    Vec2 scaledSize=entity->getComponent<CAnimation>().animation.getScaledSize();
    x=gridX*gameEngine->getTileSizeX()+scaledSize.x/2;
    y=gameEngine->getHeight()-gridY*gameEngine->getTileSizeY()-scaledSize.y/2;
    return Vec2(x,y);
}