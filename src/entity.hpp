#pragma once

#include <memory>
#include <string>
#include "components.hpp"

typedef std::tuple<CTransform, CLifespan, CInput, CBoundingBox, CAnimation, CState, CHealth, CDamage, CInvincibility, CFollowPlayer, CPatrol> ComponentsTuple;


/**
 * Contains all information about an individual Entity
 * 
 * Entities are made two identifying strings, an alive flag, and associated components that 
 * determine the systems the Entity interacts with.
 * 
 * Capabilities to add and remove components are include in this class as generic functions.
 */
class Entity{
    friend class EntityManager;
    private:
        std::string tag; /* General entity tag to determine type, e.g. dynamic, tile, dec, etc...*/
        std::string id;  /* Name of the entity, e.g. player, brick, etc... */
        bool alive=true; /* Is the entity alive (if set to false, removed at the start of the next frame)*/
        //constructor
        Entity(std::string& tag, std::string& id);

        ComponentsTuple components=std::make_tuple(CTransform(), CLifespan(5), CInput(), CBoundingBox(), CAnimation(), CState(), CHealth(), CDamage(), CInvincibility(), CFollowPlayer(), CPatrol());
    public:
        

        std::string& getTag();
        std::string& getID();
        void destroy();
        bool isActive() const;

        
        /**
         * Get the Component object
         * 
         * @tparam T Any type that is derived from a component object
         * @return T& The component object requested
         */
        template<typename T>
        T& getComponent(){
            return std::get<T>(components);
        }

        /**
         * Removes specified component from entity
         * 
         * @tparam T Component type to remove
         */
        template<typename T>
        void removeComponent(){
            getComponent<T>()=T();
        }

        /**
         * Checks if the entity has the specified component
         * 
         * @tparam T Requested component
         * @return if entity has specified component
         */
        template<typename T>
        bool hasComponent(){
            return getComponent<T>().has;
        }

        /**
         * Adds Specified component to the entity
         * 
         * @tparam T Specified component
         * @tparam TArgs Required arguments to the constructor of the component
         * @param args Required arguments to the constructor of the component
         * @return T& Created component
         */
        template<typename T, typename... TArgs>
        T& addComponent(TArgs &&...args){
            T& component = getComponent<T>();
            component=T(std::forward<TArgs>(args)...);
            component.has=true;
            return component;
        }
};
