#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

Adafruit_NeoPixel strip = Adafruit_NeoPixel(1, 21, NEO_GRB + NEO_KHZ800);

void Led_setup() {
    if (LED_STT < 0) {
        Serial.println("⚠️    LED_STT pin not set");
        return;
    }else{
        Serial.print("LED_STT pin: ");
        Serial.println(LED_STT);
    }
    strip.begin();
    strip.setBrightness(100);
    strip.show();
}

void Led_setColor(uint32_t color) {
    for (int i = 0; i < strip.numPixels(); i++) {
        strip.setPixelColor(i, color);
    }
    strip.show();
}

void Led_setPixelColor(uint16_t n, uint8_t r, uint8_t g, uint8_t b) {
    if (n < strip.numPixels()) {
        strip.setPixelColor(n, r, g, b);
        strip.show();
    }
}

void Led_off() {
    strip.clear();
    strip.show();
}

