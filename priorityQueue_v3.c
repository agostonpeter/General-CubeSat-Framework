#include "priorityQueue_v3.h"

void pushaux (heap_t *h, OnBoardTCStruct rxCompleteCmd) {

    if (h->len + 1 >= h->size) {
        h->size = h->size ? h->size * 2 : 4;
        h->nodes = (OnBoardTCStruct *)realloc(h->nodes, h->size * sizeof (OnBoardTCStruct));
    }

    int i=h->len;

    while(i>0 && (h->nodes[i].aTime >= rxCompleteCmd.aTime)){
    	h->nodes[i+1]=h->nodes[i];
    	i--;
    }
    
    h->nodes[i+1]=rxCompleteCmd;

    h->len++;
}

void push (heap_t *h, OnBoardTCStruct* rxCompleteCmd) {
	pushaux(h, *rxCompleteCmd);
}

void pop(heap_t *h,  OnBoardTCStruct* rxCompleteCmd) {
    int i, j, k, inc;
    OnBoardTCStruct tc;

    if (!h->len) {
	rxCompleteCmd = NULL;
	return;
    }
   
    tc=h->nodes[1];
    *rxCompleteCmd = tc;


    for (i=1;i<h->len;i++){

    	h->nodes[i] = h->nodes[i+1];
    }
    h->len--;

}

OnBoardTCStruct* top(heap_t *h){
	if (!h->len) {
		return NULL;
    	}
	return (&(h->nodes[1]));
	return (h->nodes[1].aTime);
}

void printQueue(heap_t *h){
	printf("Printing priority queue\n");
	printf("size: %d, length: %d\n", h->size, h->len);
	int i;
	for(i=1; i<h->len+1;i++){
		printf("node %d -- aTime: %ld, apid:%d, service:%d, subservice:%d\n",
			i,
			h->nodes[i].aTime,
			h->nodes[i].apid,
			h->nodes[i].serviceType,
			h->nodes[i].serviceSubType
			);
	}
	printf("-------\n");
}