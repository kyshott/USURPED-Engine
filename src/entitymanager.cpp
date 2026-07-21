#include "entitymanager.hpp"
#include "entity.hpp"
#include <algorithm>
#include <iostream>

EntityManager::EntityManager(){}

/**
 * Updates entity data structures to avoid iterator invalidation
 * 
 * Occurs at the start of every frame using the toAdd list
 */
void EntityManager::update(){
    //delayed effects to add entities
    for(auto& e : toAdd){
        entities.push_back(e);
        entityMap[e->getTag()].push_back(e);
    }
    EntityVector toRemoveMS;
    for(auto& e : entities){
        if(!e->alive) toRemoveMS.push_back(e);
    }
    for(auto& e : toRemoveMS){
        auto it = std::find(entities.begin(), entities.end(),e);
        if(it!= entities.end()) 
            entities.erase(it);
        
        auto it2=std::find(entityMap[e->getTag()].begin(), entityMap[e->getTag()].end(),e);
        if(it2!=entityMap[e->getTag()].end()){
            entityMap[e->getTag()].erase(it2);
        }
    }

    toRemoveMS.clear();
    toAdd.clear();
}

/**
 * Adds entity to entity vector
 * 
 * @param tag type of entity, e.g. DYNAMIC
 * @param id name of the entity, e.g. BLOCK
 * @return shared pointer to added entity
 */
std::shared_ptr<Entity> EntityManager::addEntity(std::string tag, std::string id) {
    auto e = std::shared_ptr<Entity>(new Entity(tag, id));
    toAdd.push_back(e);
    return e;
}

/**
 * Gets structure of all entities
 * 
 * @return Vector of all entities
 */
const EntityVector& EntityManager::getEntities(){
    return entities;
}

/**
 * Gets a structure of all entities with specific tag
 * 
 * @param tag type of entity, e.g. DYNAMIC
 * @return Vector of all entities
 */
const EntityVector& EntityManager::getEntities(std::string tag){
    return entityMap[tag];
}

/**
 * Gets the entire entities map
 * 
 * @return Entities Map, stored by tag
 */
const std::map<std::string,EntityVector>& EntityManager::getEntityMap(){
    return entityMap;
}