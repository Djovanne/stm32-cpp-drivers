/*
 * format.c
 *
 *  Created on: May 18, 2026
 *      Author: giova
 */

#include "DataFormatter.h"
#include <stdio.h>

///////////////////////  Definição de variáveis do Header ///////////////////////////////////

const char* FC_CSV_HEADER = "ID,Status,Time(ms),aX(m/s²),aY(m/s²),aZ(m/s²),wX(rad/s),wY(rad/s),wZ(rad/s),mX(µT),mY(µT),mZ(µT),Press(filter),Press(hPa),Alt(m),v_velocity(m/s),Temp(C),Bat(V),Lat,Lon,GPS_Alt(m),Speed(m/s),Fix,Sats";
const char* PL_CSV_HEADER = "CO2,MP25,MP10,O3,Time,Hdr";

///////////////////////  FUNÇÕES DE TRADUÇÃO (DUMP DINÂMICO)///////////////////////////////////

void formatFCData(const uint8_t* rawPayload, char* outBuffer, size_t maxLen)
{
    const Data* d = (const Data*)rawPayload;

    snprintf(outBuffer, maxLen,
                 "%u,%u,%lu,"                               // ID, Status, Timestamp
                 "%.2f,%.2f,%.2f,"                          // a[0..2]
                 "%.2f,%.2f,%.2f,"                          // w[0..2]
                 "%.2f,%.2f,%.2f,"                          // m[0..2]
                 "%.2f,%.2f,%.2f,"                          // Pressures, Alt
                 "%.2f,%.2f,%.2f,"                          // v_vel, Temp, Bat
                 "%.6f,%.6f,%.2f,%.2f,%u,%u",               // GPS: Lat, Lon, Alt, Speed, Fix, Sats
                 d->dataID, d->status, (unsigned long)d->timestamp,
                 d->a[0], d->a[1], d->a[2],
                 d->w[0], d->w[1], d->w[2],
                 d->m[0], d->m[1], d->m[2],
                 d->pressure, d->pure_pressure, d->altitude,
                 d->v_velocity, d->temperature, d->battery,
                 d->latitude, d->longitude, d->GPSaltitude,
                 d->speed_ms, d->fix_quality, d->satellites
	 );
}

void formatPayloadData(const uint8_t* rawPayload, char* outBuffer, size_t maxLen)
{
    const Payload_t* p = (const Payload_t*)rawPayload;

    snprintf(outBuffer, maxLen, "%.2f,%.2f,%.2f,%.2f,%lu,%u",
             p->co2, p->mp25, p->mp10, p->ozonio, p->tempo_da_coleta, p->cabecalho);
}
