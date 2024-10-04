#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <SPI.h>
#include <MFRC522.h>

// RFID pins (adjust if necessary)
#define SS_PIN 5    // ESP32 pin connected to RFID SS
#define RST_PIN 22  // ESP32 pin connected to RFID RST

// Create instance of MFRC522
MFRC522 rfid(SS_PIN, RST_PIN);  // Create MFRC522 instance

// Wi-Fi credentials
const char* ssid = "FTTH-DEARBC";        // Replace with your Wi-Fi SSID
const char* password = "12345678";       // Replace with your Wi-Fi password
const char* serverName = "http://192.168.1.51/check_rfid.php"; // Replace with your PHP server IP

// Function prototype declaration
void sendRFIDServer(String rfidTag); // Declare the function before setup()

void setup() {
    Serial.begin(115200);  // Initialize serial communications with the PC

    // Initialize SPI bus with explicit pin configuration for ESP32
    SPI.begin(18, 19, 23, SS_PIN);  // SCK=18, MISO=19, MOSI=23, SS=5
    rfid.PCD_Init();  // Init MFRC522
    Serial.println("RFID Reader Initialized");

    // Connect to Wi-Fi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(5000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");
    Serial.println("Scan a card...");
}

void loop() {
    // Check if a new card is present
    if (!rfid.PICC_IsNewCardPresent()) {
        delay(500);  // Small delay before trying again
        return;
    }

    // Try to read the card
    if (!rfid.PICC_ReadCardSerial()) {
        Serial.println("Failed to read card");
        delay(500);  // Add delay to avoid rechecking too fast
        return;
    }

    // Build the RFID tag UID string
    String rfidTag = "";
    for (byte i = 0; i < rfid.uid.size; i++) {
        rfidTag += String(rfid.uid.uidByte[i], HEX);
    }
    rfidTag.toUpperCase();  // Convert UID to uppercase for consistency
    Serial.println("RFID Tag: " + rfidTag);  // Print the RFID tag UID

    sendRFIDServer(rfidTag);  // Call the function to send RFID tag to server

    rfid.PICC_HaltA();  // Halt PICC (stop reading)
    delay(1000);  // Delay to avoid reading the same card multiple times rapidly
}

// Function to send RFID tag to server via HTTP GET
void sendRFIDServer(String rfidTag) {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        String url = String(serverName) + "?rfid=" + rfidTag;  // Prepare URL
        http.begin(url);
        int httpResponseCode = http.GET();  // Send HTTP GET request

        if (httpResponseCode > 0) {
            String response = http.getString();  // Get response
            Serial.println("Server Response: " + response);
        } else {
            Serial.println("Error on sending request: " + String(httpResponseCode));
        }
        http.end();  // Close connection
    } else {
        Serial.println("WiFi Disconnected");  // Check Wi-Fi status
    }
}
