#include <iostream>
#include <new>
#include <cstring>
#include "queue.h"

// 전역 함수 포인터 정의 (기본값: nullptr로 초기화)
ValueCloneFunc global_value_clone = nullptr;
ValueFreeFunc global_value_free = nullptr;

// 사용자 정의 함수 설정
void set_value_handlers(ValueCloneFunc clone_func, ValueFreeFunc free_func) {
    global_value_clone = clone_func;
    global_value_free = free_func;
}

// 깊은 복사를 위한 Item 복사 함수
Item clone_item(const Item& original) {
    Item cloned;
    cloned.key = original.key;  // 값 복사

    // 깊은 복사: 사용자 정의 함수가 있으면 사용, 없으면 nullptr
    if (global_value_clone != nullptr) {
        cloned.value = global_value_clone(original.value);
    }
    else {
        cloned.value = nullptr;  // 안전을 위해 nullptr로 설정
    }

    return cloned;
}

// Item 해제 함수
void free_item(Item& item) {
    if (global_value_free != nullptr && item.value != nullptr) {
        global_value_free(item.value);
    }
    item.value = nullptr;
}

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
            free_item(temp->item);  // 깊은 해제
            delete temp;
        }
    }

    delete queue;
}

Node* nalloc(Item item) {
    try {
        Node* node = new Node;
        node->item = clone_item(item);  // 항상 깊은 복사
        node->next = nullptr;
        return node;
    }
    catch (std::bad_alloc&) {
        return nullptr;
    }
}

void nfree(Node* node) {
    if (node != nullptr) {
        free_item(node->item);  // 깊은 해제
        delete node;
    }
}

Node* nclone(Node* node) {
    if (node == nullptr) return nullptr;

    try {
        Node* cloned = new Node;
        cloned->item = clone_item(node->item);  // 항상 깊은 복사
        cloned->next = nullptr;
        return cloned;
    }
    catch (std::bad_alloc&) {
        return nullptr;
    }
}

// 내부 함수: 락 없는 enqueue (데드락 방지용)
static Reply enqueue_unlocked(Queue* queue, Item item) {
    Reply reply = { false, {0, nullptr} };

    if (queue == nullptr) return reply;

    // 1. 기존 key가 있는지 검색하여 업데이트
    Node* current = queue->head;
    while (current != nullptr) {
        if (current->item.key == item.key) {
            // 기존 key 발견 - value 업데이트
            free_item(current->item);  // 기존 value 해제
            current->item = clone_item(item);  // 새 value로 교체

            reply.success = true;
            reply.item = clone_item(item);
            return reply;
        }
        current = current->next;
    }

    // 2. 기존 key가 없으면 새 노드 삽입
    Node* newNode = nalloc(item);
    if (newNode == nullptr) return reply;

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
        while (current->next != nullptr && current->next->item.key > item.key) {
            current = current->next;
        }
        newNode->next = current->next;
        current->next = newNode;
        if (newNode->next == nullptr) {
            queue->tail = newNode;
        }
    }

    queue->size.fetch_add(1);  // 새 노드 추가 시만 크기 증가
    reply.success = true;
    reply.item = clone_item(item);

    return reply;
}

Reply enqueue(Queue* queue, Item item) {
    Reply reply = { false, {0, nullptr} };

    if (queue == nullptr) return reply;

    std::lock_guard<std::mutex> lock(queue->queue_mutex);
    return enqueue_unlocked(queue, item);
}

Reply dequeue(Queue* queue) {
    Reply reply = { false, {0, nullptr} };

    if (queue == nullptr) return reply;

    std::lock_guard<std::mutex> lock(queue->queue_mutex);

    if (queue->head == nullptr) return reply;

    Node* nodeToRemove = queue->head;
    reply.item = clone_item(nodeToRemove->item);  // 반환값 깊은 복사

    queue->head = queue->head->next;
    if (queue->head == nullptr) {
        queue->tail = nullptr;
    }

    nfree(nodeToRemove);  // 내부에서 깊은 해제 수행
    queue->size.fetch_sub(1);  // atomic 감소
    reply.success = true;

    return reply;
}

Queue* range(Queue* queue, Key start, Key end) {
    if (queue == nullptr) return nullptr;

    Queue* rangeQueue = init();
    if (rangeQueue == nullptr) return nullptr;

    std::lock_guard<std::mutex> lock(queue->queue_mutex);

    Node* current = queue->head;
    while (current != nullptr) {
        if (current->item.key >= start && current->item.key <= end) {
            // 데드락 방지: 락 없는 버전 사용
            Reply result = enqueue_unlocked(rangeQueue, current->item);
            if (!result.success) {
                release(rangeQueue);
                return nullptr;
            }
            // enqueue에서 반환된 reply.item도 깊은 복사된 것이므로 해제
            free_item(result.item);
        }
        current = current->next;
    }

    return rangeQueue;
}

