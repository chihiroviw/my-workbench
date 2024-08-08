#ifndef VEC3_H
#define VEC3_H

#include <cmath>

/** A 3D vector */
struct vec3
{
    float x;
    float y;
    float z;

    /** Initialize all vector elements to zero */
    vec3 () {
        x = 0;
        y = 0;
        z = 0;
    }

    vec3 (float _x, float _y, float _z) {
        x = _x;
        y = _y;
        z = _z;
    }

    float magnitude () {
        return sqrtf (x*x + y*y + z*z);
    }

    vec3& normalize () {
        float m = magnitude ();
        x /= m;
        y /= m;
        z /= m;
        return *this;
    }

    void add(const vec3& b) {
        x += b.x;
        y += b.y;
        z += b.z;
    }
};

/* A few operators overloaded to make vector arithmetic more intuitive */
inline vec3 operator - (const vec3& a)    {
    return vec3 (-a.x, -a.y, -a.z);
}

inline vec3 operator - (const vec3& a, const vec3& b)    {
    return vec3 (a.x - b.x, a.y - b.y, a.z - b.z);
}

inline vec3 operator + (const vec3& a, const vec3 & b)    {
    return vec3 (a.x + b.x, a.y + b.y, a.z + b.z);
}

inline vec3 operator * (const vec3& a, float k) {
    return vec3 (a.x *k, a.y * k, a.z * k);
}

inline vec3 operator * (float k, const vec3& a) {
    return vec3 (a.x *k, a.y * k, a.z * k);
}

inline vec3 operator / (const vec3& a, float k) {
    return vec3 (a.x / k, a.y / k, a.z / k);
}

inline vec3 operator * (const vec3& a, const vec3& b) {
    return vec3 (a.x * b.x, a.y * b.y, a.z * b.z);
}

inline float dot(const vec3& a, const vec3& b) {
    return a.x * b.x + a.y *b.y + a.z * b.z;
}

inline vec3 cross(vec3 v1, vec3 v2) {
    vec3 out;
    out.x = v1.y * v2.z -v1.z * v2.y;
    out.y = v1.z * v2.x -v1.x * v2.z;
    out.z = v1.x * v2.y -v1.y * v2.x;

    return out;
}

inline vec3 longestVector (vec3 a, vec3 b, vec3 c) {
    if (a.magnitude() > b.magnitude()) {
        if (a.magnitude() > c.magnitude()) {
            return a;
        } else {
            return c;
        }
    } else if (b.magnitude() > c.magnitude()) {
        return b;
    } else {
        return c;
    }
}

#endif
