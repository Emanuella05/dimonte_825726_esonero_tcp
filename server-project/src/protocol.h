/*
 * protocol.h
 *
 * Protocol definitions for Weather TCP Client/Server
 */

#ifndef PROTOCOL_H_
#define PROTOCOL_H_

#include <stdint.h>

#define SERVER_PORT 56700
#define MAX_CITY_LEN 64
#define QUEUE_SIZE 6

// --- Tipi di richiesta ---
#define TYPE_TEMPERATURE 't'
#define TYPE_HUMIDITY    'h'
#define TYPE_WIND        'w'
#define TYPE_PRESSURE    'p'

// --- Codici di stato ---
#define STATUS_OK 0
#define STATUS_CITY_NOT_FOUND 1
#define STATUS_INVALID_REQUEST 2

// Richiesta client → server
typedef struct {
    char type;
    char city[MAX_CITY_LEN];
} weather_request_t;

// Risposta server → client
typedef struct {
    unsigned int status;
    char type;
    float value;
} weather_response_t;

// Funzioni meteo (implementate nel server)
float get_temperature(void);
float get_humidity(void);
float get_wind(void);
float get_pressure(void);

#endif
