#include "libraries.h" 
#include "globals.h" 

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