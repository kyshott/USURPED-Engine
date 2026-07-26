#include "physics.hpp"
#include <cmath>

/**
 * Compute overlap this frame
 * 
 * @param a  shared pointer to entity a
 * @param b  shared pointer to entity b
 * @return Vec2  Contains the x and y overlap of entities a and b
 */
Vec2 Physics::getOverlap(std::shared_ptr<Entity> a, std::shared_ptr<Entity> b){
    float dx = fabs((a->getComponent<CTransform>().position.x - b->getComponent<CTransform>().position.x));
    float dy = fabs((a->getComponent<CTransform>().position.y - b->getComponent<CTransform>().position.y));
    float halfWidthA = a->getComponent<CBoundingBox>().size.x / 2.0f;
    float halfwidthB = b->getComponent<CBoundingBox>().size.x / 2.0f;
    float halfHeightA = a->getComponent<CBoundingBox>().size.y / 2.0f;
    float halfHeightB = b->getComponent<CBoundingBox>().size.y / 2.0f;

    float xOver = (halfWidthA + halfwidthB) - dx;
    float yOver = (halfHeightA + halfHeightB) - dy;

    return Vec2(xOver, yOver);
}

/**
 * Compute overlap from previous frame
 * 
 * @param a  shared pointer to entity a
 * @param b  shared pointer to entity b
 * @return Vec2  Contains the x and y overlap of entities a and b
 */
Vec2 Physics::getPreviousOverlap(std::shared_ptr<Entity> a, std::shared_ptr<Entity> b){
    float dx = fabs((a->getComponent<CTransform>().prevPosition.x - b->getComponent<CTransform>().prevPosition.x));
    float dy = fabs((a->getComponent<CTransform>().prevPosition.y - b->getComponent<CTransform>().prevPosition.y));
    float halfWidthA = a->getComponent<CBoundingBox>().size.x / 2.0f;
    float halfwidthB = b->getComponent<CBoundingBox>().size.x / 2.0f;
    float halfHeightA = a->getComponent<CBoundingBox>().size.y / 2.0f;
    float halfHeightB = b->getComponent<CBoundingBox>().size.y / 2.0f;

    float xOver = (halfWidthA + halfwidthB) - dx;
    float yOver = (halfHeightA + halfHeightB) - dy;

    return Vec2(xOver, yOver);
}

/**
 * Compute if and where there is an overlap between line segment ab and cd
 * 
 * @param a  shared pointer to Vec2 a
 * @param b  shared pointer to Vec2 b
 * @param c  shared pointer to Vec2 c
 * @param d  shared pointer to Vec2 d
 * @return Intersect Struct
 */
Intersect Physics::lineIntersect(const Vec2& a, const Vec2& b, const Vec2& c, const Vec2& d){
    Vec2 u = b - a;
    Vec2 v = d - c;
    float uxv = u.cross(v);

    Vec2 cma = c - a;
    float t = (cma.cross(v)) / uxv;
    float s = (cma.cross(u)) / uxv;

    if (t >= 0 && t <= 1 && s >= 0 && s <= 1) {
        float x = a.x + t * u.x;
        float y = c.y + t * u.y;
        return { true, Vec2(x, y) };
    }
    else {
        return { false,Vec2{0,0} };
    }
}

/**
 * Computes if the line segment ab intersects with the bounding box of an entity
 * 
 * @param a  shared pointer to Vec2 a
 * @param b  shared pointer to Vec2 b
 * @param e  entity to check intersection
 * @return boolean that stores if the line segment ab intersects the bounding box of e
 */
bool Physics::entityIntersect(const Vec2& a, const Vec2& b, std::shared_ptr<Entity> e) {

	CTransform& transf = e->getComponent<CTransform>();
	Vec2 p1 = { transf.position.x - e->getComponent<CBoundingBox>().size.x / 2.0f, transf.position.y - e->getComponent<CBoundingBox>().size.y / 2.0f }; // top left
	Vec2 p2 = { transf.position.x + e->getComponent<CBoundingBox>().size.x / 2.0f, transf.position.y - e->getComponent<CBoundingBox>().size.y / 2.0f }; // top right
	Vec2 p3 = { transf.position.x + e->getComponent<CBoundingBox>().size.x / 2.0f, transf.position.y + e->getComponent<CBoundingBox>().size.y / 2.0f }; // bottom right
	Vec2 p4 = { transf.position.x - e->getComponent<CBoundingBox>().size.x / 2.0f, transf.position.y + e->getComponent<CBoundingBox>().size.y / 2.0f }; // bottom left

	if (lineIntersect(a, b, p1, p2).doesIntersect) return true; // top edge
	if (lineIntersect(a, b, p2, p3).doesIntersect) return true; // right edge
	if (lineIntersect(a, b, p3, p4).doesIntersect) return true; // bottom edge
	if (lineIntersect(a, b, p4, p1).doesIntersect) return true; // left edge

    return false;
}