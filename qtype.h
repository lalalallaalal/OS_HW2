#pragma once
#include <mutex>
#include <atomic>

typedef int Key;
typedef void* Value;

typedef struct {
    Key key;
    Value value;
} Item;

typedef struct {
    bool success; // true: 성공, false: 실패
    Item item;
} Reply;

typedef struct node_t {
    Item item;
    struct node_t* next;
} Node;

typedef struct {
    Node* head;
    Node* tail;
    std::mutex queue_mutex;
    std::atomic<int> size;
} Queue;

// 우선순위 큐 연산을 위한 함수 선언
Reply enqueue(Queue* q, Key key, Value value);
Reply dequeue(Queue* q);
Reply peek(Queue* q);
bool isEmpty(Queue* q);
int getSize(Queue* q);
void initQueue(Queue* q);
void destroyQueue(Queue* q);
