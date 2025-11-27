/**
 * Generic Arduino SPI transfer callback
 */

#define HPW5500_SPI_TRANSFER_CALLBACK_ARDUINO(spi_object, cs_pin, clock) { \
    [](uint8_t *cmd, uint8_t *dataIn, uint8_t *dataOut, uint16_t size, HPW5500 *obj) { \
        digitalWrite(cs_pin, LOW); \
        spi_object.beginTransaction(SPISettings(clock, MSBFIRST, SPI_MODE0)); \
        spi_object.transferBytes(cmd, nullptr, HPW5500_CMD_SIZE); \
        spi_object.transferBytes(dataIn, dataOut, size); \
        spi_object.endTransaction(); \
        digitalWrite(cs_pin, HIGH); \
    } \
}

/**
 * Generic Arduino millisecond-delay transfer callback
 */

#define HPW5500_DELAY_ARDUINO [](uint16_t delay_ms) { delay(delay_ms); }
