#include "main.h"
#include "read.cpp"
#include "transform.cpp"

static void error_callback(int error, const char *desc) {
    fprintf(stderr, "Error: %s\n", desc);
}

static void init_glcontext(GL_Context *context, const char *title, int major, int minor) {
    glfwInit();

    context->monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode *mode = glfwGetVideoMode(context->monitor);
    context->width = mode->width;
    context->height = mode->height;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, major);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, minor);
    glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);

    context->window = glfwCreateWindow(context->width, context->height, title, 0, 0);

    glfwSetErrorCallback(error_callback);

    glfwMakeContextCurrent(context->window);
    glewInit();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_SCISSOR_TEST);
}

static void init_mesh_buffer(const char *path, Mesh *mesh) {
    read_off(path, mesh);

    unsigned int vao, vbo, ebo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex3) * mesh->num_vertex, mesh->vertex, GL_DYNAMIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Index3) * mesh->num_faces,  mesh->index,  GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), 0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    mesh->vao = vao;
}

static void init_camera(Camera *c) {
    c->pos   = { 0.0f, 0.0f,  5.0f };
    c->front = { 0.0f, 0.0f, -1.0f };
    c->up    = { 0.0f, 1.0f,  0.0f };
    c->yaw   = -90.0f;
}

static unsigned int compile_shader(int type, const char *source) {
    unsigned int id = glCreateShader(type);
    glShaderSource(id, 1, &source, 0);
    glCompileShader(id);

    int is_compiled;
    glGetShaderiv(id, GL_COMPILE_STATUS, &is_compiled);

    if(!is_compiled) {
        char log[256];
        glGetShaderInfoLog(id, sizeof(log), 0, log);
        fprintf(stderr, "ERROR: Shader compilation failed!\n %s\n", log);
    }

    return id;
}

static unsigned int create_shader(const char *path) {
    Shader_Source source = parse_glsl(path);

    unsigned int vid = compile_shader(GL_VERTEX_SHADER, source.vertex);
    unsigned int fid = compile_shader(GL_FRAGMENT_SHADER, source.fragment);
    unsigned int pid = glCreateProgram();

    glAttachShader(pid, vid);
    glAttachShader(pid, fid);
    glLinkProgram(pid);
    glValidateProgram(pid);

    glDeleteShader(vid);
    glDeleteShader(fid);

    return pid;
}

static void init_scene(Scene *scene, const char *path, glm::vec3 light_color, glm::vec3 light_pos, glm::vec3 object_color, glm::vec4 clear) {
    scene->shader = create_shader(path);
    scene->light_pos = light_pos;
    scene->light_color = light_color;
    scene->object_color = object_color;
    scene->clear = clear;
}

static void init_transform_queue(Transform *m) {
    for (int i = 0; i < m->size; i++)
        m->queue[i] = m4x4_identity();
}

static void set_transforms(Transform_Method method, Transform_Data *tdata, M4x4 *m) {
    Rt_Data *rt = tdata->rt;
    Rf_Data *rf = tdata->rf;
    Tr_Data *tr = tdata->tr;
    Sc_Data *sc = tdata->sc;
    Sh_Data *sh = tdata->sh;

    for (int i = 0; i < tdata->rt_size; i++)
        m[rt[i].order] = rotation(method, rt[i].pos, rt[i].normal, rt[i].deg);
    for (int i = 0; i < tdata->rf_size; i++)
        m[rf[i].order] = reflection(method, rf[i].plane);
    for (int i = 0; i < tdata->tr_size; i++)
        m[tr[i].order] = translate(method, tr[i].translation);
    for (int i = 0; i < tdata->sc_size; i++)
        m[sc[i].order] = scale(method, sc[i].pos, sc[i].scale);
    for (int i = 0; i < tdata->sh_size; i++)
        m[sh[i].order] = shear(method, sh[i].axis, sh[i].shear);
}

static void init_transform(Transform *transform, const char *path, Transform_Method method) {
    Transform_Data tdata = {};
    read_txt(path, &tdata);

    transform->size = tdata.size;
    transform->size = tdata.size;
    transform->queue = (M4x4 *)malloc(transform->size * sizeof(M4x4));
    transform->queue = (M4x4 *)malloc(transform->size * sizeof(M4x4));

    init_transform_queue(transform);
    init_transform_queue(transform);

    set_transforms(method, &tdata, transform->queue);
}

static void process_input(Input *input, Camera *cam, float delta_time, GL_Context *context) {
    cam->speed = 10.0 * delta_time;

    if (glfwGetKey(context->window, GLFW_KEY_F11) == GLFW_RELEASE)
        context->f11_held = false;

    if (glfwGetKey(context->window, GLFW_KEY_F11) == GLFW_PRESS && !context->f11_held) {
        context->f11_held = true;

        if (!context->is_fullscreen) {
            glfwGetWindowPos(context->window, &context->windowed_x, &context->windowed_y);
            glfwGetWindowSize(context->window, &context->windowed_w, &context->windowed_h);
            const GLFWvidmode *mode = glfwGetVideoMode(context->monitor);
            glfwSetWindowMonitor(context->window, context->monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
            context->is_fullscreen = true;
        } else {
            glfwSetWindowMonitor(context->window, 0, context->windowed_x, context->windowed_y, context->windowed_w, context->windowed_h, GLFW_DONT_CARE);
            context->is_fullscreen = false;
        }
    }

    if (glfwGetKey(context->window, GLFW_KEY_W) == GLFW_PRESS)
        cam->pos += cam->front * cam->speed;
    if (glfwGetKey(context->window, GLFW_KEY_S) == GLFW_PRESS)
        cam->pos -= cam->front * cam->speed;
    if (glfwGetKey(context->window, GLFW_KEY_A) == GLFW_PRESS)
        cam->pos -= glm::normalize(glm::cross(cam->front, cam->up)) * cam->speed;
    if (glfwGetKey(context->window, GLFW_KEY_D) == GLFW_PRESS)
        cam->pos += glm::normalize(glm::cross(cam->front, cam->up)) * cam->speed;
    if (glfwGetKey(context->window, GLFW_KEY_R) == GLFW_PRESS)
        init_camera(cam);

    if (glfwGetMouseButton(context->window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
        cam->mouse_held = true;
    }
    if (glfwGetMouseButton(context->window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_RELEASE) {
        cam->mouse_held = false;
        cam->mouse_entered = false;
    }

    if (glfwGetKey(context->window, GLFW_KEY_T) == GLFW_PRESS && !input->t_held) {
        input->should_transform = !input->should_transform;
        input->t_held = true;
    }
    if (glfwGetKey(context->window, GLFW_KEY_T) == GLFW_RELEASE) {
        input->t_held = false;
    }


    if (cam->mouse_held) {
        double xpos, ypos;
        glfwGetCursorPos(context->window, &xpos, &ypos);

        if (!cam->mouse_entered) {
            cam->lastx = xpos;
            cam->lasty = ypos;
            cam->mouse_entered = true;
        }

        float xoffset = xpos - cam->lastx;
        float yoffset = cam->lasty - ypos;
        cam->lastx = xpos;
        cam->lasty = ypos;

        float sensitivity = 0.15f;
        xoffset *= sensitivity;
        yoffset *= sensitivity;

        cam->yaw   += xoffset;
        cam->pitch += yoffset;

        if(cam->pitch > 89.0f)  cam->pitch = 89.0f;
        if(cam->pitch < -89.0f) cam->pitch = -89.0f;

        glm::vec3 direction;
        direction.x = cos(glm::radians(cam->yaw)) * cos(glm::radians(cam->pitch));
        direction.y = sin(glm::radians(cam->pitch));
        direction.z = sin(glm::radians(cam->yaw)) * cos(glm::radians(cam->pitch));
        cam->front = glm::normalize(direction);
    }

    glfwGetFramebufferSize(context->window, &context->width, &context->height);
}

static void set_shader_mat4x4(unsigned int shader, const char *value, glm::mat4 matrix) {
    unsigned int loc = glGetUniformLocation(shader, value);
    glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(matrix));
}

static void set_shader_vec3(unsigned int shader, const char *value, glm::vec3 v) {
    unsigned int loc = glGetUniformLocation(shader, value);
    glUniform3f(loc, v.x, v.y, v.z);
}

static Window_Split get_split_side(GL_Context *context) {
    double xpos, ypos;
    glfwGetCursorPos(context->window, &xpos, &ypos);

    Window_Split result;
    if (xpos < context->width / 2.0)
        result = LEFT;
    else
        result = RIGHT;

    return result;
}

static void draw_world(World *w, Mesh *m, Window_Split side, GL_Context *context) {
    World *world = &w[side];
    Mesh *mesh = &m[side];
    Camera cam = world->cam;
    Scene scene = world->scene;
    Input input = world->input;
    Transform transform = world->transform;

    glm::mat4 model      = glm::mat4(1.0f);
    glm::mat4 view       = glm::lookAt(cam.pos, cam.pos + cam.front, cam.up);
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)context->width / (float)context->height, 0.1f, 100.0f);

    for (int i = 0; i < transform.size; i++)
        model = m4x4_to_mat4(transform.queue[i]) * model;

    Window_Split current_side = get_split_side(context);

    if (!input.should_transform)
        model = glm::inverse(model) * model;

    glUseProgram(scene.shader);
    set_shader_mat4x4(scene.shader, "model", model);
    set_shader_mat4x4(scene.shader, "view", view);
    set_shader_mat4x4(scene.shader, "projection", projection);
    set_shader_vec3(scene.shader, "view_pos", cam.pos);
    set_shader_vec3(scene.shader, "light_color", scene.light_color);
    set_shader_vec3(scene.shader, "light_pos", scene.light_pos);
    set_shader_vec3(scene.shader, "object_color", scene.object_color);

    glViewport(side * context->width / 2, 0, context->width / 2, context->height);
    glScissor(side * context->width / 2, 0, context->width / 2, context->height);
    glClearColor(scene.clear[0], scene.clear[1], scene.clear[2], scene.clear[3]);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBindVertexArray(mesh->vao);
    glDrawElements(GL_TRIANGLES, 3 * mesh->num_faces, GL_UNSIGNED_INT, 0);
}

int main(int argc, char **argv) {
    GL_Context context = {};
    Mesh mesh[2] = {};
    World world[2] = {};

    init_glcontext(&context, "Transformer", 3, 3);

    init_mesh_buffer("off/38.off", &mesh[LEFT]);
    init_mesh_buffer("off/38.off", &mesh[RIGHT]);

    init_camera(&world[LEFT].cam);
    init_camera(&world[RIGHT].cam);

    float tstart, tend;

    tstart = glfwGetTime() * 1000.0f;
    init_transform(&world[LEFT].transform, "transforms/transformations2.txt", GL);
    tend = glfwGetTime() * 1000.0f;
    fprintf(stdout, "OPENGL TRANSFORM TIME: %fms\n", tend - tstart);

    tstart = glfwGetTime() * 1000.0f;
    init_transform(&world[RIGHT].transform, "transforms/transformations2.txt", CUSTOM);
    tend = glfwGetTime() * 1000.0f;
    fprintf(stdout, "CUSTOM TRANSFORM TIME: %fms\n", tend - tstart);

    init_scene(&world[LEFT].scene, "basic.glsl", {1.0f, 1.0f, 1.0f}, {5.0f, 5.0f, 50.0f}, {1.0f, 0.5f, 0.0f}, {0.6f, 0.6f, 0.9f, 1.0f});
    init_scene(&world[RIGHT].scene, "basic.glsl", {1.0f, 1.0f, 1.0f}, {5.0f, 5.0f, 50.0f}, {0.5f, 1.0f, 0.0f}, {0.5f, 0.5f, 0.8f, 1.0f});

    float last_frame = 0.0f;
    float delta_time = 0.0f;

    while (!glfwWindowShouldClose(context.window)) {
        float current_frame = glfwGetTime();
        delta_time = current_frame - last_frame;
        last_frame = current_frame;

        Window_Split side = get_split_side(&context);
        process_input(&world[side].input, &world[side].cam, delta_time, &context);

        draw_world(world, mesh, LEFT, &context);
        draw_world(world, mesh, RIGHT, &context);

        glfwSwapBuffers(context.window);
        glfwPollEvents();
    }

    return 0;
}
