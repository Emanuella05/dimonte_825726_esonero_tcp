/*
 * server.c
 *
 * Weather TCP Server
 */

#if defined WIN32
#include <winsock.h>
#else
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#define closesocket close
extern int strcasecmp(const char*, const char*);
#endif

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "protocol.h"

void clearwinsock() {
#if defined WIN32
    WSACleanup();
#endif
}

//FUNZIONI METEO
float get_temperature(void) { return ((float)(rand() % 500) - 100) / 10.0; }
float get_humidity(void)    { return 20.0 + (float)(rand() % 800) / 10.0; }
float get_wind(void)        { return (float)(rand() % 1001) / 10.0; }
float get_pressure(void)    { return 950.0 + (float)(rand() % 1001) / 10.0; }

//CITTA' SUPPORTATE
int is_valid_city(const char *c) {
    const char *cities[] = {
        "bari", "roma", "milano", "napoli", "torino",
        "palermo", "genova", "bologna", "firenze", "venezia"
    };

    for (int i = 0; i < 10; i++)
        if (strcasecmp(c, cities[i]) == 0)
            return 1;
    return 0;
}

int main(int argc, char *argv[]) {

#if defined WIN32
    WSADATA wsa_data;
    WSAStartup(MAKEWORD(2,2), &wsa_data);
#endif

    srand(time(NULL));

    int port = SERVER_PORT;


    for (int i = 1; i < argc; i++)
        if (strcmp(argv[i], "-p") == 0 && i + 1 < argc)
            port = atoi(argv[++i]);

    //CREAZIONE SOCKET
    int s = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s < 0) {
        printf("Error creating socket\n");
        return -1;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    //BIND
    if (bind(s, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        printf("Bind error\n");
        closesocket(s);
        return -1;
    }

    //LISTEN
    if (listen(s, QUEUE_SIZE) < 0) {
        printf("Listen error\n");
        closesocket(s);
        return -1;
    }

    printf("Server in ascolto sulla porta %d...\n", port);

    //LOOP SERVER

    while (1) {

        struct sockaddr_in client_addr;

    #if defined WIN32
        int len = sizeof(client_addr);
    #else
        socklen_t len = sizeof(client_addr);
    #endif

        int client = accept(s, (struct sockaddr*)&client_addr, &len);
        if (client < 0) continue;

        weather_request_t req;
        recv(client, (char*)&req, sizeof(req), 0);

        printf("Richiesta '%c %s' dal client ip %s\n",
               req.type, req.city, inet_ntoa(client_addr.sin_addr));

        weather_response_t res;
        res.status = STATUS_OK;
        res.type = req.type;
        res.value = 0;

        if (!is_valid_city(req.city)) {
            res.status = STATUS_CITY_NOT_FOUND;
            res.type = '\0';
        }
        else if (req.type != TYPE_TEMPERATURE &&
                 req.type != TYPE_HUMIDITY &&
                 req.type != TYPE_WIND &&
                 req.type != TYPE_PRESSURE) {

            res.status = STATUS_INVALID_REQUEST;
            res.type = '\0';
        }
        else {
            if (req.type == TYPE_TEMPERATURE) res.value = get_temperature();
            if (req.type == TYPE_HUMIDITY)    res.value = get_humidity();
            if (req.type == TYPE_WIND)        res.value = get_wind();
            if (req.type == TYPE_PRESSURE)    res.value = get_pressure();
        }

        send(client, (char*)&res, sizeof(res), 0);
        closesocket(client);
    }

    closesocket(s);
    clearwinsock();
    return 0;
}
