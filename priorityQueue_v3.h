#ifndef _PRIOQ
#define _PRIOQ
//priority queue v1 by Saranya
//v2 by Agoston

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "frameworkDataStructs.h"

typedef struct{
	OnboardTC *nodes;
	int len;
	int size;

}heap_t;

void push (heap_t *h, OnboardTC* rxCompleteCmd);
void pop (heap_t *h,  OnboardTC* rxCompleteCmd);
OnboardTC* top(heap_t* h);

#endif

