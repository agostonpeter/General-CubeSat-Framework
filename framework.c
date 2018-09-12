
OnboardTC fifochannel sendTMQueue;

task Initialization( void * vParameters) {
	InitializationParameters * sParameters = (InitializationParameters *) vParameters;
	printf("%lu\tID:0x%X\ttask started\n", gettime(ms), sParameters->taskID);

	numberOfTasks = sParameters->numberOfTasks;

	uint8_t numberOfInit = 0;
	uint8_t numberOfGetTC = 0;
	uint8_t numberOfHandleCMD = 0;
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

		case TaskTypeHandleCMD :
			numberOfHandleCMD++;
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

	NumberOfPayloadQueues = numberOfControlPL;

	int iQueue = 0;

	if (numberOfGetTC == 1 && numberOfHandleCMD == 1) {

		prioq = (heap_t *)calloc(1, sizeof(heap_t ));

	}
	else printf("not exectly 1 of GetTC and 1 HandleCMD tasks created\n");

	if (numberOfSendTM == 1) {

		cinit(sendTMQueue, OnboardTCVar);

	}
	else printf("not exactly 1 SendTM is created");

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


	sParameters->initialization();
	printf("%lu\tID:0x%X\ttask finished, deleting myself\n", gettime(ms), sParameters->taskID);

	//TODO delete task here
}


task HandleCMD(void * vParameters) {
	HandleCMDParameters * sParameters = (HandleCMDParameters *) vParameters;
	long timeVal;
	long delayUntilNextCMD;

	OnboardTC tc;
	OnboardTC *nextCmd;

	printf("%lu\tID:0x%X\ttask started\n", gettime(ms), sParameters->taskID);

	int i = 0;
	while (1) {
		printf("%lu\tID:0x%X\ttask running\n", gettime(ms), sParameters->taskID);

		if (prioq->len > 0) {

			nextCmd = top(prioq);

			timeVal = gettime(ms);

			if (nextCmd->time - timeVal > 0)
				delayUntilNextCMD = nextCmd->time - timeVal;
			else
				delayUntilNextCMD = 0;


			if (sParameters->period < delayUntilNextCMD ) {
				sdelay(sParameters->period, ms);
			}
			else {
				sdelay(delayUntilNextCMD);
				pop(prioq, &tc);

				if (tc.taskID == SendTMTaskID) {
					cwrite(sendTMQueue, tc);
				}
				else {
					for (i = 0; i < NumberOfPayloadQueues; i++) {
						if (payloadQueues[i].taskID == tc.taskID)
							break;
					}
					if (i==NumberOfPayloadQueues) 
						printf("can't find queue for %u task\n",sParameters->taskID);
					else 
						cwrite(payloadQueues[i].queueHandle, tc);

				}
			}
		}
		else
			sdelay(sParameters->period);
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

	uint16_t error = 0;

	sParameters->initialization();

	printf("%lu\tID:0x%X\ttask started\n", gettime(ms), sParameters->taskID);

	while (1) {
		printf("%lu\tID:0x%X\ttask running\n", gettime(ms), sParameters->taskID);

		error = 0;

		//read values from magnetometers
		error = sParameters->receiveOnboard(&dataFromSensors);

		//calculate
		if (!error) error = sParameters->calculation(&dataFromSensors, &dataToActuators);

		//send back results to subsystem
		if (!error) error = sParameters->sendOnboard(&dataToActuators);

		sdelay(sParameters->period, ms);
	}

}


task GetTC(void * vParameters) {

	GetTCParameters * sParameters = (GetTCParameters *) vParameters;

	spriority(sParameters->priority);

	uint16_t error = 0;

	DynamicData dataFromCS;
	uint8_t dataFromCSBuffer[MAX_DATA_SIZE_TC];
	dataFromCS.data = dataFromCSBuffer;
	dataFromCS.size = MAX_DATA_SIZE_TC;

	OnboardTC receivedTC;
	receivedTC.size = MAX_DATA_SIZE_TC;

	printf("%lu\tID:0x%X\ttask started\n", gettime(ms), sParameters->taskID);

	while (1) {
		printf("%lu\tID:0x%X\ttask running\n", gettime(ms), sParameters->taskID);

		error = sParameters->receiveOnboard(&dataFromCS);

		if (!error) error = sParameters->decodeTC(&dataFromCS, &receivedTC);

		if (!error)	push(prioq, &receivedTC);

		sdelay(sParameters->period, ms);
	}

}


task SendTM(void * vParameters) {
	SendTMParameters * sParameters  = (SendTMParameters *) vParameters;

	uint16_t error = 0;

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


	printf("%lu\tID:0x%X\ttask started\n", gettime(ms), sParameters->taskID);

	while (1) {
		printf("%lu\tID:0x%X\ttask running\n", gettime(ms), sParameters->taskID);

		error = cread(sendTMQueue, receivedTC);
		error ? (error = 0) : (error = 1); //invert error, because it returns 1 on success and 0 on error
		i = 0;
		do {
			printf("%lu\tID:0x%X\ttask sending TM %d\n", gettime(ms), sParameters->taskID, i);
			if (!error) error = sParameters->readMemory(&receivedTC, &dataToSend, &moreData);

			if (!error) error = sParameters->encodeTM(&receivedTC, &dataToSend, &encodedDataToSend);

			if (!error) error = sParameters->sendOnboard(&encodedDataToSend);

			i++;

		} while (moreData && !error);
		moreData = 0;
	}

}


task CollectHK(void * vParameters) {

	CollectHKParameters * sParameters = (CollectHKParameters *) vParameters;

	spriority(sParameters->priority);

	uint16_t error = 0;

	OnboardTM receivedData;
	uint8_t receivedDataBuffer[MAX_DATA_SIZE_COLLECTHK];
	receivedData.dataField.data = receivedDataBuffer;
	receivedData.dataField.size = MAX_DATA_SIZE_COLLECTHK;

	int i = 0;

	sParameters->initialization();

	printf("%lu\tID:0x%X\ttask started\n", gettime(ms), sParameters->taskID);
	while (1) {

		printf("%lu\tID:0x%X\ttask running\n", gettime(ms), sParameters->taskID);

		for (i = 0; i < sParameters->numberOfReceiveOnboards; i++) {

			error = sParameters->receiveOnboards[i].function(&receivedData);

			if (!error) error = sParameters->writeMemory(&receivedData);

			if (error) printf("error in %d. collectHK readMemory %u\n", i, error);

		}

		sdelay(sParameters->period, ms);

	}

}

task ControlPL(void * vParameters) {
	ControlPLParameters * sParameters = (ControlPLParameters *) vParameters;

	uint16_t numberOfPolls = 0;
	uint32_t pollInterval = 0;


	spriority(sParameters->priority);

	uint16_t error = 0;

	int i = 0;
	for (i = 0; i < NumberOfPayloadQueues; i++) {
		if (payloadQueues[i].taskID == sParameters->taskID)
			break;
	}
	if (i==NumberOfPayloadQueues) printf("can't find queue for %u task\n",sParameters->taskID);

	OnboardTC receivedTC;
	receivedTC.size = MAX_DATA_SIZE_TC;

	OnboardTM PLData;
	uint8_t PLDataBuffer[MAX_DATA_SIZE_PAYLOAD_POLL];
	PLData.dataField.data = PLDataBuffer;
	PLData.dataField.size = MAX_DATA_SIZE_PAYLOAD_POLL;

	printf("%lu\tID:0x%X\ttask started\n", gettime(ms), sParameters->taskID);

	while (1) {
		printf("%lu\tID:0x%X\ttask ready to start\n", gettime(ms), sParameters->taskID);

		error = cread(payloadQueues[i].queueHandle, receivedTC);
		error ? (error = 0) : (error = 1); //invert error, because it returns 1 on success and 0 on error
		error = sParameters->startPL(&receivedTC);

		while (!error) {
			printf("%lu\tID:0x%X\ttask ready for commands\n", gettime(ms), sParameters->taskID);

			error = cread(payloadQueues[i].queueHandle, receivedTC);
			error ? (error = 0) : (error = 1); //invert error, because it returns 1 on success and 0 on error
			error = sParameters->checkTC(&receivedTC, &numberOfPolls, &pollInterval);

			if (!error) {
				int j = 0;
				while (nelem(payloadQueues[i].queueHandle) == 0  && j < numberOfPolls) {
					printf("%lu\tID:0x%X\ttask running %d/%d\n", gettime(ms), sParameters->taskID, j + 1, numberOfPolls);
					PLData.dataField.size = MAX_DATA_SIZE_PAYLOAD_POLL;
					error = sParameters->receiveOnboard(&receivedTC, &PLData);

					error = sParameters->writeMemory(&receivedTC, &PLData);

					j++;
					if (j < numberOfPolls) sdelay(pollInterval, ms);
				}
			}
			else {
				sParameters->stopPL(&receivedTC);
				printf("%lu\tID:0x%X\ttask stopped\n", gettime(ms), sParameters->taskID);

			}
		}

	}
}


