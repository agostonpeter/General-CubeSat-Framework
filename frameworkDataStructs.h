/*!
    @file frameworkDataStructs.h

    @brief The data structs that are used in the framework.

*/


#ifndef FRAMEWORKDATASTRUCTS_H_
#define FRAMEWORKDATASTRUCTS_H_

#define MAX_DATA_SIZE_TC            50      //!<Maximum size of the telecommand [bytes].

#define MAX_DATA_SIZE_SUBSYSTEM     100     //!<Maximum size of the data handled in the Subsystem task [bytes].
#define MAX_DATA_SIZE_COLLECTHK     100     //!<Maximum size of the data handled in the CollectHK task [bytes].
#define MAX_DATA_SIZE_SENDTM        300     //!<Maximum size of the telemetry handled in the SendTM task [bytes].
#define MAX_DATA_SIZE_PAYLOAD_POLL  512     //!<Maximum size of the data handled in one poll of a payload [bytes].

#include <stdint.h>

/*!
    Enumeration of the task types.
*/
typedef enum _FrameworkTaskType {
    TaskTypeInitialization =    0,      //!<Initialization task.
    TaskTypeGetTC =             1,      //!<GetTC task.
    TaskTypeHandleTCs =         2,      //!<HandleTCs task.
    TaskTypeSendTM =            3,      //!<SendTM task.
    TaskTypeCollectHK =         10,     //!<CollectHK task.
    TaskTypeSubsystem =         20,     //!<Subsystem task.
    TaskTypeControlPL =         30      //!<ControlPL task.
} FrameworkTaskType;

/*!
    The type and the ID of a task in one struct. An array of taskDefinitions shall be used to define
    all the tasks for the Initialization task of the framework.
*/
typedef struct _TaskDefinition {
    FrameworkTaskType taskType;     //!<The type of the task.
    uint8_t taskID;                 //!<The ID of the task.
} TaskDefinition;

/*!
    Data with customizable size.
*/
typedef struct _DynamicData {
    uint8_t * data;     //!<Pointer to the first byte of the data.
    uint16_t size;      //!<Size of the data [bytes].
} DynamicData;

/*!
    Struct that is used to handle the telecommands on board. The whole telecommand used in the
    application can be save to the data member of the struct. 
*/
typedef struct _OnboardTC {
    uint16_t TCID;                      //!<ID of the telecommand.
    uint16_t taskID;                    //!<ID of the task where the telecommand shall be executed.
    uint32_t time;                      //!<The time when the telecommand shall be executed.
    uint8_t data[MAX_DATA_SIZE_TC];     //!<The whole telecommand in the application or other data.
    uint16_t size;                      //!<Size of the data member [bytes].
} OnboardTC;

/*!
    Struct that is used to handle the generated telemetries on board.
*/
typedef struct _OnboardTM {
    uint16_t TCID;              //!< The ID of the telecommand that has genereated the the telemetry (optional).
    uint16_t taskID;            //!< The ID of the task that has genereated the telemetry.
    DynamicData dataField;      //!< The data and its size of the telemetry.
} OnboardTM;

//defined return values
#define GETTC_NO_TC_TO_RETRIEVE     1   //!<Return value in GetTC task when there is no telecommand to retrieve.
#define CONTROLPL_STOP_PL           1   //!<Return value in ControlPL task when payload needs to be stopped.

#endif /* FRAMEWORKDATASTRUCTS_H_ */
