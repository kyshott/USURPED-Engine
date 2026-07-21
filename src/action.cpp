#include "action.hpp"


Action::Action()=default;
Action::Action(const std::string& name, const std::string& type) : name(name), type(type){}

/**
 * Gets the action name
 * 
 * @return Action name
 */
const std::string& Action::getName() const{
    return name;
}
/**
 * Gets the action type
 * 
 * @return Action type
 */
const std::string& Action::getType() const{
    return type;
}