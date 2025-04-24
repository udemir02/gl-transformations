#if !defined(HEADER_MAIN_H)
#define HEADER_MAIN_H

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform2.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "matrix.h"

typedef enum {
    GL,
    CUSTOM
} Transform_Method;

typedef enum {
    LEFT,
    RIGHT
} Window_Split;

typedef struct {
    int width, height;
    GLFWwindow *window;

    int is_fullscreen, f11_held;
    int windowed_w, windowed_h;
    int windowed_x, windowed_y;
    GLFWmonitor *monitor;
} GL_Context;

typedef struct {
    int should_transform, t_held;
} Input;

typedef struct {
    glm::vec3 up, pos, front;
    float speed;

    int mouse_entered, mouse_held;
    float lastx, lasty;
    float yaw, pitch;
    float fov;
} Camera;

typedef struct {
    int size;
    M4x4 *queue;
} Transform;

typedef struct {
    unsigned int shader;
    glm::vec3 light_color;
    glm::vec3 light_pos;
    glm::vec3 object_color;
    glm::vec4 clear;
} Scene;

typedef struct {
    Camera cam;
    Scene scene;
    Input input;
    Transform transform;
} World;

#endif
