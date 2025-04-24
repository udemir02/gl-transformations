#if !defined(HEADER_MATRIX_H)
#define HEADER_MATRIX_H

typedef union {
    float e[4][4];
} M4x4;

inline M4x4 m4x4_identity(void) {
    M4x4 result = {{
        {1.0f, 0.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 1.0f, 0.0f},
        {0.0f, 0.0f, 0.0f, 1.0f}
    }};
    return result;
}

inline glm::mat4 m4x4_to_mat4(M4x4 m) {
    glm::mat4 result = {
        {m.e[0][0], m.e[1][0], m.e[2][0], m.e[3][0]},
        {m.e[0][1], m.e[1][1], m.e[2][1], m.e[3][1]},
        {m.e[0][2], m.e[1][2], m.e[2][2], m.e[3][2]},
        {m.e[0][3], m.e[1][3], m.e[2][3], m.e[3][3]}
    };
    return result;
}

inline M4x4 mat4_to_m4x4(glm::mat4 m) {
    M4x4 result = {{
        {m[0][0], m[1][0], m[2][0], m[3][0]},
        {m[0][1], m[1][1], m[2][1], m[3][1]},
        {m[0][2], m[1][2], m[2][2], m[3][2]},
        {m[0][3], m[1][3], m[2][3], m[3][3]}
    }};
    return result;
}

inline M4x4 m4x4_transpose(M4x4 m) {
    M4x4 result = {};

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++)
            result.e[i][j] = m.e[j][i];
    }

    return result;
}

inline M4x4 operator*(M4x4 A, M4x4 B) {
    M4x4 result = {};

    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
            result.e[r][c] = 0;
            for (int i = 0; i < 4; i++)
                result.e[r][c] += A.e[r][i] * B.e[i][c];
        }
    }

    return result;
}

#endif
