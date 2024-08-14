#include "libraries.h" 
#ifndef GLOBALS_H 
#define GLOBALS_H
sf::Clock timer;
vector<string> readMapFromFile(const string &filename);
bool checkCollision(Sprite &entity);
void EatPellets();
void EatPOWERPellets();
void handleGhostCollision();
void *pacmanMovement(void *);
void *ghostController(void *);
bool POWER_UP = false;
int time_POWER_UP = 0;

// GLOBAL VARIABLES
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////

// window size and tile size
const int tileSize = 19;
const int windowWidth = 680;
const int windowHeight = 680;

// Create window
RenderWindow window(VideoMode(windowWidth, windowHeight), "PACMAN");

// rectangles to stores WALLS
vector<RectangleShape> WALLS;
// circles to store PELLETS
vector<CircleShape> PELLETS;
// circles to store  POWER_PELLETS
vector<CircleShape> POWER_PELLETS;

// display adjustment factor
const int screen_right = 70;
const int screen_down = 50;

Sprite pacmanSprite;
Texture pacman_right, pacman_left, pacman_up, pacman_down;
float movementSpeed = 5.0f;
const int pacman_start_x = 120;
const int pacman_start_y = 315;
int currentDirection = 0; // up = 1, down = 2, right = 3, left=4

int score = 0;

// LINVES
int livesRemaining = 3;
Texture lifeTexture;
Sprite lifeIcon; 
Sprite speedPowerUp1; 
Sprite speedPowerUp2; 

// GHOST
const int numGhosts = 4;
Sprite ghostSprites[numGhosts];

Texture scared_ghost;

// mutexes
pthread_mutex_t pacmanMutex;
pthread_mutex_t pelletUpdateMutex;
// pthread_mutex_t LifeMutex;

pthread_mutex_t ghostMutexes[numGhosts];

/////////////////////////////////////////////////////////
const int NUM_KEYS = 1;
const int NUM_PERMITS = 1;

sem_t key_semaphore;
sem_t permit_semaphore;

int ghost_start_x = 325;
int ghost_start_y = 310;

bool ghost_reset[4] = {false, false, false, false};
#endif