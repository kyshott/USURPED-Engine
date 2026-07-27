#include "assets.hpp"
#include <string>
#include <fstream>
#include <iostream>
#include <raylib.h>
#include "animation.hpp"
#include "json.hpp"

using nlohmann::json;

Assets::Assets()=default;
Assets::~Assets(){
    //unload fonts
    for(auto& [name,font] : fontMap){
        UnloadFont(font);
    }
    //unload Texture2Ds
    for(auto& [name,tex] : textureMap){
        UnloadTexture(tex);
    }
    //unload sounds
    for(auto& [name,s] : soundMap){
        if(IsSoundPlaying(s)) StopSound(s);
        UnloadSound(s);
    }
    //unload Music
    for(auto& [name,m] : musicMap){
        if(IsMusicStreamPlaying(m)) StopMusicStream(m);
        UnloadMusicStream(m);
    }
}

/**
 * Loads the game textures, fonts, animation, sound, item, enemy and music definitions from the asset definition file
 * 
 * @param path string that contains the path to the asset definition file
 */
void Assets::load(const std::string path){
    std::ifstream file(path);
    std::string str;

    while (file.good()) {
        file >> str;
        std::string name, path, tex;
        int frameCount, speed, scaleFactor;
        switch (toascii(str[0])) {
            case AssetTypes::FONT: 
                file >> name >> path;
                addFont(name,path);
                break;
            case AssetTypes::TEXTURE: 
                file >> name >> path;
                addTexture(name,path);
                break;
            case AssetTypes::ANIMATION:
                file>>name>>tex>>frameCount>>speed>>scaleFactor;
                addAnimation(name,tex,frameCount,speed, scaleFactor);
                break;
            case AssetTypes::SOUND: 
                file >> name >> path;
                addSound(name,path);
                break;
            case AssetTypes::MUSIC: 
                file >> name >> path;
                addMusic(name,path);
                break; 
        }   
    }
    file.close();

	weapons = json::parse(std::ifstream("items/weapons.json"));
    items = json::parse(std::ifstream("items/items.json"));

}


/**
 * Gets the raylib texture based based on name
 * 
 * @param name Texture name
 */
const Texture2D& Assets::getTexture(const std::string& name) const{
    return textureMap.at(name);
}


/**
 * Gets the raylib font asset based on name
 * 
 * @param name Font name
 */
const Font& Assets::getFont(const std::string& name) const{
    return fontMap.at(name);
}

/**
 * Gets the animation asset based on name
 * 
 * @param name Animation name
 */
const Animation& Assets::getAnimation(const std::string& name) const{
    return animationMap.at(name);
}

/**
 * Gets the Sound asset based on name
 * 
 * @param name Sound name
 */
const Sound& Assets::getSound(const std::string& name) const{
    return soundMap.at(name);
}

/**
 * Gets the Music asset based on name
 * 
 * @param name Music name
 */
const Music& Assets::getMusic(const std::string& name) const{
    return musicMap.at(name);
}

/**
 * Loads the font using raylib library and adds to the font map data structure
 * 
 * @param name Font name
 * @param path Font path relative to the exe
 */
void Assets::addFont(const std::string& name, const std::string& path){
    Font font=LoadFont(path.c_str());
    fontMap[name]=font;
}

/**
 * Loads the texture using raylib library and adds to the texture map data structure
 * 
 * @param name Texture name
 * @param path Texture path relative to the exe
 */
void Assets::addTexture(const std::string& name, const std::string& path){
    Texture2D tex=LoadTexture(path.c_str());
    SetTextureFilter(tex,TEXTURE_FILTER_POINT);
    textureMap[name]=tex;
}

/**
 * Creates the animation object from the input information
 * 
 * @param name Animation name
 * @param textureName Texture name for use with this animation
 * @param frameCount Number of frame in this animation
 * @param speed How fast to change each frame when running
 * @param scaleFactor int that determins the up-scale of the texture when animating
 */
void Assets::addAnimation(const std::string& name, const std::string& textureName, int frameCount, int speed, int scaleFactor){
    Animation anim = Animation(name, textureMap[textureName], {(float)scaleFactor,(float)scaleFactor}, frameCount, speed);
    animationMap[name]=anim;
}

/**
 * Loads the Sound file using raylib library and adds to the Sound map data structure
 * 
 * @param name Sound name
 * @param path Sound path relative to the exe
 */
void Assets::addSound(const std::string& name, const std::string& path){
    Sound sound=LoadSound(path.c_str());
    soundMap[name]=sound;
}

/**
 * Loads the Music file using raylib library and adds to the Music map data structure
 * 
 * @param name Music name
 * @param path Music path relative to the exe
 */
void Assets::addMusic(const std::string& name, const std::string& path){
    Music music=LoadMusicStream(path.c_str());
    music.looping=true;
    musicMap[name]=music;
}


/*
* Gets the weapon information from the pre-parsed weapons.json file.
* 
* @param name Weapon name, as it appears as a key in the weapons.json file
* @return WeaponSpec struct containing the weapon information
*/
WeaponSpec Assets::getWeapon(std::string name) const {
    if (!weapons.contains(name)) {
        throw std::runtime_error("Weapon key not found in weapons.json: " + name);
    }

    const auto& data = weapons.at(name);

    WeaponSpec weapon;
	weapon.id = data.value("id", name);
    weapon.name = data.value("name", name);
    weapon.damage = data.value("damage", 0);
    weapon.cost = data.value("cost", 0);
    weapon.type = data.value("type", "");
    weapon.description = data.value("description", "");

    if (data.contains("effect") && data["effect"].is_object()) {
        const auto& effect = data["effect"];
        weapon.effect.id = effect.value("id", "none");
        weapon.effect.type = effect.value("type", "none");
        weapon.effect.magnitude = effect.value("magnitude", 0);
    }

    return weapon;
}

ItemSpec Assets::getItem(std::string name) const {
    if (!items.contains(name)) {
        throw std::runtime_error("Item key not found in items.json: " + name);
    }
    const auto& data = items.at(name);
    ItemSpec item;

    item.id = data.value("id", name);
    item.name = data.value("name", name);
    item.description = data.value("description", "");
    item.rarity = data.value("rarity", 0);
    item.cost = data.value("cost", 0);

    if (data.contains("effect") && data["effect"].is_object()) {
        const auto& effect = data["effect"];
        item.effect.id = effect.value("id", "none");
        item.effect.type = effect.value("type", "none");
        item.effect.magnitude = effect.value("magnitude", 0);
    }

    return item;
}
