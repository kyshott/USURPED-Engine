#pragma once

/**
 * Contains two float values that can be used as a vector or a point
 */
class Vec2{
    public:
        float x; /* x value in vector */
        float y; /* y value in vector */

        Vec2();
        Vec2(float x, float y);

        float length() const;
        float dist(const Vec2& v) const;
        float cross(const Vec2& v) const;
        void normalize();
        Vec2 normalized() const;

        bool operator ==(const Vec2& rhs) const;
        Vec2 operator + (const Vec2& rhs) const;
        Vec2 operator * (float value) const;
        Vec2 operator -(const Vec2& rhs) const;
        float operator *(const Vec2& rhs) const;
        void operator +=(const Vec2& rhs);
        void operator -=(const Vec2& rhs);
        void operator *=(const Vec2& rhs);
};