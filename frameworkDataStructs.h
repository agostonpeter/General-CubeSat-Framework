#ifndef FRAMEWORKDATASTRUCTS_H_
#define FRAMEWORKDATASTRUCTS_H_

#define MAX_DATA_SIZE_TC 50 

#include <stdint.h>

typedef enum _FrameworkTaskType{
	TaskTypeInitialization = 0,
	TaskTypeGetTC = 1,
	TaskTypeHandleCMD = 2,
	TaskTypeSendTM = 3,
	TaskTypeCollectHK = 10,
	TaskTypeSubsystem = 20,
	TaskTypeControlPL = 30
}FrameworkTaskType;

typedef struct _TaskDefinition{
	FrameworkTaskType taskType;
	uint8_t taskID;
}TaskDefinition;

typedef struct _DynamicData{
	uint8_t * data;
	uint16_t size;
}DynamicData;

typedef struct _OnboardTC{
	uint16_t TCID;
	uint16_t taskID;
	uint32_t time;
	uint8_t data[MAX_DATA_SIZE_TC];
	uint16_t size;
}OnboardTC;

typedef struct _OnboardTM{
	uint16_t TCID;
	uint16_t taskID;
	DynamicData dataField;
}OnboardTM;



#endif /* FRAMEWORKDATASTRUCTS_H_ */
