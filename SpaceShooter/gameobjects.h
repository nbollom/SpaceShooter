#include <allegro5/allegro5.h>
#include <allegro5/allegro_image.h>
#include <math.h>

#ifndef SpaceShooter_gameobjects_h
#define SpaceShooter_gameobjects_h

#define MAP_MAX 9999

#ifdef _WIN32

static inline double round(double val) {
	return floor(val + 0.5);
}

#endif

struct Point {
    float x;
    float y;
};

static inline struct Point makePoint(float x, float y) {
    struct Point p;
    p.x = x;
    p.y = y;
    return p;
}

static inline struct Point addPoints(struct Point p1, struct Point p2) {
    struct Point p;
    p.x = p1.x + p2.x;
    p.y = p1.y + p2.y;
    return p;
}

static inline struct Point rotatePoint(struct Point point, float rotation) {
    struct Point new;
    float cosRotation = cosf(rotation);
    float sinRotation = sinf(rotation);
    new.x = (point.x * cosRotation) - (point.y * sinRotation);
    new.y = (point.x * sinRotation) + (point.y * cosRotation);
    return new;
}

static inline float distanceBetween(struct Point p1, struct Point p2) {
    return sqrtf(powf(p1.x - p2.x, 2) + powf(p1.y - p2.y, 2));
}

struct Size {
    float width;
    float height;
};

struct Star {
    struct Point pos;
    struct Size size;
    ALLEGRO_COLOR tint;
    ALLEGRO_BITMAP *image;
};

struct Turret {
    struct Point pos;
    struct Size size;
    float rotation;
    float rotationIncrement;
    ALLEGRO_BITMAP *image;
    float shotCooldown;
    bool dead;
};

struct SpaceShip {
    struct Point pos;
    struct Point centre;
    struct Point accel;
    float rotation;
    bool imageIndex;
    ALLEGRO_BITMAP *image[2];
    float shield;
    float health;
    int shieldRecharge;
    float shotCooldown;
};

struct Lazer {
    struct Point start;
    struct Point pos;
    struct Point accel;
    float distance;
    float rotation;
    bool players;
};

extern struct SpaceShip player;
extern bool showTextures;
extern bool gameOver;
extern bool win;

void toggleSound(void);
bool loadImages(void);
bool loadSounds(void);
void unloadImages(void);
void unloadSounds(void);
void buildMap(struct Size winSize);
void moveObjects(void);
void renderObjects(void);

#endif
