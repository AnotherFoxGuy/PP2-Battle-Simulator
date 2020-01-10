#pragma once

#include "Grid.h"
#include "defines.h"
#include "explosion.h"
#include "particle_beam.h"
#include "rocket.h"
#include "smoke.h"
#include "tank.h"
#include <cstdint>
#include <iostream>

namespace PP2
{
//forward declarations
class Tank;

class Rocket;

class Smoke;

class Particle_beam;

class Node
{
public:

    Node()
        : value(0), next(nullptr) {}

    Node(int value)
        : value(value), next(nullptr) {}

    int value;
    Node* next;
};

class LinkedList
{
public:
    LinkedList()
        : head(nullptr) {}

    void InsertValue(int value)
    {
        Node* new_node = new Node(value);

        if (head == nullptr || value <= head->value)
        {
            new_node->next = head;
            head = new_node;
            return;
        }

        Node* current = head;
        while (current->next != nullptr && value >= current->next->value) { current = current->next; }

        //Add node
        new_node->next = current->next;
        current->next = new_node;
    }

    void PrintList()
    {
        Node* current = head;
        while (current != nullptr)
        {
            std::cout << current->value << ", ";
            current = current->next;
        }
    }

    Node* head;
};

class Game
{
public:
    void SetTarget(SDL_Renderer* surface) { screen = surface; }

    void Init();

    void Shutdown();

    void Update(float deltaTime);

    void Draw();

    void Tick(float deltaTime);

    void MeasurePerformance();

    Tank& FindClosestEnemy(Tank& current_tank);

    std::vector<LinkedList> Sort(std::vector<Tank*>& input, int n_buckets);

    void DrawTankHP(int i, char color, int health);

    void MouseUp(int button)
    {
        /* implement if you want to detect mouse button presses */
    }

    void MouseDown(int button)
    {
        /* implement if you want to detect mouse button presses */
    }

    void MouseMove(int x, int y)
    {
        /* implement if you want to detect mouse movement */
    }

    void KeyUp(int key)
    {
        /* implement if you want to handle keys */
    }

    void KeyDown(int key)
    {
        /* implement if you want to handle keys */
    }

private:
    SDL_Renderer* screen;
    std::vector<Tank> tanks;
    std::vector<Tank*> blueTanks;
    std::vector<Tank*> redTanks;
    std::vector<Rocket> rockets;
    std::vector<Smoke> smokes;
    std::vector<Explosion> explosions;
    std::vector<Particle_beam> particle_beams;

    //Font *frame_count_font;
    long long frame_count = 0;

    bool lock_update = false;

    void UpdateTanks();

    void UpdateSmoke();

    void UpdateRockets();

    void UpdateParticleBeams();

    void UpdateExplosions();

    void SortHealthBars();

    ~Game();
};
}; // namespace PP2
