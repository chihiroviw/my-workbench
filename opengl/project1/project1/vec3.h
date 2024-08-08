#pragma once
#include <cmath>

struct Vector3f{

    float v[3];

    Vector3f() {}

    Vector3f(float _x, float _y, float _z){
        v[0] = _x;
        v[1] = _y;
        v[2] = _z;
    }

    Vector3f(const float* pFloat){
        v[0] = pFloat[0];
        v[1] = pFloat[1];
        v[2] = pFloat[2];
    } 

    Vector3f(float f){
        v[0] = v[1] = v[2] = f;
    }

    inline float x(void)const{ return v[0]; }
    inline float y(void)const{ return v[1]; }
    inline float z(void)const{ return v[2]; }

    inline Vector3f operator+(Vector3f& r){
        return Vector3f(v[0] + r[0],
                        v[1] + r[1],
                        v[2] + r[2]);
    }
    inline Vector3f operator-(Vector3f& r) {
        return Vector3f(v[0] - r[0],
                        v[1] - r[1],
                        v[2] - r[2]);
    }
    inline Vector3f operator*(float f){
        return Vector3f(v[0] * f,
                        v[1] * f,
                        v[2] * f);
    }

    inline Vector3f& operator+=(Vector3f r){
        v[0] += r[0];
        v[1] += r[1];
        v[2] += r[2];

        return *this;
    } 

    inline Vector3f& operator-=(Vector3f r){
        v[0] -= r[0];
        v[1] -= r[1];
        v[2] -= r[2];

        return *this;
    }

    inline Vector3f& operator*=(float f){
        v[0]*= f;
        v[1]*= f;
        v[2]*= f;

        return *this;
    }

    inline bool operator==(Vector3f& r){
        return ((v[0] == r[0]) && (v[1] == r[1]) && (v[2] == r[2]));
    }

    inline bool operator!=(Vector3f& r){
        return !(*this == r);
    }

    Vector3f Cross(Vector3f& v) {
        return Vector3f(
            y()*v.z() - z() * v.y(),
            z()*v.x() - x() * v.z(),
            x()*v.y() - y() * v.x()
        );
    };

    float Dot(Vector3f& v){
        float ret = x() * v.x() + y() * v.y() + z() * v.z();
        return ret;
    }

    float Distance(Vector3f& v){
        float delta_x = x() - v.x();
        float delta_y = y() - v.y();
        float delta_z = z() - v.z();
        float distance = sqrtf(delta_x * delta_x + delta_y * delta_y + delta_z * delta_z);
        return distance;
    }

    inline float square_length(){return v[0]*v[0] + v[1]*v[1] + v[2]*v[2];}
    float Length() {return sqrtf(square_length());}

    Vector3f& Normalize() {
        float l = fast_invsqrt(square_length());
        (*this) *= l;
        return *this;
    };

    inline float& operator[](std::size_t index)&{return v[index];};
    inline float operator[](std::size_t index)&&{return v[index];};

private:
    inline float fast_invsqrt(float x){
        // y = 1/sqrt(x) = 2 ^ (-1/2 * log2(x))
        long X, Y;
        float y;
        X = *(long*)&x;
        Y = 0x5F3759DF - (X >> 1); // Magic number!
        y = *(float*)&Y;

        // Newton's method
        const float threehalfs = 1.5F;
        float x2 = x * 0.5F;
        y = y * (threehalfs - (x2 * y * y));   // 1st iteration
        y = y * (threehalfs - (x2 * y * y));   // 2nd iteration

        return y;
    }
};
