#pragma once

#define _USE_MATH_DEFINES

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include <math.h>

#include <native/native.h>
#include <gl3/gl3platform.h>
#include <al/al.h>

#include "utils.h"
#include "stack.h"

#define GAME_NAME           "Shmup"
#define GAME_VERSION        "1.0.1"

#define MAX_SHIPS           50
#define MAX_ENEMIES         100
#define MAX_BULLETS         100
#define MAX_STARS           3000
#define MAX_TIMERS          100
#define MAX_BONUSES         20
#define MAX_EMMITERS        50
#define MAX_PARTICLES       2000
#define MAX_NEBULAS         20
#define MAX_SOUND_SOURCES   10

#define ENEMY_FRIENDLY_FIRE_OFF
#define RANDOM_COLOR_STARS_OFF

#define SCROLL_SPEED        0.015
#define BACKGROUND_COLOR    0, 0, 0, 0
#define PLAYER_SHIP_COLOR   0, 1, 0
#define PLAYER_SHIP_SPEED   0.02
//#define PLAYER_SHIP_SIZE    0.1, 0.05, 0.025
#define PLAYER_ROTATION     90, 0, 180
#define PLAYER_SHIP_SIZE    0.08, 0.08, 0.08
#define PLAYER_MAX_HP       300
#define ENEMY1_SHIP_SIZE    0.06, 0.06, 0.06
#define ENEMY1_SHIP_COLOR   0, 1, 0
#define ENEMY2_SHIP_SIZE    0.12, 0.06, 0.025
#define ENEMY2_SHIP_COLOR   0, 0.8, 0
#define ENEMY1_MAX_HP       30
#define ENEMY2_MAX_HP       100
#define ENEMY3_MAX_HP       40
#define ENEMY4_MAX_HP       60
#define ENEMY5_MAX_HP       80
#define BULLET1_COLOR       0.0, 1.0, 1.0
#define BULLET2_COLOR       0.5, 0, 0.5
#define BULLET3_COLOR       1, 1, 0.5
#define BULLET1_SIZE        0.01, 0.01, 0.01
#define BULLET2_SIZE        0.05, 0.01, 0.01
#define BULLET3_SIZE        0.015, 0.015, 0.015
#define BULLET1_SPEED       0.1
#define WALL_Y_MAX          1.5
#define WALL_Y_MIN          -1.5
#define BONUS_SIZE          0.04

#define STAR_MIN_SIZE       0.008
#define STAR_MAX_SIZE       0.015

#define GUN_DELAY           8
#define ROCKET_DELAY        30
#define PLASMA_DELAY        20
#define GUN_DAMAGE          10
#define ROCKET_DAMAGE       50
#define PLASMA_DAMAGE       20

#define SHIELD_VALUE        100
#define SHIELD_ACTIVE_TIME  10.0
#define HEALTH_BONUS_VALUE  25
#define ARMOR_BONUS_VALUE   100

#define MIN_NEBULA_SURFACES 3
#define MAX_NEBULA_SURFACES 10
#define NEBULA_MIN_SIZE 0.8
#define NEBULA_MAX_SIZE 3.0

#define MAX_HEALTH_COLOR    0, 1, 0
#define HALF_HEALTH_COLOR   1, 0.5, 0
#define MIN_HEALTH_COLOR    1, 0, 0

struct CAMERA;
struct WEAPON;
struct SHIELD;
struct SHIP;
struct BULLET;
struct BONUS;
struct STAR;
struct TIMER;

typedef void(*ship_collision_callback)(struct SHIP *self, struct SHIP *target);
typedef void(*bullet_collision_callback)(struct SHIP *self, struct BULLET *bullet);
typedef void(*timer_callback)(struct TIMER *self, void*);

enum SHIP_TYPE
{
    PLAYER_SHIP,
    ENEMY1_SHIP, // soldier
    ENEMY2_SHIP, // tank
    ENEMY3_SHIP, // soldier 2
    ENEMY4_SHIP, // ghost
    ENEMY5_SHIP, // destroyer
    MAX_SHIP_TYPE
};

enum BULLET_TYPE
{
    GUN_BULLET,
    ROCKET_BULLET,
    PLASMA_BULLET
};

/*enum WEPON_TYPE
{

};*/

enum TIMER_TYPE
{
    ONCE_TIMER,
    INFINITE_TIMER
};

enum BONUS_TYPE
{
    HEALTH_BONUS,
    ARMOR_BONUS,
    WEAPON_BONUS,
    MAX_TYPE_BONUS
};

enum AI_STATE
{
    WAIT_STATE,
    ATTACK_STATE
};

enum SOUNDS
{
    SELECT_SOUND,
    COIN_SOUND,
    SHOT1_SOUND,
    SHOT2_SOUND,
    SHOT3_SOUND,
    EXPLOSION1_SOUND,
    EXPLOSION2_SOUND,
    WARP_SOUND,
    MAX_SOUND
};

struct GAME_STATE
{
    void (*process_state)(const struct window_event *event);
    void (*display_state)();
};

struct CAMERA
{
    float           x, y, z; // position
    float           a, b, c; // rotation
};

struct WEAPON
{
    int             type;
    int             delay;
    int             max_delay;
    int             damage;
};

struct SHIELD
{
    int             active;
    int             steady;
    int             value;
    int             max_value;
    float           max_time;
    float           tv;
};

struct SHIP
{
    float           x, y, z; // position
    float           v, w;    // velocity
    float           l, h, b; // size
    float           q, d, p; // rotation

    int             id;
    int             type;

    float           transform[16];

    float3          color;

    struct AABB     bbox;

    struct WEAPON   weapon;
    struct SHIELD   shield;

    int             state;
    int             active;
    int             hits;
    int             health;
    int             dead;

    float           active_distance;

    int             t1;     // ai movement callback
    int             t2;     // shield activation
    int             t3;     // ai movement callback

    ship_collision_callback     on_collision;
    bullet_collision_callback   on_hit;

    // when ship hit, it changle color as hp bar
};

struct BULLET
{
    float           x, y, z; // position
    float           v, w;    // velocity
    float           l, h, b; // size

    int             type;
    int             dead;
    int             shooter;

    float           transform[16];

    struct AABB     bbox;
};

struct BONUS
{
    float           x, y, z; // position
    float           l, h, b; // size

    int             type;
    int             dead;

    float           transform[16];

    struct AABB     bbox;
};

struct BOSS_PART
{
    float           x, y, z; // position
    float           l, h, b; // size

    float           transform[16];

    struct AABB     bbox;
};

struct BOSS
{
    float           x, y, z; // position
    float           v, w;    // velocity
    float           l, h, b; // size

    int             type;
    int             dead;
    int             health;

    float           transform[16];

    struct AABB     bbox;

    struct BOSS_PART parts[10];
};

struct STAR
{
    float           x, y, z; // position
    float           size;    // radius

    float3          color;

    float           transform[16];
};


struct NEBULA_SURFACE
{
    float           x, y, z;     // offset
    float           q, d, p;  // rotation

    float           size;

    float3          color;

    float           transform[16];
};

struct NEBULA
{
    float           x, y, z; // position
    float           v, w;    // velocity
    float           size;


    int             surfaces_count;
    struct NEBULA_SURFACE surfaces[MAX_NEBULA_SURFACES];
};

struct TIMER
{
    float           value;
    float           stop_value;

    int             id;
    int             type;

    void            *data;

    timer_callback  on_time;
};

struct PARTICLE
{
    float           x, y, z; // position
    float           v, w;    // velocity
};

#define PA_RAND_BETWEEN_POSITION    0x0001
#define PA_RAND_BETWEEN_VELOCITY    0x0002
#define PA_RAND_COLOR               0x0004
#define PA_RAND_BASE_POSITION       0x0008
#define PA_BASE_VELOCITY            0x0010

struct PARTICLE_ATTRIBUTES
{
    float3          color[2];
    float3          position[2];
    float2          velocity[2];
    float3          gravity;
    float           size;
};

struct EMMITER
{
    float           x, y, z; // position
    float           v, w;    // velocity

    float           tv;
    float           tl;

    int             active;
    void            *buffer;

    unsigned        flags;

    float3          color[2];

    float           transform[16];
};

int new_ship(int type, float px, float py, float pz, float sx, float sy, float sz);
void delete_ship(int index);

void new_bullet(struct WEAPON *weapon, float px, float py, float pz, float dx, float dy, int shooter);
void set_bullet_size(struct BULLET *bullet, float x, float y, float z);
