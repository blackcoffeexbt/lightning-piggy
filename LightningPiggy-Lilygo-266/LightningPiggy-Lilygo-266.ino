 #define LILYGO_T5_V266

#include <boards.h>
#include <GxEPD.h>

#include <GxDEPG0266BN/GxDEPG0266BN.h>    // 2.66" b/w   form DKE GROUP

// FreeFonts from Adafruit_GFX
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeMonoBold18pt7b.h>
#include <Fonts/FreeMonoBold24pt7b.h>
#include <GxIO/GxIO_SPI/GxIO_SPI.h>
#include <GxIO/GxIO.h>

GxIO_Class io(SPI,  EPD_CS, EPD_DC,  EPD_RSET);
GxEPD_Class display(io, EPD_RSET, EPD_BUSY);


void setup()
{
    Serial.begin(115200);
    Serial.println();
    Serial.println("setup");

    SPI.begin(EPD_SCLK, EPD_MISO, EPD_MOSI);
    display.init();
}

void loop() {
  GxepdPage0();
  delay(4000);
  }

void GxepdPage0()
{
  display.setTextColor(GxEPD_BLACK);
    display.setRotation(0);
    display.fillScreen(GxEPD_WHITE);
    display.setFont(&FreeMonoBold9pt7b);

    int16_t x1, x2;
    uint16_t w, h;
    display.setCursor(10, 10);
    display.println("Hello mum");
    display.update();
}

