#include "stdafx.h"
#include "C_SingleCircuList.h"

//创建队列，分配内存空间
void CreateQueue(PQUEUE Q, int maxsize)
{
	Q->pBase = (_DATA*)malloc(sizeof(_DATA)*maxsize);

	if (NULL == Q->pBase)
	{
		TRACE("Memory allocation failure\n");
		exit(-1);        //退出程序
	}

	Q->front = 0;         //初始化参数
	Q->rear = 0;
	Q->maxsize = maxsize;

}

bool FullQueue(PQUEUE Q)
{
	if (Q->front == (Q->rear + 1) % Q->maxsize)    //判断循环链表是否满，留一个预留空间不用
		return true;
	else
		return false;
}

bool EmptyQueue(PQUEUE Q)
{
	if (Q->front == Q->rear)    //判断是否为空
		return true;
	else
		return false;
}

//入队
bool Enqueue(PQUEUE Q, _DATA* data)
{
	if (FullQueue(Q) || !data)
		return false;
	else
	{
		memset((char*)&Q->pBase[Q->rear], 0, sizeof(_DATA));
		memcpy((char*)&Q->pBase[Q->rear], (char*)data, sizeof(_DATA));
		Q->rear = (Q->rear + 1) % Q->maxsize;
		
		return true;
	}
}

bool Dequeue(PQUEUE Q, _DATA *data)
{
	if (EmptyQueue(Q))
	{
		return false;
	}
	else
	{
		memcpy(data, (char*)&Q->pBase[Q->front], sizeof(_DATA));
		Q->front = (Q->front + 1) % Q->maxsize;

		return true;
	}
}

bool Clequeue(PQUEUE Q)
{
	if (EmptyQueue(Q))
	{
		return false;//原本就是空的
	}
	else
	{
		Q->front = 0;        
		Q->rear = 0;
		return true;
	}
}

//销毁队列操作

