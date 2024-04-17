#include <stdio.h>  // console input/output, perror
#include <stdlib.h> // exit
#include <string.h> // string manipulation
#include <netdb.h>  // getnameinfo
#include "http_header.h"
#include "routing.h"
#include <sys/socket.h> // socket APIs
#include <netinet/in.h> // sockaddr_in
#include <unistd.h>     // open, close

#include <signal.h> // signal handling
#include <time.h>   // time

#include <fstream>
// Path :/home/julia/CLionProjects/HTTP/json-3.11.3/include
#include <nlohmann/json.hpp>
using json = nlohmann::json;

// EasyQtSql


#define SIZE 1024  // buffer size
#define PORT 2728  // port number
#define BACKLOG 10 // number of pending connections queue will hold

int serverSocket;
int clientSocket;

char *request;

int main()
{

    // ...
    //json ex2 = R"(
    //  {
    //    "Wikipedia": "https://www.wikipedia.org/",
    //    "E-nauczanie": "https://enauczanie.pg.edu.pl/moodle/",
    //  }
    //)"_json;

    // register signal handler
    signal(SIGINT, handleSignal);

    // server internet socket address
    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;                     // IPv4
    serverAddress.sin_port = htons(PORT);                   // port number in network byte order (host-to-network short)
    serverAddress.sin_addr.s_addr = htonl(INADDR_LOOPBACK); // localhost (host to network long)

    // socket of type IPv4 using TCP protocol
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    // reuse address and port
    int one = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(int));

    // bind socket to address
    if (bind(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
    {
        printf("Error: The server is not bound to the address.\n");
        return 1;
    }

    // listen for connections
    if (listen(serverSocket, BACKLOG) < 0)
    {
        printf("Error: The server is not listening.\n");
        return 1;
    }

    // get server address information
    char hostBuffer[NI_MAXHOST], serviceBuffer[NI_MAXSERV];
    int error = getnameinfo((struct sockaddr *)&serverAddress, sizeof(serverAddress), hostBuffer,
                            sizeof(hostBuffer), serviceBuffer, sizeof(serviceBuffer), 0);

    if (error != 0)
    {
        printf("Error: %s\n", gai_strerror(error));
        return 1;
    }

    printf("\nServer is listening on http://%s:%s/\n\n", hostBuffer, serviceBuffer);

    while (1)
    {
        // buffer to store data (request)
        request = (char *)malloc(SIZE * sizeof(char));
        char method[10], route[100];

        // accept connection and read data
        clientSocket = accept(serverSocket, NULL, NULL);
        read(clientSocket, request, SIZE);

        // parse HTTP request
        sscanf(request, "%s %s", method, route);
        printf("%s %s", method, route);

        free(request);

        // only support GET method
        if (strcmp(method, "GET") != 0)
        {
            const char response[] = "HTTP/1.1 400 Bad Request\r\n\n";
            send(clientSocket, response, sizeof(response), 0);
        }
        else
        {
            char fileURL[100];

            // generate file URL
            getFileURL(route, fileURL);

            // read file
            FILE *file = fopen(fileURL, "r");
            if (!file)
            {
                const char response[] = "HTTP/1.1 404 Not Found\r\n\n";
                send(clientSocket, response, sizeof(response), 0);
            }
            else
            {
                // generate HTTP response header
                char resHeader[SIZE];

                // get current time
                char timeBuf[100];
                getTimeString(timeBuf);

                // generate mime type from file URL
                char mimeType[32];
                getMimeType(fileURL, mimeType);

                sprintf(resHeader, "HTTP/1.1 200 OK\r\nDate: %s\r\nContent-Type: %s\r\n\n", timeBuf, mimeType);
                int headerSize = strlen(resHeader);

                printf(" %s", mimeType);

                // Calculate file size
                fseek(file, 0, SEEK_END);
                long fsize = ftell(file);
                fseek(file, 0, SEEK_SET);

                // Allocates memory for response buffer and copies response header and file contents to it
                char *resBuffer = (char *)malloc(fsize + headerSize);
                strcpy(resBuffer, resHeader);

                // Starting position of file contents in response buffer
                char *fileBuffer = resBuffer + headerSize;
                fread(fileBuffer, fsize, 1, file);

                send(clientSocket, resBuffer, fsize + headerSize, 0);
                free(resBuffer);
                fclose(file);
            }
        }
        close(clientSocket);
        printf("\n");
    }
}

void handleSignal(int signal)
{
    if (signal == SIGINT)
    {
        printf("\nShutting down server...\n");

        close(clientSocket);
        close(serverSocket);

        if (request != NULL)
            free(request);

        exit(0);
    }
}

// TODO:
// 0. C++: stworz jsona reprezentującego rozdziały
// 1. JAVASCRIPT: stwórz drukowanie za pomocą query z rozdziałami: nazwa, link idk?
// 2. NA RAZIE BEZ LOGOWANIA - KAŻDY, KTO MA LINK MA DOSTĘP



// Może potem: generuj okna z tekstem jako paragrafy (textboxy na srodku ekranu jeden na drugim);
