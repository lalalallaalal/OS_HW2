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
