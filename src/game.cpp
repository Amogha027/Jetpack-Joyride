#include "../include/game.h"
#include "resource_manager.h"
#include "sprite_renderer.h"
#include "text_renderer.h"
#include "game_object.h"
#include "ball_object.h"

#include <iostream>
#include <sstream>
#include <time.h>

// Game-related State data
SpriteRenderer *Renderer;
SpriteRenderer *toGlow;
GameObject *Player;
TextRenderer *Text;
GameObject *Zapper[6];

int nzap = 4, delta = 400;
float pos1 = 0.0f, pos2 = 800.0f;
float b_speed = 0.025f, c_speed = 0.05f;
float fwidth = 800.0f, fheight = 600.0f;

Game::Game(unsigned int width, unsigned int height)
    : State(GAME_MENU), Keys(), Width(width), Height(height), Level(0), Score(0), Coins(0)
{
}

Game::~Game()
{
    delete Renderer;
    delete toGlow;
    delete Player;
    delete Text;
    for (int i = 0; i < 6; i++)
        delete Zapper[i];
}

void Game::Init()
{
    // load shaders
    ResourceManager::LoadShader("../src/shaders/sprite.vs", "../src/shaders/sprite.fs", nullptr, "sprite");
    ResourceManager::LoadShader("../src/shaders/glow.vs", "../src/shaders/glow.fs", nullptr, "glow");
    // configure shaders
    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(this->Width),
                                      static_cast<float>(this->Height), 0.0f, -1.0f, 1.0f);
    ResourceManager::GetShader("sprite").Use().SetInteger("image", 0);
    ResourceManager::GetShader("sprite").SetMatrix4("projection", projection);
    ResourceManager::GetShader("glow").Use().SetInteger("image", 0);
    ResourceManager::GetShader("glow").SetMatrix4("projection", projection);
    // set render-specific controls
    Shader shader1 = ResourceManager::GetShader("sprite");
    Renderer = new SpriteRenderer(shader1);
    Shader shader2 = ResourceManager::GetShader("glow");
    toGlow = new SpriteRenderer(shader2);
    // load text
    Text = new TextRenderer(this->Width, this->Height);
    Text->Load("../src/fonts/OCRAEXT.TTF", 24);
    // load textures
    ResourceManager::LoadTexture("../src/textures/background.png", false, "background1");
    ResourceManager::LoadTexture("../src/textures/background.png", false, "background2");
    ResourceManager::LoadTexture("../src/textures/background2.png", false, "background3");
    ResourceManager::LoadTexture("../src/textures/background2.png", false, "background4");
    ResourceManager::LoadTexture("../src/textures/background3.png", false, "background5");
    ResourceManager::LoadTexture("../src/textures/background3.png", false, "background6");    ResourceManager::LoadTexture("../src/textures/character.png", true, "character");
    ResourceManager::LoadTexture("../src/textures/coin.png", true, "coin");
    ResourceManager::LoadTexture("../src/textures/zapper.png", true, "zapper");
    // load levels
    GameLevel one;
    one.Load(this->Width / 4.0f, this->Height / 4.0f, 0);
    GameLevel two;
    two.Load(this->Width / 4.0f, this->Height / 4.0f, 0);
    GameLevel three;
    three.Load(this->Width / 4.0f, this->Height / 4.0f, 0);
    this->Levels.push_back(one);
    this->Levels.push_back(two);
    this->Levels.push_back(three);
    this->Score = 0;

    // configure game objects
    // player
    glm::vec2 playerPos = glm::vec2(this->Width / 4.0f - PLAYER_SIZE.x / 4.0f, this->Height - PLAYER_SIZE.y);
    Player = new GameObject(playerPos, PLAYER_SIZE, ResourceManager::GetTexture("character"));
    // zapper
    glm::vec2 zapperPos = glm::vec2(this->Width, this->Height / 2.0f);
    for (int i = 0; i < 6; i++)
        Zapper[i] = new GameObject(zapperPos, ZAPPER_SIZE, ResourceManager::GetTexture("zapper"));
}

void Game::ResetLevel(int num)
{
    this->Level = num;
    this->Score = this->Coins = 0;
    glm::vec2 zapperPos = glm::vec2(this->Width, this->Height / 2.0f);
    for (int i = 0; i < 6; i++)
    {
        Zapper[i]->Size = ZAPPER_SIZE;
        Zapper[i]->Position = zapperPos;
    }

    for (int i = 0; i < this->Level; i++)
    {
        this->Levels[i].Load(this->Width / 4.0f, this->Height / 4.0f, i);
    }

    if (num == 1)
    {
        nzap = 4;
        delta = 400;
        b_speed = 0.025f;
        c_speed = 0.05f;
    }
    else
    {
        nzap++;
        if (num == 2)
            delta = 320;
        else
            delta = 260;
        b_speed += 0.025f;
        c_speed = 2 * b_speed;
    }
}

void Game::ResetZapper(int i)
{
    float dist = ZAPPER_SIZE.y;
    // reset the dimensions
    Zapper[i]->Direction = 0;
    Zapper[i]->Position = glm::vec2(this->Width + i * delta + dist / 2, 0.0f);
    Zapper[i]->Size = ZAPPER_SIZE;
    Zapper[i]->Distance = 0.0f;
    Zapper[i]->Rotation = 0.0f;

    // set the new position, angle and height on random
    if (!i) {
        if (Zapper[i]->Position.y > 0) Zapper[i]->Direction = 1;
        else Zapper[i]->Direction = -1;
    } else if (i == 2) {
        Zapper[i]->Rotation += 90.0f;
    } else if (i == 3) {
        Zapper[i]->Size.y += 50.0f;
    } else if (i == 5) {
        Zapper[i]->Size.y -= 50.0f;
    }
    srand(0);
    Zapper[i]->Position.x += rand() % (delta - (int)ZAPPER_SIZE.y);
    Zapper[i]->Position.y += rand() % (this->Height - (int)dist);
}

void Game::ProcessInput(float dt)
{
    if (this->State == GAME_MENU)
    {
        if (this->Keys[GLFW_KEY_ENTER] && !this->KeysProcessed[GLFW_KEY_ENTER])
        {
            ResetLevel(this->Level + 1);
            this->State = GAME_ACTIVE;
            this->KeysProcessed[GLFW_KEY_ENTER] = true;
        }
    }
    if (this->State == GAME_ACTIVE)
    {
        float velocity = PLAYER_VELOCITY * dt;
        // move player
        if (this->Keys[GLFW_KEY_SPACE])
        {
            if (Player->Position.y > 0.0f)
                Player->Position.y -= velocity;
        }
        else
        {
            if (Player->Position.y + Player->Size.y < this->Height)
                Player->Position.y += velocity;
        }
    }
    if (this->State == GAME_FINISH)
    {
        if (this->Keys[GLFW_KEY_ENTER] && !this->KeysProcessed[GLFW_KEY_ENTER])
        {
            ResetLevel(1);
            this->State = GAME_ACTIVE;
            this->KeysProcessed[GLFW_KEY_ENTER] = true;
        }
    }
}

void Game::Update(float dt)
{
    if (this->State == GAME_ACTIVE)
    {
        this->Score += 1;
        // move the background
        pos1 -= b_speed;
        if (pos1 <= -fwidth)
            pos1 = fwidth;
        pos2 -= b_speed;
        if (pos2 <= -fwidth)
            pos2 = fwidth;

        // move the coins
        for (int i = 0; i < this->Level; i++)
        {
            unsigned int out = 0;
            for (BallObject &coin : this->Levels[i].Coins)
            {
                coin.Position.x -= c_speed;
                if (coin.Position.x > -200)
                    out++;
            }
            if (!out)
                this->Levels[i].Load(this->Width / 4.0f, this->Height / 4.0f, i);
        }

        // move the zappers
        for (int i = 0; i < nzap; i++)
        {
            if (Zapper[i]->Distance > 3 * fwidth)
            {
                ResetZapper(i);
            }
            else
            {
                Zapper[i]->Distance += c_speed;
                Zapper[i]->Position.x -= c_speed;
                if (Zapper[i]->Direction == 1)
                {
                    if (Zapper[i]->Position.y > 0)
                        Zapper[i]->Position.y -= 0.025f;
                    else
                        Zapper[i]->Direction = -1;
                }
                if (Zapper[i]->Direction == -1)
                {
                    if (Zapper[i]->Position.y + Zapper[i]->Size.y < this->Height)
                        Zapper[i]->Position.y += 0.025f;
                    else
                        Zapper[i]->Direction = 1;
                }
            }
        }

        // check for collisions
        this->DoCollisions();

        if (this->Score > this->Level * 100000)
        {
            if (this->Level == 3)
                this->State = GAME_WIN;
            else
                this->State = GAME_MENU;
        }
    }
}

void Game::Render()
{
    if (this->State == GAME_ACTIVE || this->State == GAME_MENU || this->State == GAME_WIN || this->State == GAME_FINISH)
    {
        // draw background
        if (this->Level == 1) {
            Texture2D texture1 = ResourceManager::GetTexture("background3");
            Renderer->DrawSprite(0, texture1, glm::vec2(pos1, 0.0f), glm::vec2(this->Width, this->Height), 0.0f);

            Texture2D texture2 = ResourceManager::GetTexture("background4");
            Renderer->DrawSprite(0, texture2, glm::vec2(pos2, 0.0f), glm::vec2(this->Width, this->Height), 0.0f);
        } else if (this->Level == 2) {
            Texture2D texture1 = ResourceManager::GetTexture("background1");
            Renderer->DrawSprite(0, texture1, glm::vec2(pos1, 0.0f), glm::vec2(this->Width, this->Height), 0.0f);

            Texture2D texture2 = ResourceManager::GetTexture("background2");
            Renderer->DrawSprite(0, texture2, glm::vec2(pos2, 0.0f), glm::vec2(this->Width, this->Height), 0.0f);
        } else {
            Texture2D texture1 = ResourceManager::GetTexture("background5");
            Renderer->DrawSprite(0, texture1, glm::vec2(pos1, 0.0f), glm::vec2(this->Width, this->Height), 0.0f);

            Texture2D texture2 = ResourceManager::GetTexture("background6");
            Renderer->DrawSprite(0, texture2, glm::vec2(pos2, 0.0f), glm::vec2(this->Width, this->Height), 0.0f);
        }

        // draw level
        for (int i = 0; i < this->Level; i++)
            this->Levels[i].Draw(*Renderer);
        // draw character
        Player->Draw(*Renderer, 0);
        // draw zapper
        for (int i = 0; i < nzap; i++)
            Zapper[i]->Draw(*toGlow, 1);
        // show the score
        std::stringstream ss, ll, cc, len;
        cc << this->Coins;
        ll << this->Level;
        ss << this->Score / 100;
        len << this->Level * 1000;
        Text->RenderText("Level:" + ll.str(), 5.0f, 5.0f, 1.0f);
        Text->RenderText("Coins:" + cc.str(), 5.0f, 30.0f, 1.0f);
        Text->RenderText("Distance:" + ss.str() + "/" + len.str(), 125.0f, 5.0f, 1.0f);
    }
    if (this->State == GAME_MENU)
    {
        if (this->Level == 0)
        {
            Text->RenderText("Press ENTER to start", 250.0f, this->Height / 2.0f, 1.0f);
        }
        else
        {
            std::stringstream ss;
            ss << this->Level;
            Text->RenderText("You finished Level:" + ss.str(), 250.0f, (this->Height - 25) / 2.0f, 1.0f, glm::vec3(1.0f, 1.0f, 0.0f));
            Text->RenderText("Press ENTER for next level", 225.0f, (this->Height + 25) / 2.0f, 1.0f);
        }
    }
    if (this->State == GAME_WIN)
    {
        Text->RenderText("You WON!!!", 320.0f, (this->Height - 25) / 2.0f, 1.0f, glm::vec3(0.0f, 1.0f, 0.0f));
        Text->RenderText("Press ESC to quit", 280.0f, (this->Height + 25) / 2.0f, 1.0f, glm::vec3(1.0f, 1.0f, 0.0f));
    }
    if (this->State == GAME_FINISH)
    {
        Text->RenderText("You LOSE!!!", 320.0f, this->Height / 2.0f - 20.0f, 1.0f, glm::vec3(1.0f, 0.0f, 0.0f));
        Text->RenderText("Press ENTER to retry or ESC to quit", 130.0f, this->Height / 2.0f, 1.0f, glm::vec3(1.0f, 1.0f, 0.0f));
    }
}

bool Game::CheckCoin(BallObject &one, GameObject &two) // AABB - Circle collision
{
    // get center point circle first
    glm::vec2 center(one.Position + one.Radius);
    // calculate AABB info (center, half-extents)
    glm::vec2 aabb_half_extents(two.Size.x / 2.0f, two.Size.y / 2.0f);
    glm::vec2 aabb_center(
        two.Position.x + aabb_half_extents.x,
        two.Position.y + aabb_half_extents.y);
    // get difference vector between both centers
    glm::vec2 difference = center - aabb_center;
    glm::vec2 clamped = glm::clamp(difference, -aabb_half_extents, aabb_half_extents);
    // add clamped value to AABB_center and we get the value of box closest to circle
    glm::vec2 closest = aabb_center + clamped;
    // retrieve vector between center circle and closest point AABB and check if length <= radius
    difference = closest - center;
    return glm::length(difference) < one.Radius;
}

bool Game::CheckCollision(GameObject &one, GameObject &two) // AABB - AABB collision
{
    // collision x-axis?
    bool collisionX = one.Position.x + one.Size.x >= two.Position.x &&
                      two.Position.x + two.Size.x >= one.Position.x;
    // collision y-axis?
    bool collisionY = one.Position.y + one.Size.y >= two.Position.y &&
                      two.Position.y + two.Size.y >= one.Position.y;
    // collision only if on both axes
    return collisionX && collisionY;
}

bool Game::CheckIntersect(GameObject &one, GameObject &two)
{
    // get difference vector between both centers
    glm::vec2 difference = one.Position - two.Position;
    // get the sum of both the radius
    float len = one.Size.y / 2.0f + two.Size.y / 2.0f;
    return glm::length(difference) < len;
}

void Game::DoCollisions()
{
    for (int i = 0; i < this->Level; i++)
    {
        for (BallObject &coin : this->Levels[i].Coins)
        {
            if (!coin.Destroyed)
            {
                if (CheckCoin(coin, *Player))
                {
                    this->Coins++;
                    coin.Destroyed = true;
                }
                else
                {
                    for (int j = 0; j < nzap; j++)
                        if (CheckCoin(coin, *Zapper[j]))
                            coin.Destroyed = true;
                }
            }
        }
    }
    for (int i = 0; i < nzap; i++)
    {
        if (Zapper[i]->Rotation == 90.0f)
        {
            Zapper[i]->Size = glm::vec2(ZAPPER_SIZE.y, ZAPPER_SIZE.x);
            Zapper[i]->Position.x -= 70.0f;
            Zapper[i]->Position.y += 70.0f;
        }
        if (CheckCollision(*Zapper[i], *Player))
        {
            this->State = GAME_FINISH;
        }
        if (Zapper[i]->Rotation == 90.0f)
        {
            Zapper[i]->Size = ZAPPER_SIZE;
            Zapper[i]->Position.x += 70.0f;
            Zapper[i]->Position.y -= 70.0f;
        }
    }
}