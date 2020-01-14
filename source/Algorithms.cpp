#include "Algorithms.h"

namespace PP2 {
    LinkedList::LinkedList(): head(nullptr) {}

    void LinkedList::InsertValue(int value) {
        Node *new_node = new Node(value);

        if (head == nullptr || value <= head->value) {
            new_node->next = head;
            head = new_node;
            return;
        }

        Node *current = head;
        while (current->next != nullptr && value >= current->next->value) { current = current->next; }

        //Add node
        new_node->next = current->next;
        current->next = new_node;
    }

    void LinkedList::PrintList() {
        Node *current = head;
        while (current != nullptr) {
            std::cout << current->value << ", ";
            current = current->next;
        }
    }
    Node *head;
};

