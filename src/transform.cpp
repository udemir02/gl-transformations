#if !defined(HEADER_TRANSFORM_CPP)
#define HEADER_TRANSFORM_CPP

static int min3(glm::vec3 v) {
    float min = v.x;
    int index = 0;

    if (min > v.y) {
        min = v.y;
        index = 1;
    }

    if (min > v.z) {
        min = v.z;
        index = 2;
    }

    return index;
}

static M4x4 get_align_transform(glm::vec3 u) {
    glm::vec3 v = u;

    int min = min3(u);

    v[min] = 0.0f;

    if (min == 0) {
        float tmp = -v[1];
        v[1] = v[2];
        v[2] = tmp;
    } else if (min == 1) {
        float tmp = -v[0];
        v[0] = v[2];
        v[2] = tmp;
    } else if (min == 2) {
        float tmp = -v[0];
        v[0] = v[1];
        v[1] = tmp;
    }

    glm::vec3 w = glm::cross(u, v);
    M4x4 result = {{
        {u.x,  u.y,  u.z,  0.0f},
        {v.x,  v.y,  v.z,  0.0f},
        {w.x,  w.y,  w.z,  0.0f},
        {0.0f, 0.0f, 0.0f, 1.0f}
    }};
    return result;
}

static M4x4 translate_custom(glm::vec3 t) {
    M4x4 result = {{
        {1.0f, 0.0f, 0.0f,  t.x},
        {0.0f, 1.0f, 0.0f,  t.y},
        {0.0f, 0.0f, 1.0f,  t.z},
        {0.0f, 0.0f, 0.0f, 1.0f}
    }};
    return result;
}

static M4x4 translate(Transform_Method method, glm::vec3 translation) {
    M4x4 result = m4x4_identity();

    if (method == GL) {
        glm::mat4 gl_result = m4x4_to_mat4(result);
        gl_result = glm::translate(glm::mat4(1.0f), translation);
        result = mat4_to_m4x4(gl_result);
    } else {
        result = translate_custom(translation);
    }

    return result;
}

static M4x4 rotation_x_custom(float deg) {
    float rad = deg * (M_PI / 180);

    M4x4 result = {{
        {1.0f,      0.0f,       0.0f, 0.0f},
        {0.0f, cosf(rad), -sinf(rad), 0.0f},
        {0.0f, sinf(rad),  cosf(rad), 0.0f},
        {0.0f,      0.0f,       0.0f, 1.0f}
    }};

    return result;
}

static M4x4 rotation(Transform_Method method, glm::vec3 pos, glm::vec3 normal, float deg) {
    normal = glm::normalize(normal);

    M4x4 M    = get_align_transform(normal);
    M4x4 Minv = m4x4_transpose(M);
    M4x4 T    = translate(method, -pos);
    M4x4 Tinv = translate(method, pos);

    M4x4 R    = m4x4_identity();

    if (method == GL) {
        glm::mat4 gl_R = m4x4_to_mat4(R);
        gl_R = glm::rotate(glm::mat4(1.0f), glm::radians(deg), {1.0f, 0.0f, 0.0f});
        R = mat4_to_m4x4(gl_R);
    } else {
        R = rotation_x_custom(deg);
    }

    M4x4 result = Tinv * Minv * R * M * T;
    return result;
}

static M4x4 scale_custom(glm::vec3 s) {
    M4x4 result = {{
        { s.x, 0.0f, 0.0f, 0.0f},
        {0.0f,  s.y, 0.0f, 0.0f},
        {0.0f, 0.0f,  s.z, 0.0f},
        {0.0f, 0.0f, 0.0f, 1.0f}
    }};
    return result;
}

static M4x4 scale(Transform_Method method, glm::vec3 pos, glm::vec3 scale) {
    M4x4 T    = translate(method, pos);
    M4x4 Tinv = translate(method, -pos);
    M4x4 S    = m4x4_identity();

    if (method == GL) {
        glm::mat4 gl_S = m4x4_to_mat4(S);
        gl_S = glm::scale(gl_S, scale);
        S = mat4_to_m4x4(gl_S);
    } else {
        S = scale_custom(scale);
    }

    M4x4 result = Tinv * S * T;
    return result;
}

static M4x4 reflection(Transform_Method method, glm::vec4 plane) {
    glm::vec3 normal = glm::normalize(glm::vec3(plane.x, plane.y, plane.z));
    glm::vec3 point  = { 0.0f, 0.0f, -plane.w / plane.z };

    M4x4 M    = get_align_transform(normal);
    M4x4 Minv = m4x4_transpose(M);
    M4x4 T    = translate(method, -point);
    M4x4 Tinv = translate(method, point);
    M4x4 S    = scale(method, glm::vec3(0.0f), {-1.0f, 1.0f, 1.0f});

    M4x4 result = Tinv * Minv * S * M * T;
    return result;
}

static M4x4 shear_custom(char axis, float shear) {
    M4x4 result = {};

    if (axis == 'x')
        result = {{
            { 1.0f, 0.0f, 0.0f, 0.0f},
            {shear, 1.0f, 0.0f, 0.0f},
            {shear, 0.0f, 1.0f, 0.0f},
            { 0.0f, 0.0f, 0.0f, 1.0f}
        }};
    else if (axis == 'y')
        result = {{
            {1.0f, shear, 0.0f, 0.0f},
            {0.0f,  1.0f, 0.0f, 0.0f},
            {0.0f, shear, 1.0f, 0.0f},
            {0.0f,  0.0f, 0.0f, 1.0f}
        }};
    else if (axis == 'z')
        result = {{
            {1.0f, 0.0f, shear, 0.0f},
            {0.0f, 1.0f, shear, 0.0f},
            {0.0f, 0.0f,  1.0f, 0.0f},
            {0.0f, 0.0f,  0.0f, 1.0f}
        }};

    return result;
}

static M4x4 shear(Transform_Method method, char axis, float shear) {
    M4x4 result = m4x4_identity();

    if (method == GL) {
        glm::mat4 gl_result = m4x4_to_mat4(result);
        if      (axis == 'x') gl_result = glm::shearX3D(gl_result, shear, shear);
        else if (axis == 'y') gl_result = glm::shearY3D(gl_result, shear, shear);
        else if (axis == 'z') gl_result = glm::shearZ3D(gl_result, shear, shear);
        result = mat4_to_m4x4(gl_result);
    } else {
        result = shear_custom(axis, shear);
    }

    return result;
}

#endif
