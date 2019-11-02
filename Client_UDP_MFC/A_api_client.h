#pragma once

/*************************************************************************
> File Name: client.c
> Author: lyk
> Date:	2019��5��30��10:49:21
************************************************************************/

#include <sys/types.h>
#include <winsock2.h>

#include<sys/types.h> 
#include<stdlib.h> 
#include<errno.h> 
#include<stdarg.h> 
#include<string.h> 
#include <WS2tcpip.h>
#include <stdio.h>


#define SERVER_PORT 9211 
#define BUFFER_SIZE 500 
#define FILE_NAME_MAX_SIZE 512 

/* ��ͷ */
typedef struct
{
	long int id;
	int buf_size;
	unsigned int  crc32val;   //ÿһ��buffer��crc32ֵ
	int errorflag;
}PackInfo;

/* ���հ� */
struct RecvPack
{
	PackInfo head;
	char buf[BUFFER_SIZE];
};

int main_test();