#include "../include/game_level.h"
#include "ball_object.h"

#include <fstream>
#include <sstream>
#include <iostream>

unsigned int mod = 600;

void GameLevel::Load(unsigned int levelWidth, unsigned int levelHeight, unsigned int it)
{
    // clear old data
    this->Coins.clear();
    GameLevel level;
    std::string line;
    std::vector<std::vector<unsigned int>> coinData;
    srand(time(0));
    for (int i = 0; i < 5; i++) {
        int nc = 1 + rand() % 5;
        std::vector<unsigned int> row(5);
        if (nc == 1) row[2] = 1;
        if (nc == 2) row[1] = row[3] = 1;
        if (nc == 3) row[1] = row[2] = row[3] = 1;
        if (nc == 4) row[0] = row[1] = row[3] = row[4] = 1;
        if (nc == 5) row[0] = row[1] = row[2] = row[3] = row[4] = 1;
        coinData.push_back(row);
    }
    this->init(coinData, levelWidth, levelHeight, it);
}

void GameLevel::Draw(SpriteRenderer &renderer)
{
    for (BallObject &coin : this->Coins)
        if (!coin.Destroyed)
            coin.Draw(renderer, 0);
}

bool GameLevel::IsCompleted()
{
    for (BallObject &coin : this->Coins)
        if (!coin.Destroyed)
            return false;
    return true;
}

void GameLevel::init(std::vector<std::vector<unsigned int>> coinData, unsigned int levelWidth, unsigned int levelHeight, unsigned int it)
{
    // calculate dimensions
    srand(time(0));
    unsigned int height = coinData.size();
    unsigned int width = coinData[0].size(); // note we can index vector at [0] since this function is only called if height > 0
    float unit_width = levelWidth / static_cast<float>(width), unit_height = levelHeight / height; 
    unsigned int delta_x = 800 + it*(mod + 200) + rand() % mod;
    unsigned int delta_y = rand() % (600 - (int)unit_height * height);
    // initialize level tiles based on tileData		
    for (unsigned int y = 0; y < height; ++y)
    {
        for (unsigned int x = 0; x < width; ++x)
        {
            // check block type from level data (2D level array)
            if (coinData[y][x] == 1) // solid
            {
                glm::vec2 pos(unit_width * x + delta_x, unit_height * y + delta_y);
                BallObject obj(pos, BALL_RADIUS, INITIAL_BALL_VELOCITY, ResourceManager::GetTexture("coin"));
                obj.Destroyed = false;
                this->Coins.push_back(obj);
            }
        }
    }
}