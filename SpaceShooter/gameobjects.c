#include <Allegro5/allegro_native_dialog.h>
#include <math.h>
#include "gameobjects.h"

#define MAX(a,b) (a > b ? a : b)
#define MIN(a,b) (a > b ? b : a)
#define FORCE 0.5
#define MAX_SPEED 20
#define DECAY 0.01

ALLEGRO_BITMAP *starImage = NULL;
ALLEGRO_COLOR colors[4] = {{1,1,1,1},{1,0.5,0.5,1},{0.5,1,0.5,1},{0.5,0.5,1,1}};
ALLEGRO_BITMAP *shipImages[2] = {NULL};

struct Sprite stars[MAX_STARS] = {{{0,0},{0,0},{1,1,1,1},NULL}};
struct SpaceShip player = {{0,0},{0,0},{0,0},0,0,{1,1,1,1},{NULL,NULL}};
struct Size ws;
struct Point pos = {0,0};

static float r(float max) {
    return (float)(rand() / ((float)RAND_MAX / max));
}

static bool loadImage(char name[50], ALLEGRO_BITMAP **img) {
    *img = al_load_bitmap(name);
    if (!*img) {
        al_show_native_message_box(NULL, "Error", "Error", "Failed to load required image", "OK", ALLEGRO_MESSAGEBOX_ERROR);
        return false;
    }
    return true;
}

bool loadImages(void) {
    if (!loadImage("star.png", &starImage)) {
        unloadImages();
        return false;
    }
    if (!loadImage("RocketOff.png", &shipImages[0]) || !loadImage("RocketOn.png", &shipImages[1])) {
        unloadImages();
        return false;
    }
    return true;
}

void unloadImages(void) {
    if (starImage) {
        al_destroy_bitmap(starImage);
    }
    for (int i = 0; i < 2; i++) {
        if (shipImages[i]) {
            al_destroy_bitmap(shipImages[i]);
        }
    }
}

static void fixPoint(void) {
    struct Point middle = {ws.w / 2, ws.h / 2};
    pos.x = MAX(middle.x, MIN(MAP_MAX - middle.x, pos.x));
    pos.y = MAX(middle.y, MIN(MAP_MAX - middle.y, pos.y));
}

void buildMap(struct Size winSize) {
    ws = winSize;
    int maxStars = MAX_STARS;
    for (int i = 0; i < maxStars; i++) {
        float s = r(0.4) + 0.1;
        float w = 20 * s;
        float h = 20 * s;
        float x = r(MAP_MAX - w);
        float y = r(MAP_MAX - h);
        ALLEGRO_COLOR color = colors[(int)round(r(4))];
        stars[i] = (struct Sprite){{x,y},{w,h},color,starImage};
    }
    { //player scope
        float w = 110;
        float h = 246;
        float x = r(MAP_MAX - (w * 2)) + w;
        float y = r(MAP_MAX - (h * 2)) + h;
        float ro = r(6.2);
        player = (struct SpaceShip){{x,y},{55,90},{0,0},ro,0,{1,1,1,1},{shipImages[0],shipImages[1]}};
        pos = (struct Point){x,y};
        fixPoint();
    }
}

void moveObjects(void) {
    player.a.x -= player.a.x * DECAY;
    player.a.y -= player.a.y * DECAY;
    if (player.a.x < 0.01 && player.a.x > -0.01) {
        player.a.x = 0;
    }
    if (player.a.y < 0.01 && player.a.y > -0.01) {
        player.a.y = 0;
    }
    ALLEGRO_KEYBOARD_STATE keyState;
    al_get_keyboard_state(&keyState);
    if (al_key_down(&keyState, ALLEGRO_KEY_UP)) {
//        player.a.x += sin(player.r) * FORCE;
//        player.a.y += -cos(player.r) * FORCE;
//        player.a.x = MAX(-MAX_SPEED, MIN(MAX_SPEED, player.a.x));
//        player.a.y = MAX(-MAX_SPEED, MIN(MAX_SPEED, player.a.y));
        struct Point force = {(sin(player.r) * FORCE) + player.a.x, (-cos(player.r) * FORCE) + player.a.y};
        struct Point maxForce = {sin(player.r) * MAX_SPEED, -cos(player.r) * MAX_SPEED};
        if (maxForce.x < 0 && player.a.x > maxForce.x) {
            player.a.x = MAX(maxForce.x, force.x);
        }
        else if (maxForce.x > 0 && player.a.x < maxForce.x) {
            player.a.x = MIN(maxForce.x, force.x);
        }
        if (maxForce.y < 0 && player.a.y > maxForce.y) {
            player.a.y = MAX(maxForce.y, force.y);
        }
        else if (maxForce.y > 0 && player.a.y < maxForce.y) {
            player.a.y = MIN(maxForce.y, force.y);
        }
        player.ii = true;
    }
    else {
        player.ii = false;
    }
    if (al_key_down(&keyState, ALLEGRO_KEY_LEFT)) {
        player.r -= 0.05;
    }
    else if (al_key_down(&keyState, ALLEGRO_KEY_RIGHT)) {
        player.r += 0.05;
    }
    if (player.r > 6.2) {
        player.r -= 6.2;
    }
    else if (player.r < 0) {
        player.r += 6.2;
    }
    player.p.x += player.a.x;
    player.p.y += player.a.y;
    pos = player.p;
    player.p.x = MAX(0, MIN(MAP_MAX, player.p.x));
    player.p.y = MAX(0, MIN(MAP_MAX, player.p.y));
    fixPoint();
}

void renderObjects(void) {
    struct Point offset = {(pos.x - (ws.w / 2)), (pos.y - (ws.h / 2))};
    struct Point beg = {(pos.x - (ws.w / 2)) - 200, (pos.y - (ws.h / 2)) - 200};
    struct Point end = {(pos.x + (ws.w / 2)) + 200, (pos.y + (ws.h / 2)) + 200};
    int maxStars = MAX_STARS;
    for (int i = 0; i < maxStars; i++) {
        struct Sprite s = stars[i];
        if (s.p.x > beg.x && s.p.x < end.x && s.p.y > beg.y && s.p.y < end.y) {
            al_draw_tinted_scaled_bitmap(s.i, s.t, 0, 0, 20, 20, s.p.x - offset.x, s.p.y - offset.y, s.s.w, s.s.h, 0);
        }
    }
    { //player scope
        struct SpaceShip s = player;
        if (s.p.x > beg.x && s.p.x < end.x && s.p.y > beg.y && s.p.y < end.y) {
            al_draw_tinted_rotated_bitmap(s.i[s.ii], s.t, s.c.x, s.c.y, s.p.x - offset.x, s.p.y - offset.y, s.r, 0);
        }
    }
}
