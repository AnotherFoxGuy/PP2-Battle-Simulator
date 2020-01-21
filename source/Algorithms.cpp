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
Tank* KD_Tree::findClosestTank(Tank* tank)
{
    // some tanks go outside the screen, that is why we add some margin.
    float errorMargin = 250.f;
    float max = numeric_limits<float>::infinity();
    Rectangle2D hyperplane = {{-250.f, -250.f}, {1750.f, 1750.f}};

    // root is at depth of 0
    return searchNN(root, tank, hyperplane, max, nullptr, 0);
}

Tank* KD_Tree::searchNN(KD_node* currentNode, Tank* target, Rectangle2D& hyperplane, float distanceCurrentClosestTank, Tank* currentClosestTank, int depth)
{
#ifdef USING_EASY_PROFILER
    //EASY_BLOCK("searchNN", profiler::colors::Red);
#endif
    if (currentNode == nullptr)
        return currentClosestTank;

    // X[0], Y[1] axis
    int axis = depth % 2;
    Rectangle2D leftOrTopHyperplane = {}, rightOrBottomHyperplane = {}, closestHyperplane = {}, furthestHyperplane = {};
    KD_node *closestNode = nullptr, *furthestNode = nullptr;

    // X axis, divide vertical
    if (axis == 0)
    {
        leftOrTopHyperplane = {hyperplane.min, {currentNode->tank->position.x, hyperplane.max.y}};
        rightOrBottomHyperplane = {{currentNode->tank->position.x, hyperplane.min.y}, hyperplane.max};
    }
    // Y axis, divide horizontal
    if (axis == 1)
    {
        leftOrTopHyperplane = {hyperplane.min, {hyperplane.max.x, currentNode->tank->position.y}};
        rightOrBottomHyperplane = {{hyperplane.min.x, currentNode->tank->position.y}, hyperplane.max};
    }
    // check which hyperplane the target(tank that's firing) belongs to
    if (target->position[axis] <= currentNode->tank->position[axis])
    {
        closestNode = currentNode->left;
        furthestNode = currentNode->right;
        closestHyperplane = leftOrTopHyperplane;
        furthestHyperplane = rightOrBottomHyperplane;
    }
    if (target->position[axis] > currentNode->tank->position[axis])
    {
        closestNode = currentNode->right;
        furthestNode = currentNode->left;
        closestHyperplane = rightOrBottomHyperplane;
        furthestHyperplane = leftOrTopHyperplane;
    }

    // check if the current node is closer to the target
    float dist = pow(currentNode->tank->position.x - target->position.x, 2) + pow(currentNode->tank->position.y - target->position.y, 2);
    if (dist < distanceCurrentClosestTank)
    {
        currentClosestTank = currentNode->tank;
        distanceCurrentClosestTank = dist;
    }

    // go deeper into the tree
    Tank* closestTank = searchNN(closestNode, target, closestHyperplane, distanceCurrentClosestTank, currentClosestTank, depth + 1);

    float distanceClosestTank = 0;
    dist = pow(closestTank->position.x - target->position.x, 2) + pow(closestTank->position.y - target->position.y, 2);
    if (distanceCurrentClosestTank < dist)
    {
        closestTank = currentClosestTank;
        distanceClosestTank = distanceCurrentClosestTank;
    }

    float pointX = calculateCurrentClosest(target->position.x, furthestHyperplane.min.x, furthestHyperplane.max.x);
    float pointY = calculateCurrentClosest(target->position.y, furthestHyperplane.min.y, furthestHyperplane.max.y);

    dist = pow((pointX - target->position.x), 2) + pow((pointY - target->position.y), 2);

    if (dist < distanceClosestTank)
        closestTank = searchNN(furthestNode, target, furthestHyperplane, distanceCurrentClosestTank, currentClosestTank, depth + 1);
    return closestTank;
}
float KD_Tree::calculateCurrentClosest(float targetXY, float hyperplaneMinXY, float hyperplaneMaxXY)
{
    float value = 0;
    if (hyperplaneMinXY < targetXY && targetXY > hyperplaneMaxXY)
        value = targetXY;
    else if (targetXY <= hyperplaneMinXY)
        value = hyperplaneMinXY;
    else if (targetXY >= hyperplaneMaxXY)
        value = hyperplaneMaxXY;

    return value;
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
} // namespace PP2
