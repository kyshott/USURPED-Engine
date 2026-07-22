#include "action.hpp"


Action::Action()=default;
Action::Action(const std::string& name, const std::string& type) : name(name), type(type) {}
Action::Action(const std::string& name, const std::string& type, Vec2 position) : name(name), type(type){}
Action::Action(const std::string& name, Vec2 position) : name(name), position(position) {}

/**
 * Gets the action name
 * 
 * @return Action name
 */
const std::string& Action::getName() const{
    return name;
}
/**
 * Gets the mouse position
 * 
 * @return Mouse position
 */
const Vec2& Action::getPosition() const{
    return position;
}
/**
 * Gets the action type
 * 
 * @return Action type
 */
const std::string& Action::getType() const{
    return type;
}