#ifndef UVEC3_H_INCLUDED
#define UVEC3_H_INCLUDED

/** A 3-vector of unsigned integers. Used to represent the indices of a triangle */
struct uvec3 {
    unsigned int x, y, z;

    uvec3 () {}

    uvec3(unsigned int _x, unsigned int _y, unsigned int _z) {
        x = _x;
        y = _y;
        z = _z;
    }
};

#endif // UVEC3_H_INCLUDED
