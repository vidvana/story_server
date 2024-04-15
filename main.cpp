#include <iostream>
#include <stdio.h>
#include <string.h>
#include <netdb.h>

#include <sys/socket.h>
#include<netinet/in.h>
#include<unistd.h>

#include <signal.h>
#include <time.h>

# define SIZE 100
// TUTORIAL: https://medium.com/@nipunweerasiri/a-simple-web-server-written-in-c-cf7445002e6



void getMimeType(char *file, char *mime)
{
    const char *dot = strrchr(file, '.');

    if (dot == NULL)
        strcpy(mime, "text/html");

    else if (strcmp(dot, ".html") == 0)
        strcpy(mime, "text/html");

    else if (strcmp(dot, ".css") == 0)
        strcpy(mime, "text/css");

    else if (strcmp(dot, ".js") == 0)
        strcpy(mime, "application/js");

    else if (strcmp(dot, ".jpg") == 0)
        strcpy(mime, "image/jpeg");

    else if (strcmp(dot, ".png") == 0)
        strcpy(mime, "image/png");

    else if (strcmp(dot, ".gif") == 0)
        strcpy(mime, "image/gif");

    else
        strcpy(mime, "text/html");
}


void getFileURL(char * route, char * fileURL) {
    char *question = strrchr(route, '?');
    if (question)
        *question = '\0';

    if (route[strlen(route) - 1] == '/') {
        strcat(route, "index.html");
    }
    strcpy(fileURL, "htdocs");
    strcat(fileURL, route);


    const char *dot = strrchr(fileURL, '.');
    if (!dot || dot == fileURL)
    {
        strcat(fileURL, ".html");
    }
}

void getTimeString(char * timeBuf) {
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    sprintf(timeBuf, "%d-%02d-%02d %02d:%02d:%02d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
}

int main() {
    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    int portno = 5001; // będziemy serwować z tego adresu?
    serverAddress.sin_port = htons(portno);
    serverAddress.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    int value = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &value, sizeof(int));
    // REUSEADDR_EXPLANATION: https://stackoverflow.com/questions/775638/using-so-reuseaddr-what-happens-to-previously-open-socket

    // WHAT DOES BINDING DO?: https://stackoverflow.com/questions/12763268/why-is-bind-used-in-tcp-why-is-it-used-only-on-server-side-and-not-in-client
    if (bind(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {
        printf("ERROR: server is not bound to an address.\n");
        return errno;
    }
    if (listen(serverSocket, 3) < 0) {
        return 1;
    }

    char hostBuffer[NI_MAXHOST], serviceBuffer[NI_MAXSERV];
    int error = getnameinfo((struct sockaddr *)&serverAddress, sizeof(serverAddress), hostBuffer,
                            sizeof(hostBuffer), serviceBuffer, sizeof(serviceBuffer), 0);
    if (error != 0) {
        printf("Error: %s\n", gai_strerror(error));
        return 1;
    }

    printf("\nServer is listening on http://%s:%s/\n\n", hostBuffer, serviceBuffer);

    char *request;

    while (true) {
        //printf("Inside loop.\n");
        request = (char *)malloc(SIZE * sizeof(char));
        //printf("Request read.\n");
        char method[10], route[100];
        int clientSocket = accept(serverSocket, NULL, NULL);
        read(clientSocket, request, SIZE);
        //printf("Client socket accepted.\n");
        sscanf(request, "%s %s", method, route);
        printf("%s %s\n", method, route);
        char fileURL[100];
        getFileURL(route, fileURL);
        FILE *file = fopen(fileURL, "r");
        printf("%s\n", fileURL);
        if (!file)
        {
            printf("ERROR: File not found.\n");
            const char response[] = "HTTP/1.1 404 Not Found\r\n\n";
            send(clientSocket, response, sizeof(response), 0);
        } else {
            printf("File opened.\n");
            char resHeader[SIZE];
            char mimeType[32];
            getMimeType(fileURL, mimeType);

            //char timeBuf[100];
            //getTimeString(timeBuf);

            sprintf(resHeader, "HTTP/1.1 200 OK\r\nContent-Type: %s\r\n\n", mimeType);
            strcpy(resHeader, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\n");
            printf("Res header:\n");
            printf("Res header: %s\n", resHeader);
            strcpy(resHeader, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\n");

            printf("Res header: %s\n", resHeader);
            // RES HEADER CAN'T BE PRINTED FOR SOME REASON, even tho it is saved ):
            // that's prolly the reason why it can't be copied into the response
            // use C++ strings instead idk
            int headerSize = strlen(resHeader);

            //printf(" %s", mimeType);

            fseek(file, 0, SEEK_END);
            long fsize = ftell(file);
            fseek(file, 0, SEEK_SET);

            char *resBuffer = (char *)malloc(fsize + headerSize);
            strcpy(resBuffer, resHeader);
            printf("RES BUFFER:");
            printf("RES BUFFER: %s\n", resBuffer);
            char *fileBuffer = resBuffer + headerSize;
            fread(fileBuffer, fsize, 1, file);
            //printf("%s\n", resBuffer);
            send(clientSocket, resBuffer, fsize + headerSize, 0);
            free(resBuffer);
            fclose(file);
        }
        printf("End of request handling.\n");
    }
    free(request);
    return 0;
}
