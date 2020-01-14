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

namespace PP2 {
    class Node {
    public:
        Node()
                : value(0), next(nullptr) {}

        Node(int value)
                : value(value), next(nullptr) {}

        int value;
        Node *next;
    };

    class LinkedList {
    public:
        LinkedList();

        void InsertValue(int value);

        void PrintList();

        Node *head;
    };

    class KD_node {
    public:
        KD_node(Tank *tank) : tank(tank), left(nullptr), right(nullptr) {};
        Tank *tank;
        KD_node *left, *right;
    };

    std::vector<LinkedList> HP_sort(std::vector<Tank *> &input, int n_buckets);

    std::vector<LinkedList> KD_sort(std::vector<Tank *> &input, int n_buckets);

    KD_node *insertRec(KD_node *root, Tank *tank, unsigned depth);

    KD_node *KD_insert_tank(KD_node *root, Tank *_tank);

    Tank *searchRec(KD_node *root, Tank *_tank, unsigned depth);

    Tank *KD_search_tank(KD_node *root, Tank *_tank);
}


