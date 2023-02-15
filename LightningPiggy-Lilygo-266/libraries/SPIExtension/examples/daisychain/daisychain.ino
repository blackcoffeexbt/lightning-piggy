#include <SPI.h>
#include <SPIExtension.h>

const uint8_t PIN_CS = 10;
const uint8_t NUM_DEVICE = 3;
SPIExtension<PIN_CS, NUM_DEVICE> spi;

uint8_t send_data[NUM_DEVICE] = {1, 2, 3};

void setup()
{
    Serial.begin(115200);

    SPI.begin();

    // if you want to change settings
    // SPI.setBitOrder(MSBFIRST);
    // SPI.setDataMode(SPI_MODE1);
    // SPI.setClockDivider(SPI_CLOCK_DIV8);

    // or you can do that if you want to use multiple devices with different config
    // SPI.beginTransaction(SPISettings(16000000, LSBFIRST, SPI_MODE_3));

    spi.attatch(SPI);
}

void loop()
{
    // send data[0] to far device, data[1] is middle, data[2] to near device
    spi.transfer(send_data);

    for (uint8_t i = 0; i < spi.size(); ++i)
    {
        Serial.print(spi.data(i), HEX); Serial.print(" ");
    }
    Serial.println();
}
