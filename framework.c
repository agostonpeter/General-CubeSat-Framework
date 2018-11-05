#include "framework.h"

OnboardTC fifochannel sendTMQueue;

task Initialization( void * vParameters) {
    int16_t error = 0;
    InitializationParameters * sParameters = (InitializationParameters *) vParameters;
    if (PRINT_LOG) printf("%lu\tID:0x%X\ttask started\n", timeForPrint(), sParameters->taskID);

    numberOfTasks = sParameters->numberOfTasks;

    if (numberOfTasks > MAX_NUMBER_OF_TASKS)
        printf("number of tasks to be created (%u) larger than the maximum (%u)\n",
               numberOfTasks,
               MAX_NUMBER_OF_TASKS);

    uint8_t numberOfInit = 0;
    uint8_t numberOfGetTC = 0;
    uint8_t numberOfHandleTCs = 0;
    uint8_t numberOfSendTM = 0;
    uint8_t numberOfCollectHK = 0;
    uint8_t numberOfSubsystem = 0;
    uint8_t numberOfControlPL = 0;

    // Included for fifochannel
    OnboardTC OnboardTCVar;


    for (int i = 0; i < sParameters->numberOfTasks; i++) {
        switch (sParameters->taskDefinitions[i].taskType) {
        case TaskTypeInitialization :
            numberOfInit++;
            break;

        case TaskTypeGetTC :
            numberOfGetTC++;
            GetTCTaskID = sParameters->taskDefinitions[i].taskID;
            break;

        case TaskTypeHandleTCs :
            numberOfHandleTCs++;
            break;

        case TaskTypeSendTM :
            numberOfSendTM++;
            SendTMTaskID = sParameters->taskDefinitions[i].taskID;
            break;

        case TaskTypeCollectHK :
            numberOfCollectHK++;
            break;

        case TaskTypeSubsystem :
            numberOfSubsystem++;
            break;

        case TaskTypeControlPL :
            numberOfControlPL++;
            break;

        default :
            printf("invalid TaskType in initfunction\n");
        }
    }

    if (numberOfSubsystem > MAX_NUMBER_OF_SUBSYSTEMS)
        printf("number of subsystems to be created (%u) is larger than the maximum (%u)\n",
               numberOfSubsystem,
               MAX_NUMBER_OF_SUBSYSTEMS);

    if (numberOfControlPL > MAX_NUMBER_OF_PAYLOADS)
        printf("number of payloads to be created (%u) is larger than the maximum (%u)\n",
               numberOfControlPL,
               MAX_NUMBER_OF_PAYLOADS);

    numberOfPayloadQueues = numberOfControlPL;

    int iQueue = 0;

    if (numberOfGetTC == 1 && numberOfHandleTCs == 1) {

        prioq = (heap_t *)calloc(1, sizeof(heap_t ));

    }
    else printf("not exectly 1 of GetTC and 1 HandleTCs tasks have been created\n");

    if (numberOfSendTM == 1) {

        cinit(sendTMQueue, OnboardTCVar);

    }
    else printf("not exactly 1 SendTM has been created\n");

    int j = 0;
    for (int i = 0; i < numberOfControlPL; i++) {

        while (sParameters->taskDefinitions[j].taskType != TaskTypeControlPL) {
            j++;
        }

        cinit(payloadQueues[iQueue].queueHandle, OnboardTCVar);

        payloadQueues[iQueue].taskID = sParameters->taskDefinitions[j].taskID;

        j++;
        iQueue++;

    }


    error = sParameters->initialization();
    if (error) printf("error in ID:0x%X initialization function: %d\n", sParameters->taskID, error);
    if (PRINT_LOG) printf("%lu\tID:0x%X\ttask finished, deleting myself\n",
                              timeForPrint(),
                              sParameters->taskID);

    //TODO delete task here
    return 0;
}

task GetTC(void * vParameters) {

    GetTCParameters * sParameters = (GetTCParameters *) vParameters;

    spriority(sParameters->priority);

    int16_t error = 0;

    DynamicData dataFromCS;
    uint8_t dataFromCSBuffer[MAX_DATA_SIZE_TC];
    dataFromCS.data = dataFromCSBuffer;
    dataFromCS.size = MAX_DATA_SIZE_TC;

    OnboardTC receivedTC;
    receivedTC.size = MAX_DATA_SIZE_TC;

    error = sParameters->initialization();
    if (error) printf("error in ID:0x%X initialization function: %d\n", sParameters->taskID, error);

    if (PRINT_LOG) printf("%lu\tID:0x%X\ttask started\n", timeForPrint(), sParameters->taskID);

    while (1) {
        if (PRINT_LOG) printf("%lu\tID:0x%X\ttask running\n", timeForPrint(), sParameters->taskID);

        error = sParameters->receiveOnboard(&dataFromCS);

        if (!error) error = sParameters->decodeTC(&dataFromCS, &receivedTC);

        if (!error) push(prioq, &receivedTC);

        if (error && error != GETTC_NO_TC_TO_RETRIEVE) printf("error in GetTC: %d\n", error);

        if (PRINT_LOG) {
            if (!error) printf("%lu\tID:0x%X\ttask TC successfully received\n",
                                   timeForPrint(),
                                   sParameters->taskID);
            else if (error == GETTC_NO_TC_TO_RETRIEVE) printf("%lu\tID:0x%X\ttask no TC received\n",
                        timeForPrint(),
                        sParameters->taskID);
        }

        sdelay(sParameters->period, ms);
    }

}


task HandleTCs(void * vParameters) {
    HandleTCsParameters * sParameters = (HandleTCsParameters *) vParameters;
    uint64_t timeValSec;
    uint64_t timeVal;
    uint64_t delayUntilNextTC;
    uint64_t timeOfNextTC;

    OnboardTC tc;
    OnboardTC *nextTC;

    if (PRINT_LOG) printf("%lu\tID:0x%X\ttask started\n", timeForPrint(), sParameters->taskID);

    int i = 0;
    while (1) {
        if (PRINT_LOG) printf("%lu\tID:0x%X\ttask running\n", timeForPrint(), sParameters->taskID);

        if (prioq->len > 0) {

            nextTC = top(prioq);
            timeOfNextTC = (uint64_t) nextTC->time * 1000; //convert from sec to ms

            timeValSec = gettime(sec);
            timeVal = timeValSec * 1000; //convert from sec to ms

            if (timeOfNextTC > timeVal)
                delayUntilNextTC = timeOfNextTC - timeVal;
            else
                delayUntilNextTC = 0;


            if (sParameters->threshold < delayUntilNextTC ) {
                sdelay(sParameters->threshold, ms);
            }
            else {
                sdelay(delayUntilNextTC, ms);
                pop(prioq, &tc);

                if (tc.taskID == SendTMTaskID) {
                    cwrite(sendTMQueue, tc);
                }
                else {
                    for (i = 0; i < numberOfPayloadQueues; i++) {
                        if (payloadQueues[i].taskID == tc.taskID)
                            break;
                    }
                    if (i == numberOfPayloadQueues)
                        printf("can't find queue for %u task\n", sParameters->taskID);
                    else
                        cwrite(payloadQueues[i].queueHandle, tc);

                }
            }
        }
        else
            sdelay(sParameters->threshold, ms);
    }

}

task SendTM(void * vParameters) {
    SendTMParameters * sParameters  = (SendTMParameters *) vParameters;

    int16_t error = 0;

    uint8_t moreData = 0;

    int i = 0;

    spriority(sParameters->priority);

    OnboardTC receivedTC;
    receivedTC.size = MAX_DATA_SIZE_TC;

    OnboardTM dataToSend;
    uint8_t dataToSendBuffer[MAX_DATA_SIZE_SENDTM];
    dataToSend.dataField.data = dataToSendBuffer;
    dataToSend.dataField.size = MAX_DATA_SIZE_SENDTM;

    OnboardTM encodedDataToSend;
    uint8_t encodedDataToSendBuffer[MAX_DATA_SIZE_SENDTM];
    encodedDataToSend.dataField.data = encodedDataToSendBuffer;
    encodedDataToSend.dataField.size = MAX_DATA_SIZE_SENDTM;

    error = sParameters->initialization();
    if (error) printf("error in ID:0x%X initialization function: %d\n", sParameters->taskID, error);

    if (PRINT_LOG) printf("%lu\tID:0x%X\ttask started\n", timeForPrint(), sParameters->taskID);

    while (1) {
        if (PRINT_LOG) printf("%lu\tID:0x%X\ttask ready for commands\n",
                                  timeForPrint(),
                                  sParameters->taskID);
        error = 0;
        cread(sendTMQueue, receivedTC);
        if (PRINT_LOG) printf("%lu\tID:0x%X\ttask running\n", timeForPrint(), sParameters->taskID);
        i = 0;
        do {
            if (!error) error = sParameters->readMemory(&receivedTC, &dataToSend, &moreData);

            if (!error) error = sParameters->encodeTM(&receivedTC, &dataToSend, &encodedDataToSend);

            if (!error) error = sParameters->sendOnboard(&encodedDataToSend);

            if (!error && PRINT_LOG) printf("%lu\tID:0x%X\ttask TM %d sent\n",
                                                timeForPrint(),
                                                sParameters->taskID, i);

            if (error) printf("error in SendTM: %d\n", error);

            i++;

        } while (moreData && !error && nelem(sendTMQueue) == 0);
        moreData = 0;
    }

}

task CollectHK(void * vParameters) {

    CollectHKParameters * sParameters = (CollectHKParameters *) vParameters;

    spriority(sParameters->priority);

    int16_t error = 0;

    OnboardTM receivedData;
    uint8_t receivedDataBuffer[MAX_DATA_SIZE_COLLECTHK];
    receivedData.dataField.data = receivedDataBuffer;
    receivedData.dataField.size = MAX_DATA_SIZE_COLLECTHK;

    int i = 0;

    error = sParameters->initialization();
    if (error) printf("error in ID:0x%X initialization function: %d\n", sParameters->taskID, error);

    if (PRINT_LOG) printf("%lu\tID:0x%X\ttask started\n", timeForPrint(), sParameters->taskID);
    while (1) {

        for (i = 0; i < sParameters->numberOfReceiveOnboards; i++) {
            if (PRINT_LOG) printf("%lu\tID:0x%X\ttask running %d/%d\n",
                                      timeForPrint(),
                                      sParameters->taskID,
                                      i + 1,
                                      sParameters->numberOfReceiveOnboards);

            error = sParameters->receiveOnboards[i].function(&receivedData);

            if (!error) error = sParameters->writeMemory(&receivedData);

            if (error) printf("error in %d. CollectHK readMemory %d\n", i, error);

        }

        sdelay(sParameters->period, ms);

    }

}

task Subsystem( void * vParameters) {

    SubsystemParameters * sParameters = (SubsystemParameters *) vParameters;

    spriority(sParameters->priority);

    DynamicData dataFromSensors;
    uint8_t dataFromSensorsBuffer[MAX_DATA_SIZE_SUBSYSTEM];
    dataFromSensors.data = dataFromSensorsBuffer;
    dataFromSensors.size = MAX_DATA_SIZE_SUBSYSTEM;

    DynamicData dataToActuators;
    uint8_t dataToActuatorsBuffer[MAX_DATA_SIZE_SUBSYSTEM];
    dataToActuators.data = dataToActuatorsBuffer;
    dataToActuators.size = MAX_DATA_SIZE_SUBSYSTEM;

    int16_t error = 0;

    error = sParameters->initialization();
    if (error) printf("error in ID:0x%X initialization function: %d\n", sParameters->taskID, error);

    if (PRINT_LOG) printf("%lu\tID:0x%X\ttask started\n", timeForPrint(), sParameters->taskID);

    while (1) {
        if (PRINT_LOG) printf("%lu\tID:0x%X\ttask running\n", timeForPrint(), sParameters->taskID);

        error = sParameters->receiveOnboard(&dataFromSensors);

        if (!error) error = sParameters->calculation(&dataFromSensors, &dataToActuators);

        if (!error) error = sParameters->sendOnboard(&dataToActuators);

        if (error) printf("error in ID:0x%X Subsystem: %d\n", sParameters->taskID, error);

        sdelay(sParameters->period, ms);
    }

}

task ControlPL(void * vParameters) {
    ControlPLParameters * sParameters = (ControlPLParameters *) vParameters;

    uint16_t numberOfPolls = 0;
    uint32_t pollInterval = 0;


    spriority(sParameters->priority);

    int16_t error = 0;

    int queueNumber = 0;
    for (queueNumber = 0; queueNumber < numberOfPayloadQueues; queueNumber++) {
        if (payloadQueues[queueNumber].taskID == sParameters->taskID)
            break;
    }
    if (queueNumber == numberOfPayloadQueues) printf("can't find queue for %u task\n", sParameters->taskID);

    OnboardTC receivedTC;
    receivedTC.size = MAX_DATA_SIZE_TC;

    OnboardTM PLData;
    uint8_t PLDataBuffer[MAX_DATA_SIZE_PAYLOAD_POLL];
    PLData.dataField.data = PLDataBuffer;
    PLData.dataField.size = MAX_DATA_SIZE_PAYLOAD_POLL;

    error = sParameters->initialization();
    if (error) printf("error in ID:0x%X initialization function: %d\n", sParameters->taskID, error);

    if (PRINT_LOG) printf("%lu\tID:0x%X\ttask started\n", timeForPrint(), sParameters->taskID);

    while (1) {
        if (PRINT_LOG) printf("%lu\tID:0x%X\ttask ready to start\n",
                                  timeForPrint(),
                                  sParameters->taskID);
        error = 0;
        cread(payloadQueues[queueNumber].queueHandle, receivedTC);
        error = sParameters->startPL(&receivedTC);

        if (error) printf("error in ID:0x%X ControlPL-starting: %d\n", sParameters->taskID, error);

        while (!error) {
            if (PRINT_LOG) printf("%lu\tID:0x%X\ttask ready for commands\n",
                                      timeForPrint(),
                                      sParameters->taskID);

            cread(payloadQueues[queueNumber].queueHandle, receivedTC);
            gettime(sec);
            error = sParameters->checkTC(&receivedTC, &numberOfPolls, &pollInterval);


            uint16_t i = 0;
            while (!error && nelem(payloadQueues[queueNumber].queueHandle) == 0  && i < numberOfPolls) {

                if (PRINT_LOG) printf("%lu\tID:0x%X\ttask running %d/%d\n",
                                          timeForPrint(),
                                          sParameters->taskID,
                                          i + 1,
                                          numberOfPolls);

                PLData.dataField.size = MAX_DATA_SIZE_PAYLOAD_POLL;
                error = sParameters->receiveOnboard(&receivedTC, &PLData);

                if (!error) error = sParameters->writeMemory(&receivedTC, &PLData);

                if (error) printf("error in ID:0x%X ControlPL-polling: %d\n",
                                      sParameters->taskID,
                                      error);

                i++;
                if (i < numberOfPolls && !error) sdelay(pollInterval, ms);
            }


            if (error) {
                if (error != CONTROLPL_STOP_PL)
                    printf("ID:0x%X task stopped because of an error: %d\n",
                           sParameters->taskID,
                           error);

                error = sParameters->stopPL(&receivedTC);
                
                if (PRINT_LOG)
                    printf("%lu\tID:0x%X\ttask stopped\n",
                           timeForPrint(),
                           sParameters->taskID);

            }
        }

    }

}


