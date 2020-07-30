//
//  linked_list.hpp
//  pzip
//
//  Created by Phuc Nguyen on 07/21/20.
//  Copyright © 2020 Phuc Nguyen. All rights reserved.
//

#include <iostream>

struct node {
    uint8_t value;
    node *next;
};
class list {
private:
    node *head, *tail;
public:
    list() {
        head = NULL;
        tail = NULL;
    }
    void insert_back(uint8_t value) {
        node *temp = new node;
        temp->value = value;
        temp->next = NULL;
        if (head == NULL) {
            head = temp;
            tail = temp;
            temp = NULL;
        } else {
            tail->next = temp;
            tail = temp;
        }
    }
    void insert_front(node* node) {
        node->next = head;
        head = node;
    }
    void insert_front(uint8_t value) {
        node *temp = new node;
        temp->value = value;
        temp->next = head;
        head = temp;
    }
    node* delete_at(int pos) {
        node *current = new node;
        node *previous = new node;
        current = head;
        for (int i = 0; i< pos; i++) {
            previous = current;
            current = current->next;
        }
        previous->next = current->next;
        return current;
    }

    // Move the node contains value to the front/head of list
    // Return the index of the found node
    int move_to_front(uint8_t value) {
        node *current = head;
        node *previous = NULL;
        if (current->value == value) {
            return 0;
        }
        int index = 1;
        while (current->next != NULL) {
            previous = current;
            current = current->next;
            if (current->value == value) {
                previous->next = current->next;
                current->next = head;
                head = current;
                return index;
            }
            index++;
        }
        return -1;
    }
};
