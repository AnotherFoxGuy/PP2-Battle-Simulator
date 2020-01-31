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
/**
 * Node Used by the Bucketsort/LinkedList
 * @tparam T
 */
template <class T>
class Node
{
  public:
    Node() : value(nullptr), next(nullptr) {}

    Node(T value) : value(value), next(nullptr) {}

    ~Node()
    {
        if (next != nullptr) delete next;
    }

    T value;
    Node<T>* next;
};

/**
 * Bucketsort/LinkedList
 * @tparam T
 */
template <class T>
class LinkedList
{
  public:
    LinkedList() : head(nullptr){};

    ~LinkedList()
    {
        delete head;
    }
    /**
 * Insert a value into the list
 * @param value Value to insert
 */
    void InsertValue(T value);

    /**
 * Sort the list
 * @param input List to sort
 * @param n_buckets Number of buckets to use for sorting
 * @return Sorted list
 */
    static std::vector<LinkedList<T>> Sort(std::vector<Tank*>& input, int n_buckets);

    /**
 * The head
 */
    Node<T>* head;
};

/**
 * The node used by the KD Tree
 */
class KD_node
{
  public:
    KD_node(Tank* tank) : tank(tank){};
    ~KD_node()
    {
        delete left;
        delete right;
    };

    std::string print()
    {
        char buff[50];
        sprintf(buff, "(%g,%g)", tank->position.x, tank->position.y);
        return buff;
    };

    Tank* tank = nullptr;
    KD_node* right = nullptr;
    KD_node* left = nullptr;
};
/**
 * The K-D Tree
 */
class KD_Tree
{
  public:
    explicit KD_Tree(std::vector<Tank*>& input);
    ~KD_Tree()
    {
        delete root;
    };

    /**
     * Find the closest tank
     * @param tank The tank to measure the distance
     * @return
     */
    Tank* findClosestTank(Tank* tank);

    void printTree()
    {
        auto pFile = fopen("./TreeDebug.dot", "w");
        bst_print_dot(root, pFile);
        fclose(pFile);
    };

  private:
    KD_node* root = nullptr;
    static KD_node* BuildKDTree(std::vector<Tank*> input, unsigned depth);
    static float calculateCurrentClosest(float targetXY, float hyperplaneMinXY, float hyperplaneMaxXY);
    static Tank* searchNN(KD_node* currentNode, Tank* target, Rectangle2D& hyperplane, float distanceCurrentClosestTank, Tank* currentClosestTank, int depth);

    static void bst_print_dot(KD_node* tree, FILE* stream);
    static void bst_print_dot_aux(KD_node* node, FILE* stream);
    static void bst_print_dot_null(const std::string& key, int nullCount, FILE* stream);
};

std::vector<int> CountSort(const std::vector<Tank*>& in);
} // namespace PP2
