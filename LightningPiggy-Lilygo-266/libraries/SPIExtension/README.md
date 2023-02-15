# SPIExtension
SPI extension/wrapper for Arduino

## Usage

### Daisy Chain

```C++
#include <SPI.h>
#include <SPIExtension.h>

const uint8_t PIN_CS = 10;
const uint8_t NUM_DEVICE = 3;
SPIExtension<PIN_CS, NUM_DEVICE> spi;

uint8_t send_data[NUM_DEVICE] = {1, 2, 3};

void setup()
{
    SPI.begin();
    spi.attatch(SPI);
}

void loop()
{
    // send data[0] to far device, data[1] is middle, data[2] to near device
    spi.transfer(send_data);

    // print received data
    for (uint8_t i = 0; i < spi.size(); ++i)
    {
        Serial.print(spi.data(i), HEX); Serial.print(" ");
    }
    Serial.println();
}
```

## License

MIT
