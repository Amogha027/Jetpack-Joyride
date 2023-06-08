#ifndef GAME_H
#define GAME_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <tuple>

#include "game_level.h" 

// Represents the current state of the game
enum GameState {
    GAME_ACTIVE,
    GAME_FINISH,
    GAME_MENU,
    GAME_WIN
};

const float PI = glm::pi<float>();
const glm::vec2 PLAYER_SIZE(50.0f, 100.0f);
const float PLAYER_VELOCITY(500.0f);

const glm::vec2 ZAPPER_SIZE(10.0f, 150.0f);
const float ZAPPER_VELOCITY(500.0f);

// Game holds all game-related state and functionality.
// Combines all game-related data into a single class for
// easy access to each of the components and manageability.
class Game
{
public:
    // game state
    GameState               State;	
    bool                    Keys[1024];
    unsigned int            Width, Height;
    std::vector<GameLevel>  Levels;
    unsigned int            Level;
    unsigned int            Score, Coins;
    bool                    KeysProcessed[1024];
    // constructor/destructor
    Game(unsigned int width, unsigned int height);
    ~Game();
    // initialize game state (load all shaders/textures/levels)
    void Init();
    // game loop
    void ProcessInput(float dt);
    void Update(float dt);
    void Render();
    void DoCollisions();
    // check
    bool CheckCoin(BallObject &one, GameObject &two);
    bool CheckCollision(GameObject &one, GameObject &two);
    bool CheckIntersect(GameObject &one, GameObject &two);
    // reset
    void ResetZapper(int i);
    void ResetLevel(int num);
};

#endif