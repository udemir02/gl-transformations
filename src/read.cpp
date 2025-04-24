#if !defined(HEADER_READ_CPP)
#define HEADER_READ_CPP

typedef struct {
    int order;
    float deg;
    glm::vec3 pos, normal;
} Rt_Data;

typedef struct {
    int order;
    glm::vec4 plane;
} Rf_Data;

typedef struct {
    int order;
    glm::vec3 translation;
} Tr_Data;

typedef struct {
    int order;
    glm::vec3 pos, scale;
} Sc_Data;

typedef struct {
    int order;
    char axis;
    float shear;
} Sh_Data;

typedef struct {
    unsigned int i1, i2, i3;
} Index3;

typedef struct {
    glm::vec3 v, n;
} Vertex3;

typedef struct {
    int num_faces;
    int num_vertex;
    Index3  *index;
    Vertex3 *vertex;
    unsigned int vao;
} Mesh;

typedef struct {
    Rt_Data *rt;
    Rf_Data *rf;
    Tr_Data *tr;
    Sc_Data *sc;
    Sh_Data *sh;
    int rt_size, rf_size, tr_size, sc_size, sh_size, size;
} Transform_Data;

typedef struct {
    char *vertex;
    char *fragment;
} Shader_Source;

static void strip_str(char *str) {
    for (int i = 0; i < 256; i++) {
        if (str[i] == '\n') str[i] = '\0';
        if (str[i] == '\r') str[i] = '\0';
    }
}

static void read_off(const char *path, Mesh *mesh) {
    FILE *fptr = fopen(path, "r");

    if (!fptr) {
        fprintf(stderr, "ERROR: Could not open file!\n");
        return;
    }

    char buffer[256];
    fscanf(fptr, "%s\n", buffer);

    if (strcmp(buffer, "OFF"))
        exit(EXIT_FAILURE);

    int num_vertex, num_faces, num_edges;
    fscanf(fptr, "%d %d %d\n", &num_vertex, &num_faces, &num_edges);

    mesh->num_faces  = num_faces;
    mesh->num_vertex = num_vertex;
    mesh->index      = (Index3  *)malloc(num_faces  * sizeof(*mesh->index));
    mesh->vertex     = (Vertex3 *)malloc(num_vertex * sizeof(*mesh->vertex));

    for (int i = 0; i < num_vertex; i++)
        fscanf(fptr, "%f %f %f\n",
               &mesh->vertex[i].v.x, &mesh->vertex[i].v.y, &mesh->vertex[i].v.z);

    int dim;

    for (int i = 0; i < num_faces; i++) {
        fscanf(fptr, "%d %d %d %d\n",
               &dim, &mesh->index[i].i1, &mesh->index[i].i2, &mesh->index[i].i3);

        glm::vec3 p1 = mesh->vertex[mesh->index[i].i1].v;
        glm::vec3 p2 = mesh->vertex[mesh->index[i].i2].v;
        glm::vec3 p3 = mesh->vertex[mesh->index[i].i3].v;
        glm::vec3 n = glm::normalize(glm::cross(p1 - p3, p2 - p3));

        mesh->vertex[mesh->index[i].i1].n = n;
        mesh->vertex[mesh->index[i].i2].n = n;
        mesh->vertex[mesh->index[i].i3].n = n;
    }

    fclose(fptr);
}

static void read_txt(const char *path, Transform_Data *data) {
    FILE *fptr = fopen(path, "r");

    if (!fptr) {
        fprintf(stderr, "ERROR: %s file failed to open!\n", path);
        exit(EXIT_FAILURE);
    }

    char buffer[256];
    int rt_size = 0, rf_size = 0, tr_size = 0, sc_size = 0, sh_size = 0;

    while (fgets(buffer, sizeof(buffer), fptr)) {
        strip_str(buffer);
        if (!strcmp(buffer, "#Translation")) tr_size++;
        if (!strcmp(buffer, "#Rotation"))    rt_size++;
        if (!strcmp(buffer, "#Scaling"))     sc_size++;
        if (!strcmp(buffer, "#Reflection"))  rf_size++;
        if (!strcmp(buffer, "#Shearing"))    sh_size++;
    }

    rewind(fptr);

    data->rt = (Rt_Data *)malloc(rt_size * sizeof(Rt_Data));
    data->rf = (Rf_Data *)malloc(rf_size * sizeof(Rf_Data));
    data->tr = (Tr_Data *)malloc(tr_size * sizeof(Tr_Data));
    data->sc = (Sc_Data *)malloc(sc_size * sizeof(Sc_Data));
    data->sh = (Sh_Data *)malloc(sh_size * sizeof(Sh_Data));

    data->rt_size = rt_size;
    data->rf_size = rf_size;
    data->tr_size = tr_size;
    data->sc_size = sc_size;
    data->sh_size = sh_size;
    data->size = rt_size + rf_size + tr_size + sc_size + sh_size;

    Rt_Data *rt = data->rt;
    Rf_Data *rf = data->rf;
    Tr_Data *tr = data->tr;
    Sc_Data *sc = data->sc;
    Sh_Data *sh = data->sh;

    int index = 0;

    while (fgets(buffer, sizeof(buffer), fptr)) {
        strip_str(buffer);
        if (!strcmp(buffer, "#Translation")) {
            glm::vec3 translation;
            fscanf(fptr, "%f %f %f\n", &translation.x, &translation.y, &translation.z);
            tr->order = index++;
            tr->translation = translation;
            tr++;
        } else if (!strcmp(buffer, "#Rotation")) {
            float deg;
            glm::vec3 pos, normal;
            fscanf(fptr, "%f %f %f\n", &pos.x, &pos.y, &pos.z);
            fscanf(fptr, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
            fscanf(fptr, "%f\n", &deg);
            rt->order = index++;
            rt->pos = pos;
            rt->normal = normal;
            rt->deg = deg;
            rt++;
        } else if (!strcmp(buffer, "#Scaling")) {
            glm::vec3 pos, scale;
            fscanf(fptr, "%f %f %f\n", &pos.x, &pos.y, &pos.z);
            fscanf(fptr, "%f %f %f\n", &scale.x, &scale.y, &scale.z);
            sc->order = index++;
            sc->pos = pos;
            sc->scale = scale;
            sc++;
        } else if (!strcmp(buffer, "#Reflection")) {
            glm::vec4 plane;
            fscanf(fptr, "%f %f %f %f\n", &plane.x, &plane.y, &plane.z, &plane.w);
            rf->order = index++;
            rf->plane = plane;
            rf++;
        } else if (!strcmp(buffer, "#Shearing")) {
            char axis;
            float shear;
            fscanf(fptr, "%c %f\n", &axis, &shear);
            sh->order = index++;
            sh->axis = axis;
            sh->shear = shear;
            sh++;
        }
    }

    fclose(fptr);
}

static Shader_Source parse_glsl(const char *path) {
    FILE *fptr = fopen(path, "rb");

    if (!fptr) {
        fprintf(stderr, "ERROR: Failed to open GLSL file!\n");
        exit(EXIT_FAILURE);
    }

    char buffer[256];
    long vstart = 0, vend = 0, fstart = 0, fend = 0;
    int started = 0;

    while (fgets(buffer, sizeof(buffer), fptr)) {
        strip_str(buffer);
        if (!strcmp(buffer, "#shader vertex")) {
            if (!started) {
                vstart = ftell(fptr);
                started = 1;
            } else {
                fend = ftell(fptr) - strlen(buffer) - 1;
                vstart = ftell(fptr);
            }
        } else if (!strcmp(buffer, "#shader fragment")) {
            if (!started) {
                fstart = ftell(fptr);
                started = 1;
            } else {
                vend = ftell(fptr) - strlen(buffer) - 1;
                fstart = ftell(fptr);
            }
        }
    }

    if (!started) {
        fprintf(stderr, "ERROR: Incorrect GLSL headers!\n");
        exit(EXIT_FAILURE);
    }

    if (!fend) fend = ftell(fptr);
    if (!vend) vend = ftell(fptr);

    rewind(fptr);

    int vsize = vend - vstart;
    int fsize = fend - fstart;

    char *vsource = (char *)malloc(vsize + 1);
    char *fsource = (char *)malloc(fsize + 1);

    fseek(fptr, vstart, SEEK_SET);
    fread(vsource, 1, vsize, fptr);
    fseek(fptr, fstart, SEEK_SET);
    fread(fsource, 1, fsize, fptr);

    vsource[vsize] = '\0';
    fsource[fsize] = '\0';

    fclose(fptr);
    return { vsource, fsource };
}

#endif
