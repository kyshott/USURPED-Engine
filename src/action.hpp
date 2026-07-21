#pragma once

#include <string>

/**
 * 
 * Contains information about the type of action performed.
 * 
 * Primarily used to pass action types from gameEngine to scenes.
 * 
 */
class Action{

    std::string name; /* Name of the action */
    std::string type; /* Type of action*/

    public:
        Action();
        Action(const std::string& name, const std::string& type); 

        const std::string& getName() const;
        const std::string& getType() const;
};