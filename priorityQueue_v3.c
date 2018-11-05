#include "priorityQueue_v3.h"

void pushaux (heap_t *h, OnboardTC rxCompleteCmd) {

    if (h->len + 1 >= h->size) {
        h->size = h->size ? h->size * 2 : 4;
        h->nodes = (OnboardTC *)realloc(h->nodes, h->size * sizeof (OnboardTC));
    }

    /*
    int i = h->len + 1;
    int j = i / 2;
    while (i > 1 && h->nodes[j].time > rxCompleteCmd.time) {
        h->nodes[i] = h->nodes[j];
        i = j;
        j = j / 2;
    }

	h->nodes[i].time = rxCompleteCmd.time;
    h->nodes[i].rx_length = rxCompleteCmd.rx_length;
    */

    int i=h->len;

    while(i>0 && (h->nodes[i].time > rxCompleteCmd.time)){
    	/*
    	h->nodes[i+1].time=h->nodes[i].time;
    	h->nodes[i+1].rx_length=h->nodes[i].rx_length;
    	*/
    	h->nodes[i+1]=h->nodes[i];
    	i--;
    }
    /*
    h->nodes[i+1].time = rxCompleteCmd.time;
    h->nodes[i+1].rx_length = rxCompleteCmd.rx_length;
    */

    h->nodes[i+1]=rxCompleteCmd;

  /*  for(j =0; j <rxCompleteCmd.rx_length; j++){
    	 h->nodes[i].rxFrameBuffer[j] = rxCompleteCmd.rxFrameBuffer[j];
    }*/
    h->len++;
}

void push (heap_t *h, OnboardTC* rxCompleteCmd) {
	pushaux(h, *rxCompleteCmd);
}

void pop(heap_t *h,  OnboardTC* rxCompleteCmd) {
    int i, j, k, inc;
    OnboardTC tc;

    if (!h->len) {
	//fprintf(stderr, "prioq empty\n");
	rxCompleteCmd = NULL;
	return;
    }
    /*
    rxCompleteCmd->time =  h->nodes[1].time;
    rxCompleteCmd->rx_length = h->nodes[1].rx_length;
    */
    tc=h->nodes[1];
    *rxCompleteCmd = tc;


    for (i=1;i<h->len;i++){
    	/*
    	h->nodes[i].time=h->nodes[i+1].time;
    	h->nodes[i].rx_length=h->nodes[i+1].rx_length;
    	*/

    	h->nodes[i] = h->nodes[i+1];
    }
    h->len--;




  /*  for(inc =0; inc <  h->nodes[1].rx_length; inc++){
          rxCompleteCmd->rxFrameBuffer[inc] =  h->nodes[1].rxFrameBuffer[inc];
    }*/

    /*
    h->nodes[1] = h->nodes[h->len];
    h->len--;
    i = 1;
    while (1) {
        k = i;
        j = 2 * i;
        if (j <= h->len && h->nodes[j].time < h->nodes[k].time) {
            k = j;
        }
        if (j + 1 <= h->len && h->nodes[j + 1].time < h->nodes[k].time) {
            k = j + 1;
        }
        if (k == i) {
            break;
        }
        h->nodes[i] = h->nodes[k];
        i = k;
    }
    h->nodes[i] = h->nodes[h->len + 1];
	*/
}

OnboardTC* top(heap_t *h){
	if (!h->len) {
	 // return 0;
		return NULL;
    	}
	//fprintf(stderr, " time at head %ld \n\r", h->nodes[1].time);
	return (&(h->nodes[1]));
	return (h->nodes[1].time);
}

void printQueue(heap_t *h){
	printf("Printing priority queue\n");
	printf("size: %d, length: %d\n", h->size, h->len);
	int i;
	for(i=1; i<h->len+1;i++){
		printf("node %d -- time: %ld, TCid:%d, taskID:%d\n",
			i,
			h->nodes[i].time,
			h->nodes[i].TCID,
			h->nodes[i].taskID
			);
	}
	printf("-------\n");
}
