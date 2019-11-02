#pragma once
#include <basetsd.h>
#include <iostream>
#include <WinSock2.h>
#include "st_MSG.h"

using namespace std;

typedef struct queue
{
	_DATA *pBase;
	int front;    //指向队列第一个元素
	int rear;    //指向队列最后一个元素的下一个元素
	int maxsize; //循环队列的最大存储空间
}QUEUE, *PQUEUE;

void CreateQueue(PQUEUE Q, int maxsize);

bool FullQueue(PQUEUE Q);
bool EmptyQueue(PQUEUE Q);

bool Enqueue(PQUEUE Q, _DATA* data);
bool Dequeue(PQUEUE Q, _DATA *data);

bool Clequeue(PQUEUE Q);
