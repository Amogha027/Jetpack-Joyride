#ifndef GAMELEVEL_H
#define GAMELEVEL_H
#include <vector>

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "ball_object.h"
#include "sprite_renderer.h"
#include "resource_manager.h"


const glm::vec2 INITIAL_BALL_VELOCITY(0.0f, 0.0f);
const float BALL_RADIUS = 12.5f;

/// GameLevel holds all Tiles as part of a Breakout level and 
/// hosts functionality to Load/render levels from the harddisk.
class GameLevel
{
public:
    // level state
    std::vector<BallObject> Coins;
    // constructor
    GameLevel() { }
    // loads level from file
    void Load(unsigned int levelWidth, unsigned int levelHeight, unsigned int it);
    // render level
    void Draw(SpriteRenderer &renderer);
    // check if the level is completed (all non-solid tiles are destroyed)
    bool IsCompleted();
private:
    // initialize level from tile data
    void init(std::vector<std::vector<unsigned int>> coinData, unsigned int levelWidth, unsigned int levelHeight, unsigned int it);
};

#endif