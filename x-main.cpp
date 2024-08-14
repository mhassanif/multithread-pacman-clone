#include <SFML/Graphics.hpp>
#include <iostream>
#include <fstream>
#include <vector>
#include <unistd.h>
#include <signal.h>
#include <cstdlib>
#include <ctime>
#include <pthread.h>
#include <semaphore.h>
#include <SFML/System.hpp>

sf::Clock timer;

using namespace std;
using namespace sf;

vector<string> readMapFromFile(const string &filename);
bool checkCollision(Sprite &entity);
void EatPellets();
void EatPOWERPellets();
void handleGhostCollision();
void *pacmanMovement(void *);
void *ghostController(void *);
bool POWER_UP = false, speedDemon = false;
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
const int pacman_start_x = 320;
const int pacman_start_y = 488;
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

/////////////////////////////////////////////////////////
void *speedBoost(void *arg)
{
  float rx, ry;
  Sprite *speedsprite = reinterpret_cast<Sprite *>(arg);
  FloatRect boostBounds = speedsprite->getGlobalBounds();
  for (auto &walls : WALLS)
    // cout << "fwfafd\n";
    do
    {
      rx = rand() % 680;
      ry = rand() % 680;
    } while (boostBounds.intersects(walls.getGlobalBounds()));

  speedsprite->setPosition(rx, ry);

  return nullptr;
}

/////////////////////////////////////////////////////////

void *ghostController(void *arg)
{
  int ghostID = *reinterpret_cast<int *>(arg);
  Sprite &ghost = ghostSprites[ghostID];
  pthread_mutex_t *mutex = &ghostMutexes[ghostID];
  // Seed the random number generator
  srand(static_cast<unsigned int>(time(nullptr)));

  float x = ghost_start_x;
  float y = ghost_start_y;

  int ghost_direction = 1; //  up = 1, down = 2, right = 3, left=4
  int ghost_movement_speed = movementSpeed / 2;

  // Define leave home state
  bool atHome = true;
  bool leaveHome = false;
  bool normal = false;

  while (window.isOpen())
  {

    x = ghost.getPosition().x;
    y = ghost.getPosition().y;

    // pthread_mutex_lock(mutex);

    if (atHome)
    {
      sem_wait(&key_semaphore);
      sem_wait(&permit_semaphore);
      // cout << ghostID << " got key and permit" << endl;
      atHome = false;
      leaveHome = true;
    }
    else if (leaveHome)
    {
      // Move the ghost upward until it leaves home
      ghost.move(0, -ghost_movement_speed);
      if (y <= ghost_start_y - 50) // Adjust the threshold as needed
      {
        ghost_direction = rand() % 2 + 3; // either left or
        leaveHome = false;
        normal = true;
        timer.restart();
      }
    }
    else if (normal) // Normal movement state
    {
      switch (ghost_direction)
      {
      case 1:
        // up
        ghost.move(0, -ghost_movement_speed);
        if (checkCollision(ghostSprites[ghostID]))
        {
          ghost.move(0, ghost_movement_speed);
          ghost_direction = rand() % 4 + 1;
        }
        break;
      case 2:
        // down
        ghost.move(0, ghost_movement_speed);
        if (checkCollision(ghostSprites[ghostID]))
        {
          ghost.move(0, -ghost_movement_speed);
          ghost_direction = rand() % 4 + 1;
        }
        break;
      case 3:
        // right
        ghost.move(ghost_movement_speed, 0);
        if (checkCollision(ghostSprites[ghostID]))
        {
          ghost.move(-ghost_movement_speed, 0);
          ghost_direction = rand() % 4 + 1;
        }
        break;
      case 4:
        // left
        ghost.move(-ghost_movement_speed, 0);
        if (checkCollision(ghostSprites[ghostID]))
        {
          ghost.move(ghost_movement_speed, 0);
          ghost_direction = rand() % 4 + 1;
        }
        break;

      default:
        break;
      }
      if (timer.getElapsedTime().asSeconds() > 5)
      {
        // cout << ghostID << " releasing resource..." << endl;
        sem_post(&key_semaphore);
        sem_post(&permit_semaphore);
        timer.restart();
      }

      // fix later when ghost resets
      if (ghost.getPosition().x == ghost_start_x && ghost.getPosition().y == ghost_start_y)
      {
        atHome = true;
        leaveHome = false;
        normal = false;
      }
    }

    // pthread_mutex_unlock(mutex);

    usleep(50000);
  }
  return nullptr;
}

/////////////////////////////////////////////////////////

int main()
{

  // Initialize semaphores
  sem_init(&key_semaphore, 0, NUM_KEYS);
  sem_init(&permit_semaphore, 0, NUM_PERMITS);

  // initalliz
  pthread_mutex_init(&pacmanMutex, NULL);
  // pthread_mutex_init(&LifeMutex, NULL);
  pthread_mutex_init(&pelletUpdateMutex, NULL);
  for (int i = 0; i < numGhosts; ++i)
    pthread_mutex_init(&ghostMutexes[i], nullptr);

  // map image texture
  Texture mapTexture;
  Sprite mapSprite;

  // load map from file
  vector<string> map = readMapFromFile("textures/Map.txt");

  // dummy wall
  RectangleShape *brick = new RectangleShape(Vector2f(tileSize, tileSize));
  brick->setFillColor(Color(0, 0, 128)); // dark blue

  // dummy pellet
  CircleShape *pellet = new CircleShape(tileSize / 8.0f);
  pellet->setFillColor(Color::Yellow);

  // dummy power pellet
  CircleShape *power_pellet = new CircleShape(tileSize / 3.0f);
  power_pellet->setFillColor(Color::White);

  Font font;
  font.loadFromFile("textures/cool_font.ttf"); // Load a font
  Text scoreText;

  // postion vars
  float positionX;
  float positionY;

  // read map and initiallize elements
  for (int y = 0; y < map.size(); ++y)
  {
    for (int x = 0; x < map[y].size(); ++x)
    {
      switch (map[y][x])
      {
      case '|': // WALL
        brick->setPosition(x * tileSize + screen_right, y * tileSize + screen_down);
        WALLS.push_back(*brick);
        break;
      case '.': // PELLET
        // Calculate center to place pellet
        positionX = x * tileSize + tileSize / 2.0f + screen_right;
        positionY = y * tileSize + tileSize / 2.0f + screen_down;
        pellet->setPosition(positionX, positionY);
        PELLETS.push_back(*pellet);
        break;
      case 'o': // POWER PELLET
        positionX = x * tileSize + tileSize / 2.0f + screen_right;
        positionY = y * tileSize + tileSize / 2.0f + screen_down;
        // Subtract half of the power pellet's radius from both x and y positions
        power_pellet->setPosition(positionX - tileSize / 6.0f, positionY - tileSize / 6.0f);
        POWER_PELLETS.push_back(*power_pellet);
        break;

      default:
        break;
      }
    }
  }

  // delete dummies
  delete brick;
  brick = nullptr;
  delete pellet;
  pellet = nullptr;
  delete power_pellet;
  power_pellet = nullptr;

  // load image and set map
  mapTexture.loadFromFile("textures/map.png"); // "map_white.png"
  mapSprite.setTexture(mapTexture);
  mapSprite.setPosition(screen_right + 2.5, screen_down - 55);
  mapSprite.setScale(1.18f, 1.18f);

  // pacman textures

  pacman_right.loadFromFile("textures/pacman_right.png");
  pacman_left.loadFromFile("textures/pacman_left.png");
  pacman_up.loadFromFile("textures/pacman_up.png");
  pacman_down.loadFromFile("textures/pacman_down.png");

  pacmanSprite.setTexture(pacman_right);
  pacmanSprite.setPosition(pacman_start_x, pacman_start_y);
  pacmanSprite.setScale(0.4f, 0.4f);

  // ghost

  Texture ghostTextures[numGhosts];
  ghostTextures[0].loadFromFile("textures/red.png");
  ghostTextures[1].loadFromFile("textures/pink.png");
  ghostTextures[2].loadFromFile("textures/blue.png");
  ghostTextures[3].loadFromFile("textures/orange.png");

  lifeTexture.loadFromFile("textures/life.png");
  lifeIcon.setTexture(lifeTexture);
  lifeIcon.setScale(1.2f, 1.2f);

  for (int i = 0; i < numGhosts; ++i)
  {
    ghostSprites[i].setTexture(ghostTextures[i]);
    ghostSprites[i].setPosition(ghost_start_x, ghost_start_y);
    ghostSprites[i].setScale(1.0f, 1.0f);
  }
  // speed boost

  Texture boosttex;
  boosttex.loadFromFile("textures/boost.png");
  speedPowerUp1.setTexture(boosttex);
  speedPowerUp2.setTexture(boosttex);

  //////////////////////////////////
  // THREADS
  //////////////////////////////////

  pthread_t pacmanThread;
  pthread_create(&pacmanThread, NULL, pacmanMovement, NULL);

  int ghostIDs[numGhosts];
  pthread_t ghostThreads[numGhosts];
  for (int i = 0; i < numGhosts; ++i)
  {
    ghostIDs[i] = i;
    pthread_create(&ghostThreads[i], nullptr, ghostController, &ghostIDs[i]);
  }

  // SPEED BOOST 
  pthread_t speedBoost1, speedBoost2;
  pthread_create(&speedBoost1, nullptr, speedBoost, &speedPowerUp1);
  pthread_create(&speedBoost2, nullptr, speedBoost, &speedPowerUp2);

  //////////////////////////////////

  // Main loop
  while (window.isOpen())
  {
    // Handle events
    Event event;
    while (window.pollEvent(event))
    {
      if (event.type == Event::Closed)
        window.close();
    }
    time_POWER_UP++;
    if (POWER_UP)
      // cout<<"POWER UP IS ACTIVE\n";

      if (POWER_UP && (time_POWER_UP % 25000) == 0)
      {
        POWER_UP = false;
        // cout<<"POWER UP RAN OUT\n";
        time_POWER_UP = 0;
      }

    handleGhostCollision();
    // Clear the window
    window.clear();

    // // Draw the walls
    // for (const auto &brick : WALLS)
    //     window.draw(brick);

    // pthread_mutex_lock(&LifeMutex);
    if (score >= 240 || livesRemaining < 0)
    {
      cout << "Game End!" << endl;
      break;
    }
    for (int i = 0; i < livesRemaining; ++i)
    {
      lifeIcon.setPosition(80 + i * 40, 640);
      window.draw(lifeIcon);
    }
    // pthread_mutex_unlock(&LifeMutex);

    pthread_mutex_lock(&pelletUpdateMutex);
    for (const auto &pellet : PELLETS)
      window.draw(pellet);
    pthread_mutex_unlock(&pelletUpdateMutex);

    // Draw power pellets
    for (const auto &power_pellet : POWER_PELLETS)
      window.draw(power_pellet);

    // Draw the map design image
    window.draw(mapSprite);

    // draw pacman
    pthread_mutex_lock(&pacmanMutex);
    window.draw(pacmanSprite);
    pthread_mutex_unlock(&pacmanMutex);

    // draw ghosts
    for (int i = 0; i < numGhosts; ++i)
    {
      pthread_mutex_lock(&ghostMutexes[i]); // Lock the mutex before accessing ghostSprites
      window.draw(ghostSprites[i]);
      pthread_mutex_unlock(&ghostMutexes[i]); // Unlock the mutex after accessing ghostSprites
    }

    window.draw(scoreText); // Add this line to draw the score text
    window.draw(speedPowerUp1);
    cout << speedPowerUp1.getPosition().x << ", " << speedPowerUp1.getPosition().y << '\n';
    window.draw(speedPowerUp2);
    cout << speedPowerUp2.getPosition().x << ", " << speedPowerUp2.getPosition().y << '\n';

    // Display the window
    window.display();

    scoreText.setFont(font); // set the font

    scoreText.setString("Score: " + to_string(score)); // set the string to display
    scoreText.setCharacterSize(24);                    // set the character size
    scoreText.setFillColor(Color::White);              // set the color
    scoreText.setPosition(80, 20);                     // adjust the position

    // change direction on key press
    if (Keyboard::isKeyPressed(Keyboard::A))
      currentDirection = 4; // left
    else if (Keyboard::isKeyPressed(Keyboard::D))
      currentDirection = 3; // right
    else if (Keyboard::isKeyPressed(Keyboard::W))
      currentDirection = 1; // up
    else if (Keyboard::isKeyPressed(Keyboard::S))
      currentDirection = 2; // down
  }

  sem_destroy(&key_semaphore);
  sem_destroy(&permit_semaphore);
  return 0;
}

//////////////////////////
// FUNCTION DEFINATIONS///
//////////////////////////

void handleGhostCollision()
{
  FloatRect pacmanBounds = pacmanSprite.getGlobalBounds();
  for (int i = 0; i < numGhosts; ++i)
  {
    pthread_mutex_lock(&ghostMutexes[i]); // Lock the ghost mutex before accessing ghostSprites
    FloatRect ghostBounds = ghostSprites[i].getGlobalBounds();
    pthread_mutex_unlock(&ghostMutexes[i]); // Unlock the ghost mutex after accessing ghostSprites

    if (pacmanBounds.intersects(ghostBounds))
    {
      if (POWER_UP)
      {
        // Set ghost position to starting position
        pthread_mutex_lock(&ghostMutexes[i]);
        ghostSprites[i].setPosition(ghost_start_x, ghost_start_x);
        pthread_mutex_unlock(&ghostMutexes[i]);
      }
      else
      {
        // Decrement lives
        livesRemaining--;
        usleep(500000);
        pthread_mutex_lock(&pacmanMutex);
        pacmanSprite.setPosition(pacman_start_x, pacman_start_y);
        currentDirection = 0;
        pacmanSprite.setTexture(pacman_right);
        pthread_mutex_unlock(&pacmanMutex);
      }
    }
  }
}

void *pacmanMovement(void *arg)
{
  while (window.isOpen())
  {
    pthread_mutex_lock(&pacmanMutex);

    switch (currentDirection)
    {
    case 1:
      // up
      pacmanSprite.setTexture(pacman_up);
      pacmanSprite.move(0, -movementSpeed);
      if (checkCollision(pacmanSprite))
        pacmanSprite.move(0, movementSpeed);
      break;
    case 2:
      // down
      pacmanSprite.setTexture(pacman_down);
      pacmanSprite.move(0, movementSpeed);
      if (checkCollision(pacmanSprite))
        pacmanSprite.move(0, -movementSpeed);
      break;
    case 3:
      // right
      pacmanSprite.setTexture(pacman_right);
      pacmanSprite.move(movementSpeed, 0);
      if (checkCollision(pacmanSprite))
        pacmanSprite.move(-movementSpeed, 0);
      break;
    case 4:
      // left
      pacmanSprite.setTexture(pacman_left);
      pacmanSprite.move(-movementSpeed, 0);
      if (checkCollision(pacmanSprite))
        pacmanSprite.move(movementSpeed, 0);
      break;

    default:
      break;
    }

    // teleportation tunnel
    if (pacmanSprite.getPosition().x < 70)
      pacmanSprite.setPosition(windowWidth - 100, pacmanSprite.getPosition().y);
    else if (pacmanSprite.getPosition().x > windowWidth - 100)
      pacmanSprite.setPosition(70, pacmanSprite.getPosition().y);

    // lock so its not drawn while its updating
    pthread_mutex_lock(&pelletUpdateMutex);
    EatPellets();

    if (!POWER_UP)
      EatPOWERPellets();

    pthread_mutex_unlock(&pelletUpdateMutex);

    pthread_mutex_unlock(&pacmanMutex);

    usleep(50000);
  }
  return nullptr;
}

// Function to read the map from a text file
vector<string> readMapFromFile(const string &filename)
{
  vector<string> map;
  ifstream file(filename);
  if (!file.is_open())
  {
    cout << "Failed to open file: " << filename << endl;
    return map;
  }

  string line;

  // read the map line by line and push into map vector
  while (getline(file, line))
    map.push_back(line);

  file.close();
  return map;
}

bool checkCollision(Sprite &entity)
{
  // Get the current bounds of the entity sprite
  FloatRect entityBounds = entity.getGlobalBounds();

  // Define a factor to reduce the size of the collision bounds (adjust as needed)
  float collisionReductionFactor = 0.1f; // Example: Reduce bounds by 20%

  // Decrease the size of the bounds by the reduction factor
  float reducedWidth = entityBounds.width * collisionReductionFactor;
  float reducedHeight = entityBounds.height * collisionReductionFactor;

  // Calculate the offset to center the reduced bounds around the sprite
  float xOffset = (entityBounds.width - reducedWidth) / 2.0f;
  float yOffset = (entityBounds.height - reducedHeight) / 2.0f;

  // Create the reduced bounds
  FloatRect reducedBounds(entityBounds.left + xOffset, entityBounds.top + yOffset, reducedWidth, reducedHeight);

  // Check for collision with walls using the reduced bounds
  for (const auto &wall : WALLS)
  {
    if (reducedBounds.intersects(wall.getGlobalBounds()))
    {
      return true; // Collision detected
    }
  }

  return false; // No collision detected
}

void EatPellets()
{
  FloatRect pacmanBounds = pacmanSprite.getGlobalBounds();
  vector<int> pelletsToRemove;

  int i = 0;
  for (auto &pellet : PELLETS)
  {
    if (pacmanBounds.intersects(pellet.getGlobalBounds()))
    {
      pelletsToRemove.push_back(i);
      score++;
    }
    i++;
  }

  // Erase the pellets that need to be deleted
  for (int j = pelletsToRemove.size() - 1; j >= 0; j--)
    PELLETS.erase(PELLETS.begin() + pelletsToRemove[j]);
}

void EatPOWERPellets()
{
  FloatRect pacmanBounds = pacmanSprite.getGlobalBounds();
  vector<int> pelletsToRemove;

  int i = 0;
  for (auto &powpellet : POWER_PELLETS)
  {
    // cout << "fwfafd\n";
    if (pacmanBounds.intersects(powpellet.getGlobalBounds()))
    {
      pelletsToRemove.push_back(i);
      POWER_UP = true;
    }
    i++;
  }

  // Erase the pellets that need to be deleted
  for (int j = pelletsToRemove.size() - 1; j >= 0; j--)
    POWER_PELLETS.erase(POWER_PELLETS.begin() + pelletsToRemove[j]);
}