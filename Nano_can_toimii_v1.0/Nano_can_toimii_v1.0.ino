#include <Wire.h>
#include "Longan_I2C_CAN_Arduino.h" // Ensure the correct library is included

const int i2cAddress = 0x25; // I2C address of the Longan I2C CAN bus module
I2C_CAN CAN(i2cAddress); // Initialize the CAN object

void setup() {
    Serial.begin(115200);
    Wire.begin(); // Initialize I2C communication

    // Initialize CAN bus
    while (CAN_OK != CAN.begin(CAN_250KBPS)) {
        Serial.println("CAN BUS FAIL!");
        delay(100);
    }
    Serial.println("CAN BUS OK!");
}

void loop() {
    unsigned char len = 0;
    unsigned char buf[8];
    if (CAN_MSGAVAIL == CAN.checkReceive()) { // Check for incoming CAN messages
        CAN.readMsgBuf(&len, buf); // Read message into buffer
        unsigned long canId = CAN.getCanId();

        // Send CAN message to ESP32 via Serial
        Serial.print(canId, HEX);
        Serial.print(" ");
        for (int i = 0; i < len; i++) {
            Serial.print(buf[i], HEX);
            Serial.print(" ");
        }
        Serial.println();
    }
}
