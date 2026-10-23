/*
 * ConfigLoader.h
 *
 *  Created on: May 18, 2026
 *      Author: giova
 */

#ifndef CONFIG_LOADER_H
#define CONFIG_LOADER_H

#include "main.h"
#include "SDCard.hh"
#include "FlashStorage.h"

struct ConfigParams
{
	// Temporizador dos estados
	uint32_t baroSampleTime;
	uint32_t flightSyncTime;
	uint32_t liftoffLogTime;
	uint32_t liftoffTxTime;
	uint32_t flightLogTime;
	uint32_t flightTxTime;
	uint32_t apogeeLogTime;
	uint32_t apogeeTxTime;
	uint32_t recLogTime;
	uint32_t recTxTime;

	// UART config
	uint8_t loraBaudRate;
	uint8_t loraParity;

	// Payload Config
	uint32_t payloadSyncTime;
	uint32_t payloadBaudRate;
	uint8_t payloadParity;

	// Parâmetros do Filtro NLMS
	float nlmsLearningRate;
	float nlmsRegulation;
	float matchedFilterCorrelation;
	float matchedFilterAmplitude;

	float abgDt;
	float abgAlfa;
	float abgBeta;
	float abgGama;
	float ewmaAlfa;

	// Triggers da FSM
	float liffAltArm;
	uint32_t liffTimeArm;
	uint32_t drogueTime;
	float drogueAlt;
	float drogueDelta;
	float drogueVeloc;

	// Configurações Gerais
	bool formatFlash;
	bool enableSerial;
	bool loadFlashBin;

	// Parametros Simulacao
	bool simulationMode;
	char simulationFile[32];

	// Configurações do LoRa
	uint8_t loraChannel;
	uint8_t loraNetId;
	uint16_t tellerAddress;
	uint16_t fetcherAddress;
	uint8_t loraPower;
	uint8_t loraAirRate;
	bool loraRssi;
};

class ConfigLoader
{
public:

    ConfigParams params;

    ConfigLoader();
    bool loadFromSD(SDCard& sd, const char* filename);
    bool loadFromFlash(FlashStorage& flash, const char* filename);
    void printConfig();

private:

    void loadDefaults();
    void trim(char* str);
    void parseLine(const char* key, const char* value);
};

#endif // CONFIG_LOADER_H
