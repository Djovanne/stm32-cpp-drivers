/*
 * ConfigLoader.cpp
 *
 *  Created on: May 18, 2026
 *      Author: giova
 */

#include "ConfigLoader.h"
#include "FC_Config.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

ConfigLoader::ConfigLoader()
{
	// Carrega o default do sistema
    loadDefaults();
}

void ConfigLoader::loadDefaults()
{
    // Tempos
    params.baroSampleTime = BARO_SAMPLE_TIME;
    params.flightSyncTime = FLIGHT_SYNC_TIME;
    params.liftoffLogTime = LIFTOFF_LOG_TIME;
    params.liftoffTxTime  = LIFTOFF_TX_TIME;
    params.flightLogTime  = FLIGHT_LOG_TIME;
    params.flightTxTime   = FLIGHT_TX_TIME;
    params.apogeeLogTime  = APOGEE_LOG_TIME;
    params.apogeeTxTime   = APOGEE_TX_TIME;
    params.recLogTime     = REC_LOG_TIME;
    params.recTxTime      = REC_TX_TIME;

    // UART

    params.loraBaudRate    = LORA_BAUD_RATE;
	params.loraParity      = LORA_PARITY;

	// Payload

	params.payloadBaudRate = PAYLOAD_BAUD_RATE;
	params.payloadParity   = PAYLOAD_PARITY;
	params.payloadSyncTime = PAYLOAD_TIMER;

    // Filtros
    params.nlmsLearningRate      = LEARNING_RATE;
    params.nlmsRegulation        = REGULATION;
    params.matchedFilterCorrelation    = KP_MIM;
    params.matchedFilterAmplitude = ERROR_MAX;

    params.abgDt                 = DT_ABG;
    params.abgAlfa               = ALFA_ABG;
    params.abgBeta               = BETA_ABG;
    params.abgGama               = GAMA_ABG;
    params.ewmaAlfa              = ALFA_EWMA;

    // FSM (Máquina de Estados)
    params.liffAltArm   = LIFF_ALT_ARM;
    params.liffTimeArm  = LIFF_TIME_ARM;
    params.drogueTime   = DROGUE_TIME;
    params.drogueAlt    = DROGUE_ALT;
    params.drogueDelta  = DROGUE_DELTA;
    params.drogueVeloc  = DROGUE_VELO;

    // Gerais
    params.formatFlash  = FORMAT_FLASH;
    params.enableSerial = true;
    params.loadFlashBin = true;

    // Simulacao
    params.simulationMode = false;
    strcpy(params.simulationFile, "sim.bin");

    // Rádio E22
    params.loraChannel    = LORA_CHANNEL;
    params.loraNetId      = LORA_NETID;
    params.tellerAddress  = TELLER_ADRESS;
    params.fetcherAddress = FETCHER_ADRESS;
    params.loraPower      = LORA_POWER;
	params.loraAirRate    = AIR_DATA_RATE;
	params.loraRssi       = LORA_RSSI;
}

void ConfigLoader::trim(char* str)
{
    if(!str) return;

    // Remove espaços/quebras do fim
    char* p = str;
    int l = strlen(p);
    while(l > 0 && isspace((unsigned char)p[l - 1])) p[--l] = 0;

    // Remove espaços do começo
    while(*p && isspace((unsigned char)*p)) ++p, --l;

    // Move a string limpa para o ponteiro original
    memmove(str, p, l + 1);
}

bool ConfigLoader::loadFromSD(SDCard& sd, const char* filename)
{
    // Certifica-se de que o SD está montado
    if (!sd.mount()) return false;
    // Tenta abrir o arquivo em modo leitura
    if (!sd.openFileForRead(filename))
    {
        LogPrint("ConfigLoader: Arquivo '%s' nao encontrado. Usando defaults nativos.\n\r", filename);
        return false;
    }
    LogPrint("ConfigLoader: Arquivo '%s' aberto. Processando linhas...\n\r", filename);
    char line[128]; // Buffer temporário para ler linha a linha
    // Enquanto houver linhas para ler (readLine retorna != NULL enquanto houver texto)
    while (sd.readLine(line, sizeof(line)))
    {
        // 1. Ignorar Comentários: Trunca a string onde achar '#' ou ';'
        char* comment = strchr(line, '#');
        if (comment) *comment = '\0';
        comment = strchr(line, ';');
        if (comment) *comment = '\0';
        // 2. Limpa espaços e pula linhas que ficaram vazias
        trim(line);
        if (strlen(line) == 0) continue;
        // 3. Procura o sinal de igualdade '=' separador
        char* equalsToken = strchr(line, '=');
        if (equalsToken)
        {
            *equalsToken = '\0'; // Divide a string em duas substituindo '=' por '\0'
            char* key = line;
            char* value = equalsToken + 1;
            trim(key);
            trim(value);
            // Delega para o dicionário que converte para a memória RAM
            parseLine(key, value);
        }
    }
    // Fecha o arquivo corretamente
    sd.closeFile();

    LogPrint("ConfigLoader: Configuracoes carregadas com sucesso!\n\r");
    return true;
}

bool ConfigLoader::loadFromFlash(FlashStorage& flash, const char* filename)
{
    // Certifica-se de que a Flash está montada (sem formatar em caso de falha durante a leitura)
    if (!flash.mount(false)) return false;

    // Struct do arquivo
    FlashFile configFile;

    // Tenta abrir o arquivo na Flash
    if (!flash.openFileForRead(configFile, filename))
    {
        LogPrint("ConfigLoader: Arquivo '%s' nao encontrado na Flash.\n\r", filename);
        return false;
    }

    LogPrint("ConfigLoader: Arquivo '%s' aberto na Flash. Processando...\n\r", filename);
    char line[128];

    // O loop utiliza o novo readLine da Flash
    while (flash.readLine(configFile, line, sizeof(line)))
    {
        // 1. Ignorar Comentários
        char* comment = strchr(line, '#');
        if (comment) *comment = '\0';
        comment = strchr(line, ';');
        if (comment) *comment = '\0';

        // 2. Limpa espaços
        trim(line);
        if (strlen(line) == 0) continue;

        // 3. Procura o separador '='
        char* equalsToken = strchr(line, '=');
        if (equalsToken)
        {
            *equalsToken = '\0';
            char* key = line;
            char* value = equalsToken + 1;
            trim(key);
            trim(value);

            // Converte para RAM
            parseLine(key, value);
        }
    }

    flash.closeFile(configFile);

    LogPrint("ConfigLoader: Configuracoes carregadas da Flash com sucesso!\n\r");
    return true;
}


void ConfigLoader::parseLine(const char* key, const char* value)
{
    // Tempos

	if      (strcmp(key, "FORMAT_FLASH") == 0)     params.formatFlash    = (bool)atoi(value);
	else if (strcmp(key, "LOAD_FLASH_BIN") == 0)   params.loadFlashBin   = (bool)atoi(value);
	else if (strcmp(key, "ENABLE_SERIAL") == 0)    params.enableSerial   = (bool)atoi(value);
	else if (strcmp(key, "BARO_SAMPLE_TIME") == 0) params.baroSampleTime = (uint32_t)atoi(value);
    else if (strcmp(key, "FLIGHT_SYNC_TIME") == 0) params.flightSyncTime = (uint32_t)atoi(value);
    else if (strcmp(key, "LIFTOFF_LOG_TIME") == 0) params.liftoffLogTime = (uint32_t)atoi(value);
    else if (strcmp(key, "LIFTOFF_TX_TIME") == 0)  params.liftoffTxTime  = (uint32_t)atoi(value);
    else if (strcmp(key, "FLIGHT_LOG_TIME") == 0)  params.flightLogTime  = (uint32_t)atoi(value);
    else if (strcmp(key, "FLIGHT_TX_TIME") == 0)   params.flightTxTime   = (uint32_t)atoi(value);
    else if (strcmp(key, "APOGEE_LOG_TIME") == 0)  params.apogeeLogTime  = (uint32_t)atoi(value);
    else if (strcmp(key, "APOGEE_TX_TIME") == 0)   params.apogeeTxTime   = (uint32_t)atoi(value);
	else if (strcmp(key, "REC_TX_TIME") == 0)      params.recTxTime      = (uint32_t)atoi(value);
	else if (strcmp(key, "REC_LOG_TIME") == 0)     params.recLogTime     = (uint32_t)atoi(value);

	// Simulacao
	else if (strcmp(key, "SIMULATION_MODE") == 0)  params.simulationMode = (bool)atoi(value);
	else if (strcmp(key, "SIMULATION_FILE") == 0)  strncpy(params.simulationFile, value, 31); // copia nome de ate 32 caracteres

    // Filtros
    else if (strcmp(key, "LEARNING_RATE") == 0)    params.nlmsLearningRate         = (float)atof(value);
    else if (strcmp(key, "REGULATION") == 0)       params.nlmsRegulation           = (float)atof(value);
    else if (strcmp(key, "KP_MIM") == 0)           params.matchedFilterCorrelation = (float)atof(value);
    else if (strcmp(key, "ERROR_MAX") == 0)        params.matchedFilterAmplitude   = (float)atof(value);
    else if (strcmp(key, "DT_ABG") == 0)           params.abgDt                    = (float)atof(value);
	else if (strcmp(key, "ALFA_ABG") == 0)         params.abgAlfa                  = (float)atof(value);
	else if (strcmp(key, "BETA_ABG") == 0)         params.abgBeta                  = (float)atof(value);
	else if (strcmp(key, "GAMA_ABG") == 0)         params.abgGama                  = (float)atof(value);
	else if (strcmp(key, "ALFA_EWMA") == 0)        params.ewmaAlfa                 = (float)atof(value);

    // FSM (Máquina de Estados)
    else if (strcmp(key, "LIFF_ALT_ARM") == 0)     params.liffAltArm   = (float)atof(value);
    else if (strcmp(key, "LIFF_TIME_ARM") == 0)    params.liffTimeArm  = (uint32_t)atoi(value);
    else if (strcmp(key, "DROGUE_TIME") == 0)     params.drogueTime    = (uint32_t)atoi(value);
    else if (strcmp(key, "DROGUE_ALT") == 0)       params.drogueAlt    = (float)atof(value);
    else if (strcmp(key, "DROGUE_DELTA") == 0)     params.drogueDelta  = (float)atof(value);
    else if (strcmp(key, "DROGUE_VELOCITY") == 0)  params.drogueVeloc  = (float)atof(value);

    // LoRa E22
    else if (strcmp(key, "LORA_CHANNEL") == 0)     params.loraChannel    = (uint8_t)atoi(value);
    else if (strcmp(key, "LORA_NETID") == 0)       params.loraNetId      = (uint8_t)atoi(value);
    else if (strcmp(key, "TELLER_ADRESS") == 0)    params.tellerAddress  = (uint16_t)atoi(value);
    else if (strcmp(key, "FETCHER_ADRESS") == 0)   params.fetcherAddress = (uint16_t)atoi(value);
    else if (strcmp(key, "LORA_POWER") == 0)       params.loraPower      = (uint8_t)atoi(value);
	else if (strcmp(key, "LORA_AIR_RATE") == 0)    params.loraAirRate    = (uint8_t)atoi(value);
	else if (strcmp(key, "LORA_RSSI") == 0)        params.loraRssi       = (atoi(value) > 0);
	else if (strcmp(key, "LORA_BAUD_RATE") == 0)   params.loraBaudRate   = (uint8_t)atoi(value);
	else if (strcmp(key, "LORA_PARITY") == 0)      params.loraParity     = (uint8_t)atoi(value);

	else if (strcmp(key, "PAYLOAD_BAUD_RATE") == 0) params.payloadBaudRate = (uint32_t)atoi(value);
	else if (strcmp(key, "PAYLOAD_PARITY") == 0)    params.payloadParity   = (uint8_t)atoi(value);
	else if (strcmp(key, "PAYLOAD_SYNC_TIME") == 0) params.payloadSyncTime = (uint32_t)atoi(value);
}

void ConfigLoader::printConfig()
{
	LogPrint("\n\r======================================\n\r");
	LogPrint("       MISSION CONFIG LOADED          \n\r");
	LogPrint("======================================\n\r");

	LogPrint("Format Flash   : %s \n\r", params.formatFlash ? "TRUE" : "FALSE");
	LogPrint("Serial Log     : %s \n\r", params.enableSerial ? "TRUE" : "FALSE");
	LogPrint("Load Flash Bin : %s \n\r", params.loadFlashBin ? "TRUE" : "FALSE");
	LogPrint("Simulation Mode: %s \n\r", params.simulationMode ? "TRUE" : "FALSE");
	LogPrint("Simulation File: %s \n\r", params.simulationFile);

	// --- TIMINGS ---
	LogPrint("[ TIMINGS (ms) ]\n\r");
	LogPrint("Baro Sample  : %lu\n\r", params.baroSampleTime);
	LogPrint("Flight Sync  : %lu\n\r", params.flightSyncTime);
	LogPrint("Liftoff      : Log=%lu | Tx=%lu\n\r", params.liftoffLogTime, params.liftoffTxTime);
	LogPrint("Flight       : Log=%lu | Tx=%lu\n\r", params.flightLogTime, params.flightTxTime);
	LogPrint("Apogee       : Log=%lu | Tx=%lu\n\r", params.apogeeLogTime, params.apogeeTxTime);
	LogPrint("Recovery     : Log=%lu | Tx=%lu\n\r", params.recLogTime, params.recTxTime);

	// --- FSM TRIGGERS ---
	LogPrint("\n\r[ FSM TRIGGERS ]\n\r");
	LogPrint("Liff Alt Arm : %.1f m\n\r", params.liffAltArm);
	LogPrint("Liff Time Arm: %lu amostras\n\r", params.liffTimeArm);
	LogPrint("Drogue Alt   : %.1f m\n\r", params.drogueAlt);
	LogPrint("Drogue Delta : %.2f m\n\r", params.drogueDelta);
	LogPrint("Drogue Count : %lu amostras\n\r", params.drogueTime);
	LogPrint("Drogue Veloc : %.2f m/s\n\r", params.drogueVeloc);

	// --- FILTERS ---
	LogPrint("\n\r[ FILTERS ]\n\r");
	LogPrint("NLMS L_Rate  : %.4f\n\r", params.nlmsLearningRate);
	LogPrint("NLMS Regul   : %.6f\n\r", params.nlmsRegulation);
	LogPrint("MF Kp Min    : %.6f\n\r", params.matchedFilterCorrelation);
	LogPrint("MF Err Max   : %.2f\n\r", params.matchedFilterAmplitude);

	LogPrint("DT Filtro ABG  : %.4f\n\r", params.abgDt);
	LogPrint("ALFA filtro ABG: %.4f\n\r", params.abgAlfa);
	LogPrint("BETA filtro ABG: %.4f\n\r", params.abgBeta);
	LogPrint("GAMA filtro ABG: %.4f\n\r", params.abgGama);
	LogPrint("ALFA EWMA      : %.4f\n\r", params.ewmaAlfa);

	// --- LORA CONFIG ---
	LogPrint("\n\r[ LORA E22 ]\n\r");
	LogPrint("Channel      : %d\n\r", params.loraChannel);
	LogPrint("Net ID       : %d\n\r", params.loraNetId);
	LogPrint("Teller Addr  : %u\n\r", params.tellerAddress);
	LogPrint("Fetcher Addr : %u\n\r", params.fetcherAddress);
	LogPrint("Power (Enum) : %d\n\r", params.loraPower);
	LogPrint("AirRate (En) : %d\n\r", params.loraAirRate);
	LogPrint("RSSI Enable  : %s\n\r", params.loraRssi ? "SIM" : "NAO");
	LogPrint("BaudRate(En) : %d\n\r", params.loraBaudRate);
	LogPrint("Parity(En)   : %d\n\r", params.loraParity);

	// --- SIMULACAO ---
	LogPrint("\n\r[ SIMULATION HIL ]\n\r");
	LogPrint("Sim Mode      : %s \n\r", params.simulationMode ? "HABILITADO" : "DESABILITADO");
	LogPrint("Sim File      : %s \n\r", params.simulationFile);

	// --- PAYLOAD CONFIG ---
	LogPrint("\n\r[ PAYLOAD UART ]\n\r");
	LogPrint("Baud Rate    : %lu\n\r", params.payloadBaudRate);
	LogPrint("Parity Mode  : %d (0=N, 1=E, 2=O)\n\r", params.payloadParity);
	LogPrint("Sync Time    : %lu (ms)\n\r", params.payloadSyncTime);
	LogPrint("======================================\n\r\n\r");
}
