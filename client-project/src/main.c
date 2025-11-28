/*
 * client.c
 *
 * Weather TCP Client
 *
 */

#if defined WIN32
#include <winsock.h>
#else
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#define closesocket close
#endif

#include <stdio.h>
#include <stdlib.h>
#include "protocol.h"

void clearwinsock() {
#if defined WIN32
    WSACleanup();
#endif
}

int main(int argc, char *argv[]) {

#if defined WIN32
    WSADATA wsa_data;
    WSAStartup(MAKEWORD(2,2), &wsa_data);
#endif

    if (argc < 3) {
        printf("Uso corretto: ./client-project [-s server] [-p port] -r \"type city\"\n");
        return -1;
    }

    char server_ip[64] = "127.0.0.1";
    int port = SERVER_PORT;
    char type = 0;
    char city[MAX_CITY_LEN] = {0};
    int request_ok = 0;


    for (int i = 1; i < argc; ++i) {

        if (!strcmp(argv[i], "-s") && i+1 < argc)
            strcpy(server_ip, argv[++i]);

        else if (!strcmp(argv[i], "-p") && i+1 < argc)
            port = atoi(argv[++i]);

        else if (!strcmp(argv[i], "-r") && i+1 < argc) {
            sscanf(argv[++i], "%c %63s", &type, city);
            request_ok = 1;
        }
    }

    if (!request_ok) {
        printf("Uso corretto: ./client-project [-s server] [-p port] -r \"type city\"\n");
        return -1;
    }

    //CREAZIONE SOCKET
    int s = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(server_ip);

    if (connect(s, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        printf("Connessione al server fallita.\n");
        return -1;
    }

    //RICHIESTA
    weather_request_t req;
    req.type = type;
    strncpy(req.city, city, MAX_CITY_LEN);
    req.city[MAX_CITY_LEN - 1] = '\0';

    send(s, (char*)&req, sizeof(req), 0);

    //RISPOSTA
    weather_response_t res;
    recv(s, (char*)&res, sizeof(res), 0);

    printf("Ricevuto risultato dal server ip %s. ", server_ip);

    if (res.status == STATUS_OK) {

        if (res.type == TYPE_TEMPERATURE)
            printf("%s: Temperatura = %.1f°C\n", city, res.value);

        else if (res.type == TYPE_HUMIDITY)
            printf("%s: Umidità = %.0f%%\n", city, res.value);

        else if (res.type == TYPE_WIND)
            printf("%s: Vento = %.0f km/h\n", city, res.value);

        else if (res.type == TYPE_PRESSURE)
            printf("%s: Pressione = %.0f hPa\n", city, res.value);

    } else if (res.status == STATUS_CITY_NOT_FOUND) {
        printf("Città non disponibile\n");

    } else if (res.status == STATUS_INVALID_REQUEST) {
        printf("Richiesta non valida\n");
    }

    closesocket(s);
    clearwinsock();
    return 0;
}
