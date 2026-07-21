#pragma once
#include <string>
#include <raylib.h>
#include <map>
#include "animation.hpp"
#include "items.hpp"

struct ItemSpec;

/**
 * Contains the ascii code for the first letter of the asset type
 */
enum AssetTypes {TEXTURE=84, ANIMATION=65, FONT=70, SOUND=83, MUSIC=77};

/**
 * 
 * Contains maps (string->Asset Type) for Font, Texture, and Animation objects. Maps are filled when a scene loads.
 * 
 * Primarily used to retrieve an asset by string name.
 * 
 */
class Assets{
    private:
        std::map<std::string, Texture2D> textureMap;   /* Map from string texture name to raylib Texture2D struct */
        std::map<std::string, Font> fontMap;           /* Map from string font name to raylib Font struct */
        std::map<std::string, Animation> animationMap; /* Map from string animation name to Animation object */
        std::map<std::string, Sound> soundMap;         /* Map from string sound effect name to raylib Sound object */
		std::map<std::string, ItemSpec> itemMap;         /* Map from string item name to itemSpec object */
        // map for enemies goes here

    public:
        std::map<std::string, Music> musicMap;         /* Map from string name to raylib Music object */

        Assets();
        ~Assets();
        void load(const std::string path);
        void loadItems(const std::string path);
        const Texture2D& getTexture(const std::string& name) const;
		const ItemSpec& getItem(const std::string& name) const;
        const Font& getFont(const std::string& name) const;
        const Animation& getAnimation(const std::string& name) const;
        const Sound& getSound(const std::string& name) const;
        const Music& getMusic(const std::string& name) const;

        void addItem(const std::string& type, const std::string& name, int damage, int speed, int cost, int manacost, int rarity, int lifespan);
        void addFont(const std::string& name, const std::string& path);
        void addTexture(const std::string& name, const std::string& path);
        void addAnimation(const std::string& name, const std::string& texture, int frameCount, int speed, int scaleFactor);
        void addSound(const std::string& name, const std::string& path);
        void addMusic(const std::string& name, const std::string& path);
};