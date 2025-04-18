#ifndef UTILS_H
#define UTILS_H

void macStrToBytes(const char* macStr, uint8_t* macBytes);
void macBytesToStr(const uint8_t* macBytes, char* macStr, size_t size);
void processSerialInput();
void handleButtonPress();
void checkButton();

#endif // UTILS_H
