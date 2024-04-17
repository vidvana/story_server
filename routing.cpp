#include "routing.h"
#include <string.h>

void getFileURL(char *route, char *fileURL)
{
    // if route has parameters, remove them
    char *question = strrchr(route, '?');
    if (question)
        *question = '\0';

    // if route is empty, set it to index.html
    if (route[strlen(route) - 1] == '/')
    {
        strcat(route, "index.html");
    }

    // get filename from route
    strcpy(fileURL, "htdocs");
    strcat(fileURL, route);

    // if filename does not have an extension, set it to .html
    const char *dot = strrchr(fileURL, '.');
    if (!dot || dot == fileURL)
    {
        strcat(fileURL, ".html");
    }
}
