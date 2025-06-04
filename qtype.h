#ifndef _QTYPE_H  // header guard
#define _QTYPE_H

#include <mutex>
#include <atomic>

// ==========이 파일은 수정 가능==========

typedef unsigned int Key;  // 값이 클수록 높은 우선순위
typedef void* Value;

typedef struct {
    Key key;
    Value value;
} Item;

typedef struct {
    bool success;   // true: 성공, false: 실패
    Item item;
    // 필드 추가 가능
} Reply;

typedef struct node_t {
    Item item;
    struct node_t* next;
    // 필드 추가 가능
} Node;

typedef struct {
    Node* head;
    Node* tail;
    std::mutex queue_mutex;
    std::atomic<int> size;
    // 필드 추가 가능
} Queue;

// 깊은 복사를 위한 함수 포인터 타입 정의
typedef Value(*ValueCloneFunc)(Value value);
typedef void (*ValueFreeFunc)(Value value);

// 전역 함수 포인터
extern ValueCloneFunc global_value_clone;
extern ValueFreeFunc global_value_free;

void set_value_handlers(ValueCloneFunc clone_func, ValueFreeFunc free_func);

#endif
