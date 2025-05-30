#include <iostream>
#include <new>
#include "queue.h"

Queue* init(void) {
    try {
        Queue* queue = new Queue;
        queue->head = nullptr;
        queue->tail = nullptr;
        queue->size = 0;
        return queue;
    }
    catch (std::bad_alloc&) {
        return nullptr;
    }
}

void release(Queue* queue) {
    if (queue == nullptr) return;

    {
        std::lock_guard<std::mutex> lock(queue->queue_mutex);
        Node* current = queue->head;
        while (current != nullptr) {
            Node* temp = current;
            current = current->next;
            delete temp;
        }
    }  // 여기서 lock 해제

    delete queue;
}


Node* nalloc(Item item) {
    try {
        Node* node = new Node;
        node->item = item;
        node->next = nullptr;
        return node;
    }
    catch (std::bad_alloc&) {
        return nullptr;
    }
}

void nfree(Node* node) {
    if (node != nullptr) {
        delete node;
    }
}

Node* nclone(Node* node) {
    if (node == nullptr) return nullptr;

    try {
        Node* cloned = new Node;
        cloned->item = node->item;
        cloned->next = nullptr;
        return cloned;
    }
    catch (std::bad_alloc&) {
        return nullptr;
    }
}
