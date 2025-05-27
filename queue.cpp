#include "queue.h"
#include "qtype.h"
#include <iostream>
#include <thread>
#include <atomic>
#include <mutex>

// �켱���� ť�� ���� �迭 ��� ���� �� ����
typedef struct {
    Item* heap;           // �� �迭
    int capacity;         // �ִ� �뷮
    std::atomic<int> size; // ���� ũ��
    std::mutex heap_mutex; // �� ����� ���ؽ�
} PriorityQueue;

// ���� �켱���� ť
static PriorityQueue* global_pq = nullptr;

// �� �ε��� ��� �Լ���
int parent(int i) { return (i - 1) / 2; }
int leftChild(int i) { return 2 * i + 1; }
int rightChild(int i) { return 2 * i + 2; }

// ��� ��ȯ �Լ�
void swap(Item* heap, int i, int j) {
    Item temp = heap[i];
    heap[i] = heap[j];
    heap[j] = temp;
}

// �켱���� �� (Key ���� �������� �켱������ ����)
bool hasHigherPriority(const Item& a, const Item& b) {
    return a.key > b.key;
}

// �� �Ӽ� ���� - ���� ����
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

// �� �Ӽ� ���� - ���� ����
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