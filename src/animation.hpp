#pragma once
#include <raylib.h>
#include "vec2.hpp"
#include <string>

/**
 * 
 * Contains all size, state, and asset information related to a single animation
 * 
 * Animations are created for each texture in the game. Single textures are considered to have one frame of animation that repeats.
 * All entities that appear in the game should have an animation component with an associated animation object.
 * 
 */
class Animation{
    private:
        Texture2D texture;     /* Raylib texture that contains the full animation sheet */
        int frameCount=1;      /* Number of frames in this animation */
        int currentFrame=0;    /* Current animation frame */
        int speed=0;           /* Frame delay between animation frames (small is faster) */
        Vec2 size={1,1};       /* Size (pixels) of the animation frame */
        Vec2 scaledSize={1,1}; /* Scaled size (pixels) of the animation frame */
        Rectangle frameRect;   /* Raylib rectangle */
        std::string name;      /* Animation name */

    public:

        Animation();
        Animation(const std::string& name, const Texture2D& tex, Vec2 scaledSize);
        Animation(const std::string& name, const Texture2D& tex, Vec2 scaledSize, int frameCount, int speed);

        void update();
        bool hasEnded() const;
        const std::string& getName() const;
        const Vec2& getScaledSize() const;
        const Vec2& getSize() const;
        const Texture2D& getTexture() const;
        const Rectangle& getFrameRect() const;

};