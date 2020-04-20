#pragma once

#include "utilities.h"

#include <math.h>

union Vector3 {
    f32 v[3];
    struct {
        f32 x;
        f32 y;
        f32 z;
    };
    Vector3(){}
    Vector3(f32 v) : x(v), y(v), z(v){}
    Vector3(f32 x, f32 y, f32 z) : x(x), y(y), z(z){}
};

union Vector4 {
    f32 v[4];
    struct {
        f32 x;
        f32 y;
        f32 z;
        f32 w;
    };
    Vector4(){}
    Vector4(f32 v) : x(v), y(v), z(v), w(v){}
    Vector4(f32 x, f32 y, f32 z, f32 w) : x(x), y(y), z(z), w(w){}
};

union Quaternion {
    f32 v[4];
    struct {
        f32 x;
        f32 y;
        f32 z;
        f32 w;
    };

    Quaternion() : x(0), y(0), z(0), w(1){}
};

union Matrix4 {
    f32 m[16];
    f32 m2[4][4];

    Matrix4(){}
    Matrix4(f32 v){
        m[0] = v;m[1] = 0; m[2] = 0; m[3] = 0;
        m[4] = 0;m[5] = v; m[6] = 0; m[7] = 0;
        m[8] = 0;m[9] = 0; m[10] = v; m[11] = 0;
        m[12] = 0; m[13] = 0; m[14] = 0; m[15] = v;
    }
};

static Vector3 add(Vector3& v1, Vector3& v2){
    return Vector3(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z);
}

static Vector3 scale(Vector3& v1, f32 amt){
    return Vector3(v1.x * amt, v1.y * amt, v1.z * amt);
}

static Matrix4 createIdentityMatrix(){
    Matrix4 m;
    m.m[0] = 1; m.m[1] = 0; m.m[2] = 0; m.m[3] = 0;
    m.m[4] = 0; m.m[5] = 1; m.m[6] = 0; m.m[7] = 0;
    m.m[8] = 0; m.m[9] = 0; m.m[10] = 1; m.m[11] = 0;
    m.m[12] = 0; m.m[13] = 0; m.m[14] = 0; m.m[15] = 1;
    return m;
}

static Matrix4 multiply(Matrix4& m1, Matrix4& m2){
    Matrix4 m;

    m.m[0] = m1.m2[0][0] * m2.m2[0][0] + m1.m2[1][0] * m2.m2[0][1] + m1.m2[2][0] * m2.m2[0][2] + m1.m2[3][0] * m2.m2[0][3];
    m.m[1] = m1.m2[0][1] * m2.m2[0][0] + m1.m2[1][1] * m2.m2[0][1] + m1.m2[2][1] * m2.m2[0][2] + m1.m2[3][1] * m2.m2[0][3];
    m.m[2] = m1.m2[0][2] * m2.m2[0][0] + m1.m2[1][2] * m2.m2[0][1] + m1.m2[2][2] * m2.m2[0][2] + m1.m2[3][2] * m2.m2[0][3];
    m.m[3] = m1.m2[0][3] * m2.m2[0][0] + m1.m2[1][3] * m2.m2[0][1] + m1.m2[2][3] * m2.m2[0][2] + m1.m2[3][3] * m2.m2[0][3];

    m.m[4] = m1.m2[0][0] * m2.m2[1][0] + m1.m2[1][0] * m2.m2[1][1] + m1.m2[2][0] * m2.m2[1][2] + m1.m2[3][0] * m2.m2[1][3];
    m.m[5] = m1.m2[0][1] * m2.m2[1][0] + m1.m2[1][1] * m2.m2[1][1] + m1.m2[2][1] * m2.m2[1][2] + m1.m2[3][1] * m2.m2[1][3];
    m.m[6] = m1.m2[0][2] * m2.m2[1][0] + m1.m2[1][2] * m2.m2[1][1] + m1.m2[2][2] * m2.m2[1][2] + m1.m2[3][2] * m2.m2[1][3];
    m.m[7] = m1.m2[0][3] * m2.m2[1][0] + m1.m2[1][3] * m2.m2[1][1] + m1.m2[2][3] * m2.m2[1][2] + m1.m2[3][3] * m2.m2[1][3];

    m.m[8] = m1.m2[0][0] * m2.m2[2][0] + m1.m2[1][0] * m2.m2[2][1] + m1.m2[2][0] * m2.m2[2][2] + m1.m2[3][0] * m2.m2[2][3];
    m.m[9] = m1.m2[0][1] * m2.m2[2][0] + m1.m2[1][1] * m2.m2[2][1] + m1.m2[2][1] * m2.m2[2][2] + m1.m2[3][1] * m2.m2[2][3];
    m.m[10] = m1.m2[0][2] * m2.m2[2][0] + m1.m2[1][2] * m2.m2[2][1] + m1.m2[2][2] * m2.m2[2][2] + m1.m2[3][2] * m2.m2[2][3];
    m.m[11] = m1.m2[0][3] * m2.m2[2][0] + m1.m2[1][3] * m2.m2[2][1] + m1.m2[2][3] * m2.m2[2][2] + m1.m2[3][3] * m2.m2[2][3];

    m.m[12] = m1.m2[0][0] * m2.m2[3][0] + m1.m2[1][0] * m2.m2[3][1] + m1.m2[2][0] * m2.m2[3][2] + m1.m2[3][0] * m2.m2[3][3];
    m.m[13] = m1.m2[0][1] * m2.m2[3][0] + m1.m2[1][1] * m2.m2[3][1] + m1.m2[2][1] * m2.m2[3][2] + m1.m2[3][1] * m2.m2[3][3];
    m.m[14] = m1.m2[0][2] * m2.m2[3][0] + m1.m2[1][2] * m2.m2[3][1] + m1.m2[2][2] * m2.m2[3][2] + m1.m2[3][2] * m2.m2[3][3];
    m.m[15] = m1.m2[0][3] * m2.m2[3][0] + m1.m2[1][3] * m2.m2[3][1] + m1.m2[2][3] * m2.m2[3][2] + m1.m2[3][3] * m2.m2[3][3];

    return m;
}

static Matrix4 createPerspectiveProjection(f32 fov, f32 aspect, f32 znear, f32 zfar){
    Matrix4 m;
    m.m[0] = 1 / (aspect * tan(fov / 2.0));
    m.m[1] = 0;
    m.m[2] = 0;
    m.m[3] = 0;
    m.m[4] = 0;
    m.m[5] = 1 / (tan(fov / 2.0));
    m.m[6] = 0;
    m.m[7] = 0;
    m.m[8] = 0;
    m.m[9] = 0;
    m.m[10] = -(zfar + znear) / (zfar - znear);
    m.m[11] = -1;
    m.m[12] = 0;
    m.m[13] = 0;
    m.m[14] = -(zfar * znear) / (zfar - znear);
    m.m[15] = 0;
    return m;
}

static Matrix4 quaternionToMatrix4(Quaternion q){
    Matrix4 m(0);

    m.m2[0][0] = 1 - (2 * q.y * q.y) - (2 * q.z * q.z);
    m.m2[0][1] = (2 * q.x * q.y) + (2 * q.z * q.w);
    m.m2[0][2] = (2 * q.x * q.z) - (2 * q.y * q.w);
    m.m2[0][3] = 0;
    m.m2[1][0] = (2 * q.x * q.y) - (2 * q.z * q.w);
    m.m2[1][1] = 1 - (2 * q.x * q.x) - (2 * q.z * q.z);
    m.m2[1][2] = (2 * q.y * q.z) + (2 * q.x * q.w);
    m.m2[1][3] = 0;
    m.m2[2][0] = (2 * q.x * q.z) + (2 * q.y * q.w);
    m.m2[2][1] = (2 * q.y * q.z) - (2 * q.x * q.w);
    m.m2[2][2] = 1 - (2 * q.x * q.x) - (2 * q.y * q.y);
    m.m2[2][3] = 0;
    m.m2[3][0] = 0;
    m.m2[3][1] = 0;
    m.m2[3][2] = 0;
    m.m2[3][3] = 1;

    return m;
}

static void scale(Matrix4* m, f32 amt){
    m->m2[0][0] *= amt;
    m->m2[1][1] *= amt;
    m->m2[2][2] *= amt;
}

static f32 length(Quaternion q){
    return sqrt(q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w);
}

static void normalize(Quaternion* q){
    f32 len = length(*q);
    if(len != 0){
        q->x /= len;
        q->y /= len;
        q->z /= len;
        q->w /= len;
    }else{
        q->x = 0;
        q->y = 0;
        q->z = 0;
        q->w = 0;
    }
}

static Quaternion rotationToQuaternion(Vector3 axis, f32 angle){
    f32 halfAng = angle * 0.5;
    f32 sinHalfAng = sin(halfAng);
    f32 cosHalfAng = cos(halfAng);
    Quaternion q;

    q.x = sinHalfAng * axis.x;
    q.y = sinHalfAng * axis.y;    
    q.z = sinHalfAng * axis.z;
    q.w = cosHalfAng;

    return q;
}

static Quaternion multiply(Quaternion& q1, Quaternion& q2){
    Quaternion q;
    q.x = q1.x * q2.w + q1.y * q2.z - q1.z * q2.y + q1.w * q2.x;
    q.y = -q1.x * q2.z + q1.y * q2.w + q1.z * q2.x + q1.w * q2.y; 
    q.z = q1.x * q2.y - q1.y * q2.x + q1.z * q2.w + q1.w * q2.z;
    q.w = -q1.x * q2.x - q1.y * q2.y - q1.z * q2.z + q1.w * q2.w;
    return q;
}

static void rotate(Quaternion* q, Vector3 axis, f32 angle){
    Quaternion rot = rotationToQuaternion(axis, angle);
    *q = multiply(*q, rot);
    normalize(q);
}

static void translate(Matrix4* m, Vector3 v){
    m->m2[3][0] += v.x;
    m->m2[3][1] += v.y;
    m->m2[3][2] += v.z;
}

static Matrix4 operator*(Matrix4& m1, Matrix4& m2){
    return multiply(m1, m2);
}

static Vector3 operator+(Vector3& v1, Vector3& v2){
    return add(v1, v2);
}

static void operator+=(Vector3& v1, Vector3& v2){
    v1.x += v2.x;
    v1.y += v2.y;
    v1.z += v2.z;
}

static void operator-=(Vector3& v1, Vector3& v2){
    v1.x -= v2.x;
    v1.y -= v2.y;
    v1.z -= v2.z;
}

static Vector3 operator*(Vector3& v1, f32 amt){
    return scale(v1, amt);
}