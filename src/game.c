#include "game.h"
#include "draw.h"
#include "shaders.h"
#include "meshes.h"
#include "textures.h"
#include "sounds.h"
#include "resources/resources.h"

#define debug_log(format, ...) fprintf(stdout, format, __VA_ARGS__)

int             screen_width = 0;
int             screen_height = 0;
int             glow_framebuffer_width;
int             glow_framebuffer_height;
int             fullscreen = 0;

int             cursor_x;
int             cursor_y;

GLuint          cube_mesh;
GLuint          sphere_mesh;
GLuint          quad_mesh;
GLuint          cylinder_mesh;
GLuint          sphere_mesh_instanced;
GLuint          stars_color_buffer;
GLuint          stars_matrix_buffer;

GLuint          ship_meshes[10];
GLsizei         ship_meshes_count[10];
struct AABB     ship_meshes_aabb[10];

GLuint          bonus_meshes[5];
GLsizei         bonus_meshes_count[10];
struct AABB     bonus_meshes_aabb[10];

GLuint          fullscreen_va;
GLuint          fullscreen_vb;

GLuint          ui_va;
GLuint          ui_vb;

GLuint          coloring_shader;
GLuint          color_instanced_shader;
GLuint          postprocess_shader;
GLuint          hblur_shader;
GLuint          vblur_shader;
GLuint          ui_shader;
GLuint          particles_draw_shader;
GLuint          particles_update_shader;
GLuint          texturing_shader;
GLuint          background_shader;

GLuint          color_map;
GLuint          depth_map;
GLuint          blur_map;
GLuint          glow_map;
GLuint          ui_map;

GLuint          noise_map;
GLuint          gradient_map;

GLuint          textures[20];

GLuint          texture_sampler;
GLuint          ui_sampler;

GLuint          color_framebuffer;
GLuint          blur_framebuffer;
GLuint          glow_framebuffer;
GLuint          sample_framebuffer;
GLuint          ui_framebuffer;

GLuint          color0buffer;
GLuint          depth0buffer;
GLuint          depth1buffer;

GLint           max_supported_samples;

struct PARTICLES_BUFFER particles_buffers[MAX_EMMITERS];

ALuint          sound_sources[MAX_SOUND_SOURCES];
ALuint          sounds[MAX_SOUND];

int             sound_off = 0;

struct CAMERA   camera;
struct BULLET   bullets[MAX_BULLETS];
struct STAR     stars[MAX_STARS];
struct SHIP     ships[MAX_SHIPS];
struct BONUS    bonuses[MAX_BONUSES];
struct TIMER    timers[MAX_TIMERS];
struct EMMITER  emmiters[MAX_EMMITERS];
struct NEBULA   nebulas[MAX_NEBULAS];
struct BOSS     boss;

struct SHIP     *player_ship;

int             bullets_count;
int             ships_count;
int             bonuses_count;
int             timers_count;

int             enemies_count;

struct GAME_STATE game_states[10];
struct STACK    *stack_state;
int             selected_menu_item = 0;

int             score;
int             game_over;

float           interpolation = 0;

void display_mainmenu();
void display_game();
void display_pausemenu();
void display_aboutmenu();
void display_settingsmenu();
void process_game(const struct window_event *event);
void process_mainmenu(const struct window_event *event);
void process_pausemenu(const struct window_event *event);
void process_aboutmenu(const struct window_event *event);
void process_settingsmenu(const struct window_event *event);

int play_sound(enum SOUNDS sound, int looped);

////////////////////////////////////////////////////////////////////////////////

void delete_timer(int id);
void set_timer_data(int id, void *data);
void* get_timer_data(int id);

static void
default_on_hit(struct SHIP *self, struct BULLET *bullet)
{
    //printf("on hit\n");
}

int
new_ship(int type, float px, float py, float pz, float sx, float sy, float sz)
{
    if(ships_count > MAX_SHIPS)
        return -1;

    static int ship_id = 0;

    struct SHIP *sh = &ships[ships_count];

    sh->id = ship_id++;
    sh->type = type;
    sh->active = 0;
    sh->x = px, sh->y = py, sh->z = pz;
    sh->l = sx, sh->h = sy, sh->b = sz;

    /*sh->bbox.min[0] = sh->x - sh->l;
    sh->bbox.min[1] = sh->y - sh->h;
    sh->bbox.min[2] = sh->z - sh->b;

    sh->bbox.max[0] = sh->x + sh->l;
    sh->bbox.max[1] = sh->y + sh->h;
    sh->bbox.max[2] = sh->z + sh->b;*/

    make_aabb0(sh->x, sh->y,sh->z, sh->l, sh->h, sh->b, &sh->bbox);

    sh->on_hit = default_on_hit;

    float3 color = {0, 0.8, 0};

    memcpy(&sh->color, &color, sizeof(sh->color));

    switch(type)
    {
    case PLAYER_SHIP:
        sh->active = 1;
        sh->health = PLAYER_MAX_HP;
        sh->weapon.type = GUN_BULLET;
        sh->weapon.damage = GUN_DAMAGE;
        sh->weapon.max_delay = GUN_DELAY;
        break;
    case ENEMY1_SHIP:
        sh->health = ENEMY1_MAX_HP;
        sh->weapon.type = GUN_BULLET;
        sh->weapon.damage = GUN_DAMAGE;
        sh->weapon.max_delay = GUN_DELAY;
        break;
    case ENEMY2_SHIP:
        sh->health = ENEMY2_MAX_HP;
        sh->weapon.type = ROCKET_BULLET;
        sh->weapon.damage = ROCKET_DAMAGE;
        sh->weapon.max_delay = ROCKET_DELAY;
        break;
    case ENEMY3_SHIP:
        sh->health = ENEMY2_MAX_HP;
        sh->weapon.type = GUN_BULLET;
        sh->weapon.damage = GUN_DAMAGE;
        sh->weapon.max_delay = GUN_DELAY;
        break;
    case ENEMY4_SHIP:
        sh->health = ENEMY2_MAX_HP;
        sh->weapon.type = GUN_BULLET;
        sh->weapon.damage = GUN_DAMAGE;
        sh->weapon.max_delay = GUN_DELAY;
        break;
    case ENEMY5_SHIP:
        sh->health = ENEMY2_MAX_HP;
        sh->weapon.type = PLASMA_BULLET;
        sh->weapon.damage = PLASMA_DAMAGE;
        sh->weapon.max_delay = PLASMA_DELAY;
        break;
    }

    debug_log("new ship id[%d]\n", sh->id);

    ships_count++;

    return sh->id;
}

struct SHIP* get_ship(int id)
{
    for(int i = 0; i < MAX_SHIPS; i++)
        if(id == ships[i].id)
            return &ships[i];

    return NULL;
}

void
delete_ship(int id)
{
    struct SHIP *s = get_ship(id);

    if(!s)
        return;

    debug_log("delete ship id[%d]\n", s->id);

    if(s->t1 != 0)
    {
        if(ships[ships_count - 1].t1 != 0)
            set_timer_data(ships[ships_count - 1].t1, s);

        delete_timer(s->t1);
    }

    if(s->t2)
    {
        if(ships[ships_count - 1].t2 != 0)
            set_timer_data(ships[ships_count - 1].t2, s);

        delete_timer(s->t2);
    }

    if(s->t3)
    {
        if(ships[ships_count - 1].t3 != 0)
            set_timer_data(ships[ships_count - 1].t3, s);

        delete_timer(s->t3);
    }

    if(ships_count > 2)
        memcpy(s, &ships[ships_count - 1], sizeof(struct SHIP));

    memset(&ships[ships_count - 1], 0, sizeof(struct SHIP));
    ships_count--;
}

void
set_ship_active_distance(int id, float distance)
{
    struct SHIP *s = get_ship(id);

    if(!s)
        return;

    s->active_distance = distance;
}

void
set_ship_size(int id, float x, float y, float z)
{
    struct SHIP *s = get_ship(id);

    if(!s)
        return;

    s->l = x;
    s->h = y;
    s->b = z;
}

void
set_ship_rotation(int id, float x, float y, float z)
{
    struct SHIP *s = get_ship(id);

    if(!s)
        return;

    s->q = x;
    s->d = y;
    s->p = z;
}

void
set_bullet_size(struct BULLET *bullet, float x, float y, float z)
{
    assert(bullet != 0);

    bullet->l = x;
    bullet->h = y;
    bullet->b = z;
}

void
new_bullet(struct WEAPON *weapon, float px, float py, float pz, float dx, float dy, int shooter)
{
    struct BULLET *b = &bullets[bullets_count];

    b->x = px, b->y = py, b->z = pz;
    b->v = BULLET1_SPEED * dx, b->w = 1 * dy;
    b->type = weapon->type;
    b->dead = 1;
    b->shooter = shooter;

    switch(weapon->type)
    {
    case GUN_BULLET:
        set_bullet_size(b, BULLET1_SIZE);
        break;
    case ROCKET_BULLET:
        set_bullet_size(b, BULLET2_SIZE);
        break;
    case PLASMA_BULLET:
        set_bullet_size(b, BULLET3_SIZE);
        break;
    }    

    b->bbox.min[0] = b->x - b->l;
    b->bbox.min[1] = b->y - b->h;
    b->bbox.min[2] = b->z - b->b;

    b->bbox.max[0] = b->x + b->l;
    b->bbox.max[1] = b->y + b->h;
    b->bbox.max[2] = b->z + b->b;

    weapon->delay = 0;

    bullets_count++;
    bullets_count =  bullets_count  % MAX_BULLETS;
}

void
new_bonus(int type, float x, float y, float z)
{
    if(bonuses_count > MAX_BONUSES)
        return;

    struct BONUS *b = &bonuses[bonuses_count];

    b->type = type;
    b->dead = 0;
    b->x = x, b->y = y, b->z = z;
    b->l = BONUS_SIZE, b->h = BONUS_SIZE, b->b = BONUS_SIZE;

    b->bbox.min[0] = b->x - b->l;
    b->bbox.min[1] = b->y - b->h;
    b->bbox.min[2] = b->z - b->b;

    b->bbox.max[0] = b->x + b->l;
    b->bbox.max[1] = b->y + b->h;
    b->bbox.max[2] = b->z + b->b;

    bonuses_count++;

    debug_log("new bonus type[%d]\n", type);
}

void
new_boss(int type)
{
    boss.type = type;
    boss.dead = 0;

    boss.l = 0.1, boss.h = 0.1, boss.b = 0.1;
    boss.x = player_ship->x + 5.0;
    boss.v = SCROLL_SPEED;
}

void
delete_bonus(int index)
{
    debug_log("delete bonus type[%d]\n", bonuses[index].type);

    if(bonuses_count > 2)
        memcpy(&bonuses[index], &bonuses[bonuses_count - 1], sizeof(struct BONUS));

    memset(&bonuses[bonuses_count - 1], 0, sizeof(struct BONUS));
    bonuses_count--;
}

static void
default_on_time(struct TIMER *self, void* data)
{
    //printf("on time\n");
}

int
new_timer(int type, float tv, timer_callback tc, void *data)
{
    static int timer_id = 0;

    struct TIMER *t = &timers[timers_count];

    t->id = ++timer_id;
    t->type = type;
    t->value = 0;
    t->stop_value = tv;
    t->on_time = default_on_time;
    t->data = data;

    if(tc)
        t->on_time = tc;

    timers_count++;

    debug_log("new timer id[%d]\n", t->id);

    return t->id;
}

struct TIMER*
get_timer(int id)
{
    for(int i = 0; i < timers_count; i++)
        if(timers[i].id == id)
            return &timers[i];

    return NULL;
}

void
delete_timer(int id)
{
    struct TIMER *t = get_timer(id);

    if(!t)
        return;

    debug_log("delete timer id[%d]\n", t->id);

    if(timers_count > 2)
        memcpy(t, &timers[timers_count - 1], sizeof(struct TIMER));

    memset(&timers[timers_count - 1], 0, sizeof(struct TIMER));
    timers_count--;
}

void
set_timer_data(int id, void *data)
{
    struct TIMER *t = get_timer(id);

    if(!t)
        return;

    t->data = data;
}

void*
get_timer_data(int id)
{
    struct TIMER *t = get_timer(id);

    if(!t)
        return NULL;

    return t->data;
}

static struct PARTICLE particles[MAX_PARTICLES];

int
new_emmiter(int particles_count)
{
    for(int i = 0; i < MAX_EMMITERS; i++)
        if(emmiters[i].active == 0)
        {
            emmiters[i].active = 1;

            if(emmiters[i].buffer == NULL)
                emmiters[i].buffer = &particles_buffers[i];

            struct PARTICLES_BUFFER *pb = (struct PARTICLES_BUFFER*)emmiters[i].buffer;

            pb->count = particles_count;

            debug_log("new emmiter index[%d]\n", i);

            return i;
        }

    return -1;
}

void
set_emmiter_attributes(int index, struct PARTICLE_ATTRIBUTES *attr, float t, unsigned flags)
{
    if(index < 0 || index > MAX_EMMITERS)
        return;

    emmiters[index].tl = t;
    emmiters[index].tv = 0;
    emmiters[index].flags = flags;

    struct PARTICLES_BUFFER *pb = (struct PARTICLES_BUFFER*)emmiters[index].buffer;

    pb->point_size = attr->size;

    for(int i = 0; i < MAX_PARTICLES; i++)
    {
        if(flags & PA_RAND_BETWEEN_POSITION)
        {
            particles[i].x = emmiters[index].x + randf2(attr->position[0][0], attr->position[1][0]);
            particles[i].y = emmiters[index].y + randf2(attr->position[0][1], attr->position[1][1]);
            particles[i].z = emmiters[index].z + randf2(attr->position[0][2], attr->position[1][2]);
        }
        else if(flags & PA_RAND_BASE_POSITION)
        {
            float3 p;

            randv3(p);

            particles[i].x = emmiters[index].x + attr->position[0][0] * p[0];
            particles[i].y = emmiters[index].y + attr->position[0][1] * p[1];
            particles[i].z = emmiters[index].z + attr->position[0][2] * p[2];
        }
        else
        {
            particles[i].x = emmiters[index].x + attr->position[0][0];
            particles[i].y = emmiters[index].y + attr->position[0][1];
            particles[i].z = emmiters[index].z + attr->position[0][2];
        }

        if(flags & PA_RAND_BETWEEN_VELOCITY)
        {
            float3 v;

            randv3(v);

            particles[i].v = attr->velocity[0][0] + (attr->velocity[1][0] - attr->velocity[0][0]) * v[0];
            particles[i].w = attr->velocity[0][1] + (attr->velocity[1][1] - attr->velocity[0][1]) * v[1];
        }
        else if(flags & PA_BASE_VELOCITY)
        {
            particles[i].v = (particles[i].x - emmiters[index].x) * attr->velocity[0][0] + emmiters[index].v;
            particles[i].w = (particles[i].y - emmiters[index].y) * attr->velocity[0][1] + emmiters[index].w;
        }
        else
        {
            particles[i].w = attr->velocity[0][0];
            particles[i].v = attr->velocity[0][1];
        }

        memcpy(emmiters[index].color, attr->color, sizeof(float3) * 2);

        glBindVertexArray(pb->va);

        glBindBuffer(GL_ARRAY_BUFFER, pb->vb);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(struct PARTICLE) * pb->count, particles);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(struct PARTICLE), (void*)offsetof(struct PARTICLE, x));
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(struct PARTICLE), (void*)offsetof(struct PARTICLE, v));
        glEnableVertexAttribArray(1);

        glBindVertexArray(pb->fa);

        glBindBuffer(GL_ARRAY_BUFFER, pb->fb);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(struct PARTICLE) * pb->count, particles);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(struct PARTICLE), (void*)offsetof(struct PARTICLE, x));
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(struct PARTICLE), (void*)offsetof(struct PARTICLE, v));
        glEnableVertexAttribArray(1);
    }
}

void
set_emmiter_position(int index, float x, float y, float z)
{
    if(index < 0 || index > MAX_EMMITERS)
        return;

    emmiters[index].x = x, emmiters[index].y = y, emmiters[index].z = z;
}

void
set_emmiter_velocity(int index, float x, float y)
{
    if(index < 0 || index > MAX_EMMITERS)
        return;

    emmiters[index].v = x, emmiters[index].w = y;
}

void
delete_emmiter(int index)
{
    if(index < 0 || index > MAX_EMMITERS)
        return;

    debug_log("delete emmiter index[%d]\n", index);

    memset(&emmiters[index], 0, sizeof(struct EMMITER));
}

////////////////////////////////////////////////////////////////////////////////

static void
check_framebuffer()
{
    const GLenum err = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    switch (err)
    {
    case GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE:
        break;
    case GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME:
        break;
    case GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL:
        break;
    case GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE:
        break;
    case GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LAYER:
        break;
    case GL_FRAMEBUFFER_COMPLETE:
        break;
    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
        fprintf(stderr, "Setup FBO failed. Duplicate attachment.\n");
        break;
    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
         fprintf(stderr, "Setup FBO failed. Missing attachment.\n");
        break;
    case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
         fprintf(stderr, "Setup FBO failed. Missing draw buffer.\n");
        break;
    case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
         fprintf(stderr, "Setup FBO failed. Missing read buffer.\n");
        break;
    case GL_FRAMEBUFFER_UNSUPPORTED:
         fprintf(stderr, "Setup FBO failed. Unsupported framebuffer format.\n");
        break;
    case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
         fprintf(stderr, "Setup FBO failed. Attached images must have the same number of samples.\n");
        break;
    default:
         fprintf(stderr, "Setup FBO failed. Fatal error.\n");
    }
}

static float
v2angle(float x0, float y0, float z0, float x1, float y1, float z1)
{
    float v = x0 * x1 + y0 * y1 + z0 * z1;
    float w = sqrtf(x0 * x0 + y0 * y0 + z0 * z0) * sqrtf(x1 * x1 + y1 * y1 + z1 * z1);

    return acosf(v / w);
}

static int
init_graphics()
{
    const char* ship_meshes_names[] =
    {
        "./resources/player_ship.obj",
        "./resources/ship1.obj",
        "./resources/ship2.obj",
        "./resources/ship3.obj",
        "./resources/ship4.obj",
        "./resources/ship5.obj"
    };

    /*for(int i = 0; i < 6; i++)
    {
        float *vertices;
        unsigned short *indices;
        int vn;
        int in;

        printf("%s\n", ship_meshes_names[i]);

        load_wavefront(ship_meshes_names[i], &vertices, &vn, &indices, &in);

        ship_meshes[i] = new_mesh(vertices, vn, indices, in);
        ship_meshes_count[i] = in;
        make_aabb(vertices, vn, &ship_meshes_aabb[i]);

        printf("min %f %f %f\n", ship_meshes_aabb[i].min[0], ship_meshes_aabb[i].min[1], ship_meshes_aabb[i].min[2]);
        printf("max %f %f %f\n", ship_meshes_aabb[i].max[0], ship_meshes_aabb[i].max[1], ship_meshes_aabb[i].max[2]);

        printf("(min + max)/2 %f %f %f\n", ship_meshes_aabb[i].min[0] + (ship_meshes_aabb[i].max[0] - ship_meshes_aabb[i].min[0]) / 2,
                ship_meshes_aabb[i].min[1] + (ship_meshes_aabb[i].max[1] - ship_meshes_aabb[i].min[1]) / 2,
                ship_meshes_aabb[i].min[2] + (ship_meshes_aabb[i].max[2] - ship_meshes_aabb[i].min[2]) / 2);

    }*/

    ship_meshes[0] = new_mesh(player_ship_model_vertices, player_ship_model_vertices_num, player_ship_model_indices, player_ship_model_indices_num);
    ship_meshes_count[0] = player_ship_model_indices_num;
    make_aabb(player_ship_model_vertices, player_ship_model_vertices_num, &ship_meshes_aabb[0]);

    ship_meshes[1] = new_mesh(ship1_model_vertices, ship1_model_vertices_num, ship1_model_indices, ship1_model_indices_num);
    ship_meshes_count[1] = ship1_model_indices_num;
    make_aabb(ship1_model_vertices, ship1_model_vertices_num, &ship_meshes_aabb[1]);

    ship_meshes[2] = new_mesh(ship2_model_vertices, ship2_model_vertices_num, ship2_model_indices, ship2_model_indices_num);
    ship_meshes_count[2] = ship2_model_indices_num;
    make_aabb(ship2_model_vertices, ship2_model_vertices_num, &ship_meshes_aabb[1]);

    ship_meshes[3] = new_mesh(ship3_model_vertices, ship3_model_vertices_num, ship3_model_indices, ship3_model_indices_num);
    ship_meshes_count[3] = ship3_model_indices_num;
    make_aabb(ship3_model_vertices, ship3_model_vertices_num, &ship_meshes_aabb[1]);

    ship_meshes[4] = new_mesh(ship4_model_vertices, ship4_model_vertices_num, ship4_model_indices, ship4_model_indices_num);
    ship_meshes_count[4] = ship4_model_indices_num;
    make_aabb(ship4_model_vertices, ship4_model_vertices_num, &ship_meshes_aabb[1]);

    ship_meshes[5] = new_mesh(ship5_model_vertices, ship5_model_vertices_num, ship5_model_indices, ship5_model_indices_num);
    ship_meshes_count[5] = ship5_model_indices_num;
    make_aabb(ship5_model_vertices, ship5_model_vertices_num, &ship_meshes_aabb[1]);

    /*const char* bonus_meshes_names[] =
    {
        "./resources/bonus1.obj",
        "./resources/bonus2.obj",
        "./resources/bonus3.obj",
    };

    for(int i = 0; i < 3; i++)
    {
        float *vertices;
        unsigned short *indices;
        int vn;
        int in;

        printf("%s\n", bonus_meshes_names[i]);

        load_wavefront(bonus_meshes_names[i], &vertices, &vn, &indices, &in);

        bonus_meshes[i] = new_mesh(vertices, vn, indices, in);
        bonus_meshes_count[i] = in;
        make_aabb(vertices, in, &bonus_meshes_aabb[i]);
    }*/

    bonus_meshes[0] = new_mesh(bonus1_model_vertices, bonus1_model_vertices_num, bonus1_model_indices, bonus1_model_indices_num);
    bonus_meshes_count[0] = bonus1_model_indices_num;
    make_aabb(bonus1_model_vertices, bonus1_model_vertices_num, &bonus_meshes_aabb[0]);

    bonus_meshes[1] = new_mesh(bonus2_model_vertices, bonus2_model_vertices_num, bonus2_model_indices, bonus2_model_indices_num);
    bonus_meshes_count[1] = bonus2_model_indices_num;
    make_aabb(bonus2_model_vertices, bonus2_model_vertices_num, &bonus_meshes_aabb[1]);

    bonus_meshes[2] = new_mesh(bonus3_model_vertices, bonus3_model_vertices_num, bonus3_model_indices, bonus3_model_indices_num);
    bonus_meshes_count[2] = bonus3_model_indices_num;
    make_aabb(bonus3_model_vertices, bonus3_model_vertices_num, &bonus_meshes_aabb[2]);

    glow_framebuffer_width = screen_width / 2;
    glow_framebuffer_height = screen_height / 2;

    glGetIntegerv(GL_MAX_SAMPLES, &max_supported_samples);

    glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);

    cube_mesh = new_cube();
    sphere_mesh = new_sphere(16, 16);
    quad_mesh = new_quad();
    cylinder_mesh = new_cylinder(1.f, 1.f, 5);
    sphere_mesh_instanced = new_sphere(8, 8);

    glBindVertexArray(sphere_mesh_instanced);

    glGenBuffers(1, &stars_color_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, stars_color_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float3) * MAX_STARS, 0, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(3);
    glVertexAttribDivisor(3, 1);

    glGenBuffers(1, &stars_matrix_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, stars_matrix_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float4x4) * MAX_STARS, 0, GL_DYNAMIC_DRAW);

    for(int i = 0; i < 4; i++)
    {
        glVertexAttribPointer(4 + i, 4, GL_FLOAT, GL_FALSE, sizeof(float) * 16, (void *)(sizeof(float4) * i));
        glEnableVertexAttribArray(4 + i);
        glVertexAttribDivisor(4 + i, 1);
    }

    glGenVertexArrays(1, &fullscreen_va);
    glBindVertexArray(fullscreen_va);

    glGenBuffers(1, &fullscreen_vb);

    glBindBuffer(GL_ARRAY_BUFFER, fullscreen_vb);
    glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(v3t2_t), fullscreen_quad_vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(v3t2_t), (void*)offsetof(v3t2_t, x));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(v3t2_t), (void*)offsetof(v3t2_t, u));
    glEnableVertexAttribArray(1);

    glGenVertexArrays(1, &ui_va);
    glBindVertexArray(ui_va);

    glGenBuffers(1, &ui_vb);

    glBindBuffer(GL_ARRAY_BUFFER, ui_vb);
    glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(v3t2_t), NULL, GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(v3t2_t), (void*)offsetof(v3t2_t, x));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(v3t2_t), (void*)offsetof(v3t2_t, u));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    const char* vars_names[] =
    {
        "vertex",
        "velocity",
        0
    };

    struct FEEDBACK_VARYINGS vars = {vars_names, 2, GL_INTERLEAVED_ATTRIBS};

    coloring_shader = new_shader(color_vertex_shader, color_fragment_shader, NULL);
    color_instanced_shader = new_shader(color_instanced_vertex_shader, color_instanced_fragment_shader, NULL);
    postprocess_shader = new_shader(postprocess_vertex_shader, postprocess_fragment_shader, NULL);
    hblur_shader = new_shader(postprocess_vertex_shader, hblur_fragment_shader, NULL);
    vblur_shader = new_shader(postprocess_vertex_shader, vblur_fragment_shader, NULL);
    ui_shader = new_shader(postprocess_vertex_shader, ui_fragment_shader, NULL);
    particles_draw_shader = new_shader(particles_draw_vertex_shader, particles_draw_fragment_shader, NULL);
    particles_update_shader = new_shader(particles_update_vertex_shader, particles_udpate_fragment_shader, &vars);
    texturing_shader = new_shader(color_vertex_shader, texturing_fragment_shader, NULL);
    background_shader = new_shader(postprocess_vertex_shader, texturing_fragment_shader, NULL);

    color_map = new_texture2D(GL_RGB8, screen_width, screen_height, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    depth_map = new_texture2D(GL_DEPTH_COMPONENT16, screen_width, screen_height, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT, NULL);
    ui_map = new_texture2D(GL_RGB8, screen_width, screen_height, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    blur_map = new_texture2D(GL_RGB8, glow_framebuffer_width, glow_framebuffer_height, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glow_map = new_texture2D(GL_RGB8, glow_framebuffer_width, glow_framebuffer_height, GL_RGB, GL_UNSIGNED_BYTE, NULL);

    //noise_map = get_texture("./resources/noise1.tga");
    //gradient_map = get_texture("./resources/gradient1.tga");

    //textures[0] = get_texture("./resources/shmup.tga");
    //textures[1] = get_texture("./resources/new_game.tga");
    //textures[2] = get_texture("./resources/settings.tga");
    //textures[3] = get_texture("./resources/exit_game.tga");
    //textures[4] = get_texture("./resources/return_game.tga");
    //textures[5] = get_texture("./resources/return_mainmenu.tga");
    //textures[6] = get_texture("./resources/score.tga");
    //textures[7] = get_texture("./resources/numbers.tga");
    //textures[8] = get_texture("./resources/about_game.tga");
    //textures[9] = get_texture("./resources/developer.tga");
    //textures[10] = get_texture("./resources/thanks.tga");
    //textures[11] = get_texture("./resources/sound_on.tga");
    //textures[12] = get_texture("./resources/sound_off.tga");
    //textures[13] = get_texture("./resources/game_over.tga");
    //textures[14] = get_texture("./resources/version.tga");

    noise_map = new_texture2D(noise1_internal_format, noise1_width, noise1_height, noise1_format, GL_UNSIGNED_BYTE, (void*)noise1_data);
    gradient_map = new_texture2D(gradient1_internal_format, gradient1_width, gradient1_height, gradient1_format, GL_UNSIGNED_BYTE, (void*)gradient1_data);

    textures[0] = new_texture2D(shmup_internal_format, shmup_width, shmup_height, shmup_format, GL_UNSIGNED_BYTE, (void*)shmup_data);
    textures[1] = new_texture2D(new_game_internal_format, new_game_width, new_game_height, new_game_format, GL_UNSIGNED_BYTE, (void*)new_game_data);
    textures[2] = new_texture2D(settings_internal_format, settings_width, settings_height, settings_format, GL_UNSIGNED_BYTE, (void*)settings_data);
    textures[3] = new_texture2D(exit_game_internal_format, exit_game_width, exit_game_height, exit_game_format, GL_UNSIGNED_BYTE, (void*)exit_game_data);
    textures[4] = new_texture2D(return_game_internal_format, return_game_width, return_game_height, return_game_format, GL_UNSIGNED_BYTE, (void*)return_game_data);
    textures[5] = new_texture2D(return_mainmenu_internal_format, return_mainmenu_width, return_mainmenu_height, return_mainmenu_format, GL_UNSIGNED_BYTE, (void*)return_mainmenu_data);
    textures[6] = new_texture2D(score_internal_format, score_width, score_height, score_format, GL_UNSIGNED_BYTE, (void*)score_data);
    textures[7] = new_texture2D(numbers_internal_format, numbers_width, numbers_height, numbers_format, GL_UNSIGNED_BYTE, (void*)numbers_data);
    textures[8] = new_texture2D(about_game_internal_format, about_game_width, about_game_height, about_game_format, GL_UNSIGNED_BYTE, (void*)about_game_data);
    textures[9] = new_texture2D(developer_internal_format, developer_width, developer_height, developer_format, GL_UNSIGNED_BYTE, (void*)developer_data);
    textures[10] = new_texture2D(thanks_internal_format, thanks_width, thanks_height, thanks_format, GL_UNSIGNED_BYTE, (void*)thanks_data);
    textures[11] = new_texture2D(sound_on_internal_format, sound_on_width, sound_on_height, sound_on_format, GL_UNSIGNED_BYTE, (void*)sound_on_data);
    textures[12] = new_texture2D(sound_off_internal_format, sound_off_width, sound_off_height, sound_off_format, GL_UNSIGNED_BYTE, (void*)sound_off_data);
    textures[13] = new_texture2D(game_over_internal_format, game_over_width, game_over_height, game_over_format, GL_UNSIGNED_BYTE, (void*)game_over_data);
    textures[14] = new_texture2D(version_internal_format, version_width, version_height, version_format, GL_UNSIGNED_BYTE, (void*)version_data);

    texture_sampler = new_sampler2D(GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE);
    ui_sampler = new_sampler2D(GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE);

    glGenFramebuffers(1, &color_framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, color_framebuffer);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color_map, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_map, 0);

    check_framebuffer();

    glGenFramebuffers(1, &ui_framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, ui_framebuffer);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ui_map, 0);

    check_framebuffer();

    glGenFramebuffers(1, &blur_framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, blur_framebuffer);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, blur_map, 0);

    check_framebuffer();

    glGenFramebuffers(1, &glow_framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, glow_framebuffer);

    glGenRenderbuffers(1, &depth1buffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depth1buffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, screen_width, screen_height);

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth1buffer);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, glow_map, 0);

    check_framebuffer();

    glGenFramebuffers(1, &sample_framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, sample_framebuffer);

    glGenRenderbuffers(1, &color0buffer);
    glBindRenderbuffer(GL_RENDERBUFFER, color0buffer);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, max_supported_samples, GL_RGBA8, screen_width, screen_width);

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, color0buffer);

    glGenRenderbuffers(1, &depth0buffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depth0buffer);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, max_supported_samples, GL_DEPTH_COMPONENT16, screen_width, screen_width);

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth0buffer);

    check_framebuffer();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // init particles buffers

    for(int i = 0; i < MAX_EMMITERS; i++)
    {
        struct PARTICLES_BUFFER *pb = &particles_buffers[i];

        glGenVertexArrays(1, &pb->va);
        glGenBuffers(1, &pb->vb);

        glBindVertexArray(pb->va);

        glBindBuffer(GL_ARRAY_BUFFER, pb->vb);
        glBufferData(GL_ARRAY_BUFFER, sizeof(struct PARTICLE) * MAX_PARTICLES, 0, GL_STATIC_DRAW);

        glGenVertexArrays(1, &pb->fa);
        glGenBuffers(1, &pb->fb);

        glBindVertexArray(pb->fa);

        glBindBuffer(GL_ARRAY_BUFFER, pb->fb);
        glBufferData(GL_ARRAY_BUFFER, sizeof(struct PARTICLE) * MAX_PARTICLES, 0, GL_STATIC_DRAW);

        glBindVertexArray(0);
    }

    return 0;
}

static int
clean_graphics()
{
    glDeleteVertexArrays(1, &fullscreen_va);
    glDeleteVertexArrays(1, &ui_va);

    glDeleteBuffers(1, &fullscreen_vb);
    glDeleteBuffers(1, &ui_vb);

    glDeleteVertexArrays(1, &cube_mesh);
    glDeleteVertexArrays(1, &sphere_mesh);
    glDeleteVertexArrays(1, &quad_mesh);

    glDeleteProgram(coloring_shader);
    glDeleteProgram(postprocess_shader);
    glDeleteProgram(hblur_shader);
    glDeleteProgram(vblur_shader);
    glDeleteProgram(ui_shader);
    glDeleteProgram(particles_draw_shader);
    glDeleteProgram(particles_update_shader);
    glDeleteProgram(texturing_shader);

    glDeleteTextures(1, &color_map);
    glDeleteTextures(1, &depth_map);
    glDeleteTextures(1, &blur_map);
    glDeleteTextures(1, &glow_map);

    glDeleteSamplers(1, &texture_sampler);

    glDeleteFramebuffers(1, &color_framebuffer);
    glDeleteFramebuffers(1, &glow_framebuffer);
    glDeleteFramebuffers(1, &blur_framebuffer);
    glDeleteFramebuffers(1, &sample_framebuffer);


    for(int i = 0; i < MAX_EMMITERS; i++)
    {
        struct PARTICLES_BUFFER *pb = &particles_buffers[i];

        glDeleteVertexArrays(1, &pb->va);
        glDeleteVertexArrays(1, &pb->fa);

        glDeleteBuffers(1, &pb->vb);
        glDeleteBuffers(1, &pb->fb);
    }

    return 0;
}

static int
init_audio()
{
    alGenSources(MAX_SOUND_SOURCES, sound_sources);

    //sounds[COIN_SOUND] = get_sound("./resources/coin.wav");
    //sounds[SHOT1_SOUND] = get_sound("./resources/shot1.wav");
    //sounds[SHOT2_SOUND] = get_sound("./resources/shot2.wav");
    //sounds[SHOT3_SOUND] = get_sound("./resources/shot3.wav");
    //sounds[EXPLOSION1_SOUND] = get_sound("./resources/explosion1.wav");
    //sounds[EXPLOSION2_SOUND] = get_sound("./resources/explosion3.wav");
    //sounds[WARP_SOUND] = get_sound("./resources/warp.wav");
    //sounds[SELECT_SOUND] = get_sound("./resources/select.wav");

    sounds[COIN_SOUND] = new_sound(coin_format, coin_frequency, coin_size, coin_data);
    sounds[SHOT1_SOUND] = new_sound(shot1_format, shot1_frequency, shot1_size, shot1_data);
    sounds[SHOT2_SOUND] = new_sound(shot2_format, shot2_frequency, shot2_size, shot2_data);
    sounds[SHOT3_SOUND] = new_sound(shot3_format, shot3_frequency, shot3_size, shot3_data);
    sounds[EXPLOSION1_SOUND] = new_sound(explosion1_format, explosion1_frequency, explosion1_size, explosion1_data);
    sounds[EXPLOSION2_SOUND] = new_sound(explosion3_format, explosion3_frequency, explosion3_size, explosion3_data);
    sounds[WARP_SOUND] = new_sound(warp_format, warp_frequency, warp_size, warp_data);
    sounds[SELECT_SOUND] = new_sound(select_format, select_frequency, select_size, select_data);

    const ALfloat ori[] = {0.0, 0.0, -1.0, 0.0, 1.0, 0.0};

    alListener3f(AL_POSITION, 0, 0, 0);
    alListener3f(AL_VELOCITY, 0, 0, 0);
    alListenerfv(AL_ORIENTATION, ori);

    return 0;
}

static int
clean_audio()
{
    alDeleteSources(MAX_SOUND_SOURCES, sound_sources);

    for(int i = 0; i < MAX_SOUND; i++)
        alDeleteBuffers(1, &sounds[i]);

    return 0;
}

int
play_sound(enum SOUNDS sound, int looped)
{
    if(sound_off == 1)
        return 0;

    for(int i = 0; i < MAX_SOUND_SOURCES; i++)
    {
        ALint status = 0;

        alGetSourcei(sound_sources[i], AL_SOURCE_STATE, &status);

        if((status == AL_INITIAL) || (status == AL_STOPPED) || (status == 0))
        {
            alSource3f(sound_sources[i], AL_POSITION, 0.0f, 0.0f, 0.0f);
            alSource3f(sound_sources[i], AL_VELOCITY, 0.0f, 0.0f, 0.0f);
            alSourcef(sound_sources[i], AL_PITCH, 1.0f);
            alSourcef(sound_sources[i], AL_GAIN, 1.0f);

            if(looped != 0)
                alSourcei(sound_sources[i], AL_LOOPING, AL_TRUE);

            alSourcei(sound_sources[i], AL_BUFFER, sounds[sound]);
            alSourcePlay(sound_sources[i]);

            return i;
        }
    }

    return 0;
}

int
pause_sound(int source)
{
    alSourcePause(sound_sources[source]);
    return source;
}

int
unpause_sound(int source)
{
    alSourcePlay(sound_sources[source]);
    return source;
}

void play_weapon_sound(int weapon)
{
    switch(weapon)
    {
    case GUN_BULLET:
        play_sound(SHOT1_SOUND, 0);
        break;
    case ROCKET_BULLET:
        play_sound(SHOT2_SOUND, 0);
        break;
    case PLASMA_BULLET:
        play_sound(SHOT3_SOUND, 0);
        break;
    }
}

static void
generate_stars()
{
    const GLfloat identitymatrix[16] = IDENTITY_MATRIX4;

    for(int i = 0; i < MAX_STARS; i++)
    {
        stars[i].x = randf2(-20, 25), stars[i].y = randf2(-10, 10), stars[i].z = randf2(-5, -20);
        stars[i].size = randf2(STAR_MIN_SIZE, STAR_MAX_SIZE);
#ifdef RANDOM_COLOR_STARS_OFF
        const int c = 2;
#else
        int c = rand() % 6;
#endif

        float3 colors[] =
        {
            {0, 0.76, 0.85},            // blue
            {0.498, 0.78, 1},           // white-blue
            {1, 1, 1},                  // white
            {0.98, 0.925, 0.364},       // white-yellow
            {1, 1, 0},                  // yellow
            {1, 0.647, 0},              // orange
            {1, 0.2, 0}                 // red
        };

        memcpy(stars[i].color, &colors[c], sizeof(float3));

        memcpy(stars[i].transform, identitymatrix, sizeof(identitymatrix));

        scale(stars[i].transform, stars[i].size, stars[i].size, stars[i].size);
        translate(stars[i].transform, stars[i].x, stars[i].y, stars[i].z);
    }
}

static void
update_stars()
{
    const GLfloat identitymatrix[16] = IDENTITY_MATRIX4;

    for(int i = 0; i < MAX_STARS; i++)
    {
        if(abs(stars[i].x - camera.x) > 20)
        {
            stars[i].x = camera.x + 40 + randf2(-20, 20), stars[i].y = randf2(-10, 10), stars[i].z = randf2(-5, -20);
            stars[i].size = randf2(STAR_MIN_SIZE, STAR_MAX_SIZE);

            memcpy(stars[i].transform, identitymatrix, sizeof(identitymatrix));

            scale(stars[i].transform, stars[i].size, stars[i].size, stars[i].size);
            translate(stars[i].transform, stars[i].x, stars[i].y, stars[i].z);
        }
    }
}

static void
generate_nebulas()
{
    const GLfloat identitymatrix[16] = IDENTITY_MATRIX4;

    for(int j = 0; j < MAX_NEBULAS; j++)
    {
        nebulas[j].surfaces_count = MIN_NEBULA_SURFACES + rand() % (MAX_NEBULA_SURFACES - MIN_NEBULA_SURFACES);
        nebulas[j].x = randf2(0, 25);
        nebulas[j].y = randf2(-8, 8);
        nebulas[j].z = randf2(-5, -15);
        nebulas[j].size = randf2(NEBULA_MIN_SIZE, NEBULA_MAX_SIZE);
        nebulas[j].v = randf2(SCROLL_SPEED / 4, SCROLL_SPEED / 2);

        for(int i = 0; i < MAX_NEBULA_SURFACES; i++)
        {
            float3 colors[] =
            {
                {0.5, 0.1, 0.8},
                {0.1, 0.3, 0.6},
                {0.2, 0.2, 0.7},
                {0.7, 0.2, 0.5},
                {0.7, 0.3, 0.4},
                {0.6, 0.6, 0.2},
                {0.8, 0.8, 0.85},
                {0.7, 0.5, 0.1}
            };

            int c = rand() % 8;

            memcpy(nebulas[j].surfaces[i].color, &colors[c], sizeof(colors[c]));

            nebulas[j].surfaces[i].x = randf2(-nebulas[j].size / 3, nebulas[j].size / 3);
            nebulas[j].surfaces[i].y = randf2(-nebulas[j].size / 3, nebulas[j].size / 3);
            nebulas[j].surfaces[i].z = randf2(0, 1);

            nebulas[j].surfaces[i].q = randf2(0, 20.0);
            nebulas[j].surfaces[i].d = randf2(0, 20.0);
            nebulas[j].surfaces[i].p = randf2(0, 360.0);

            nebulas[j].surfaces[i].size = randf2(-nebulas[j].size / 4, nebulas[j].size / 4);

            float size = nebulas[j].size + nebulas[j].surfaces[i].size;

            memcpy(nebulas[j].surfaces[i].transform, identitymatrix, sizeof(identitymatrix));

            scale(nebulas[j].surfaces[i].transform, size, size, size);
            rotate(nebulas[j].surfaces[i].transform, nebulas[j].surfaces[i].q, X_AXIS);
            rotate(nebulas[j].surfaces[i].transform, nebulas[j].surfaces[i].d, Y_AXIS);
            rotate(nebulas[j].surfaces[i].transform, nebulas[j].surfaces[i].p, Z_AXIS);
            translate(nebulas[j].surfaces[i].transform,
                      nebulas[j].x + nebulas[j].surfaces[i].x,
                      nebulas[j].y + nebulas[j].surfaces[i].y,
                      nebulas[j].z + nebulas[j].surfaces[i].z);
        }
    }
}

static void
update_nebulas()
{
    const GLfloat identitymatrix[16] = IDENTITY_MATRIX4;

    for(int j = 0; j < MAX_NEBULAS; j++)
    {
        if(abs(nebulas[j].x - camera.x) > 20)
        {
            nebulas[j].surfaces_count = 2 + rand() % (MAX_NEBULA_SURFACES - 2);
            nebulas[j].x = 40 + camera.x + randf2(-20, 20);
            nebulas[j].y = randf2(-8, 8);
            nebulas[j].z = randf2(-5, -15);
            nebulas[j].v = randf2(SCROLL_SPEED / 4, SCROLL_SPEED / 2);
            nebulas[j].size = randf2(NEBULA_MIN_SIZE, NEBULA_MAX_SIZE);
            nebulas[j].v = randf2(SCROLL_SPEED / 4, SCROLL_SPEED / 2);

            for(int i = 0; i < MAX_NEBULA_SURFACES; i++)
            {
                nebulas[j].surfaces[i].x = randf2(-nebulas[j].size / 3, nebulas[j].size / 3);
                nebulas[j].surfaces[i].y = randf2(-nebulas[j].size / 3, nebulas[j].size / 3);
                nebulas[j].surfaces[i].z = randf2(0, 1);

                nebulas[j].surfaces[i].q = randf2(0, 20.0);
                nebulas[j].surfaces[i].d = randf2(0, 20.0);
                nebulas[j].surfaces[i].p = randf2(0, 360.0);

                nebulas[j].surfaces[i].size = randf2(-nebulas[j].size / 4, nebulas[j].size / 4);
            }
        }

        for(int i = 0; i < nebulas[j].surfaces_count; i++)
        {
            float size = nebulas[j].size + nebulas[j].surfaces[i].size;

            memcpy(nebulas[j].surfaces[i].transform, identitymatrix, sizeof(identitymatrix));

            scale(nebulas[j].surfaces[i].transform, size, size, size);
            rotate(nebulas[j].surfaces[i].transform, nebulas[j].surfaces[i].q, X_AXIS);
            rotate(nebulas[j].surfaces[i].transform, nebulas[j].surfaces[i].d, Y_AXIS);
            rotate(nebulas[j].surfaces[i].transform, nebulas[j].surfaces[i].p, Z_AXIS);
            translate(nebulas[j].surfaces[i].transform, nebulas[j].x + nebulas[j].surfaces[i].x, nebulas[j].y + nebulas[j].surfaces[i].y, nebulas[j].z);
        }

        nebulas[j].x += nebulas[j].v;
    }
}

static void
new_enemies_group()
{
    float base_x = player_ship->x;
    int id = 0;

    int s[12] = {};

    float offsets_x[] =
    {
        6.50, 6.50,
        6.75, 6.75,
        7.00, 7.00,
        7.25, 7.25,
        7.50, 7.50,
        7.75, 7.75
    };
    float offsets_y[] =
    {
        0.4, -0.4,
        0.6, -0.6,
        0.8, -0.8,
        1.0, -1.0,
        1.2, -1.2,
        1.4, -1.4
    };

    for(int i = 0; i < countof(s); i++)
    {
        s[i] = 1 + rand() % (MAX_SHIP_TYPE - 1);
    }


    for(int i = 0; i < 12; i++)
    {
        id = new_ship(s[i], base_x + offsets_x[i], offsets_y[i], 0, ENEMY1_SHIP_SIZE);
        set_ship_active_distance(id, 4.0);
        set_ship_rotation(id, 90, 0, 0);
    }

    /*id = new_ship(s0, base_x + 6.0, 0.4, 0, ENEMY1_SHIP_SIZE);
    set_ship_active_distance(id, 4.0);
    set_ship_rotation(id, 90, 0, 0);

    id = new_ship(s0, base_x + 6.0, -0.4, 0, ENEMY1_SHIP_SIZE);
    set_ship_active_distance(id, 4.0);
    set_ship_rotation(id, 90, 0, 0);

    id = new_ship(s0, base_x + 6.5, 0.6, 0, ENEMY1_SHIP_SIZE);
    set_ship_active_distance(id, 4.0);
    set_ship_rotation(id, 90, 0, 0);

    id = new_ship(s0, base_x + 6.5, -0.6, 0, ENEMY1_SHIP_SIZE);
    set_ship_active_distance(id, 4.0);
    set_ship_rotation(id, 90, 0, 0);

    id = new_ship(s0, base_x + 6.5, 0.8, 0, ENEMY1_SHIP_SIZE);
    set_ship_active_distance(id, 4.0);
    set_ship_rotation(id, 90, 0, 0);

    id = new_ship(s0, base_x + 6.5, -0.8, 0, ENEMY1_SHIP_SIZE);
    set_ship_active_distance(id, 4.0);
    set_ship_rotation(id, 90, 0, 0);

    id = new_ship(s1, base_x + 7, 1.0, 0, ENEMY1_SHIP_SIZE);
    set_ship_active_distance(id, 4.0);
    set_ship_rotation(id, 90, 0, 0);

    id = new_ship(s1, base_x + 7, -1.0, 0, ENEMY1_SHIP_SIZE);
    set_ship_active_distance(id, 4.0);
    set_ship_rotation(id, 90, 0, 0);

    id = new_ship(s2, base_x + 7.5, 1.2, 0, ENEMY1_SHIP_SIZE);
    set_ship_active_distance(id, 4.0);
    set_ship_rotation(id, 90, 0, 0);

    id = new_ship(s2, base_x + 7.5, -1.2, 0, ENEMY1_SHIP_SIZE);
    set_ship_active_distance(id, 4.0);
    set_ship_rotation(id, 90, 0, 0);

    id = new_ship(s3, base_x + 8.0, 1.4, 0, ENEMY1_SHIP_SIZE);
    set_ship_active_distance(id, 4.0);
    set_ship_rotation(id, 90, 0, 0);

    id = new_ship(s3, base_x + 8.0, -1.4, 0, ENEMY1_SHIP_SIZE);
    set_ship_active_distance(id, 4.0);
    set_ship_rotation(id, 90, 0, 0);*/

    enemies_count = 12;
}

static void
enemies_group_callback(struct TIMER *self, void *data)
{
    if(enemies_count <= 0)
        new_enemies_group();
}

static void
cleanup_objects()
{
    bullets_count = 0;
    memset(bullets, 0, sizeof(bullets));

    ships_count = 0;
    enemies_count = 0;
    memset(ships, 0, sizeof(ships));

    bonuses_count = 0;
    memset(bonuses, 0, sizeof(bonuses));

    timers_count = 0;
    memset(timers, 0, sizeof(timers));

    memset(emmiters, 0, sizeof(emmiters));

    memset(&boss, 0, sizeof(boss));
}

static int
restart_game()
{
    cleanup_objects();

    camera.x = 0, camera.y = 0, camera.z = 5;
    camera.a = 0, camera.b = 0, camera.c = 0;

    player_ship = &ships[0];
    boss.dead = 1;

    int id = new_ship(PLAYER_SHIP, -2.5, 0, 0, PLAYER_SHIP_SIZE);
    set_ship_rotation(id, 90, 0, 180);

    new_enemies_group();

    new_timer(INFINITE_TIMER, 2.0, enemies_group_callback, NULL);

    //new_boss(0);

    player_ship->v = SCROLL_SPEED;

    generate_stars();
    generate_nebulas();

    game_over = 0;

    return 0;
}

static int
startup_game()
{
    if(init_graphics() != 0)
        return -1;

    if(init_audio() !=0)
        return -1;

    game_states[0].display_state = display_mainmenu;
    game_states[0].process_state = process_mainmenu;
    game_states[1].display_state = display_game;
    game_states[1].process_state = process_game;
    game_states[2].display_state = display_pausemenu;
    game_states[2].process_state = process_pausemenu;
    game_states[3].display_state = display_aboutmenu;
    game_states[3].process_state = process_aboutmenu;
    game_states[4].display_state = display_settingsmenu;
    game_states[4].process_state = process_settingsmenu;

    stack_state = new_stack(sizeof(struct GAME_STATE), 10);
    push_stack(stack_state, &game_states[0]);

    return 0;
}

static int
shutdown_game()
{
    delete_stack(stack_state);

    clean_graphics();
    clean_audio();

    return 0;
}

static void
draw_ship(float *projection, float *view, const struct SHIP *ship)
{
    const GLfloat identitymatrix[16] = IDENTITY_MATRIX4;
    GLfloat mvp[16];

    memcpy(mvp, identitymatrix, sizeof(mvp));
    multiply4x4(mvp, ship->transform);
    multiply4x4(mvp, view);
    multiply4x4(mvp, projection);

    //draw_cube(coloring_shader, ship->color, mvp);

    if(ship->type == PLAYER_SHIP)
        draw_mesh(ship_meshes[0], coloring_shader, ship->color, mvp, ship_meshes_count[0]);

    if(ship->type == ENEMY1_SHIP)
        draw_mesh(ship_meshes[1], coloring_shader, ship->color, mvp, ship_meshes_count[1]);

    if(ship->type == ENEMY2_SHIP)
        draw_mesh(ship_meshes[2], coloring_shader, ship->color, mvp, ship_meshes_count[2]);

    if(ship->type == ENEMY3_SHIP)
        draw_mesh(ship_meshes[3], coloring_shader, ship->color, mvp, ship_meshes_count[3]);

    if(ship->type == ENEMY4_SHIP)
        draw_mesh(ship_meshes[4], coloring_shader, ship->color, mvp, ship_meshes_count[4]);

    if(ship->type == ENEMY5_SHIP)
        draw_mesh(ship_meshes[5], coloring_shader, ship->color, mvp, ship_meshes_count[5]);

    if(ship->shield.active)
    {
        GLfloat m[16];
        float a = 0.3 * (1.0 - ship->shield.tv / ship->shield.max_time) * ((float)ship->shield.value / (float)ship->shield.max_value);
        float color[4] = {0.3, 0.35, 0.6, a};
        float sz = ship->l + 0.03;

        memcpy(m, identitymatrix, sizeof(m));
        scale(m, sz, sz, sz);
        rotate(m, 0, X_AXIS);
        rotate(m, 0, Y_AXIS);
        rotate(m, 0, Z_AXIS);
        translate(m, ship->transform[12], ship->transform[13], ship->transform[14]);

        memcpy(mvp, identitymatrix, sizeof(mvp));
        multiply4x4(mvp, m);
        multiply4x4(mvp, view);
        multiply4x4(mvp, projection);

        draw_transparent_sphere(coloring_shader, color, mvp);
    }
}

static void
draw_bullet(float *projection, float *view, const struct BULLET *bullet)
{
    const GLfloat identitymatrix[16] = IDENTITY_MATRIX4;
    GLfloat mvp[16];

    if(bullet->type == GUN_BULLET)
    {
        float color[3] = {BULLET1_COLOR};

        memcpy(mvp, identitymatrix, sizeof(mvp));
        multiply4x4(mvp, bullet->transform);
        multiply4x4(mvp, view);
        multiply4x4(mvp, projection);

        draw_sphere(coloring_shader, color, mvp);

        return;
    }

    if(bullet->type == ROCKET_BULLET)
    {
        float color[3] = {BULLET2_COLOR};

        memcpy(mvp, identitymatrix, sizeof(mvp));
        multiply4x4(mvp, bullet->transform);
        multiply4x4(mvp, view);
        multiply4x4(mvp, projection);

        draw_cube(coloring_shader, color, mvp);

        return;
    }

    if(bullet->type == PLASMA_BULLET)
    {
        float color[3] = {BULLET3_COLOR};

        memcpy(mvp, identitymatrix, sizeof(mvp));
        multiply4x4(mvp, bullet->transform);
        multiply4x4(mvp, view);
        multiply4x4(mvp, projection);

        draw_sphere(coloring_shader, color, mvp);

        return;
    }
}

static void
draw_bonus(float *projection, float *view, const struct BONUS *bonus)
{
    const GLfloat identitymatrix[16] = IDENTITY_MATRIX4;
    GLfloat mvp[16];

    memcpy(mvp, identitymatrix, sizeof(mvp));
    multiply4x4(mvp, bonus->transform);
    multiply4x4(mvp, view);
    multiply4x4(mvp, projection);

    if(bonus->type == HEALTH_BONUS)
    {
        float color[3] = {0, 1, 0};

        draw_mesh(bonus_meshes[0], coloring_shader, color, mvp, bonus_meshes_count[0]);
        //draw_cube(coloring_shader, color, mvp);

        return;
    }

    if(bonus->type == ARMOR_BONUS)
    {
        float color[3] = {0.7, 0.7, 0.72};

        draw_mesh(bonus_meshes[1], coloring_shader, color, mvp, bonus_meshes_count[1]);
        //draw_cube(coloring_shader, color, mvp);

        return;
    }

    if(bonus->type == WEAPON_BONUS)
    {
        float color[3] = {1.0, 0, 0};

        draw_mesh(bonus_meshes[2], coloring_shader, color, mvp, bonus_meshes_count[2]);
        //draw_cube(coloring_shader, color, mvp);

        return;
    }
}

static void
draw_boss(float *projection, float *view, struct BOSS *boss)
{
    const GLfloat identitymatrix[16] = IDENTITY_MATRIX4;
    GLfloat mvp[16];

    float color[3] = {1, 0, 0};

    memcpy(mvp, identitymatrix, sizeof(mvp));
    multiply4x4(mvp, boss->transform);
    multiply4x4(mvp, view);
    multiply4x4(mvp, projection);

    draw_cube(coloring_shader, color, mvp);
}

/*static void
draw_star(float *projection, float *view, struct STAR *star)
{
    const GLfloat identitymatrix[16] = IDENTITY_MATRIX4;
    GLfloat mvp[16];

    memcpy(mvp, identitymatrix, sizeof(mvp));
    multiply4x4(mvp, star->transform);
    multiply4x4(mvp, view);
    multiply4x4(mvp, projection);

    draw_sphere(coloring_shader, star->color, mvp);
}*/

static void
draw_stars(float *projection, float *view)
{
    const GLfloat identitymatrix[16] = IDENTITY_MATRIX4;
    GLfloat mvp[16];

    float3 colors[MAX_STARS];

    for(int i = 0; i < MAX_STARS; i++)
        memcpy(&colors[i][0], &stars[i].color[0], sizeof(colors[i]));

    glBindBuffer(GL_ARRAY_BUFFER, stars_color_buffer);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(colors), colors);

    glBindBuffer(GL_ARRAY_BUFFER, stars_matrix_buffer);
    float4x4 *matrices = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);

    for(int i = 0; i < MAX_STARS; i++)
    {
        memcpy(mvp, identitymatrix, sizeof(mvp));
        multiply4x4(mvp, stars[i].transform);
        multiply4x4(mvp, view);
        multiply4x4(mvp, projection);

        memcpy(&matrices[i], mvp, sizeof(matrices[i]));
    }

    glUnmapBuffer(GL_ARRAY_BUFFER);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    draw_instanced_sphere(color_instanced_shader, MAX_STARS);
}

static void
draw_emmiters(float *projection, float *view)
{
    const GLfloat identitymatrix[16] = IDENTITY_MATRIX4;

    GLfloat mvp[16];

    for(int i = 0; i < MAX_EMMITERS; i++)
        if(emmiters[i].active)
        {
            memcpy(mvp, identitymatrix, sizeof(mvp));

            multiply4x4(mvp, emmiters[i].transform);
            multiply4x4(mvp, view);
            multiply4x4(mvp, projection);

            struct PARTICLES_BUFFER *pb = (struct PARTICLES_BUFFER*)emmiters[i].buffer;

            float3 color = {0, 0, 0};

            if(emmiters[i].flags & PA_RAND_COLOR)
                mix3(emmiters[i].color[1], emmiters[i].color[0], color, emmiters[i].tv / emmiters[i].tl);
            else
                memcpy(color, emmiters[i].color[0], sizeof(color));

            draw_particles(particles_draw_shader, color, -emmiters[i].tv / emmiters[i].tl, mvp, pb);
        }
}

static void
draw_nebula(float *projection, float *view, struct NEBULA *nebula)
{
    const GLfloat identitymatrix[16] = IDENTITY_MATRIX4;
    GLfloat mvp[16];

    for(int i = 0; i < nebula->surfaces_count; i++)
    {

        memcpy(mvp, identitymatrix, sizeof(mvp));
        multiply4x4(mvp, nebula->surfaces[i].transform);
        multiply4x4(mvp, view);
        multiply4x4(mvp, projection);

        draw_textured_quad(texturing_shader, noise_map, gradient_map, ui_sampler, nebula->surfaces[i].color, mvp);
    }
}

static void
update_emmiters_particles()
{
    for(int i = 0; i < MAX_EMMITERS; i++)
        if(emmiters[i].active)
        {
            struct PARTICLES_BUFFER *pb = (struct PARTICLES_BUFFER*)emmiters[i].buffer;

            update_particles(particles_update_shader, emmiters[i] .tv, pb);

            GLuint a = pb->va;
            GLuint b = pb->vb;
            pb->va = pb->fa;
            pb->vb = pb->fb;
            pb->fa = a;
            pb->fb= b;
        }
}

void
print_number(int number, GLuint shader, GLuint texture, GLuint sampler, float3 color, float2 position, float2 size, int n)
{
    int digits_count = 0;
    int digits[20] = {};

    while(number > 0)
    {
        digits[digits_count++] = number % 10;

        number /= 10;
    }

    if(n > 0 && n > digits_count)
        digits_count = n;

    for(int i = 0; i < digits_count; i++)
    {
        float2 p = {position[0] + size[0] * i * 2.f, position[1]};

        draw_ui(shader, texture, sampler, color, p, size, digits[digits_count - 1 - i], 10);
    }
}

static void
explosion_effect(int type, float tv, float x, float y, float z)
{
    struct PARTICLE_ATTRIBUTES pa = {};

    int count = 0;

    switch(type)
    {
    case 0:
        pa.size = 8.0;
        pa.velocity[0][0] = -0.05;
        pa.velocity[0][1] = -0.05;
        pa.velocity[1][0] = 0.05;
        pa.velocity[1][1] = 0.05;

        pa.color[0][0] = 1;
        pa.color[0][1] = 1;
        pa.color[0][2] = 0;
        pa.color[1][0] = 1;
        pa.color[1][1] = 0;
        pa.color[1][2] = 0;

        count = 100;

        break;
    case 1:
        pa.size = 6.0;
        pa.velocity[0][0] = -0.05;
        pa.velocity[0][1] = -0.05;
        pa.velocity[1][0] = 0.05;
        pa.velocity[1][1] = 0.05;

        pa.color[0][0] = 1;
        pa.color[0][1] = 1;
        pa.color[0][2] = 0;
        pa.color[1][0] = 1;
        pa.color[1][1] = 0;
        pa.color[1][2] = 0;

        count = 100;
        break;
    }

    int id = new_emmiter(count);

    if(id == -1)
        return;

    set_emmiter_position(id, x, y, z);
    set_emmiter_attributes(id, &pa, tv, PA_RAND_BETWEEN_VELOCITY | PA_RAND_COLOR);
}

static void
warp_effect(int type, float tv, float x, float y, float z)
{
    struct PARTICLE_ATTRIBUTES pa = {};

    switch(type)
    {
    case 0:
        pa.velocity[0][0] = -0.2;
        pa.velocity[0][1] = -0.2;

        pa.color[0][0] = 0;
        pa.color[0][1] = 1;
        pa.color[0][2] = 1;
        pa.color[1][0] = 1;
        pa.color[1][1] = 0;
        pa.color[1][2] = 1;
        break;

    case 1:
        pa.velocity[0][0] = 0.05;
        pa.velocity[0][1] = 0.05;

        pa.color[0][0] = 1;
        pa.color[0][1] = 0;
        pa.color[0][2] = 1;
        pa.color[1][0] = 0;
        pa.color[1][1] = 1;
        pa.color[1][2] = 1;
        break;
    }

    pa.size = 4.0;

    pa.position[0][0] = 0.5;
    pa.position[0][1] = 0.5;
    pa.position[0][2] = 0.5;

    int id = new_emmiter(300);
    set_emmiter_position(id, x, y, z);
    set_emmiter_attributes(id, &pa, tv, PA_BASE_VELOCITY | PA_RAND_COLOR | PA_RAND_BASE_POSITION);
}

void
draw_blur(GLuint shader, GLuint tex)
{
    int loc_tex0 = glGetUniformLocation(shader, "tex0");
    int loc_sz = glGetUniformLocation(shader, "size");

    float2 size;
    size[0] = 1.0 / (float)glow_framebuffer_width;
    size[1] = 1.0 / (float)glow_framebuffer_height;

    glUniform1i(loc_tex0, 0);
    glUniform2f(loc_sz, size[0], size[1]);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);

    glBindSampler(0, texture_sampler);

    glBindVertexArray(fullscreen_va);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindVertexArray(0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void
display_game()
{
    const GLfloat identitymatrix[16] = IDENTITY_MATRIX4;

    GLfloat projection[16];
    GLfloat view[16];

    perspective(projection, 45.0, (float)screen_width / screen_height, 0.1, 1000.0);

    memcpy(view, identitymatrix, sizeof(view));
    rotate(view, camera.a, X_AXIS);
    rotate(view, camera.b, Y_AXIS);
    rotate(view, camera.c, Z_AXIS);
    translate(view, -camera.x, -camera.y, -camera.z);

    glBindFramebuffer(GL_FRAMEBUFFER, sample_framebuffer);
    glViewport(0, 0, screen_width, screen_height);
    glClearColor(BACKGROUND_COLOR);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(background_shader);

    {
        GLfloat mvp[16];
        //float color[4] = {0.22, 0.2, 0.4, 1};
        float color[4] = {0.088, 0.08, 0.15, 1};

        memcpy(mvp, identitymatrix, sizeof(mvp));
        draw_textured_quad(texturing_shader, noise_map, gradient_map, ui_sampler, color, mvp);
    }

    glUseProgram(texturing_shader);

    for(int i = 0; i < MAX_NEBULAS; i++)
        draw_nebula(projection, view, &nebulas[i]);

    glUseProgram(color_instanced_shader);

    draw_stars(projection, view);    

    glUseProgram(coloring_shader);

    for(int i = 0; i < MAX_BULLETS; i++)
        if(bullets[i].dead != 0)
            draw_bullet(projection, view, &bullets[i]);

    for(int i = 0; i < MAX_SHIPS; i++)
        if(ships[i].dead == 0)
            draw_ship(projection, view, &ships[i]);

    for(int i = 0; i < bonuses_count; i++)
        if(bonuses[i].dead == 0)
            draw_bonus(projection, view, &bonuses[i]);

    if(boss.dead == 0)
        draw_boss(projection, view, &boss);

    glUseProgram(particles_draw_shader);

    draw_emmiters(projection, view);

    glUseProgram(particles_update_shader);

    update_emmiters_particles();

    glUseProgram(0);

    glBindFramebuffer(GL_FRAMEBUFFER, ui_framebuffer);
    glViewport(0, 0, screen_width, screen_height);

    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(ui_shader);

    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    // HUD

    {
        float3 c = {1, 1, 0};
        float2 p = {-0.8, 0.8};
        float2 s = {0.05, 0.01};

        draw_ui(ui_shader, textures[6], ui_sampler, c, p, s, 0, 1);
    }

    {
        float3 c = {1, 1, 0};
        float2 p = {-0.7, 0.8};
        float2 s = {0.01, 0.01};

        print_number(score, ui_shader, textures[7], ui_sampler, c, p, s, 10);
    }

    if(game_over)
    {
        float3 c = {1, 0, 0};
        float2 p = {0, 0};
        float2 s = {0.2, 0.2};

        draw_ui(ui_shader, textures[13], ui_sampler, c, p, s, 0, 1);
    }

    // glow
    glBindFramebuffer(GL_FRAMEBUFFER, glow_framebuffer);
    glViewport(0, 0, glow_framebuffer_width, glow_framebuffer_height);
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // draw glow

    glUseProgram(coloring_shader);

    for(int i = 0; i < MAX_BULLETS; i++)
        if((bullets[i].dead != 0) && (bullets[i].type == PLASMA_BULLET))
            draw_bullet(projection, view, &bullets[i]);

    glUseProgram(color_instanced_shader);

    draw_stars(projection, view);

    glUseProgram(0);

    glBindFramebuffer(GL_FRAMEBUFFER, blur_framebuffer);
    glViewport(0, 0, glow_framebuffer_width, glow_framebuffer_height);

    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(vblur_shader);

    draw_blur(vblur_shader, glow_map);

    glBindFramebuffer(GL_FRAMEBUFFER, glow_framebuffer);
    glViewport(0, 0, glow_framebuffer_width, glow_framebuffer_height);

    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(hblur_shader);

    draw_blur(hblur_shader, blur_map);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, sample_framebuffer);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, color_framebuffer);
    glBlitFramebuffer(0, 0, screen_width, screen_height, 0, 0, screen_width, screen_height, GL_COLOR_BUFFER_BIT, GL_LINEAR);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, screen_width, screen_height);

    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(postprocess_shader);

    draw_fullscreen_quad(postprocess_shader, texture_sampler);

    glUseProgram(0);
}

void
display_mainmenu()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, screen_width, screen_height);

    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(ui_shader);

    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    float3 sc = {0, 0.9, 0.9};

    {
        float3 c = {1, 1, 1};
        float2 p = {0, 0.5};
        float2 s = {0.3, 0.08};

        draw_ui(ui_shader, textures[0], ui_sampler, c, p, s, 0, 1);
    }

    {
        float3 c = {0.0, 0.5, 0.9};
        float2 p = {0, 0.0};
        float2 s = {0.1, 0.035};

        if(selected_menu_item == 0)
            memcpy(c, sc, sizeof(c));

        draw_ui(ui_shader, textures[1], ui_sampler, c, p, s, 0, 1);
    }

    {
        float3 c = {0.0, 0.5, 0.9};
        float2 p = {0, -0.15};
        float2 s = {0.1, 0.035};

        if(selected_menu_item == 1)
            memcpy(c, sc, sizeof(c));

        draw_ui(ui_shader, textures[2], ui_sampler, c, p, s, 0, 1);
    }

    {
        float3 c = {0.0, 0.5, 0.9};
        float2 p = {0, -0.3};
        float2 s = {0.1, 0.035};

        if(selected_menu_item == 2)
            memcpy(c, sc, sizeof(c));

        draw_ui(ui_shader, textures[8], ui_sampler, c, p, s, 0, 1);
    }

    {
        float3 c = {0.0, 0.5, 0.9};
        float2 p = {0, -0.45};
        float2 s = {0.1, 0.035};

        if(selected_menu_item == 3)
            memcpy(c, sc, sizeof(c));

        draw_ui(ui_shader, textures[3], ui_sampler, c, p, s, 0, 1);
    }

    {
        float3 c = {0.0, 0.5, 0.9};
        float2 p = {0.8, -0.8};
        float2 s = {0.06, 0.01};

        draw_ui(ui_shader, textures[14], ui_sampler, c, p, s, 0, 1);
    }

    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);

    glUseProgram(0);
}

void display_pausemenu()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, screen_width, screen_height);

    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(ui_shader);

    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    float3 sc = {0, 0.9, 0.9};

    {
        float3 c = {0.0, 0.5, 0.9};
        float2 p = {0, 0.0};
        float2 s = {0.15, 0.035};

        if(selected_menu_item == 0)
            memcpy(c, sc, sizeof(c));

        draw_ui(ui_shader, textures[4], ui_sampler, c, p, s, 0, 1);
    }

    {
        float3 c = {0.0, 0.5, 0.9};
        float2 p = {0, -0.15};
        float2 s = {0.1, 0.035};

        if(selected_menu_item == 1)
            memcpy(c, sc, sizeof(c));

        draw_ui(ui_shader, textures[2], ui_sampler, c, p, s, 0, 1);
    }

    {
        float3 c = {0.0, 0.5, 0.9};
        float2 p = {0, -0.3};
        float2 s = {0.2, 0.035};

        if(selected_menu_item == 2)
            memcpy(c, sc, sizeof(c));

        draw_ui(ui_shader, textures[5], ui_sampler, c, p, s, 0, 1);
    }

    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);

    glUseProgram(0);
}

void
display_aboutmenu()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, screen_width, screen_height);

    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(ui_shader);

    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    {
        float3 c = {0.0, 0.5, 0.9};
        float2 p = {0, 0.4};
        float2 s = {0.15, 0.035};

        draw_ui(ui_shader, textures[9], ui_sampler, c, p, s, 0, 1);
    }

    {
        float3 c = {0.0, 0.5, 0.9};
        float2 p = {0, 0.0};
        float2 s = {0.3, 0.06};

        draw_ui(ui_shader, textures[10], ui_sampler, c, p, s, 0, 1);
    }

    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);

    glUseProgram(0);
}

void display_settingsmenu()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, screen_width, screen_height);

    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(ui_shader);

    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    float3 sc = {0, 0.9, 0.9};

    {
        float3 c = {0.0, 0.5, 0.9};
        float2 p = {0, 0.0};
        float2 s = {0.1, 0.035};

        if(selected_menu_item == 0)
            memcpy(c, sc, sizeof(c));

        GLuint tex = textures[11];

        if(sound_off == 1)
        {
            tex = textures[12];
            s[0] = 0.105;
        }

        draw_ui(ui_shader, tex, ui_sampler, c, p, s, 0, 1);
    }

    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);

    glUseProgram(0);
}

static void
process_collision()
{
    // bullets and ships
    for(int i = 0; i < ships_count; i++)
        for(int j = 0; j < bullets_count; j++)
            if(bullets[j].dead == 1)
                if(overlaps(&ships[i].bbox, &bullets[j].bbox)
#ifdef ENEMY_FRIENDLY_FIRE_OFF
                   && ((bullets[j].shooter == PLAYER_SHIP) || (ships[i].type == PLAYER_SHIP))
#endif
                   )
                {
                    bullets[j].dead = 0;

                    if(ships[i].active)
                    {
                        ships[i].on_hit(&ships[i], &bullets[j]);
                        ships[i].hits++;

                        if(ships[i].shield.active)
                        {
                            if(ships[i].shield.value >= 0)
                                ships[i].shield.value -= ships[i].weapon.damage;
                            else
                                ships[i].health -= ships[i].weapon.damage;
                        }
                        else
                            ships[i].health -= ships[i].weapon.damage;
                    }
                }

    for(int i = 0; i < ships_count; i++)
        for(int j = 0; j < bonuses_count; j++)
            if(bonuses[j].dead == 0 && ships[i].dead == 0)
                if(overlaps(&ships[i].bbox, &bonuses[j].bbox))
                {
                    if(ships[i].type == PLAYER_SHIP)
                    {
                        bonuses[j].dead = 1;
                        play_sound(COIN_SOUND, 0);

                        switch(bonuses[j].type)
                        {
                        case HEALTH_BONUS:
                            player_ship->health += HEALTH_BONUS_VALUE;
                            break;
                        case ARMOR_BONUS:
                            player_ship->shield.value += ARMOR_BONUS_VALUE;
                            break;
                        case WEAPON_BONUS:
                            break;
                        }
                    }
                }

    for(int j = 0; j < bullets_count; j++)
        if((bullets[j].dead == 1) && (boss.dead == 0))
            if(overlaps(&boss.bbox, &bullets[j].bbox))
            {
                bullets[j].dead = 0;
            }
}

static void
random_movement_callback(struct TIMER *self, void *data)
{
    struct SHIP *ship = (struct SHIP *)(data);

    if(!ship->active)
        return;

    int d = rand() % 2;
    int b = rand() % 2;

    if(d == 0)
        ship->w = 0.005;

    if(d == 1)
        ship->w = -0.005;

    if((b == 0) && fabs(ship->x - player_ship->x) > 2)
        ship->v = SCROLL_SPEED + 0.005;

    if((b == 1) && fabs(ship->x - player_ship->x) < 4)
        ship->v = SCROLL_SPEED - 0.005;

    if(fabs(ship->x - player_ship->x) < 1)
        ship->v = SCROLL_SPEED + 0.005;

    if(fabs(ship->x - player_ship->x) > 5)
        ship->v = SCROLL_SPEED - 0.005;
}

static void
random_warp_movement_callback(struct TIMER *self, void *data)
{
    struct SHIP *ship = (struct SHIP *)(data);

    if(!ship->active)
        return;

    //if(player_ship)
    {
        warp_effect(0, 0.7, ship->x, ship->y, 0);
        play_sound(WARP_SOUND, 0);

        float x = player_ship->x + randf2(2.0, 5.0);
        float y = randf2(-1.5, 1.5);

        ship->x = x;
        ship->y = y;

        //warp_effect(1, 0.5, ship->x, ship->y, 0);
    }
}

static void
shield_activation_callback(struct TIMER *self, void *data)
{
    struct SHIP *ship = (struct SHIP *)(data);

    if(ship->shield.active)
    {
        if(!ship->shield.steady)
            ship->shield.tv += 0.25;

        if(ship->shield.tv > ship->shield.max_time)
        {
            delete_timer(ship->t2);

            ship->shield.active = 0;
            ship->shield.tv = 0;
            ship->t2 = 0;
        }
    }
}

static void
process_ai()
{
    for(int i = 0; i < ships_count; i++)
    {
        if((fabs(ships[i].x - player_ship->x) < ships[i].active_distance) && (ships[i].active == 0))
        {
            ships[i].v = SCROLL_SPEED;
            ships[i].active = 1;
            ships[i].state = WAIT_STATE;
        }

        if(ships[i].type == ENEMY1_SHIP)
        {
            if(ships[i].active)
            {
                /*
                 *       []
                 *        \
                 *         \
                 *     angle\
                 *  <-------[]
                 */
                float v0 = ships[i].x - player_ship->x;
                float v1 = ships[i].y - player_ship->y;
                float v2 = ships[i].z - player_ship->z;

                float w0 = ships[i].x;
                float w1 = 0;
                float w2 = 0;

                float angle = v2angle(w0, w1, w2, v0, v1, v2) * 180 / M_PI;

                if(angle < 3 && ships[i].active)
                    ships[i].state = ATTACK_STATE;
                else
                   ships[i].state = WAIT_STATE;

                if((ships[i].hits > 0) && (ships[i].t1 == 0))
                    ships[i].t1 = new_timer(INFINITE_TIMER, 2.0, random_movement_callback, (void*)&ships[i]);
            }
        }

        if(ships[i].type == ENEMY2_SHIP)
        {
            float v0 = ships[i].x - player_ship->x;
            float v1 = ships[i].y - player_ship->y;
            float v2 = ships[i].z - player_ship->z;

            float w0 = ships[i].x;
            float w1 = 0;
            float w2 = 0;

            float angle = v2angle(w0, w1, w2, v0, v1, v2) * 180 / M_PI;

            if(angle < 3 && ships[i].active)
                ships[i].state = ATTACK_STATE;
            else
                ships[i].state = WAIT_STATE;

            if(ships[i].hits > 0)
            {
                if(ships[i].t1 == 0)
                    ships[i].t1 = new_timer(INFINITE_TIMER, 2.0, random_movement_callback, (void*)&ships[i]);

                if(ships[i].t2 == 0)
                {
                    /*ships[i].shield.active = 1;
                    ships[i].shield.value = 30;
                    ships[i].shield.max_value = 30;
                    ships[i].shield.max_time = 1.0;
                    ships[i].shield.steady = 1;
                    ships[i].t2 = new_timer(ONCE_TIMER, 0.25, shield_activation_callback, (void*)&ships[i]);*/
                }
            }
        }
        if(ships[i].type == ENEMY3_SHIP)
        {
            float v0 = ships[i].x - player_ship->x;
            float v1 = ships[i].y - player_ship->y;
            float v2 = ships[i].z - player_ship->z;

            float w0 = ships[i].x;
            float w1 = 0;
            float w2 = 0;

            float angle = v2angle(w0, w1, w2, v0, v1, v2) * 180 / M_PI;

            if(angle < 3 && ships[i].active)
                ships[i].state = ATTACK_STATE;
            else
                ships[i].state = WAIT_STATE;

            if((ships[i].hits > 0) && (ships[i].t1 == 0))
                ships[i].t1 = new_timer(INFINITE_TIMER, 2.0, random_movement_callback, (void*)&ships[i]);
        }
        if(ships[i].type == ENEMY4_SHIP)
        {
            float v0 = ships[i].x - player_ship->x;
            float v1 = ships[i].y - player_ship->y;
            float v2 = ships[i].z - player_ship->z;

            float w0 = ships[i].x;
            float w1 = 0;
            float w2 = 0;

            float angle = v2angle(w0, w1, w2, v0, v1, v2) * 180 / M_PI;

            if(angle < 5 && ships[i].active)
                ships[i].state = ATTACK_STATE;
            else
                ships[i].state = WAIT_STATE;

            if((ships[i].hits > 0) && (ships[i].t1 == 0))
                ships[i].t1 = new_timer(INFINITE_TIMER, 2.0, random_movement_callback, (void*)&ships[i]);

            if((ships[i].hits > 0) && (ships[i].t3 == 0))
                ships[i].t3 = new_timer(INFINITE_TIMER, 3.0, random_warp_movement_callback, (void*)&ships[i]);
        }
        if(ships[i].type == ENEMY5_SHIP)
        {
            float v0 = ships[i].x - player_ship->x;
            float v1 = ships[i].y - player_ship->y;
            float v2 = ships[i].z - player_ship->z;

            float w0 = ships[i].x;
            float w1 = 0;
            float w2 = 0;

            float angle = v2angle(w0, w1, w2, v0, v1, v2) * 180 / M_PI;

            if(angle < 5 && ships[i].active)
                ships[i].state = ATTACK_STATE;
            else
                ships[i].state = WAIT_STATE;
        }
    }
}

static void
update_ship_state(struct SHIP *ship)
{
    switch(ship->state)
    {
    case WAIT_STATE:
        break;
    case ATTACK_STATE:

        if(ship->weapon.delay == 0)
        {
            switch(ship->type)
            {
            case PLAYER_SHIP:
                new_bullet(&ship->weapon, ship->x + 0.35, ship->y, 0, 1, 0, ship->type);
                play_weapon_sound(player_ship->weapon.type);
                break;
            case ENEMY1_SHIP:
            case ENEMY2_SHIP:
                new_bullet(&ship->weapon, ship->x - 0.35, ship->y, 0, -1, 0, ship->type);
                play_weapon_sound(ship->weapon.type);
                break;
            case ENEMY3_SHIP:
                new_bullet(&ship->weapon, ship->x - 0.35, ship->y + 0.04, 0, -1, 0, ship->type);
                new_bullet(&ship->weapon, ship->x - 0.35, ship->y - 0.04, 0, -1, 0, ship->type);
                play_weapon_sound(ship->weapon.type);
                break;
            case ENEMY4_SHIP:
                new_bullet(&ship->weapon, ship->x - 0.35, ship->y + 0.04, 0, -1, 0.01, ship->type);
                new_bullet(&ship->weapon, ship->x - 0.35, ship->y - 0.04, 0, -1, -0.01, ship->type);
                play_weapon_sound(ship->weapon.type);
                break;
            case ENEMY5_SHIP:
                new_bullet(&ship->weapon, ship->x - 0.35, ship->y, 0, -1, 0, ship->type);
                play_weapon_sound(ship->weapon.type);
                break;
            }
        }

        break;
    }
}

static void
process_objects()
{
    static float angle = 0;
    angle += 0.05f;

    if(game_over == 1)
        return;

    const GLfloat identitymatrix[16] = IDENTITY_MATRIX4;

    for(int i = 0; i < MAX_SHIPS; i++)
    {
        memcpy(ships[i].transform, identitymatrix, sizeof(identitymatrix));

        // update transformation
        scale(ships[i].transform, ships[i].l, ships[i].h, ships[i].b);
        rotate(ships[i].transform, ships[i].q + angle * 50, X_AXIS);
        rotate(ships[i].transform, ships[i].d, Y_AXIS);
        rotate(ships[i].transform, ships[i].p, Z_AXIS);
        translate(ships[i].transform, ships[i].x, ships[i].y, ships[i].z);

        // update position
        ships[i].y += ships[i].w * interpolation;
        ships[i].x += ships[i].v * interpolation;

        // update AABB
        ships[i].bbox.min[0] = ships[i].x - ships[i].l;
        ships[i].bbox.min[1] = ships[i].y - ships[i].h;
        ships[i].bbox.min[2] = ships[i].z - ships[i].b;

        ships[i].bbox.max[0] = ships[i].x + ships[i].l;
        ships[i].bbox.max[1] = ships[i].y + ships[i].h;
        ships[i].bbox.max[2] = ships[i].z + ships[i].b;

        if(ships[i].y > WALL_Y_MAX)
            ships[i].y = WALL_Y_MAX;

        if(ships[i].y < WALL_Y_MIN)
            ships[i].y = WALL_Y_MIN;

        //float3 max_color = {MAX_HEALTH_COLOR};
        float3 half_color = {HALF_HEALTH_COLOR};
        float3 min_color = {MIN_HEALTH_COLOR};

        switch(ships[i].type)
        {
        case PLAYER_SHIP:
            if(ships[i].health < PLAYER_MAX_HP / 2)
                memcpy(&ships[i].color, half_color, sizeof(ships[i].color));
            if(ships[i].health < PLAYER_MAX_HP / 4)
                memcpy(&ships[i].color, min_color, sizeof(ships[i].color));
            break;
        case ENEMY1_SHIP:
            if(ships[i].health < ENEMY1_MAX_HP / 2)
                memcpy(&ships[i].color, half_color, sizeof(ships[i].color));
            if(ships[i].health < ENEMY1_MAX_HP / 4)
                memcpy(&ships[i].color, min_color, sizeof(ships[i].color));
            break;
        case ENEMY2_SHIP:
            if(ships[i].health < ENEMY2_MAX_HP / 2)
                memcpy(&ships[i].color, half_color, sizeof(ships[i].color));
            if(ships[i].health < ENEMY2_MAX_HP / 4)
                memcpy(&ships[i].color, min_color, sizeof(ships[i].color));
            break;
        case ENEMY3_SHIP:
            if(ships[i].health < ENEMY3_MAX_HP / 2)
                memcpy(&ships[i].color, half_color, sizeof(ships[i].color));
            if(ships[i].health < ENEMY3_MAX_HP / 4)
                memcpy(&ships[i].color, min_color, sizeof(ships[i].color));
            break;
        case ENEMY4_SHIP:
            if(ships[i].health < ENEMY4_MAX_HP / 2)
                memcpy(&ships[i].color, half_color, sizeof(ships[i].color));
            if(ships[i].health < ENEMY4_MAX_HP / 4)
                memcpy(&ships[i].color, min_color, sizeof(ships[i].color));
            break;
        case ENEMY5_SHIP:
            if(ships[i].health < ENEMY5_MAX_HP / 2)
                memcpy(&ships[i].color, half_color, sizeof(ships[i].color));
            if(ships[i].health < ENEMY5_MAX_HP / 4)
                memcpy(&ships[i].color, min_color, sizeof(ships[i].color));
            break;
        }

        update_ship_state(&ships[i]);

        if(ships[i].health < 0)
        {
            ships[i].dead = 1;

            switch(ships[i].type)
            {
            case PLAYER_SHIP:
                game_over = 1;
                break;
            case ENEMY1_SHIP:
                score += 10;
                break;
            case ENEMY2_SHIP:
                score += 30;
                break;
            case ENEMY3_SHIP:
                score += 20;
                break;
            case ENEMY4_SHIP:
                score += 40;
                break;
            case ENEMY5_SHIP:
                score += 50;
                break;
            }


            explosion_effect(1, 1.0, ships[i].x, ships[i].y, 0);

            play_sound(EXPLOSION1_SOUND, 0);
            play_sound(EXPLOSION2_SOUND, 0);

            int r = rand() % 3;

            if((r == 1) && (ships[i].type != PLAYER_SHIP))
                new_bonus(rand() % 2, ships[i].x, ships[i].y, 0);

            enemies_count--;
            delete_ship(ships[i].id);
        }

        if(fabs(ships[i].x - camera.x > 8.0))
        {
            enemies_count--;
            delete_ship(ships[i].id);
        }
    }

    for(int i = 0; i < MAX_BULLETS; i++)
    {
        memcpy(bullets[i].transform, identitymatrix, sizeof(identitymatrix));

        // update transformation
        scale(bullets[i].transform, bullets[i].l, bullets[i].h, bullets[i].b);
        translate(bullets[i].transform, bullets[i].x, bullets[i].y, bullets[i].z);

        // update position
        bullets[i].x += bullets[i].v * interpolation;
        bullets[i].y += bullets[i].w * interpolation;

        // update AABB
        bullets[i].bbox.min[0] = bullets[i].x - bullets[i].l;
        bullets[i].bbox.min[1] = bullets[i].y - bullets[i].h;
        bullets[i].bbox.min[2] = bullets[i].z - bullets[i].b;

        bullets[i].bbox.max[0] = bullets[i].x + bullets[i].l;
        bullets[i].bbox.max[1] = bullets[i].y + bullets[i].h;
        bullets[i].bbox.max[2] = bullets[i].z + bullets[i].b;
    }

    if(boss.dead == 0)
    {
        memcpy(boss.transform, identitymatrix, sizeof(identitymatrix));

        // update transformation
        scale(boss.transform, boss.l, boss.h, boss.b);
        translate(boss.transform, boss.x, boss.y, boss.z);

        // update AABB
        boss.bbox.min[0] = boss.x - boss.l;
        boss.bbox.min[1] = boss.y - boss.h;
        boss.bbox.min[2] = boss.z - boss.b;

        boss.bbox.max[0] = boss.x + boss.l;
        boss.bbox.max[1] = boss.y + boss.h;
        boss.bbox.max[2] = boss.z + boss.b;

        // update position
        boss.y += boss.w * interpolation;
        boss.x += boss.v * interpolation;

        if(boss.y > WALL_Y_MAX)
            boss.y = WALL_Y_MAX;

        if(boss.y < WALL_Y_MIN)
            boss.y = WALL_Y_MIN;
    }

    // update bonuses
    for(int i = 0; i < bonuses_count; i++)
    {
        memcpy(bonuses[i].transform, identitymatrix, sizeof(identitymatrix));

        // update transformation
        scale(bonuses[i].transform, bonuses[i].l, bonuses[i].h, bonuses[i].b);
        rotate(bonuses[i].transform, angle * 50.0, X_AXIS);
        rotate(bonuses[i].transform, 0, Y_AXIS);
        rotate(bonuses[i].transform, angle * 100.0, Z_AXIS);
        translate(bonuses[i].transform, bonuses[i].x, bonuses[i].y, bonuses[i].z);
    }

    // update weapon
    for(int i = 0; i < ships_count; i++)
    {
        ships[i].weapon.delay--;

        if(ships[i].weapon.delay < 0)
            ships[i].weapon.delay = ships[i].weapon.max_delay;
    }

    // update timers
    for(int i = 0; i < timers_count; i++)
        switch(timers[i].type)
        {
        case ONCE_TIMER:
            if(timers[i].value >= timers[i].stop_value)
            {
                timers[i].value = 0;
                timers[i].on_time(&timers[i], timers[i].data);
                timers[i].type = -1;
            }
            break;
        case INFINITE_TIMER:
            if(timers[i].value >= timers[i].stop_value)
            {
                timers[i].value = 0;
                timers[i].on_time(&timers[i], timers[i].data);
            }
            break;
        }

    for(int i = 0; i < MAX_TIMERS; i++)
        if(timers[i].type == -1)
            delete_timer(i);

    // update_emmiters
    for(int i = 0; i < MAX_EMMITERS; i++)
    {
        memcpy(emmiters[i].transform, identitymatrix, sizeof(identitymatrix));
        translate(emmiters[i].transform, emmiters[i].x - camera.x, emmiters[i].y - camera.y, emmiters[i].z - camera.z);

        emmiters[i].x += emmiters[i].v * interpolation;
        emmiters[i].y += emmiters[i].w * interpolation;

        if(emmiters[i].active)
        {
            if(emmiters[i].tv >= emmiters[i].tl)
                delete_emmiter(i);
        }
    }

    update_stars();
    update_nebulas();

    camera.x += SCROLL_SPEED * interpolation;
}

static void
process_events(const struct window_event *event)
{
    if(event->type == MOUSE_MOVE)
    {
        cursor_x = event->motion.x;
        cursor_y = event->motion.y;
    }

    if(event->type == KEYBOARD_KEY_DOWN)
    {
        if(event->key_down.keys_state[KEY_UP])
        {
            player_ship->w = PLAYER_SHIP_SPEED;
        }

        if(event->key_down.keys_state[KEY_DOWN])
        {
            player_ship->w = -PLAYER_SHIP_SPEED;
        }

        if(event->key_down.keys_state[KEY_SPACE])
        {
            // shot
            if(player_ship->weapon.delay == 0)
            {
                //new_bullet(&player_ship->weapon, player_ship->x + 0.35, player_ship->y, 0, 1, 0, PLAYER_SHIP);
                //play_weapon_sound(player_ship->weapon.type);

                player_ship->state = ATTACK_STATE;
            }
        }
        if(event->key_down.keys_state[KEY_1])
        {
            player_ship->weapon.type = GUN_BULLET;
            player_ship->weapon.damage = GUN_DAMAGE;
            player_ship->weapon.max_delay = GUN_DELAY;
        }
        if(event->key_down.keys_state[KEY_2])
        {
            player_ship->weapon.type = ROCKET_BULLET;
            player_ship->weapon.damage = ROCKET_DAMAGE;
            player_ship->weapon.max_delay = ROCKET_DELAY;
        }
        if(event->key_down.keys_state[KEY_3])
        {
            player_ship->weapon.type = PLASMA_BULLET;
            player_ship->weapon.damage = PLASMA_DAMAGE;
            player_ship->weapon.max_delay = PLASMA_DELAY;
        }

        if(event->key_down.keys_state[KEY_S])
        {
            if(player_ship->t2 == 0)
            {
                player_ship->shield.active = 1;
                player_ship->shield.value = SHIELD_VALUE;
                player_ship->shield.max_value = SHIELD_VALUE;
                player_ship->shield.max_time = SHIELD_ACTIVE_TIME;
                player_ship->t2 = new_timer(INFINITE_TIMER, 0.25, shield_activation_callback, player_ship);
            }
        }

        if(event->key_down.keys_state[KEY_E])
        {
            struct PARTICLE_ATTRIBUTES pa = {};

            pa.size = 4.0;

            pa.position[0][0] = 0.5;
            pa.position[0][1] = 0.5;
            pa.position[0][2] = 0.5;

            pa.velocity[0][0] = -0.05;
            pa.velocity[0][1] = -0.05;
            //pa.velocity[1][0] = 0.005;
           // pa.velocity[1][1] = 0.005;

            pa.color[0][0] = 0;
            pa.color[0][1] = 1;
            pa.color[0][2] = 1;
            pa.color[1][0] = 1;
            pa.color[1][1] = 0;
            pa.color[1][2] = 1;

            int id = new_emmiter(500);
            set_emmiter_position(id, camera.x, camera.y, 0);
            set_emmiter_velocity(id, SCROLL_SPEED, 0);
            set_emmiter_attributes(id, &pa, 0.3, PA_BASE_VELOCITY | PA_RAND_COLOR | PA_RAND_BASE_POSITION);

            //explosion_effect(1, 1.0, camera.x, camera.y, 0);
            //new_bonus(0, camera.x, camera.y, 0);
        }

        if(event->key_down.keys_state[KEY_ESCAPE])
        {
            struct GAME_STATE *state = top_stack(stack_state);

            if(state != &game_states[2])
                push_stack(stack_state, &game_states[2]);
        }
    }

    if(event->type == KEYBOARD_KEY_UP)
    {
        if(event->key_up.keys_state[KEY_SPACE] == 0)
        {
            player_ship->state = WAIT_STATE;
        }

        if(event->key_up.keys_state[KEY_UP] == 0)
        {
            player_ship->w = 0;
        }

        if(event->key_up.keys_state[KEY_DOWN] == 0)
        {
            player_ship->w = 0;
        }
        if(event->key_up.keys_state[KEY_SPACE] == 0)
        {
            // shot out
        }
        if(event->type == KEYBOARD_KEY_UP)
        {
            if(event->key_up.keys_state[KEY_ESCAPE] == 0)
            {

            }
        }
    }
}

void
update_timers(float tv)
{
    //printf("tv %f\n", tv);
    for(int i = 0; i < timers_count; i++)
        if(timers[i].type != -1)
            timers[i].value += tv;
}

void
update_emmiters(float tv)
{
    for(int i = 0; i < MAX_EMMITERS; i++)
        if(emmiters[i].active)
            emmiters[i].tv += tv;
}

void
process_game(const struct window_event *event)
{
    process_events(event);
    process_collision();
    process_ai();
    process_objects();
}

void
process_mainmenu(const struct window_event *event)
{
    const uint8_t *keymap = input_get_keymap();

    static int modif = 1;
    const int max_menu_items = 3;

    if(event->type == KEYBOARD_KEY_DOWN)
    {
        if(keymap[KEY_ENTER] && modif)
        {
            switch(selected_menu_item)
            {
            case 0:
                push_stack(stack_state, &game_states[1]);
                restart_game();
                break;
            case 1:
                push_stack(stack_state, &game_states[4]);
                break;
            case 2:
                push_stack(stack_state, &game_states[3]);
                break;
            case 3:
                shutdown_game();
                exit(0);
                break;
            }
        }

        if(keymap[KEY_UP] && modif)
        {
            if(selected_menu_item > 0)
                selected_menu_item--;

            play_sound(SELECT_SOUND, 0);
        }

        if(keymap[KEY_DOWN] && modif)
        {
            if(selected_menu_item < max_menu_items)
                selected_menu_item++;

            play_sound(SELECT_SOUND, 0);
        }

        modif = 0;
    }

    if(event->type == KEYBOARD_KEY_UP)
    {
        if(keymap[KEY_UP] == 0)
        {
        }

        if(keymap[KEY_DOWN] == 0)
        {

        }

        modif = 1;
    }
}
void
process_pausemenu(const struct window_event *event)
{
    const uint8_t *keymap = input_get_keymap();

    static int modif = 1;
    const int max_menu_items = 2;

    if(event->type == KEYBOARD_KEY_DOWN)
    {
        if(keymap[KEY_ENTER] && modif)
        {
            switch(selected_menu_item)
            {
            case 0:
                pop_stack(stack_state);
                selected_menu_item = 0;
                break;
            case 1:
                push_stack(stack_state, &game_states[4]);
                selected_menu_item = 0;
                break;
            case 2:
                pop_stack(stack_state);
                pop_stack(stack_state);
                selected_menu_item = 0;
                break;
            }
        }

        if(keymap[KEY_UP] && modif)
        {
            if(selected_menu_item > 0)
                selected_menu_item--;

            play_sound(SELECT_SOUND, 0);
        }

        if(keymap[KEY_DOWN] && modif)
        {
            if(selected_menu_item < max_menu_items)
                selected_menu_item++;

            play_sound(SELECT_SOUND, 0);
        }

        modif = 0;
    }


    if(event->type == KEYBOARD_KEY_UP)
    {
        if(keymap[KEY_ESCAPE] == 0)
        {

        }

        modif = 1;
    }
}

void
process_aboutmenu(const struct window_event *event)
{
    const uint8_t *keymap = input_get_keymap();

    static int modif = 1;

    if(event->type == KEYBOARD_KEY_DOWN)
    {
        if(keymap[KEY_ESCAPE] && modif)
        {
            pop_stack(stack_state);
            selected_menu_item = 0;
        }

        modif = 0;
    }

    if(event->type == KEYBOARD_KEY_UP)
    {
        modif = 1;
    }
}

void
process_settingsmenu(const struct window_event *event)
{
    const uint8_t *keymap = input_get_keymap();

    static int modif = 1;

    selected_menu_item = 0;

    if(event->type == KEYBOARD_KEY_DOWN)
    {
        if(keymap[KEY_ESCAPE] && modif)
        {
            pop_stack(stack_state);
            selected_menu_item = 0;
        }

        if(keymap[KEY_ENTER] && modif)
        {
            sound_off = !sound_off;

            play_sound(SELECT_SOUND, 0);
        }

        modif = 0;
    }

    if(event->type == KEYBOARD_KEY_UP)
    {
        modif = 1;
    }
}

int
main(int argc, char* argv[])
{
    srand(time(0));

    atexit(opengl_cleanup);
    atexit(openal_cleanup);

    if(window_create(screen_width, screen_height, WF_FULLSCREEN | WF_HIDEN_CURSOR) == -1)
        return -1;

    window_get_size(&screen_width, &screen_height);

    if(opengl_init() == -1)
        return -1;

    if(openal_init() == -1)
        return -1;

    /*const char *version = glGetString(GL_VERSION);
    const char *renderer = glGetString(GL_RENDERER);
    const char *vendor = glGetString(GL_VENDOR);
    const char *extensions = glGetString(GL_EXTENSIONS);

    printf("game name: %s\ngame version: %s\n", GAME_NAME, GAME_VERSION);
    printf("opengl version: %s\nopengl renderer: %s\nopengl vendor: %s\n", version, renderer, vendor);*/

    if(startup_game() != 0)
    {
        fprintf(stderr, "%s\n", "Can\'t start game");
        return 0;
    }

    struct window_event event;

    int64_t next_time = time_get();
    int64_t last_time = next_time;
    int64_t t = 0;

    const int TICKS_PER_SECOND = 30;
    const int SKIP_TICKS =  time_frequency() / TICKS_PER_SECOND;
    const int MAX_FRAMESKIP = 5;

    int loops;

    while(window_poll(&event))
    {
        loops = 0;
        t = time_get();

        double tv = (double) (t - last_time) / time_frequency();

        update_timers(tv);
        update_emmiters(tv);

        while(time_get() > next_time && loops < MAX_FRAMESKIP)
        {
            ((struct GAME_STATE*)top_stack(stack_state))->process_state(&event);

            next_time += SKIP_TICKS;
            loops++;
        }

        interpolation = (float)(time_get() + SKIP_TICKS - next_time) / (float)(SKIP_TICKS);
        interpolation = 1;
        last_time = time_get();

        ((struct GAME_STATE*)top_stack(stack_state))->display_state();

        window_process();
    }

    shutdown_game();

    return 0;
}
