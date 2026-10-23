/*
 * format.h
 *
 *  Created on: May 18, 2026
 *      Author: giova
 */

#ifndef DATA_FORMATTER_H
#define DATA_FORMATTER_H

#include <stdint.h>
#include <stddef.h>

#include "Data.h"
#include "Payload_data.h"
#include "FC_Config.h"
#include "main.h"

extern const char* FC_CSV_HEADER; // cabeçalho tellerlétrico
extern const char* PL_CSV_HEADER; // cabeçalho payload

// Calback de formatação dos dados

void formatFCData(const uint8_t* rawPayload, char* outBuffer, size_t maxLen);
void formatPayloadData(const uint8_t* rawPayload, char* outBuffer, size_t maxLen);

#endif
