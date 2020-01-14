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

    // Inserts a new node and returns root of modified tree
    // The parameter depth is used to decide axis of comparison
    KD_node *insertRec(KD_node *root, Tank *tank, unsigned depth) {
        // Tree is empty?
        if (root == nullptr)
            return new KD_node(tank);

        // Calculate current dimension of comparison
        // Compare the new point with root on current dimension 'cd'
        // and decide the left or right subtree
        if (depth % 2 == 0 ? tank->Get_Position().x < root->tank->Get_Position().x
                           : tank->Get_Position().y < root->tank->Get_Position().y)
            root->left = insertRec(root->left, tank, depth + 1);
        else
            root->right = insertRec(root->right, tank, depth + 1);

        return root;
    }

    // Function to insert a new point with given point in
    // KD Tree and return new root. It mainly uses above recursive
    // function "insertRec()"
    KD_node *insert(KD_node *root, Tank *_tank) {
        return insertRec(root, _tank, 0);
    }

    // Searches a Point represented by "point[]" in the K D tree.
    // The parameter depth is used to determine current axis.
    bool searchRec(KD_node *root, Tank *_tank, unsigned depth) {
        // Base cases
        if (root == nullptr)
            return false;
        if (root->tank == _tank)
            return true;

        // Current dimension is computed using current depth and total
        // Compare point with root with respect to cd (Current dimension)
        if (depth % 2 == 0 ? _tank->Get_Position().x < root->tank->Get_Position().x
                           : _tank->Get_Position().y < root->tank->Get_Position().y)
            return searchRec(root->left, _tank, depth + 1);

        return searchRec(root->right, _tank, depth + 1);
    }

    // Searches a Point in the K D tree. It mainly uses
    // searchRec()
    bool search(KD_node *root, Tank *_tank) {
        // Pass current depth as 0
        return searchRec(root, _tank, 0);
    }
}


