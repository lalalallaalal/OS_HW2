#pragma once
#include <mutex>
#include <atomic>

typedef int Key;
typedef int Value;

typedef struct {
    Key key;
    Value value;
} Item;

typedef struct {
    bool success; // true: ����, false: ����
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

// �켱���� ť ������ ���� �Լ� ����
Reply enqueue(Queue* q, Key key, Value value);
Reply dequeue(Queue* q);
Reply peek(Queue* q);
bool isEmpty(Queue* q);
int getSize(Queue* q);
void initQueue(Queue* q);
void destroyQueue(Queue* q);
