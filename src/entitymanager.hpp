#pragma once

#include <vector>
#include <map>
#include <memory>
#include "entity.hpp"

typedef std::vector<std::shared_ptr<Entity>> EntityVector;
typedef std::map<std::string,EntityVector> EntityMap;

/**
 * Contains references to all entities, entities by tag, and entities to add ad the start of each frame.
 */
class EntityManager{
    private:
        EntityMap entityMap;    /* Maps Entity shared pointers to type */
        EntityVector entities;  /* List of all Entity shared pointers in the game */
        EntityVector toAdd;     /* List of new Entity shared pointers to add at the start of the next frame, cleared very frame*/
        size_t totalEntities=0; /* Total number of entities that have been created */
    public:
        EntityManager();
        void update();
        std::shared_ptr<Entity> addEntity(std::string tag, std::string id);
        const EntityVector& getEntities();
        const EntityVector& getEntities(std::string tag);
        const std::map<std::string,EntityVector>& getEntityMap();



};