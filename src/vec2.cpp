#include "vec2.hpp"
#include <math.h>


Vec2::Vec2(){
    this->x=0;
    this->y=0;
}

Vec2::Vec2(float x, float y) : x(x), y(y){}

/**
 * Computes the length of the vector
 * 
 * @return float that contains the length of the vector
 */
float Vec2::length() const{
    return sqrtf(x*x+y*y);
}

/**
 * Computes the distance between this vector and v
 * 
 * @param v vector to compute distance to
 */
float Vec2::dist(const Vec2& v) const{
    return (v-*this).length();
}

/**
 * Cross product for 2D vectors
 * 
 * @param v Vector to compute 2D cross product with
 * @return float that contains the 2D cross product
 */
float Vec2::cross(const Vec2& v) const{
    return this->x*v.y-v.x*this->y;
}

/**
 * Normalizes the vector to length 1 and returns normalized vector
 * 
 * @return Vec2 with length 1
 */
Vec2 Vec2::normalized() const{
    float l=length();
    return Vec2(x/l,y/l);
}

/**
 * Normalizes the vector to length 1 in place
 */
void Vec2::normalize(){
    float l=length();
    x=x/l;
    y=y/l;
}

//Operator overloads

bool Vec2::operator ==(const Vec2& rhs) const{
    return (rhs.x==x && rhs.y==y);
}
Vec2 Vec2::operator + (const Vec2& rhs) const{
    return Vec2(x+rhs.x,y+rhs.y);
}
Vec2 Vec2::operator * (float value) const{
    return Vec2(x*value,y*value);
}
void Vec2::operator +=(const Vec2& rhs){
    x+=rhs.x;
    y+=rhs.y;
}
Vec2 Vec2::operator -(const Vec2& rhs) const{
    return Vec2(x-rhs.x, y-rhs.y);
}
void Vec2::operator -=(const Vec2& rhs){
    x-=rhs.x;
    y-=rhs.y;
}
float Vec2::operator *(const Vec2& rhs) const{
    return x*rhs.x+y*rhs.y;
}
void Vec2::operator *=(const Vec2& rhs){
    x=x*rhs.x;
    y=y*rhs.y;
}