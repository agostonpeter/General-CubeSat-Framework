#ifndef FRAMEWORK_H_
#define FRAMEWORK_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "priorityQueue_v3.h"		//prioq implementation
#include "frameworkDataStructs.h"

#define MAX_NUMBER_OF_TASKS 		20
#define MAX_NUMBER_OF_SUBSYSTEMS	5
#define MAX_NUMBER_OF_PAYLOADS 		10

#define MAX_DATA_SIZE_SUBSYSTEM		100
#define MAX_DATA_SIZE_COLLECTHK		100
#define MAX_DATA_SIZE_SENDTM		300
#define MAX_DATA_SIZE_PAYLOAD_POLL	512

#define FIFOCHANNEL_MAXDELAY portMAX_DELAY
#define EMPTY_FIFO_CHANNEL pdFALSE

uint8_t numberOfTasks;


//taskIDs of core tasks
uint8_t GetTCTaskID;
uint8_t SendTMTaskID;



//parameters for Initialization task
typedef struct _InitializationParameters {
	uint16_t taskID;
	uint16_t priority;
	uint8_t numberOfTasks;
	TaskDefinition * taskDefinitions;
	int16_t (*initialization)();
} InitializationParameters;



//parameters for GetTC task
typedef struct _GetTCParameters {
	uint16_t taskID;
	uint16_t priority;
	uint16_t period;
	int16_t (*receiveOnboard)(DynamicData * dataFromCS);
	int16_t (*decodeTC)(DynamicData * dataFromCS, OnboardTC * receivedTC);
} GetTCParameters;



//parameters for HandleCMD task
typedef struct _HandleCMDParameters {
	uint16_t taskID;
	uint16_t priority;
	uint16_t period;
} HandleCMDParameters;



//parameters for SendTM task
typedef struct _SendTMParameters {
	uint16_t taskID;
	uint16_t priority;
	int16_t (*readMemory)(OnboardTC * receivedTC, OnboardTM * dataToSend, uint8_t * moreData);
	int16_t (*encodeTM)(OnboardTC * receivedTC, OnboardTM * dataToSend, OnboardTM * encodedDataToSend);
	int16_t (*sendOnboard)(OnboardTM * encodedDataToSend);
} SendTMParameters;



//parameters for CollectHK task
typedef struct _ReceiveOnboard {
	uint8_t taskID;
	int16_t (*function)(OnboardTM * receivedData);
} ReceiveOnboard;

typedef struct _CollectHKParameters {
	uint16_t taskID;
	uint16_t priority;
	uint16_t period;
	int16_t (*initialization)();
	ReceiveOnboard * receiveOnboards;
	uint8_t numberOfReceiveOnboards;
	int16_t (*writeMemory) (OnboardTM * receivedData);
} CollectHKParameters;



//parameters for Subsystem tasks
typedef struct _SubsystemParameters {
	uint16_t taskID;
	uint16_t priority;
	uint16_t period;
	int16_t (*initialization)();
	int16_t (*receiveOnboard)(DynamicData * dataFromSensors);
	int16_t (*calculation)(DynamicData * dataFromSensors, DynamicData * dataToActuators);
	int16_t (*sendOnboard)(DynamicData * dataToActuators);
} SubsystemParameters;

//parameters for ControlPL task
typedef struct _ControlPLParameters {
	uint16_t taskID;
	uint16_t priority;
	int16_t (*startPL)(OnboardTC * receivedTC);
	int16_t (*checkTC)(OnboardTC * receivedTC, uint16_t * numberOfPolls, uint32_t * pollInterval);
	int16_t (*receiveOnboard)(OnboardTC * receivedTC, OnboardTM * PLData);
	int16_t (*writeMemory)(OnboardTC * receivedTC, OnboardTM * PLData);
	int16_t (*stopPL)(OnboardTC * receivedTC);
} ControlPLParameters;



//struct for queue handling
typedef struct _FrameworkQueue {
	uint8_t taskID;
	xQueueHandle queueHandle;  //TODO
} FrameworkQueue;



//queues
heap_t* prioq;

FrameworkQueue payloadQueues[MAX_NUMBER_OF_PAYLOADS];
uint8_t numberOfPayloadQueues;

#endif
