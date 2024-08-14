#include "libraries.h"
#include "globals.h"

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

    // deal reset
    if (ghost_reset[ghostID] && x == ghost_start_x && ghost_start_y == y)
    {
      atHome = true;
      leaveHome = false;
      normal = false;
      ghost_reset[ghostID] = false;
    }

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
    }

    // pthread_mutex_unlock(mutex);

    usleep(50000);
  }
  return nullptr;
}

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
        // reset ghost
        ghost_reset[i] = true;
        ghostSprites[i].setPosition(ghost_start_x, ghost_start_y);
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
