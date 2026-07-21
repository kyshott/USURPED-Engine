#include "entity.hpp"

Entity::Entity(std::string& tag, std::string& id) : tag(tag), id(id){}

/**
 * Gets the entity's tag, e.g. DYNAMIC
 * 
 * @return entity tag
 */
std::string& Entity::getTag() {return tag;}

/**
 * Remove entity from scene (occurs on the next frame)
 */
void Entity::destroy() {alive=false;}

/**
 * Gets the entity's id, e.g. BLOCK
 * 
 * @return id of entity
 */
std::string& Entity::getID() {return id;}