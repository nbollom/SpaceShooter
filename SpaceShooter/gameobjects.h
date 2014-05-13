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
    float w;
    float h;
};

struct Sprite {
    struct Point p;
    struct Size s;
    ALLEGRO_COLOR t;
    ALLEGRO_BITMAP *i;
};

struct SpaceShip {
    struct Point p;
    struct Point c;
    struct Point a;
    float r;
    bool ii;
    ALLEGRO_COLOR t;
    ALLEGRO_BITMAP *i[2];
};

extern struct SpaceShip player;

bool loadImages(void);
void unloadImages(void);
void buildMap(struct Size winSize);
void moveObjects(void);
void renderObjects(void);

#endif
