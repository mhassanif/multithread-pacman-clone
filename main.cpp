#include "pacman.h"
#include "ghost.h"
#include "layout.h"

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

  scared_ghost.loadFromFile("textures/scared_ghost.png");

  for (int i = 0; i < numGhosts; ++i)
  {
    ghostSprites[i].setTexture(ghostTextures[i]);
    ghostSprites[i].setPosition(ghost_start_x, ghost_start_y);
    ghostSprites[i].setScale(1.0f, 1.0f);
  }

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

  //SPEED BOOST
  pthread_t speedBoost1, speedBoost2;
  // pthread_create(&speedBoost1, nullptr, speedBoost, &speedPowerUp1);
  // pthread_create(&speedBoost2, nullptr, speedBoost, &speedPowerUp2);

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
      cout << "POWER UP IS ACTIVE\n";

    if (POWER_UP && (time_POWER_UP % 10000) == 0)
    {
      POWER_UP = false;
      cout << "POWER UP RAN OUT\n";
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

    for (int i = 0; i < numGhosts; ++i)
    {
      pthread_mutex_lock(&ghostMutexes[i]); // Lock the mutex before accessing ghostSprites

      if (POWER_UP)
        ghostSprites[i].setTexture(scared_ghost);
      else
        ghostSprites[i].setTexture(ghostTextures[i]);

      window.draw(ghostSprites[i]);
      pthread_mutex_unlock(&ghostMutexes[i]); // Unlock the mutex after accessing ghostSprites
    }

    window.draw(scoreText); // Add this line to draw the score text

    window.draw(speedPowerUp1);
    window.draw(speedPowerUp2);

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

// Function to read the map from a text file
