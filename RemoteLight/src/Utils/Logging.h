#ifndef LOGGING_H
#define LOGGING_H

#define ENABLE_LOGGING 0
#include <Arduino.h>
#include <string>

#define COLOR_RESET   "\033[0m"

#define COLOR_BLACK   "\033[30m"
#define COLOR_RED     "\033[31m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_BLUE    "\033[34m"
#define COLOR_MAGENTA "\033[35m"
#define COLOR_CYAN    "\033[36m"
#define COLOR_WHITE   "\033[37m"

#define COLOR_BOLD_BLACK   "\033[1;30m"
#define COLOR_BOLD_RED     "\033[1;31m"
#define COLOR_BOLD_GREEN   "\033[1;32m"
#define COLOR_BOLD_YELLOW  "\033[1;33m"
#define COLOR_BOLD_BLUE    "\033[1;34m"
#define COLOR_BOLD_MAGENTA "\033[1;35m"
#define COLOR_BOLD_CYAN    "\033[1;36m"
#define COLOR_BOLD_WHITE   "\033[1;37m"

#define LOGE(message, ...) printLog("ERROR", __FILE__, __func__, __LINE__, message, ##__VA_ARGS__)
#define LOGW(message, ...) printLog("WARN", __FILE__, __func__, __LINE__, message, ##__VA_ARGS__)
#define LOGI(message, ...) printLog("INFO", __FILE__, __func__, __LINE__, message, ##__VA_ARGS__)
#define LOGD(message, ...) printLog("DEBUG", __FILE__, __func__, __LINE__, message, ##__VA_ARGS__)
#define LOGV(message, ...) printLog("VERBOSE", __FILE__, __func__, __LINE__, message, ##__VA_ARGS__)

void printLog(const char* level, const char* path, const char* fileName, int32_t line, const char* message, ...);

#endif // LOGGING_H