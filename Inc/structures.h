/*
 * structures.h
 *
 *  Created on: 20 ���. 2019 �.
 *      Author: ADiKo
 */

#ifndef STRUCTURES_H_
#define STRUCTURES_H_

#include "globals.h"


#define	STATUS_NORMAL				0
#define	STATUS_ERROR_OVERCURRENT	1
#define	STATUS_ERROR_VALVE			2



typedef struct{
	uint8_t  rfChannel;
	uint8_t  rere;
	uint16_t clientID;
	float impUpCoeff[4];
	float impDownCoeff[4];
	uint16_t offsetPressure[4];
	uint32_t writeCounter;
} ControllerSettings;

typedef struct{
	uint8_t version;
	uint8_t settingsAddress;
	uint16_t connectionNum;
	uint32_t turnOnNum;
	uint32_t aligningNum;
	uint32_t errorAligningNum;
}ControllerInfo;


typedef enum{
	COMPENSATION_STATE_FREE,
	COMPENSATION_STATE_WORKING
}CompensationWorkState;

typedef enum{
	COMPENSATION_OFF,
	COMPENSATION_ON,
}PressureCompensation;

typedef enum{
	NORMAL_INDICATION,
	SEARCH_INDICATION
}SoundIndicationState;


typedef enum{
	COMPRESSOR,
	RECEIVER
}AirSystemType;

//typedef enum{
//	STATUS_NORMAL,
//	STATUS_ERROR,
//	STATUS_ERROR_OVERCURRENT,
//	STATUS_ERROR_VALVE
//} ControllerErrorStatus;

typedef struct{
	CompensationWorkState analyzeState;
	PressureCompensation pressureCompensation;
	PressureCompensation prevPressureCompensation;
	SoundIndicationState soundIndicationState;
	AirSystemType airSystemType;
	//ControllerErrorStatus errorStatus;
	uint8_t errorStatus;
	uint16_t nessPressure[4];
	uint16_t filteredData[4];
	uint16_t serverUID;
	uint8_t errorByte;
	uint8_t errorMeaningByte;
	uint8_t waysType;
	uint8_t analyzeAccuracy;
	uint8_t status;
	uint8_t numberOfTries;
	uint16_t lastTimeCommand;
	uint32_t compressorWorkTime;
	uint16_t currentSensVoltage;
} PressureControllerState;


extern PressureControllerState controllerState;
extern volatile ControllerSettings controllerData;
extern ControllerInfo controllerInfo;

#endif /* STRUCTURES_H_ */
