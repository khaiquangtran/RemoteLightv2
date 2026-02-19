#include "Logging.h"

void printLog(const char* level, const char* path, const char* fileName, int32_t line, const char* message, ...) {
  #if ENABLE_LOGGING
    if (!level || !path || !fileName || !message) {
      return;
    }
    const char* color = COLOR_RESET;
    if (strcmp(level, "ERROR") == 0)
    {
      color = COLOR_RED;
    }
    else if (strcmp(level, "WARN") == 0)
    {
      color = COLOR_YELLOW;
    }
    else if (strcmp(level, "INFO") == 0)
    {
      color = COLOR_BLUE;
    }
    else
    {
      color = COLOR_RESET;
    }

    char* pos = strrchr(path, '/');
    char *pathFile = (char *)malloc(20 * sizeof(char));
    int32_t found = pos - path + 1;
    int32_t i = 0;
    while (path[found] != '\0') {
        pathFile[i++] = path[found++];
    }
    pathFile[i] = '\0';

    char buffer[256];
    va_list args;
    va_start(args, message);
    snprintf(buffer, sizeof(buffer), "%s[%s]\t[%s:%d %s()]: ",color, level, pathFile, line, fileName);
    vsnprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), message, args);
    va_end(args);

    Serial.println(buffer);

    free(pathFile);
  #else
    return;
  #endif
}