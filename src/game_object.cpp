#include "../include/game_object.h"


GameObject::GameObject() 
    : Position(0.0f, 0.0f), Size(1.0f, 1.0f), Velocity(0.0f), Color(1.0f), Rotation(0.0f), Sprite(), Direction(0), Destroyed(false) { }

GameObject::GameObject(glm::vec2 pos, glm::vec2 size, Texture2D sprite, glm::vec3 color, glm::vec2 velocity) 
    : Position(pos), Size(size), Velocity(velocity), Color(color), Rotation(0.0f), Sprite(sprite), Direction(0), Destroyed(false) { }

void GameObject::Draw(SpriteRenderer &renderer, int num)
{
    renderer.DrawSprite(0, this->Sprite, this->Position, this->Size, this->Rotation, this->Color);
}