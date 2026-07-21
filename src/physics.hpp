#pragma once
#include "vec2.hpp"
#include "entity.hpp"
#include <memory>

/**
 * Contains information about an intersection
 */
struct Intersect{
    bool doesIntersect=false;
    Vec2 pos={0,0};
};

/**
 * Contains only static functions for computing bounding box overlap between two entities and line intersections
 */
class Physics{
    public:
        static Vec2 getOverlap(std::shared_ptr<Entity> a, std::shared_ptr<Entity> b);
        static Vec2 getPreviousOverlap(std::shared_ptr<Entity> a, std::shared_ptr<Entity> b);
        static Intersect lineIntersect(const Vec2& a, const Vec2& b, const Vec2& c, const Vec2& d);
        static bool entityIntersect(const Vec2& a, const Vec2& b, std::shared_ptr<Entity> e);
};