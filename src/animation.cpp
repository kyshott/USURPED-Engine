#include "animation.hpp"
#include <iostream>

Animation::Animation()=default;

Animation::Animation(const std::string& name, const Texture2D &tex, Vec2 scaledSize) : Animation(name,tex,scaledSize,1,0) {}

Animation::Animation(const std::string& name, const Texture2D& tex, Vec2 scaledSize, int frameCount, int speed) : name(name), texture(tex), frameCount(frameCount), speed(speed) {
    this->scaledSize=scaledSize;
    this->scaledSize.x*=tex.width/frameCount;
    this->scaledSize.y*=tex.height;
    size={(float)(tex.width/frameCount),(float)tex.height};
    frameRect={currentFrame*size.x,0,(float)size.x,(float)size.y};
}

/**
 * Updates the raylib rectangle that is used as the source texture for the animation based on anim speed and frame count
 */
void Animation::update(){
    currentFrame++;
    int animationFrame = (currentFrame / speed) % frameCount;
    frameRect.x=animationFrame*size.x;
}

/**
 * Checks if the animation has reached its frame count
 * 
 * @return boolean if the animation as reached the end of its frame count
 */
bool Animation::hasEnded() const{
    if ((currentFrame / speed) % frameCount == frameCount - 1){
        return true;
    }
    else{
        return false;
    }
}

/**
 * Gets the string name of the animation
 * 
 * @return name of the animation
 */
const std::string& Animation::getName() const{
    return name;
}

/**
 * Checks if the animation has reached its frame count
 * 
 * @return boolean if the animation as reached the end of its frame count
 */
const Vec2& Animation::getSize() const{
    return size;
}

/**
 * Gets the (x,y) scaled size of the animation
 * 
 * @return Vec2 that contains the x an y scaled size of the animation
 */
const Vec2& Animation::getScaledSize() const{
    return scaledSize;
}

/**
 * Gets the entire animation texture
 * 
 * @return Raylib texture2D that contains all animation frames
 */
const Texture2D& Animation::getTexture() const{
    return texture;
}

/**
 * Gets the rectangle that corresponds to the current animation frame
 * 
 * @return Raylib rectangle that frames the current animation frame in the animation texture
 */
const Rectangle& Animation::getFrameRect() const{
    return frameRect;
}