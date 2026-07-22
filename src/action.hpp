#pragma once

#include <string>
#include "vec2.hpp"

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
	Vec2 position; /* Mouse position */

    public:
        Action();
		Action(const std::string& name, const std::string& type);
        Action(const std::string& name, const std::string& type, Vec2 position); 
        Action(const std::string& name, Vec2 position);

        const std::string& getName() const;
        const std::string& getType() const;
		const Vec2& getPosition() const;
};