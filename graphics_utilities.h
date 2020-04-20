#pragma once

#include "mathematics.h"

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

static void updateCameraView(Camera* camera){
    camera->view = Matrix4(1);
    Matrix4 camRotation = quaternionToMatrix4(camera->orientation);
    translate(&camera->view, camera->position);
    camera->view = camRotation * camera->view;
    camera->right = Vector3(camera->view.m2[0][0], camera->view.m2[1][0], camera->view.m2[2][0]);
    camera->up = Vector3(camera->view.m2[0][1], camera->view.m2[1][1], camera->view.m2[2][1]);
    camera->forward = Vector3(-camera->view.m2[0][2], -camera->view.m2[1][2], -camera->view.m2[2][2]);
}