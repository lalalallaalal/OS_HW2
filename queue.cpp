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

Reply enqueue(Queue* queue, Item item) {
    Reply reply = { false, {0, nullptr} };

    if (queue == nullptr) return reply;

    Node* newNode = nalloc(item);
    if (newNode == nullptr) return reply;

    std::lock_guard<std::mutex> lock(queue->queue_mutex);

    // 우선순위 큐: 키값이 클수록 앞에 위치
    if (queue->head == nullptr || queue->head->item.key < item.key) {
        // 맨 앞에 삽입
        newNode->next = queue->head;
        queue->head = newNode;
        if (queue->tail == nullptr) {
            queue->tail = newNode;
        }
    }
    else {
        // 적절한 위치 찾아서 삽입
        Node* current = queue->head;
        while (current->next != nullptr && current->next->item.key >= item.key) {
            current = current->next;
        }
        newNode->next = current->next;
        current->next = newNode;
        if (newNode->next == nullptr) {
            queue->tail = newNode;
        }
    }

    queue->size++;
    reply.success = true;
    reply.item = item;

    return reply;
}

