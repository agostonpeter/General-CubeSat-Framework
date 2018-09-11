#ifndef _PRIOQ
#define _PRIOQ
//priority queue v1 by Saranya
//v3 by Agoston

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>


typedef struct _OnBoardTCStruct{
    long time;
    int apid;
    int serviceType;
    int serviceSubType;
    long data;
}OnBoardTCStruct;

typedef struct{
	OnBoardTCStruct *nodes;
	int len;
	int size;

}heap_t;

void push (heap_t *h, OnBoardTCStruct* rxCompleteCmd);
void pop (heap_t *h,  OnBoardTCStruct* rxCompleteCmd);
OnBoardTCStruct* top(heap_t* h);

#endif

