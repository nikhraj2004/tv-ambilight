#include "FastLED.h"

#define MAX_LEDS 300
#define LED_TYPE WS2812B
#define DATA_PIN 12
#define COLOR_ORDER GRB
#define OFF_TIMEOUT 8000
#define BRIGHTNESS 255
#define DITHER_MODE BINARY_DITHER
#define COLOR_TEMPERATURE CRGB(255, 255, 255)
#define COLOR_CORRECTION TypicalLEDStrip
#define BAUD_RATE 115200

uint8_t prefix[] = {'A', 'd', 'a'};
unsigned long endTime;
CRGB leds[MAX_LEDS];

void showColor(const CRGB &color) {
    LEDS.showColor(color);
}

void switchOff() {
    memset(leds, 0, MAX_LEDS * sizeof(CRGB));
    FastLED.show();
}

bool dataAvailable() {
    while (!Serial.available()) {
        if (OFF_TIMEOUT > 0 && endTime < millis()) {
            switchOff();
            endTime = millis() + OFF_TIMEOUT;
            return false;
        }
    }
    return true;
}

void setup() {
    Serial.begin(BAUD_RATE);
    Serial.print("Ada\n");

    FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, MAX_LEDS);
    FastLED.setBrightness(BRIGHTNESS);
    FastLED.setTemperature(COLOR_TEMPERATURE);
    FastLED.setCorrection(COLOR_CORRECTION);
    FastLED.setDither(DITHER_MODE);

    for (int i = 0; i < BRIGHTNESS; i++) {
        showColor(CRGB(255, 0, 0));
        delay(150 / BRIGHTNESS);
    }
    for (int i = 0; i < BRIGHTNESS; i++) {
        showColor(CRGB(0, 255, 0));
        delay(150 / BRIGHTNESS);
    }
    for (int i = 0; i < BRIGHTNESS; i++) {
        showColor(CRGB(0, 0, 255));
        delay(150 / BRIGHTNESS);
    }

    showColor(CRGB(0, 0, 0));
    bool transmissionSuccess;
    unsigned long sum_r, sum_g, sum_b;

    for (;;) {
        for (uint8_t i = 0; i < sizeof(prefix); i++) {
            if (!dataAvailable() || prefix[i] != Serial.read()) {
                i = 0;
            }
        }

        if (!dataAvailable()) continue;
        uint8_t hi = Serial.read();
        if (!dataAvailable()) continue;
        uint8_t lo = Serial.read();
        if (!dataAvailable()) continue;
        uint8_t chk = Serial.read();

        if (chk != (hi ^ lo ^ 0x55)) continue;

        int num_leds = min(MAX_LEDS, (hi << 8) + lo + 1);
        memset(leds, 0, num_leds * sizeof(CRGB));
        transmissionSuccess = true;

        for (int idx = 0; idx < num_leds; idx++) {
            if (!dataAvailable()) {
                transmissionSuccess = false;
                break;
            }
            leds[idx].r = Serial.read();
            if (!dataAvailable()) {
                transmissionSuccess = false;
                break;
            }
            leds[idx].g = Serial.read();
            if (!dataAvailable()) {
                transmissionSuccess = false;
                break;
            }
            leds[idx].b = Serial.read();
        }

        if (transmissionSuccess) {
            endTime = millis() + OFF_TIMEOUT;
            FastLED.show();
        }
    }
}

void loop() {}
