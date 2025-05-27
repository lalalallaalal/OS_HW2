#include "queue.h"
#include "qtype.h"
#include <iostream>
#include <thread>
#include <atomic>
#include <mutex>

// 우선순위 큐를 위한 배열 기반 이진 힙 구조
typedef struct {
    Item* heap;           // 힙 배열
    int capacity;         // 최대 용량
    std::atomic<int> size; // 현재 크기
    std::mutex heap_mutex; // 힙 연산용 뮤텍스
} PriorityQueue;

// 전역 우선순위 큐
static PriorityQueue* global_pq = nullptr;

// 힙 인덱스 계산 함수들
int parent(int i) { return (i - 1) / 2; }
int leftChild(int i) { return 2 * i + 1; }
int rightChild(int i) { return 2 * i + 2; }

// 요소 교환 함수
void swap(Item* heap, int i, int j) {
    Item temp = heap[i];
    heap[i] = heap[j];
    heap[j] = temp;
}

// 우선순위 비교 (Key 값이 높을수록 우선순위가 높음)
bool hasHigherPriority(const Item& a, const Item& b) {
    return a.key > b.key;
}

// 힙 속성 유지 - 상향 정렬
void heapifyUp(Item* heap, int index) {
    while (index > 0) {
        int parentIndex = parent(index);
        if (hasHigherPriority(heap[index], heap[parentIndex])) {
            swap(heap, index, parentIndex);
            index = parentIndex;
        }
        else {
            break;
        }
    }
}

// 힙 속성 유지 - 하향 정렬
void heapifyDown(Item* heap, int size, int index) {
    while (true) {
        int largest = index;
        int left = leftChild(index);
        int right = rightChild(index);

        if (left < size && hasHigherPriority(heap[left], heap[largest])) {
            largest = left;
        }

        if (right < size && hasHigherPriority(heap[right], heap[largest])) {
            largest = right;
        }

        if (largest != index) {
            swap(heap, index, largest);
            index = largest;
        }
        else {
            break;
        }
    }
}