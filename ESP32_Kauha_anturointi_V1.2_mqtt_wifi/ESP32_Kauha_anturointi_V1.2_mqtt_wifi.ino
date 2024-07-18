#include <WiFi.h>
#include <PubSubClient.h>
#include <TFT_eSPI.h>
#include <SPI.h>

// WiFi credentials for client mode
const char* ssid = "Testi1";
const char* password = "12345678";

// MQTT Broker details
const char* mqtt_server = "192.168.10.1";
const int mqtt_port = 1883;

// TFT display setup
TFT_eSPI tft = TFT_eSPI();  // Create TFT object

// Use Hardware Serial1 on ESP32
HardwareSerial mySerial(1);

// Define CAN device IDs
const String BOOM_ID = "CF1291F";
const String STICK_ID1 = "CF2292G";
const String STICK_ID2 = "CF22920";
const String BUCKET_ID = "CF014E2";

bool boomAlive = false;
bool stickAlive = false;
bool bucketAlive = false;

unsigned long lastBoomMessage = 0;
unsigned long lastStickMessage1 = 0;
unsigned long lastStickMessage2 = 0;
unsigned long lastBucketMessage = 0;
unsigned long lastPublishTime = 0;
unsigned long lastReconnectAttempt = 0;

// Button setup
const int buttonPin = 5;  // GPIO pin connected to the button
int currentPage = 0; // Variable to track the current page
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 200;   // the debounce time; increase if the output flickers
bool lastButtonState = HIGH;  // the previous state of button pin
bool buttonState = HIGH;

// MQTT client setup
WiFiClient espClient;
PubSubClient client(espClient);

bool mqttConnected = false;

void setup() {
    Serial.begin(115200); // Initialize Serial for debugging
    mySerial.begin(115200, SERIAL_8N1, 16, 17); // Initialize Hardware Serial1 for communication with Arduino Nano

    // Initialize TFT display
    tft.init();
    tft.setRotation(1); // Set display orientation to landscape
    tft.fillScreen(TFT_BLACK); // Set background to black

    // Initialize button
    pinMode(buttonPin, INPUT_PULLUP);

    // Draw initial screen
    drawPage1();
    
    // Connect to WiFi in non-blocking manner
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi ");
    Serial.println(ssid);
}

void loop() {
    unsigned long currentTime = millis();

    // Handle WiFi connection
    if (WiFi.status() == WL_CONNECTED) {
        if (!mqttConnected && currentTime - lastReconnectAttempt > 5000) { // Attempt reconnect every 5 seconds
            lastReconnectAttempt = currentTime;
            Serial.println("WiFi connected.");
            Serial.print("IP address: ");
            Serial.println(WiFi.localIP());

            // Setup MQTT client
            client.setServer(mqtt_server, mqtt_port);
            client.setCallback(callback);

            mqttConnected = reconnect(); // Attempt to connect to MQTT
        }
    } else {
        Serial.print(".");
        delay(1000); // Print dot while waiting for connection
        tft.setCursor(10, 240);
        tft.setTextColor(TFT_RED, TFT_BLACK);
        tft.println("Connecting to WiFi...");
    }

    // Check button state with debounce
    bool reading = digitalRead(buttonPin);
    if (reading != lastButtonState) {
        lastDebounceTime = currentTime;
    }

    if ((currentTime - lastDebounceTime) > debounceDelay) {
        if (reading != buttonState) {
            buttonState = reading;
            if (buttonState == LOW) {
                currentPage = !currentPage;
                Serial.print("Button pressed. Changing to page: ");
                Serial.println(currentPage);
                if (currentPage == 0) {
                    drawPage1();
                } else {
                    drawPage2();
                }
            }
        }
    }

    lastButtonState = reading;

    // Only process CAN messages if on page 1
    if (mySerial.available()) {
        String canMessage = mySerial.readStringUntil('\n');
        Serial.println(canMessage);

        if (currentPage == 0) {
            // Extract CAN ID from the message
            int firstSpace = canMessage.indexOf(' ');
            String canId = canMessage.substring(0, firstSpace);

            // Update last message times and status based on CAN ID
            if (canId == BOOM_ID) {
                lastBoomMessage = currentTime;
                boomAlive = true;
            } else if (canId == STICK_ID1) {
                lastStickMessage1 = currentTime;
                stickAlive = true;
            } else if (canId == STICK_ID2) {
                lastStickMessage2 = currentTime;
                stickAlive = true;
            } else if (canId == BUCKET_ID) {
                lastBucketMessage = currentTime;
                bucketAlive = true;
            }

            // Update TFT display with device statuses
            updateDeviceStatus(150, 60, boomAlive);
            updateDeviceStatus(150, 120, stickAlive);
            updateDeviceStatus(150, 180, bucketAlive);
        } else {
            // Display raw CAN messages on page 2
            displayRawCANMessage(canMessage);
        }
    }

    // Check if devices are still alive based on last message time
    if (currentTime - lastBoomMessage > 2000) { // 2 seconds timeout
        boomAlive = false;
        updateDeviceStatus(150, 60, boomAlive);
    }
    if (currentTime - lastStickMessage1 > 2000 && currentTime - lastStickMessage2 > 2000) {
        stickAlive = false;
        updateDeviceStatus(150, 120, stickAlive);
    }
    if (currentTime - lastBucketMessage > 2000) {
        bucketAlive = false;
        updateDeviceStatus(150, 180, bucketAlive);
    }

    // Ensure MQTT client stays connected
    if (mqttConnected) {
        if (!client.connected()) {
            mqttConnected = reconnect();
        }
        client.loop();

        // Publish status periodically
        if (currentTime - lastPublishTime > 5000) { // Publish every 5 seconds
            lastPublishTime = currentTime;
            publishStatus();
        }
    }
}

void drawPage1() {
    Serial.println("Drawing Page 1");
    tft.fillScreen(TFT_BLACK); // Clear screen
    // Draw title
    tft.setTextSize(3);
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.setCursor(10, 10);
    tft.println("KAUHA-ANTUROINTI");

    // Draw device labels
    tft.setTextSize(2);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setCursor(10, 60);
    tft.println("BOOM:");
    tft.setCursor(10, 120);
    tft.println("STICK:");
    tft.setCursor(10, 180);
    tft.println("BUCKET:");

    // Draw futuristic separators
    tft.drawLine(0, 50, 480, 50, TFT_CYAN);
    tft.drawLine(0, 110, 480, 110, TFT_CYAN);
    tft.drawLine(0, 170, 480, 170, TFT_CYAN);
    tft.drawLine(0, 230, 480, 230, TFT_CYAN);
}

void drawPage2() {
    Serial.println("Drawing Page 2");
    tft.fillScreen(TFT_BLACK); // Clear screen
    // Draw title for raw CAN bus stream
    tft.setTextSize(3);
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.setCursor(10, 10);
    tft.println("RAW CAN BUS STREAM");

    // Draw labels for CAN IDs
    tft.setTextSize(2);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setCursor(10, 60);
    tft.println("BOOM:");
    tft.setCursor(10, 120);
    tft.println("STICK:");
    tft.setCursor(10, 180);
    tft.println("BUCKET:");

    // Display WiFi status
    tft.setTextSize(2);
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.setCursor(10, 240);
    tft.print("WiFi: ");
    if (WiFi.status() == WL_CONNECTED) {
        tft.println("Connected");
        tft.setCursor(10, 270);
        tft.print("SSID: ");
        tft.println(ssid);
    } else {
        tft.println("Not Connected");
    }
}

void displayRawCANMessage(String message) {
    // Extract CAN ID from the message
    int firstSpace = message.indexOf(' ');
    String canId = message.substring(0, firstSpace);
    String canData = message.substring(firstSpace + 1);

    // Display the message based on CAN ID
    if (canId == BOOM_ID) {
        tft.fillRect(10, 80, 460, 20, TFT_BLACK); // Clear previous data
        tft.setCursor(10, 80);
        tft.setTextColor(TFT_GREEN, TFT_BLACK);
        tft.print(canData);
    } else if (canId == STICK_ID1 || canId == STICK_ID2) {
        tft.fillRect(10, 140, 460, 20, TFT_BLACK); // Clear previous data
        tft.setCursor(10, 140);
        tft.setTextColor(TFT_GREEN, TFT_BLACK);
        tft.print(canData);
    } else if (canId == BUCKET_ID) {
        tft.fillRect(10, 200, 460, 20, TFT_BLACK); // Clear previous data
        tft.setCursor(10, 200);
        tft.setTextColor(TFT_GREEN, TFT_BLACK);
        tft.print(canData);
    }
}

void updateDeviceStatus(int x, int y, bool isAlive) {
    if (currentPage == 0) {
        tft.fillRect(x, y, 100, 20, TFT_BLACK); // Clear previous status
        tft.setTextColor(isAlive ? TFT_GREEN : TFT_RED, TFT_BLACK);
        tft.setCursor(x, y);
        tft.print(isAlive ? "Alive" : "Dead");
    }
}

void publishStatus() {
    // Publish the status of each device
    client.publish("can/boom/status", boomAlive ? "Alive" : "Dead");
    client.publish("can/stick/status", stickAlive ? "Alive" : "Dead");
    client.publish("can/bucket/status", bucketAlive ? "Alive" : "Dead");
}

bool reconnect() {
    // Attempt to connect
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP32Client")) {
        Serial.println("connected");
        // Once connected, publish an announcement...
        client.publish("can/status", "ESP32 connected");
        // ... and resubscribe
        client.subscribe("inTopic");
        return true;
    } else {
        Serial.print("failed, rc=");
        Serial.print(client.state());
        Serial.println(" try again in 5 seconds");
        return false;
    }
}

void callback(char* topic, byte* payload, unsigned int length) {
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    for (unsigned int i = 0; i < length; i++) {
        Serial.print((char)payload[i]);
    }
    Serial.println();
}
