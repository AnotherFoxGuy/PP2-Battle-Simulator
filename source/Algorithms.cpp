#include "Algorithms.h"
#include <cmath>

#ifdef USING_EASY_PROFILER
#include <easy/profiler.h>
#endif

using namespace std;

namespace PP2
{

template <class T>
void LinkedList<T>::InsertValue(T value)
{
    Node<T>* new_node = new Node<T>(value);

    if (head == nullptr || value <= head->value)
    {
        new_node->next = head;
        head = new_node;
        return;
    }

    Node<T>* current = head;
    while (current->next != nullptr && value >= current->next->value) { current = current->next; }

    //Add node
    new_node->next = current->next;
    current->next = new_node;
}

// -----------------------------------------------------------
// Sort tanks by health value using bucket sort
// -----------------------------------------------------------
template <>
vector<LinkedList<int>> LinkedList<int>::Sort(vector<Tank*>& input, int n_buckets)
{
    vector<LinkedList<int>> buckets(n_buckets);
    for (auto tank : input) { buckets.at(tank->health / n_buckets).InsertValue(tank->health); }
    return buckets;
}

KD_Tree::KD_Tree(std::vector<Tank*>& input)
{
    std::vector<Tank*> activeTanks = {};

    // only use active tanks for building the KD tree
    for (auto tank : input)
        if (tank->active)
            activeTanks.emplace_back(tank);

    root = BuildKDTree(activeTanks, 0);
}
// Inserts list of tanks in the tree and return the root
// The parameter depth is used to decide axis of comparison
KD_node* KD_Tree::BuildKDTree(std::vector<Tank*> input, unsigned depth)
{
    // Tree is empty?
    if (input.size() == 1)
        return new KD_node(input[0]);

    if (input.empty())
        return nullptr;

    unsigned axis = depth % 2;

    // Sort input based on current depth
    sort(input.begin(), input.end(), [axis](Tank* a, Tank* b) {
        return a->position[axis] < b->position[axis];
    });

    Tank* tank = input[input.size() / 2];
    input.erase(input.begin() + (input.size() / 2));
    vector<Tank*> left(input.begin(), input.begin() + (input.size() / 2));
    vector<Tank*> right(input.begin() + ((input.size() / 2) + 1), input.end());

    KD_node* root = new KD_node(tank);
    root->left = BuildKDTree(left, depth + 1);
    root->right = BuildKDTree(right, depth + 1);

    return root;
}

// Searches the closest enemy tank in the K D tree.
Tank* KD_Tree::findClosestTankV2(Tank* tank)
{
    // some tanks go outside the screen, that is why we add some margin.
    float errorMargin = 250.f;
    float max = numeric_limits<float>::infinity();
    vec2<> hyperplane[] = {vec2<>(0 - errorMargin, 0 - errorMargin), vec2<>(1280 + errorMargin, 720 + errorMargin)};

    // root is at depth of 0
    return searchNN(root, tank, hyperplane, max, nullptr, 0);
}

Tank* KD_Tree::searchNN(KD_node* currentNode, Tank* target, vec2<> hyperplane[], float distanceCurrentClosestTank, Tank* currentClosestTank, int depth)
{
#ifdef USING_EASY_PROFILER
    //EASY_BLOCK("searchNN", profiler::colors::Red);
#endif
    if (currentNode == nullptr)
        return currentClosestTank;

    // X[0], Y[1] axis
    int axis = depth % 2;
    vec2<> leftOrTopHyperplane[2] = {}, rightOrBottomHyperplane[2] = {};
    vec2<> closestHyperplane[2] = {}, furthestHyperplane[2] = {};
    KD_node *closestNode = nullptr, *furthestNode = nullptr;

    // X axis, divide vertical
    if (axis == 0)
    {
        leftOrTopHyperplane[0] = hyperplane[0];
        leftOrTopHyperplane[1] = vec2<>(currentNode->tank->position[0], hyperplane[1][1]);
        rightOrBottomHyperplane[0] = vec2<>(currentNode->tank->position[0], hyperplane[0][1]);
        rightOrBottomHyperplane[1] = hyperplane[1];
    }
    // Y axis, divide horizontal
    if (axis == 1)
    {
        leftOrTopHyperplane[0] = hyperplane[0];
        leftOrTopHyperplane[1] = vec2<>(hyperplane[1][0], currentNode->tank->position[1]);
        rightOrBottomHyperplane[0] = vec2<>(hyperplane[0][0], currentNode->tank->position[1]);
        rightOrBottomHyperplane[1] = hyperplane[1];
    }
    // check which hyperplane the target(tank that's firing) belongs to
    if (target->position[axis] <= currentNode->tank->position[axis])
    {
        closestNode = currentNode->left;
        furthestNode = currentNode->right;
        memcpy(closestHyperplane, leftOrTopHyperplane, sizeof(closestHyperplane));
        memcpy(furthestHyperplane, rightOrBottomHyperplane, sizeof(furthestHyperplane));
    }
    if (target->position[axis] > currentNode->tank->position[axis])
    {
        closestNode = currentNode->right;
        furthestNode = currentNode->left;
        memcpy(closestHyperplane, rightOrBottomHyperplane, sizeof(closestHyperplane));
        memcpy(furthestHyperplane, leftOrTopHyperplane, sizeof(furthestHyperplane));
    }

    // check if the current node is closer to the target
    float dist = pow(currentNode->tank->position[0] - target->position[0], 2) + pow(currentNode->tank->position[1] - target->position[1], 2);
    if (dist < distanceCurrentClosestTank)
    {
        currentClosestTank = currentNode->tank;
        distanceCurrentClosestTank = dist;
    }

    // go deeper into the tree
    Tank* closestTank = searchNN(closestNode, target, closestHyperplane, distanceCurrentClosestTank, currentClosestTank, depth + 1);

    float distanceClosestTank = 0;
    dist = pow(closestTank->position[0] - target->position[0], 2) + pow(closestTank->position[1] - target->position[1], 2);
    if (distanceCurrentClosestTank < dist)
    {
        closestTank = currentClosestTank;
        distanceClosestTank = distanceCurrentClosestTank;
    }

    float pointX = calculateCurrentClosest(target->position[0], furthestHyperplane[0][0], furthestHyperplane[1][0]);
    float pointY = calculateCurrentClosest(target->position[1], furthestHyperplane[1][1], furthestHyperplane[0][1]);

    dist = pow((pointX - target->position[0]), 2) + pow((pointY - target->position[1]), 2);

    if (dist < distanceClosestTank)
        closestTank = searchNN(furthestNode, target, furthestHyperplane, distanceCurrentClosestTank, currentClosestTank, depth + 1);
    return closestTank;
}
float KD_Tree::calculateCurrentClosest(float targetXY, float hyperplaneMinXY, float hyperplaneMaxXY)
{
    if (targetXY <= hyperplaneMinXY)
        return hyperplaneMinXY;
    else if (targetXY >= hyperplaneMaxXY)
        return hyperplaneMaxXY;

    return targetXY;
}

void KD_Tree::bst_print_dot_null(const string& key, int nullCount, FILE* stream)
{
    fprintf(stream, "    null%d [shape=point];\n", nullCount);
    fprintf(stream, "    \"%s\" -> null%d;\n", key.c_str(), nullCount);
}

void KD_Tree::bst_print_dot_aux(KD_node* node, FILE* stream)
{
    static int nullCount = 0;

    if (node->left)
    {
        fprintf(stream, "    \"%s\" -> \"%s\";\n", node->print().c_str(), node->left->print().c_str());
        bst_print_dot_aux(node->left, stream);
    }
    else
        bst_print_dot_null(node->print(), nullCount++, stream);

    if (node->right)
    {
        fprintf(stream, "    \"%s\" -> \"%s\";\n", node->print().c_str(), node->right->print().c_str());
        bst_print_dot_aux(node->right, stream);
    }
    else
        bst_print_dot_null(node->print(), nullCount++, stream);
}

void KD_Tree::bst_print_dot(KD_node* tree, FILE* stream)
{
    fprintf(stream, "digraph BST {\n");
    fprintf(stream, "    node [fontname=\"Arial\"];\n");

    if (!tree)
        fprintf(stream, "\n");
    else if (!tree->right && !tree->left)
        fprintf(stream, "    \"%s\";\n", tree->print().c_str());
    else
        bst_print_dot_aux(tree, stream);

    fprintf(stream, "}\n");
}

}; // namespace PP2
