-Download BPS_status_thingy.zip
-Unpack to the Arduino IDE folder
-Upload code to devices
-Wire devices accordingly
-Profit




Project Pinout and Connection Details

Components:
1. ESP32
2. Arduino Nano
3. Longan Labs I2C CAN Bus Module
4. 3.5" TFT Display (ILI9488)
5. Button

ESP32 Pinout:
- TFT Display:
  - TFT_CS (Chip Select): GPIO 15
  - TFT_DC (Data/Command): GPIO 2
  - TFT_RST (Reset): GPIO 4
  - TFT_MOSI (Data Out): GPIO 23
  - TFT_SCLK (Clock Out): GPIO 18
  - TFT_MISO (Data In): GPIO 19
- Button:
  - Button Pin: GPIO 5

Arduino Nano Pinout:
- CAN Bus Module (Longan Labs I2C CAN Bus Module):
  - SDA: A4
  - SCL: A5
  - VCC: 5V
  - GND: GND

Connections:

ESP32 to TFT Display:
| TFT Display Pin | ESP32 Pin |
|-----------------|-----------|
| CS              | GPIO 15   |
| DC              | GPIO 2    |
| RST             | GPIO 4    |
| MOSI            | GPIO 23   |
| SCLK            | GPIO 18   |
| MISO            | GPIO 19   |
| VCC             | 3.3V or 5V|
| GND             | GND       |

ESP32 to Button:
| Button Pin | ESP32 Pin |
|------------|-----------|
| One Lead   | GPIO 5    |
| Other Lead | GND       |

Arduino Nano to CAN Bus Module:
| CAN Bus Module Pin | Arduino Nano Pin |
|--------------------|------------------|
| SDA                | A4               |
| SCL                | A5               |
| VCC                | 5V               |
| GND                | GND              |

Serial Communication:
- ESP32 and Arduino Nano communicate via Serial1 on the ESP32:
  - TX (Arduino Nano): Connected to RX (GPIO 16) on ESP32
  - RX (Arduino Nano): Connected to TX (GPIO 17) on ESP32
