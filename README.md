# Multi-threaded Pac-Man Clone

## Project Overview

This project is a multi-threaded clone of the classic Pac-Man game, developed in C++ using the SFML library. The project was part of the Operating Systems course at the National University of Computer and Emerging Sciences, FAST School of Computing, Islamabad Campus, Spring 2024. The primary objective was to implement various synchronization concepts and OS principles in a game environment.

## Key Features

### Game Structure
- **Game Engine Thread**: Manages the core game loop, handles player input, updates the game state, and renders graphics.
- **User Interface Thread**: Manages the UI components, including menus, scoreboards, and HUD elements.
- **Ghost Controller Threads**: Four individual threads control the behavior of each ghost, with potential for additional ghosts as the game progresses.

### Synchronization Scenarios
- **Scenario 1: Ghost Movement**  
  Implemented using the **Reader-Writer Problem**. Ghosts read the game board to decide their next move while Pac-Man writes to the board by eating pellets. Proper synchronization ensures no read operation occurs during a write.
  
- **Scenario 2: Eating Power Pellets**  
  Modeled after the **Producer-Consumer Problem**. Pac-Man consumes power pellets, causing ghosts to turn blue, and power pellets respawn after being consumed. Synchronization ensures that no two power pellets are eaten simultaneously, and only available pellets are consumed.
  
- **Scenario 3: Ghost House**  
  Solved using the **Dining Philosophers Problem**. Ghosts require a key and an exit permit to leave the ghost house, both of which are limited resources. Synchronization prevents deadlock when multiple ghosts attempt to leave the house simultaneously.
  
- **Scenario 4: Prioritizing Certain Ghosts**  
  This scenario also uses a variation of the **Producer-Consumer Problem** with race condition considerations. Faster ghosts require speed boosts, which are limited. Synchronization ensures fair access to these boosts among the faster ghosts.

## Technical Details

- **Programming Language**: C++
- **Graphics Library**: SFML
- **Synchronization Mechanisms**: Pthreads, Semaphores, Mutexes

### Compilation and Execution
To compile and run the project, ensure you have the SFML library installed on your system. Use the run.sh file to compile and execute the project! 

### Dependencies
- **SFML**: Simple and Fast Multimedia Library for handling graphics and input.

### Instructions
- Use WASD to control pacmans movement and navigate the maze.
- Your Goal is to Avoid the Ghosts and collect all the pellets to win the game.



