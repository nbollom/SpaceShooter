#include <Allegro5/allegro_native_dialog.h>
#include <allegro5/allegro_primitives.h>
#include <math.h>
#include "gameobjects.h"

#ifdef _WIN32

static double round(double val) {
	return floor(val + 0.5);
}

#endif

#define MAX(a,b) (a > b ? a : b)
#define MIN(a,b) (a > b ? b : a)

#define MAX_STARS (int)((float)MAP_MAX / 1.2)
#define MAX_TURRETS (MAP_MAX / 100)
#define FORCE 0.5
#define MAX_SPEED 20
#define DECAY 0.01
#define MAX_SHIELDS 1
#define MAX_LAZERS 100
#define LAZER_FORCE 30
#define LAZER_MAX_DISTANCE 1000
#define MAX_RADUINS 6.283185
#define TURRET_SHOT_RATE 6

ALLEGRO_BITMAP *starImage = NULL;
ALLEGRO_COLOR colors[4] = {{1,1,1,1},{1,0.2,0.2,1},{0.2,1,0.2,1},{0.2,0.2,1,1}};
ALLEGRO_BITMAP *shipImages[2] = {NULL};
ALLEGRO_BITMAP *shieldImage = NULL;
ALLEGRO_BITMAP *lazerImage = NULL;
ALLEGRO_BITMAP *turretImage = NULL;

struct Star stars[MAX_STARS] = {{{0,0},{0,0},{1,1,1,1},NULL}};
struct Turret turrets[MAX_TURRETS];
struct SpaceShip player = {{0,0},{0,0},{0,0},0,0,{NULL,NULL}};
struct Size windowSize;
struct Point camera = {0,0};
struct Lazer lazers[MAX_LAZERS];
int lazerCount;

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
    if (!loadImage("shield.png", &shieldImage)) {
        unloadImages();
        return false;
    }
    if (!loadImage("lazer.png", &lazerImage)) {
        unloadImages();
        return false;
    }
    if (!loadImage("turret.png", &turretImage)) {
        unloadImages();
        return false;
    }
    return true;
}

void unloadImages(void) {
	int i;
    if (starImage) {
        al_destroy_bitmap(starImage);
    }
    for (i = 0; i < 2; i++) {
        if (shipImages[i]) {
            al_destroy_bitmap(shipImages[i]);
        }
    }
    if (shieldImage) {
        al_destroy_bitmap(shieldImage);
    }
    if (lazerImage) {
        al_destroy_bitmap(lazerImage);
    }
    if (turretImage) {
        al_destroy_bitmap(turretImage);
    }
}

static void fixCamera(void) {
    struct Point middle = {windowSize.width / 2, windowSize.height / 2};
    camera.x = MAX(middle.x, MIN(MAP_MAX - middle.x, camera.x));
    camera.y = MAX(middle.y, MIN(MAP_MAX - middle.y, camera.y));
}

void buildMap(struct Size winSize) {
    windowSize = winSize;
    int maxStars = MAX_STARS;
    for (int i = 0; i < maxStars; i++) {
        float s = r(0.4) + 0.1;
		stars[i].size.width = 20 * s;
        stars[i].size.height = 20 * s;
        stars[i].pos.x = r(MAP_MAX - stars[i].size.width);
        stars[i].pos.y = r(MAP_MAX - stars[i].size.height);
        stars[i].tint = colors[(int)round(r(4))];
        stars[i].image = starImage;
    }
    int maxTurrets = MAX_TURRETS;
    for (int i = 0; i < maxTurrets; i++) {
        turrets[i].size.width = 100;
        turrets[i].size.height = 100;
        turrets[i].pos.x = r(MAP_MAX - turrets[i].size.width);
        turrets[i].pos.y = r(MAP_MAX - turrets[i].size.height);
        turrets[i].image = turretImage;
        turrets[i].rotation = r(MAX_RADUINS);
        turrets[i].rotationIncrement = round(r(1.0)) ? -0.05 : 0.05;
        turrets[i].shotCooldown = TURRET_SHOT_RATE;
    }
    { //player scope
        float w = 110;
        float h = 246;
        player.pos.x = r(MAP_MAX - (w * 2)) + w;
        player.pos.y = r(MAP_MAX - (h * 2)) + h;
        player.rotation = r(MAX_RADUINS);
		player.centre.x = 55;
		player.centre.y = 95;
		player.image[0] = shipImages[0];
		player.image[1] = shipImages[1];
        player.shield = MAX_SHIELDS;
		camera = player.pos;
        fixCamera();
    }
}

void updateRemoveLazers() {
    int removeIndexes[MAX_LAZERS] = {0};
    int removeCount = 0;
    for (int i = 0; i < lazerCount; i++) {
        lazers[i].distance = sqrtf(powf(lazers[i].pos.x - lazers[i].start.x, 2) + powf(lazers[i].pos.y - lazers[i].start.y, 2));
        if (lazers[i].pos.x < 0 || lazers[i].pos.x > MAP_MAX || lazers[i].pos.y < 0 || lazers[i].pos.y > MAP_MAX || lazers[i].distance > LAZER_MAX_DISTANCE) {
            removeIndexes[removeCount] = i;
            removeCount++;
        }
        else {
            lazers[i].pos.x += lazers[i].accel.x;
            lazers[i].pos.y += lazers[i].accel.y;
        }
    }
    for (int i = removeCount; i > 0 ; i--) {
        int index = removeIndexes[i - 1];
        for (int j = index; j < lazerCount - 1; j++) {
            lazers[j] = lazers[j + 1];
        }
        lazerCount--;
    }
}

void updateTurrets() {
    int maxTurrets = MAX_TURRETS;
    for (int i = 0; i < maxTurrets; i++) {
        turrets[i].rotation += turrets[i].rotationIncrement;
        if (turrets[i].rotation > MAX_RADUINS) {
            turrets[i].rotation -= MAX_RADUINS;
        }
        if (turrets[i].rotation < 0) {
            turrets[i].rotation += MAX_RADUINS;
        }
        if (turrets[i].shotCooldown <= 0) {
            if (lazerCount < MAX_LAZERS && sqrtf(powf(turrets[i].pos.x - player.pos.x, 2) + powf(turrets[i].pos.y - player.pos.y, 2)) < 700) {
                struct Lazer lazer;
                lazer.rotation = -atan2f(turrets[i].pos.x - player.pos.x, turrets[i].pos.y - player.pos.y) + (r(0.2) - 0.1);
                lazer.start.x = lazer.pos.x = turrets[i].pos.x + (sin(lazer.rotation) * 30);
                lazer.start.y = lazer.pos.y = turrets[i].pos.y + (-cos(lazer.rotation) * 30);
                lazer.accel.x = (sin(lazer.rotation) * LAZER_FORCE);
                lazer.accel.y = (-cos(lazer.rotation) * LAZER_FORCE);
                lazer.players = false;
                lazer.distance = 0;
                lazers[lazerCount] = lazer;
                lazerCount++;
                turrets[i].shotCooldown = TURRET_SHOT_RATE;
            }
        }
        else {
            turrets[i].shotCooldown -= 0.2;
        }
    }
}

void moveObjects(void) {
	ALLEGRO_KEYBOARD_STATE keyState;
    player.accel.x -= player.accel.x * DECAY;
    player.accel.y -= player.accel.y * DECAY;
    if (player.accel.x < 0.01 && player.accel.x > -0.01) {
        player.accel.x = 0;
    }
    if (player.accel.y < 0.01 && player.accel.y > -0.01) {
        player.accel.y = 0;
    }
    if (player.shotCooldown > 0) {
        player.shotCooldown -= 0.2;
    }
    updateTurrets();
    updateRemoveLazers();
    al_get_keyboard_state(&keyState);
    if (al_key_down(&keyState, ALLEGRO_KEY_UP)) {
        struct Point force = {(sin(player.rotation) * FORCE) + player.accel.x, (-cos(player.rotation) * FORCE) + player.accel.y};
        struct Point maxForce = {sin(player.rotation) * MAX_SPEED, -cos(player.rotation) * MAX_SPEED};
        if (maxForce.x < 0 && player.accel.x > maxForce.x) {
            player.accel.x = MAX(maxForce.x, force.x);
        }
        else if (maxForce.x > 0 && player.accel.x < maxForce.x) {
            player.accel.x = MIN(maxForce.x, force.x);
        }
        if (maxForce.y < 0 && player.accel.y > maxForce.y) {
            player.accel.y = MAX(maxForce.y, force.y);
        }
        else if (maxForce.y > 0 && player.accel.y < maxForce.y) {
            player.accel.y = MIN(maxForce.y, force.y);
        }
        player.imageIndex = true;
    }
    else {
        player.imageIndex = false;
    }
    player.pos.x += player.accel.x;
    player.pos.y += player.accel.y;
    camera = player.pos;
    player.pos.x = MAX(0, MIN(MAP_MAX, player.pos.x));
    player.pos.y = MAX(0, MIN(MAP_MAX, player.pos.y));
    fixCamera();
    if (al_key_down(&keyState, ALLEGRO_KEY_LEFT)) {
        player.rotation -= 0.05;
    }
    if (al_key_down(&keyState, ALLEGRO_KEY_RIGHT)) {
        player.rotation += 0.05;
    }
    if (player.rotation > MAX_RADUINS) {
        player.rotation -= MAX_RADUINS;
    }
    else if (player.rotation < 0) {
        player.rotation += MAX_RADUINS;
    }
    if (al_key_down(&keyState, ALLEGRO_KEY_SPACE) && player.shotCooldown <= 0 && lazerCount < MAX_LAZERS) {
        player.shotCooldown = 1;
        struct Lazer lazer;
        lazer.rotation = player.rotation;
        lazer.start.x = lazer.pos.x = player.pos.x + (sin(lazer.rotation) * 100);
        lazer.start.y = lazer.pos.y = player.pos.y + (-cos(lazer.rotation) * 100);
        lazer.accel.x = (sin(lazer.rotation) * LAZER_FORCE);
        lazer.accel.y = (-cos(lazer.rotation) * LAZER_FORCE);
        lazer.distance = 0;
        lazer.players = true;
        lazers[lazerCount] = lazer;
        lazerCount++;
    }
}

void renderObjects(void) {
    struct Point offset = {(camera.x - (windowSize.width / 2)), (camera.y - (windowSize.height / 2))};
    struct Point beg = {(camera.x - (windowSize.width / 2)) - 200, (camera.y - (windowSize.height / 2)) - 200};
    struct Point end = {(camera.x + (windowSize.width / 2)) + 200, (camera.y + (windowSize.height / 2)) + 200};
    int maxStars = MAX_STARS;
    for (int i = 0; i < maxStars; i++) {
        struct Star s = stars[i];
        if (s.pos.x > beg.x && s.pos.x < end.x && s.pos.y > beg.y && s.pos.y < end.y) {
            al_draw_tinted_scaled_bitmap(s.image, s.tint, 0, 0, 20, 20, s.pos.x - offset.x, s.pos.y - offset.y, s.size.width, s.size.height, 0);
        }
    }
    int maxTurrets = MAX_TURRETS;
    for (int i = 0; i < maxTurrets; i++) {
        struct Turret t = turrets[i];
        if (t.pos.x > beg.x && t.pos.x < end.x && t.pos.y > beg.y && t.pos.y < end.y) {
            al_draw_rotated_bitmap(t.image, 50, 50, t.pos.x - offset.x, t.pos.y - offset.y, t.rotation, 0);
        }
    }
    for (int i = 0; i < lazerCount; i++) {
        struct Lazer lazer = lazers[i];
        float tint = (((LAZER_MAX_DISTANCE - lazer.distance) / LAZER_MAX_DISTANCE) * 0.3) + 0.7;
        al_draw_tinted_rotated_bitmap(lazerImage, al_map_rgba_f(tint, tint, tint, tint), 10, 18, lazer.pos.x - offset.x, lazer.pos.y - offset.y, lazer.rotation, 0);
    }
    { //player scope
        struct SpaceShip s = player;
        if (s.shield > 0) {
            float tint = ((s.shield / MAX_SHIELDS) * 0.7) + 0.25;
            al_draw_tinted_rotated_bitmap(shieldImage, al_map_rgba_f(tint, tint, tint, tint), 75, 125, s.pos.x - offset.x, s.pos.y - offset.y, s.rotation, 0);
        }
        al_draw_rotated_bitmap(s.image[s.imageIndex], s.centre.x, s.centre.y, s.pos.x - offset.x, s.pos.y - offset.y, s.rotation, 0);
    }
}
