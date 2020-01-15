#include "Algorithms.h"

using namespace std;
namespace PP2 {
    template<class T>
    LinkedList<T>::LinkedList() : head(nullptr) {}

    template<class T>
    void LinkedList<T>::InsertValue(T value) {
        Node<T> *new_node = new Node<T>(value);

        if (head == nullptr || value <= head->value) {
            new_node->next = head;
            head = new_node;
            return;
        }

        Node<T> *current = head;
        while (current->next != nullptr && value >= current->next->value) { current = current->next; }

        //Add node
        new_node->next = current->next;
        current->next = new_node;
    }
    template<class T>
    void Node<T>::DNode() {
        delete next;
    }
    template <>
    void Node<vec2<>>::DNode() {
        if (next) delete next;
    }
    // -----------------------------------------------------------
    // Sort tanks by health value using bucket sort
    // -----------------------------------------------------------
    vector <LinkedList<int>> HP_sort(vector<Tank *> &input, int n_buckets) {
        vector <LinkedList<int>> buckets(n_buckets);
        for (auto tank : input) { buckets.at(tank->health / n_buckets).InsertValue(tank->health); }
        return buckets;
    }

    // -----------------------------------------------------------
    // Sort tanks by x(coordinates) value using bucket sort for even KD_tree distribution
    // -----------------------------------------------------------
    vector <LinkedList<vec2<>>> KD_sort(vector<Tank *> &input, int n_buckets) {
        vector <LinkedList<vec2<>>> buckets(n_buckets);
        for (auto &tank : input) { buckets.at(tank->Get_Position().x / n_buckets).InsertValue(tank->position); }
        return buckets;
    }

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

    // Function to insert a new point with given _tank in
    // KD Tree and return new root. It mainly uses above recursive
    // function "insertRec()"
    KD_node *KD_insert_tank(KD_node *root, Tank *_tank) {
        return insertRec(root, _tank, 0);
    }


    // Searches a Point represented by "_tank" in the K D tree.
    // The parameter depth is used to determine current axis.
    Tank *searchRec(KD_node *root, Tank *_tank, unsigned depth) {
        float min_distance = numeric_limits<float>::infinity();
        Tank* closest_Tank = root->tank;

        float sqrDist = fabsf((_tank->Get_Position() - root->tank->Get_Position()).sqrLength());
        if (sqrDist < min_distance) {
            min_distance = sqrDist;
            closest_Tank = root->tank;
        }

        if (root->left == nullptr)
            return closest_Tank;

        // Current dimension is computed using current depth and total
        // Compare point with root with respect to cd (Current dimension)
        if (depth % 2 == 0 ? _tank->Get_Position().x < root->tank->Get_Position().x
                           : _tank->Get_Position().y < root->tank->Get_Position().y)
            return searchRec(root->left, _tank, depth + 1);

        return searchRec(root->right, _tank, depth + 1);
    }

    // Searches a Point in the K D tree. It mainly uses
    // searchRec()
    Tank *KD_search_closest_tank(KD_node *root, Tank *_tank) {
        // Pass current depth as 0
        return searchRec(root, _tank, 0);
    }
};

