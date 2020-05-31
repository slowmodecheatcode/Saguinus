#pragma once

#include "utilities.h"

#include <math.h> //remove this some day

#define TAU 6.28318530718f
#define PI 3.14159265359f
#define HALF_PI 1.570796326795

union Vector2 {
    f32 v[2];
    struct {
        f32 x;
        f32 y;
    };
    Vector2(){}
    Vector2(f32 v) : x(v), y(v){}
    Vector2(f32 x, f32 y) : x(x), y(y){}
};

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
    Vector4(Vector3 v, f32 w) : x(v.x), y(v.y), z(v.z), w(w){}
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
    Quaternion(f32 x, f32 y, f32 z, f32 w) : x(x), y(y), z(z), w(w){}
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

static Vector2 add(Vector2& v1, Vector2& v2);
static Vector3 add(Vector3& v1, Vector3& v2);
static Matrix4 multiply(Matrix4& m1, Matrix4& m2);
static Vector3 scale(Vector3& v1, f32 amt);

static f32 absoluteValue(f32 v){
    return v < 0 ? -v : v;
}

static u32 xorshift(u32 x){
    x ^= x << 13;
	x ^= x >> 7;
	x ^= x << 17;
    return x;
}

static f32 maximumOf(f32 v1, f32 v2){
    return v1 > v2 ? v1 : v2;
}

static Vector3 operator*(Vector3& v1, f32 amt){
    return scale(v1, amt);
}

static Vector2 add(Vector2& v1, Vector2& v2){
    return Vector2(v1.x + v2.x, v1.y + v2.y);
}

static Vector3 add(Vector3& v1, Vector3& v2){
    return Vector3(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z);
}

static Vector3 sub(Vector3& v1, Vector3& v2){
    return Vector3(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z);
}

static Vector3 scale(Vector3& v1, f32 amt){
    return Vector3(v1.x * amt, v1.y * amt, v1.z * amt);
}

static Vector4 add(Vector4& v1, Vector4& v2){
    return Vector4(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z, v1.w + v2.w);
}

static Vector4 sub(Vector4& v1, Vector4& v2){
    return Vector4(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z, v1.w - v2.w);
}

static Vector4 scale(Vector4& v1, f32 amt){
    return Vector4(v1.x * amt, v1.y * amt, v1.z * amt, v1.w * amt);
}

static Vector3 linearInterpolation(Vector3 v1, Vector3 v2, f32 t){
    f32 x = v1.x + (v2.x - v1.x) * t;
    f32 y = v1.y + (v2.y - v1.y) * t;
    f32 z = v1.z + (v2.z - v1.z) * t;
    return Vector3(x, y, z);
}

static f32 dot(Vector3 v1, Vector3 v2){
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

static Vector3 cross(Vector3 v1, Vector3 v2){
    /*
        cx = aybz − azby
        cy = azbx − axbz
        cz = axby − aybx
    */

   return Vector3(v1.y * v2.z - v1.z * v2.y,
                  v1.z * v2.x - v1.x * v2.z,
                  v1.x * v2.y - v1.y * v2.x);
}


static f32 dot(Quaternion q1, Quaternion q2){
    return q1.x * q2.x + q1.y * q2.y + q1.z * q2.z + q1.w * q2.w;
}

static Quaternion add(Quaternion& q1, Quaternion& q2){
    return Quaternion(q1.x + q2.x, q1.y + q2.y, q1.z + q2.z, q1.w + q2.w);
}

static Quaternion sub(Quaternion& q1, Quaternion& q2){
    return Quaternion(q1.x - q2.x, q1.y - q2.y, q1.z - q2.z, q1.w - q2.w);
}

static Quaternion scale(Quaternion& q, f32 amt){
    return Quaternion(q.x * amt, q.y * amt, q.z * amt, q.w * amt);
}

static Matrix4 operator*(Matrix4& m1, Matrix4& m2){
    return multiply(m1, m2);
}

static Vector2 operator+(Vector2& v1, Vector2& v2){
    return add(v1, v2);
}

static Vector3 operator+(Vector3& v1, Vector3& v2){
    return add(v1, v2);
}

static Vector4 operator+(Vector4& v1, Vector4& v2){
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

static void operator+=(Vector4& v1, Vector4& v2){
    v1.x += v2.x;
    v1.y += v2.y;
    v1.z += v2.z;
    v1.w += v2.w;
}

static void operator-=(Vector4& v1, Vector4& v2){
    v1.x -= v2.x;
    v1.y -= v2.y;
    v1.z -= v2.z;
    v1.w -= v2.w;
}

static Vector3 operator-(Vector3& v1, Vector3& v2){
    return sub(v1, v2);
}

static Vector3 operator-(Vector3& v){
    return Vector3(-v.x, -v.y, -v.z);
}

static Vector4 operator-(Vector4& v1, Vector4& v2){
    return sub(v1, v2);
}

static Vector4 operator-(Vector4& v){
    return Vector4(-v.x, -v.y, -v.z, -v.w);
}

static Quaternion operator+(Quaternion& q1, Quaternion& q2){
    return add(q1, q2);
}

static Quaternion operator-(Quaternion& q){
    return Quaternion(-q.x, -q.y, -q.z, -q.w);
}

static Quaternion operator-(Quaternion& q1, Quaternion& q2){
    return sub(q1, q2);
}

static Quaternion operator*(Quaternion& q, f32 amt){
    return scale(q, amt);
}

static f32 length(Vector3 v){
    return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

static Vector3 normalOf(Vector3 v){
    f32 len = length(v);
    if(len != 0){
        v.x /= len;
        v.y /= len;
        v.z /= len;
    }else{
        v.x = 0;
        v.y = 0;
        v.z = 0;
    }
    return v;
}

static Quaternion matrix4ToQuaternion(Matrix4* mat){
    Quaternion q;

    f32 qw = sqrt(1 + mat->m2[0][0] + mat->m2[1][1] + mat->m2[2][2]) * 0.5;
    f32 qwX4 = 4 * qw;
    q.x = (mat->m2[1][2] - mat->m2[2][1]) / qwX4;
    q.y = (mat->m2[2][0] - mat->m2[0][2]) / qwX4;
    q.z = (mat->m2[0][1] - mat->m2[1][0]) / qwX4;
    q.w = qw;

    return q;
}

static Matrix4 createIdentityMatrix(){
    Matrix4 m;
    m.m[0] = 1; m.m[1] = 0; m.m[2] = 0; m.m[3] = 0;
    m.m[4] = 0; m.m[5] = 1; m.m[6] = 0; m.m[7] = 0;
    m.m[8] = 0; m.m[9] = 0; m.m[10] = 1; m.m[11] = 0;
    m.m[12] = 0; m.m[13] = 0; m.m[14] = 0; m.m[15] = 1;
    return m;
}

static Vector4 multiply(Matrix4* m, Vector4 v){
    Vector4 r;

    r.x = m->m2[0][0] * v.x + m->m2[1][0] * v.y * m->m2[2][0] * v.z + m->m2[3][0] * v.w;
    r.y = m->m2[0][1] * v.x + m->m2[1][1] * v.y * m->m2[2][1] * v.z + m->m2[3][1] * v.w;
    r.z = m->m2[0][2] * v.x + m->m2[1][2] * v.y * m->m2[2][2] * v.z + m->m2[3][2] * v.w;
    r.w = m->m2[0][3] * v.x + m->m2[1][3] * v.y * m->m2[2][3] * v.z + m->m2[3][3] * v.w;

    return r;
}

static Vector4 operator*(Matrix4& m, Vector4& v){
    return multiply(&m, v);
}

static Vector4 operator*(Vector4& v, f32 amt){
    return scale(v, amt);
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

static Matrix4 createOrthogonalProjection(f32 left, f32 right, f32 bottom, f32 top, f32 znear, f32 zfar){
    f32 rminl = right - left;
    f32 tminb = top - bottom;
    Matrix4 m;
    m.m[0] = 2 / rminl;
    m.m[1] = 0;
    m.m[2] = 0;
    m.m[3] = 0;
    m.m[4] = 0;
    m.m[5] = 2 / tminb;
    m.m[6] = 0;
    m.m[7] = 0;
    m.m[8] = 0;
    m.m[9] = 0;
    m.m[10] = 2 / (znear - zfar);
    m.m[11] = 0;
    m.m[12] = -rminl / rminl;;
    m.m[13] = -(top + bottom) / tminb;
    m.m[14] = -(zfar + znear) / (zfar - znear);
    m.m[15] = 1;
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

static void scaleMatrix(Matrix4* m, Vector3 amt){
    m->m2[0][0] *= amt.x;
    m->m2[1][1] *= amt.y;
    m->m2[2][2] *= amt.z;
}

static void scaleMatrix(Matrix4* m, f32 amt){
    m->m2[0][0] *= amt;
    m->m2[1][1] *= amt;
    m->m2[2][2] *= amt;
}

static void translateMatrix(Matrix4* m, Vector3 v){
    m->m2[3][0] += v.x;
    m->m2[3][1] += v.y;
    m->m2[3][2] += v.z;
}

static Matrix4 buildModelMatrix(Vector3 position, Vector3 scale, Quaternion orientation){
    Matrix4 mat(1);
    translateMatrix(&mat, position);
    scaleMatrix(&mat, scale);
    Matrix4 rot = quaternionToMatrix4(orientation);
    return mat * rot;
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

static Quaternion slerp(Quaternion q1, Quaternion q2, f32 t){
    normalize(&q1);
    normalize(&q2);

    f32 dp = dot(q1, q2);

    if (dp < 0.0f) {
        q1 = -q1;
        dp = -dp;
    }

    if (dp > 0.9995) {
        Quaternion result = q1 + (q2 - q1) * t;
        normalize(&result);
        return result;
    }

    f32 theta_0 = acos(dp);        
    f32 theta = theta_0 * t;          
    f32 sin_theta = sin(theta);     
    f32 sin_theta_0 = sin(theta_0); 

    f32 s0 = cos(theta) - dp * sin_theta / sin_theta_0;  
    f32 s1 = sin_theta / sin_theta_0;

    return (q1 * s0) + (q2 * s1);
}

static void rotate(Quaternion* q, Vector3 axis, f32 angle){
    Quaternion rot = rotationToQuaternion(axis, angle);
    *q = multiply(*q, rot);
    normalize(q);
}

