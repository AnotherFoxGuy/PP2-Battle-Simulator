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
    template<class T>
    class Node {
    public:
        Node() : value(nullptr), next(nullptr) {}

        Node(T value) : value(value), next(nullptr) {}

        ~Node() { DNode(); }

        void DNode();
        
        T value;
        Node<T> *next;
    };

    template<class T>
    class LinkedList {
    public:
        LinkedList();

        ~LinkedList() { if(head != nullptr) delete head; }

        void InsertValue(T value);

        Node<T> *head;
    };

    class KD_node {
    public:
        KD_node(Tank *tank) : tank(tank), left(nullptr), right(nullptr) {};
        Tank *tank;
        KD_node *left, *right;
    };

    std::vector<LinkedList<int>> HP_sort(std::vector<Tank *> &input, int n_buckets);

    std::vector<LinkedList<vec2<>>> KD_sort(std::vector<Tank *> &input, int n_buckets);

    KD_node *insertRec(KD_node *root, Tank *tank, unsigned depth);

    KD_node *KD_insert_tank(KD_node *root, Tank *_tank);

    Tank *searchRec(KD_node *root, Tank *_tank, unsigned depth);

    Tank *KD_search_closest_tank(KD_node *root, Tank *_tank);
}


