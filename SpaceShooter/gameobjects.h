#include <allegro5/allegro5.h>
#include <allegro5/allegro_image.h>

#ifndef SpaceShooter_gameobjects_h
#define SpaceShooter_gameobjects_h

#define MAP_MAX 9999

struct Point {
    float x;
    float y;
};

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
};

struct SpaceShip {
    struct Point pos;
    struct Point centre;
    struct Point accel;
    float rotation;
    bool imageIndex;
    ALLEGRO_BITMAP *image[2];
    float shield;
    float shieldRecharge;
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

bool loadImages(void);
void unloadImages(void);
void buildMap(struct Size winSize);
void moveObjects(void);
void renderObjects(void);

#endif
