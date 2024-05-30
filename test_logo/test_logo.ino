#include "SPI.h"
#include <Adafruit_ILI9341.h>
#include "grafana.h"

#define TFT_DC D4
#define TFT_CS  D2

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

void setup() {
    Serial.begin(115200);
    tft.begin();
}

void loop(void) {
    for(uint8_t r=0; r<4; r++) {
        tft.setRotation(r);
        tft.fillScreen(ILI9341_BLACK);
        tft.drawRGBBitmap(0, 0, grafana, 240, 240);
        delay(1000);
        Serial.println(r);
    }
}
