/*!
    @file framework.h

    @brief The global variables and the task parameter structs of the framework.

*/


#ifndef FRAMEWORK_H_
#define FRAMEWORK_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "priorityQueue_v3.h"
#include "frameworkDataStructs.h"

#define PRINT_LOG   1       //!<If !=0 then logs are printed.

#define MAX_NUMBER_OF_TASKS         20      //!<A limit for the maximum number of tasks that can be created.
#define MAX_NUMBER_OF_SUBSYSTEMS    5       //!<A limit for the maximum number of Subsystem tasks that can be created.
#define MAX_NUMBER_OF_PAYLOADS      10      //!<A limit for the maximum number of ControlPL tasks than can be created.



uint8_t numberOfTasks;  //!<The total number of tasks.


//taskIDs of core tasks
uint8_t GetTCTaskID;
uint8_t SendTMTaskID;

/*!
    \brief Callback function that returns the actual time for logging the run of the program.
    \return The actual time when the function is called.
 */
uint32_t (*timeForPrint)(void); 

/*!
    Input parameters for the Initialization task.
*/
typedef struct _InitializationParameters {
    uint16_t taskID;                        //!<The ID of the task.
    uint16_t priority;                      //!<The priority of the task.
    uint8_t numberOfTasks;                  //!<The total number of thasks created in the framework (including Initialization task).
    TaskDefinition * taskDefinitions;       //!<An array of the task definitions.
    /*!
    \brief The callback function for the application specific initializations.
    \return Error code.
    */
    int16_t (*initialization)(void);
} InitializationParameters;



/*!
    Input parameters for the GetTC task.
*/
typedef struct _GetTCParameters {
    uint16_t taskID;        //!<The ID of the task.
    uint16_t priority;      //!<The priority of the task.
    uint16_t period;        //!<The period of the task [ms].

    /*!
    \brief The callback function for the task specific initializations.
    \return 0 if no error
    \return else error code.
    */
    int16_t (*initialization)(void);

    /*!
    \brief The callback function for receiving the telecommand from the communication subsystem.
    \param[out] dataFromCS Pointer to the struct where the telecommnad shall be saved.
    \return 0 on successful telecommand retrive
    \return GETTC_NO_TC_TO_RETRIEVE if no telecommand to be retrieved
    \return else error code.
    */
    int16_t (*receiveOnboard)(DynamicData * dataFromCS);

    /*!
    \brief The callback function for decoding the telecommand.
    \param[in] dataFromCS Pointer to the received telecommand in the application specific format.
    \param[out] receivedTC Pointer to the receivedTC in the on-board format.
    \return 0 if no error
    \return else error code.
    */
    int16_t (*decodeTC)(DynamicData * dataFromCS, OnboardTC * receivedTC);
} GetTCParameters;



/*!
    Input parameters for the HandleTCs task.
*/
typedef struct _HandleTCsParameters {
    uint16_t taskID;        //!<The ID of the task.
    uint16_t priority;      //!<The priority of the task.
    uint16_t threshold;     //!<The threshold time to check the priority queue for new telecommands [ms].
} HandleTCsParameters;



/*!
    Input parameters for the SendTM task.
*/
typedef struct _SendTMParameters {
    uint16_t taskID;        //!<The ID of the task.
    uint16_t priority;      //!<The priority of the task.

    /*!
    \brief The callback function for the task specific initializations.
    \return 0 if no error
    \return else error code.
    */
    int16_t (*initialization)(void);

    /*!
    \brief The callback function for reading the data that will be sent from the memory.
    \param[in] receivedTC Pointer to the received telecommand that triggered the current running of 
                the task.
    \param[out] dataToSend Pointer to raw the data retrived from the memory.
    \param[out] moreData If 0 then there is no more data to send, the execution of the task will be 
                suspended after the currentl retrieved data has been sent.
    \return 0 if no error
    \return else error code.
    */
    int16_t (*readMemory)(OnboardTC * receivedTC, OnboardTM * dataToSend, uint8_t * moreData);

    /*!
    \brief The callback function for encoding the telemetry for the application specific format.
    \param[in] receivedTC Pointer to the received telecommand that triggered the current running of 
                the task.
    \param[in] dataToSend Pointer to the raw data retrived from the memory.
    \param[out] encodedDataToSend Pointer to the telemetry where the data member contains the 
                encoded telemetry to the application specific format.
    \return 0 if no error
    \return else error code.
    */
    int16_t (*encodeTM)(OnboardTC * receivedTC, OnboardTM * dataToSend, OnboardTM * encodedDataToSend);

    /*!
    \brief The callback function for sending the telemetry to the communication system.
    \param[in] encodedDataToSend Pointer to the telemetry where the data member contains the 
                encoded telemetry to the application specific format.
    \return 0 if no error
    \return else error code.
    */
    int16_t (*sendOnboard)(OnboardTM * encodedDataToSend);
} SendTMParameters;



/*!
    Struct to handle the receiveOnboard functions in the CollectHK task.
*/
typedef struct _ReceiveOnboard {
    uint8_t taskID;     //!<ID of the task that handles the unit from where the function collects housekeeping data.

    /*!
    \brief The callback function for receiving housekeepig data from a unit.
    \param[out] receivedData Pointer to the struct where the received data shall be stored.
    \return 0 if no error
    \return else error code.
    */
    int16_t (*function)(OnboardTM * receivedData);
} ReceiveOnboard;

/*!
    Input parameters for the CollectHK task.
*/
typedef struct _CollectHKParameters {
    uint16_t taskID;        //!<The ID of the task.
    uint16_t priority;      //!<The priority of the task.
    uint16_t period;        //!<The peroid of the task [ms].

    /*!
    \brief The callback function for the task specific initializations.
    \return 0 if no error
    \return else error code.
    */
    int16_t (*initialization)(void);

    ReceiveOnboard * receiveOnboards;       //!<An array of receiveOnboards from where all functions will be called in every running of the task.
    uint8_t numberOfReceiveOnboards;        //!<Number of the receiveOnboards in the array.

    /*!
    \brief The callback function for writing data to the non-volatile memory.
    \param[in] receivedData Pointer to the struct from where the data should be copied to the non-volatile memory.
    \return 0 if no error
    \return else error code.
    */
    int16_t (*writeMemory) (OnboardTM * receivedData);
} CollectHKParameters;



/*!
    Input parameters for the Subsystem task.
*/
typedef struct _SubsystemParameters {
    uint16_t taskID;        //!<The ID of the task.
    uint16_t priority;      //!<The priority of the task.
    uint16_t period;        //!<The peroid of the task [ms].

    /*!
    \brief The callback function for the task specific initializations.
    \return 0 if no error
    \return else error code.
    */
    int16_t (*initialization)(void);

    /*!
    \brief The callback function for receiving data from the subsystem sensors.
    \param[out] dataFromSensors Pointer to the struct where the received sensor data shall be stored.
    \return 0 if no error
    \return else error code.
    */
    int16_t (*receiveOnboard)(DynamicData * dataFromSensors);

    /*!
    \brief The callback function for the calculations of the subsystem.
    \param[in] dataFromSensors Pointer to the struct where the received sensor data is stored.
    \param[out] dataToActuators Pointer to the struct where the results of the calculations shall be stored.
    \return 0 if no error
    \return else error code.
    */
    int16_t (*calculation)(DynamicData * dataFromSensors, DynamicData * dataToActuators);

    /*!
    \brief The callback function for sending the results to the subsystem.
    \param[in] dataToActuators Pointer to the struct where the results of the calculations is stored.
    \return 0 if no error
    \return else error code.
    */
    int16_t (*sendOnboard)(DynamicData * dataToActuators);
} SubsystemParameters;

/*!
    Input parameters for the ControlPL task.
*/
typedef struct _ControlPLParameters {
    uint16_t taskID;        //!<The ID of the task.
    uint16_t priority;      //!<The priority of the task.

    /*!
    \brief The callback function for the task specific initializations.
    \return 0 if no error
    \return else error code.
    */
    int16_t (*initialization)(void);

    /*!
    \brief The callback function for starting the payload.
    \param[in] receivedTC Pointer to the received telecommand that triggered the current running of 
                the task.
    \return 0 if no error
    \return else error code.
    */
    int16_t (*startPL)(OnboardTC * receivedTC);

    /*!
    \brief The callback function for checking the received telecommand and setting the polling parameters.
    \param[in] receivedTC Pointer to the received telecommand that triggered the current running of 
                the task.
    \param[out] numberOfPolls The number of how many time will be the polling callback functions 
                called. Set it to 1 for a single execution without any delay. If set to 0 the 
                polling callback functions are not called.
    \param[out] pollInterval The interval for the telemetry collection [ms]. This will be ignored if
                numberOfPolls is 1 or 0.
    \return 0 if telemetry collection should be started, or if payload should not yet be stopped.
    \return CONTROLPL_STOP_PL if payload shall be stopped.
    \return else error code.

    \note Payload will be always stopped in case of an error.
    */
    int16_t (*checkTC)(OnboardTC * receivedTC, uint16_t * numberOfPolls, uint32_t * pollInterval);

    /*!
    \brief The callback function for receiving data from the payload.
    \param[in] receivedTC Pointer to the received telecommand that triggered the current running of 
                the telemetry collection.
    \param[out] PLData Pointer to the struct where the received data shall be stored.
    \return 0 if no error
    \return else error code.
    */
    int16_t (*receiveOnboard)(OnboardTC * receivedTC, OnboardTM * PLData);

    /*!
    \brief The callback function for writing data to the non-volatile memory.
    \param[in] receivedTC Pointer to the received telecommand that triggered the current running of 
                the telemetry collection.
    \param[in] PLData Pointer to the struct from where the data should be copied to the non-volatile
                memory.
    \return 0 if no error
    \return else error code.
    */
    int16_t (*writeMemory)(OnboardTC * receivedTC, OnboardTM * PLData);

    /*!
    \brief The callback function for stopping the payload.
    \param[in] receivedTC Pointer to the last received telecommand.
    \return 0 if no error
    \return else error code.
    */
    int16_t (*stopPL)(OnboardTC * receivedTC);
} ControlPLParameters;



/*!
    The handle of a fifochannel and the ID of the task where it is used.
*/
typedef struct _FrameworkQueue {
    uint8_t taskID;                 //!<The ID of the task where the fifochannel is used.
    xQueueHandle queueHandle;       //!<The handle of the fifochannel. (TODO)
} FrameworkQueue;



//queues
heap_t* prioq;

FrameworkQueue payloadQueues[MAX_NUMBER_OF_PAYLOADS];
uint8_t numberOfPayloadQueues;

#endif
