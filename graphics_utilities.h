#pragma once

#include "mathematics.h"

static f32 cubeVertices[] = {
    -0.5, -0.5, 0.5,    
    -0.5,  0.5, 0.5,    
    0.5,  0.5, 0.5,     
    0.5, -0.5, 0.5,     

    -0.5,  0.5, -0.5,   
    -0.5, -0.5, -0.5,   
    0.5, -0.5, -0.5,    
    0.5,  0.5, -0.5,    

    -0.5,  0.5,  0.5,   
    -0.5,  0.5, -0.5,   
    0.5,  0.5, -0.5,    
    0.5,  0.5,  0.5,    

    -0.5, -0.5, -0.5,   
    -0.5, -0.5,  0.5,   
    0.5, -0.5,  0.5,    
    0.5, -0.5, -0.5,    

    -0.5, -0.5, -0.5,   
    -0.5,  0.5, -0.5,   
    -0.5,  0.5,  0.5,   
    -0.5, -0.5,  0.5,   

    0.5, -0.5,  0.5,    
    0.5,  0.5,  0.5,    
    0.5,  0.5, -0.5,    
    0.5, -0.5, -0.5    
};

static f32 texturedCubeVerticesWithNormals[] = {
    -0.5, -0.5, 0.5,        0.0, 0.0, 1.0,      0.0, 1.0,
    -0.5,  0.5, 0.5,        0.0, 0.0, 1.0,      0.0, 0.0,
    0.5,  0.5, 0.5,        0.0, 0.0, 1.0,      1.0, 0.0,
    0.5, -0.5, 0.5,        0.0, 0.0, 1.0,      1.0, 1.0,

    -0.5,  0.5, -0.5,       0.0, 0.0, -1.0,     0.0, 1.0,
    -0.5, -0.5, -0.5,       0.0, 0.0, -1.0,     0.0, 0.0,
    0.5, -0.5, -0.5,       0.0, 0.0, -1.0,     1.0, 0.0,
    0.5,  0.5, -0.5,       0.0, 0.0, -1.0,     1.0, 1.0,

    -0.5,  0.5,  0.5,       0.0,  1.0,  0.0,    0.0, 1.0,
    -0.5,  0.5, -0.5,       0.0,  1.0,  0.0,    0.0, 0.0,
    0.5,  0.5, -0.5,       0.0,  1.0,  0.0,    1.0, 0.0,
    0.5,  0.5,  0.5,       0.0,  1.0,  0.0,    1.0, 1.0,

    -0.5, -0.5, -0.5,       0.0,  -1.0,  0.0,    0.0, 1.0,
    -0.5, -0.5,  0.5,       0.0,  -1.0,  0.0,    0.0, 0.0,
    0.5, -0.5,  0.5,       0.0,  -1.0,  0.0,    1.0, 0.0,
    0.5, -0.5, -0.5,       0.0,  -1.0,  0.0,    1.0, 1.0,

    -0.5, -0.5, -0.5,       -1.0,  0.0,  0.0,    0.0, 1.0,
    -0.5,  0.5, -0.5,       -1.0,  0.0,  0.0,    0.0, 0.0,
    -0.5,  0.5,  0.5,       -1.0,  0.0,  0.0,    1.0, 0.0,
    -0.5, -0.5,  0.5,       -1.0,  0.0,  0.0,    1.0, 1.0,

    0.5, -0.5,  0.5,        1.0,  0.0,  0.0,    0.0, 1.0,
    0.5,  0.5,  0.5,        1.0,  0.0,  0.0,    0.0, 0.0,
    0.5,  0.5, -0.5,        1.0,  0.0,  0.0,    1.0, 0.0,
    0.5, -0.5, -0.5,        1.0,  0.0,  0.0,    1.0, 1.0,
};

static u16 cubeIndices[] = {
    0, 1, 2, 2, 3, 0,
    4, 5, 6, 6, 7, 4,
    8, 9, 10, 10, 11, 8,
    12, 13, 14, 14, 15, 12,
    16, 17, 18, 18, 19, 16,
    20, 21, 22, 22, 23, 20
};

struct Camera {
    Matrix4 projection;
    Matrix4 view;
    Quaternion orientation;
    Vector3 position;
    Vector3 forward;
    Vector3 up;
    Vector3 right;
    f32 moveSpeed;
    f32 rotateSpeed;
    f32 mouseSensitivity;

    Camera(){
        view = Matrix4(1);
        position = Vector3(0);
        forward = Vector3(0, 0, 1);
        up = Vector3(0, 1, 0);
        right = Vector3(1, 0, 0);
    }
};

struct PointLight {
    Vector3 position;
    Vector3 ambient;
    Vector3 diffuse;
    Vector3 specular;

    PointLight(){
        position = Vector3(0);
        ambient = Vector3(0.2);
        diffuse = Vector3(1);
        specular = Vector3(1);
    }
};

static void updateCameraView(Camera* camera){
    camera->view = Matrix4(1);
    Matrix4 camRotation = quaternionToMatrix4(camera->orientation);
    translateMatrix(&camera->view, -camera->position);
    camera->view = camRotation * camera->view;
    camera->right = Vector3(camera->view.m2[0][0], camera->view.m2[1][0], camera->view.m2[2][0]);
    camera->up = Vector3(camera->view.m2[0][1], camera->view.m2[1][1], camera->view.m2[2][1]);
    camera->forward = Vector3(-camera->view.m2[0][2], -camera->view.m2[1][2], -camera->view.m2[2][2]);
}