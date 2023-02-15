#pragma once
#ifndef SPIEXTENSION_H
#define SPIEXTENSION_H

#include <Arduino.h>
#include "RingBuffer.h"

template <size_t PIN_CS, size_t N_CHAINS>
class SPIExtension
{
public:

    SPIExtension()
    {
        pinMode(PIN_CS, OUTPUT);
        digitalWrite(PIN_CS, HIGH);
    }

    void attatch(SPIClass& s) { spi = &s; }

    void transfer(uint8_t* chained_value)
    {
        recv_data.clear();
        digitalWrite(PIN_CS, LOW);
        for (uint8_t i = 0; i < N_CHAINS; i++)
            recv_data.push(spi->transfer(chained_value[i]));
        digitalWrite(PIN_CS, HIGH);
    }

    const uint8_t* data() const { return recv_data.data(); }
    uint8_t data(uint8_t idx) const { return recv_data[idx]; }
    uint8_t size() const { return recv_data.size(); }

private:

    SPIClass* spi;
    RingBuffer<uint8_t, N_CHAINS> recv_data;
};

#endif // SPIEXTENSION_H
