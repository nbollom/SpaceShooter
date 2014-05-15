#include <allegro5/allegro5.h>
#include <allegro5/allegro_image.h>

#ifndef SpaceShooter_gameobjects_h
#define SpaceShooter_gameobjects_h

#define MAP_MAX 9999
#define MAX_STARS (int)((float)MAP_MAX / 1.5)
#define MAX_PLANETS (MAP_MAX / 4) / 8

struct Point {
    float x;
    float y;
};

struct Size {
    float width;
    float height;
};

struct Sprite {
    struct Point pos;
    struct Size size;
    ALLEGRO_COLOR tint;
    ALLEGRO_BITMAP *image;
};

struct SpaceShip {
    struct Point pos;
    struct Point centre;
    struct Point accel;
    float rotation;
    bool imageIndex;
    ALLEGRO_COLOR tint;
    ALLEGRO_BITMAP *image[2];
    float shield;
    float shieldRecharge;
    float shotCooldown;
};

extern struct SpaceShip player;

bool loadImages(void);
void unloadImages(void);
void buildMap(struct Size winSize);
void moveObjects(void);
void renderObjects(void);

#endif
