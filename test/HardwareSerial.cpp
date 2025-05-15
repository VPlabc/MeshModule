#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#include "pins_arduino.h"
#include "HardwareSerial.h"

#ifndef RX1
#define RX1 5
#endif

#ifndef TX1
#define TX1 18
#endif

#ifndef RX2
#define RX2 16
#endif

#ifndef TX2
#define TX2 17
#endif

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_SERIAL)
HardwareSerial1 Serial_0(0);
HardwareSerial1 Serial_1(1);
HardwareSerial1  Serial_2(2);
#endif

HardwareSerial1::HardwareSerial1(int uart_nr) : _uart_nr(uart_nr), _uart(NULL) {}

void HardwareSerial1::begin(unsigned long baud, uint32_t config, int8_t rxPin, int8_t txPin, bool invert, unsigned long timeout_ms) {
    if (0 > _uart_nr || _uart_nr > 2) {
        log_e("Serial number is invalid, please use 0, 1 or 2");
        return;
    }
    if (_uart) {
        end();
    }
    if (_uart_nr == 0 && rxPin < 0 && txPin < 0) {
        rxPin = 3;
        txPin = 1;
    }
    if (_uart_nr == 1 && rxPin < 0 && txPin < 0) {
        rxPin = 15;
        txPin = 5;
    }
    if (_uart_nr == 2 && rxPin < 0 && txPin < 0) {
        rxPin = 16;
        txPin = 17;
    }

    _uart = uartBegin(_uart_nr, baud ? baud : 9600, config, rxPin, txPin, 256, 256, invert, 112);
    _tx_pin = txPin;
    _rx_pin = rxPin;

    if (!baud) {
        uartStartDetectBaudrate(_uart);
        time_t startMillis = millis();
        unsigned long detectedBaudRate = 0;
        while (millis() - startMillis < timeout_ms && !(detectedBaudRate = uartDetectBaudrate(_uart))) {
            yield();
        }

        end();

        if (detectedBaudRate) {
            delay(100); // Give some time...
            _uart = uartBegin(_uart_nr, detectedBaudRate, config, rxPin, txPin, 256, 256, invert, 112);
        } else {
            log_e("Could not detect baudrate. Serial data at the port must be present within the timeout for detection to be possible");
            _uart = NULL;
            _tx_pin = 255;
            _rx_pin = 255;
        }
    }
}

void HardwareSerial1::updateBaudRate(unsigned long baud)
{
	uartSetBaudRate(_uart, baud);
}

void HardwareSerial1::end() {
    if (uartGetDebug() == _uart_nr) {
        uartSetDebug(0);
    }
    log_v("pins %d %d", _tx_pin, _rx_pin);
    uartEnd(_uart);
    _uart = 0;
}

size_t HardwareSerial1::setRxBufferSize(size_t new_size) {
    log_e("uartResizeRxBuffer is not supported in this library version");
    return 0;
}

void HardwareSerial1::setDebugOutput(bool en)
{
    if(_uart == 0) {
        return;
    }
    if(en) {
        uartSetDebug(_uart);
    } else {
        if(uartGetDebug() == _uart_nr) {
            uartSetDebug(0);
        }
    }
}

int HardwareSerial1::available(void)
{
    return uartAvailable(_uart);
}
int HardwareSerial1::availableForWrite(void)
{
    return uartAvailableForWrite(_uart);
}

int HardwareSerial1::peek(void)
{
    if (available()) {
        return uartPeek(_uart);
    }
    return -1;
}

int HardwareSerial1::read(void)
{
    if(available()) {
        return uartRead(_uart);
    }
    return -1;
}

// read characters into buffer
// terminates if size characters have been read, or no further are pending
// returns the number of characters placed in the buffer
// the buffer is NOT null terminated.
size_t HardwareSerial1::read(uint8_t *buffer, size_t size)
{
    size_t avail = available();
    if (size < avail) {
        avail = size;
    }
    size_t count = 0;
    while(count < avail) {
        *buffer++ = uartRead(_uart);
        count++;
    }
    return count;
}

void HardwareSerial1::flush(void)
{
    uartFlush(_uart);
}

void HardwareSerial1::flush(bool txOnly)
{
    uartFlushTxOnly(_uart, txOnly);
}

size_t HardwareSerial1::write(uint8_t c)
{
    uartWrite(_uart, c);
    return 1;
}

size_t HardwareSerial1::write(const uint8_t *buffer, size_t size)
{
    uartWriteBuf(_uart, buffer, size);
    return size;
}
uint32_t  HardwareSerial1::baudRate()

{
	return uartGetBaudRate(_uart);
}
HardwareSerial1::operator bool() const
{
    return true;
}

void HardwareSerial1::setRxInvert(bool invert)
{
    uartSetRxInvert(_uart, invert);
}
