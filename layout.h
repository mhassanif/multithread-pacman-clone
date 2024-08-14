#include "libraries.h" 
#include "globals.h" 

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

