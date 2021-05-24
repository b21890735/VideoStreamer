// C program for array implementation of queue
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
 
typedef struct 
{
    int rate;
    char rate_str[70];
    char frame[13*70];
} Frame;

// A structure to represent a queue
typedef struct {
    int front, rear, size;
    unsigned capacity;
    Frame *array;
} Queue;
 

Frame EMPTY_FRAME;
// function to create a queue
// of given capacity.
// It initializes size of queue as 0
Queue* create_buffer(unsigned capacity)
{
    Queue* queue = (Queue*)malloc(
        sizeof(Queue));
    queue->capacity = capacity;
    queue->front = queue->size = 0;
 
    // This is important, see the enqueue
    queue->rear = capacity - 1;
    queue->array = (Frame *)malloc(
        queue->capacity * sizeof(Frame));
    return queue;
}
 
// queue is full when size becomes
// equal to the capacity
int is_full(Queue* queue)
{
    return (queue->size == queue->capacity);
}
 
// queue is empty when size is 0
int is_empty(Queue* queue)
{
    return (queue->size == 0);
}
 
// function to add an item to the queue.
// it changes rear and size
void enqueue(Queue* queue, Frame item)
{
    if (is_full(queue)){
        return;
    }
    queue->rear = (queue->rear + 1)
                  % queue->capacity;
    queue->array[queue->rear] = item;
    queue->size = queue->size + 1;
}
 
// function to remove an item from queue.
// it changes front and size
Frame dequeue(Queue* queue)
{
    if (is_empty(queue))
        return EMPTY_FRAME;
    Frame item = queue->array[queue->front];
    queue->front = (queue->front + 1)
                   % queue->capacity;
    queue->size = queue->size - 1;
    return item;
}
 
// function to get front of queue
Frame front(Queue* queue)
{
    if (is_empty(queue))
        return EMPTY_FRAME;
    return queue->array[queue->front];
}
 
// function to get rear of queue
Frame rear(Queue* queue)
{
    if (is_empty(queue))
        return EMPTY_FRAME;
    return queue->array[queue->rear];
}