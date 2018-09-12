#ifndef FRAMEWORK_H_
#define FRAMEWORK_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "priorityQueue_v3.h"		//prioq implementation
#include "frameworkDataStructs.h"

#define MAX_NUMBER_OF_TASKS 	20
#define MAX_NUMBER_OF_PAYLOADS 	10

#define MAX_DATA_SIZE_SUBSYSTEM		100
#define MAX_DATA_SIZE_COLLECTHK		100
#define MAX_DATA_SIZE_SENDTM		300
#define MAX_DATA_SIZE_PAYLOAD_POLL	512

#define FIFOCHANNEL_MAXDELAY portMAX_DELAY
#define EMPTY_FIFO_CHANNEL pdFALSE

//parameters for Initialization task
typedef struct _InitializationParameters{
	uint16_t taskID;
	uint16_t priority;
	uint8_t numberOfTasks;
	TaskDefinition * taskDefinitions;
	uint16_t (*initialization)();
}InitializationParameters;

typedef struct _HandleCMDParameters{
	uint16_t taskID;
	uint16_t priority;
	uint16_t period;
}HandleCMDParameters;

//parameters for ADCS and EPS tasks
typedef struct _SubsystemParameters{
	uint16_t taskID;
	uint16_t priority;
	uint16_t period;
	uint16_t (*initialization)();
	uint16_t (*receiveOnboard)(DynamicData * dataFromSensors);
	uint16_t (*calculation)(DynamicData * dataFromSensors, DynamicData * dataToActuators);
	uint16_t (*sendOnboard)(DynamicData * dataToActuators);
}SubsystemParameters;


//parameters for GetTC task
typedef struct _GetTCParameters{
	uint16_t taskID;
	uint16_t priority;
	uint16_t period;
	uint16_t (*receiveOnboard)(DynamicData * dataFromCS);
	uint16_t (*decodeTC)(DynamicData * dataFromCS, OnboardTC * receivedTC);
}GetTCParameters;


//parameters for SendTM task

typedef struct _SendTMParameters{
	uint16_t taskID;
	uint16_t priority;
	uint16_t (*readMemory)(OnboardTC * receivedTC, OnboardTM * dataToSend, uint8_t * moreData);
	uint16_t (*encodeTM)(OnboardTC * receivedTC, OnboardTM * dataToSend, OnboardTM * encodedDataToSend);
	uint16_t (*sendOnboard)(OnboardTM * encodedDataToSend);
}SendTMParameters;


//parameters for CollectHK task
typedef struct _ReceiveOnboard{
	uint8_t taskID;
	uint16_t (*function)(OnboardTM * receivedData);	
}ReceiveOnboard;

typedef struct _CollectHKParameters{
	uint16_t taskID;
	uint16_t priority;
	uint16_t period;
	uint16_t (*initialization)();
	ReceiveOnboard * receiveOnboards;
	uint8_t numberOfReceiveOnboards;
	uint16_t (*writeMemory) (OnboardTM * receivedData);	
}CollectHKParameters;

//parameters for ControlPL task
typedef struct _ControlPLParameters{
	uint16_t taskID;
	uint16_t priority;
	uint16_t (*startPL)(OnboardTC * receivedTC);
	uint16_t (*checkTC)(OnboardTC * receivedTC, uint16_t * numberOfPolls, uint32_t * pollInterval);
	uint16_t (*receiveOnboard)(OnboardTC * receivedTC, OnboardTM * PLData);
	uint16_t (*writeMemory)(OnboardTC * receivedTC, OnboardTM * PLData);
	uint16_t (*stopPL)(OnboardTC * receivedTC);
}ControlPLParameters;

typedef struct _FrameworkQueue{
	uint8_t taskID;
	xQueueHandle queueHandle;	
}FrameworkQueue;

uint8_t numberOfTasks;

uint8_t GetTCTaskID;
uint8_t SendTMTaskID;

//queues
heap_t* prioq;

FrameworkQueue payloadQueues[MAX_NUMBER_OF_PAYLOADS];
uint8_t NumberOfPayloadQueues;

#endif
