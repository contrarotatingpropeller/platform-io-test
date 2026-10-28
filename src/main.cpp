#include <Arduino.h>
#include <WiFi.h>

const char* ssid     = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";

// --- Task handles ---
TaskHandle_t Task1Handle = NULL;
TaskHandle_t Task2Handle = NULL;

// ── LED Blink Test (Core 0) ──────────────────────────
void blinkLEDTask(void *pvParameters) {
    const int ledPin = 2;  // Built-in LED on most ESP32 dev boards
    pinMode(ledPin, OUTPUT);

    for (;;) {
        digitalWrite(ledPin, HIGH);
        Serial.println("[Core 0] LED ON");
        vTaskDelay(500 / portTICK_PERIOD_MS);

        digitalWrite(ledPin, LOW);
        Serial.println("[Core 0] LED OFF");
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}

// ── Wi-Fi Connection Test (Core 1) ────────────────────
void wifiTask(void *pvParameters) {
    Serial.println("[Core 1] Connecting to WiFi...");
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        vTaskDelay(500 / portTICK_PERIOD_MS);
        Serial.print(".");
        attempts++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println();
        Serial.print("[Core 1] WiFi connected! IP: ");
        Serial.println(WiFi.localIP());
        Serial.print("[Core 1] RSSI: ");
        Serial.print(WiFi.RSSI());
        Serial.println(" dBm");
    } else {
        Serial.println();
        Serial.println("[Core 1] WiFi connection FAILED.");
    }

    // Keep task alive — periodically report uptime
    for (;;) {
        Serial.printf("[Core 1] Uptime: %lu s | Free heap: %u bytes\n",
                      millis() / 1000, ESP.getFreeHeap());
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}

void setup() {
    Serial.begin(115200);
    delay(1000);  // Give serial monitor time to open

    Serial.println("==================================");
    Serial.println("   ESP32 PlatformIO Test  ");
    Serial.println("==================================");
    Serial.printf("Chip model : %s\n", ESP.getChipModel());
    Serial.printf("Revision   : %d\n", ESP.getChipRevision());
    Serial.printf("Cores      : %d\n", ESP.getChipCores());
    Serial.printf("Flash size : %u bytes\n", ESP.getFlashChipSize());
    Serial.printf("Free heap  : %u bytes\n", ESP.getFreeHeap());
    Serial.println("==================================");

    // Create task on Core 0 (LED blink)
    xTaskCreatePinnedToCore(
        blinkLEDTask,   // Task function
        "BlinkLED",    // Name
        2048,          // Stack size
        NULL,          // Parameters
        1,             // Priority
        &Task1Handle,  // Handle
        0              // Core 0
    );

    // Create task on Core 1 (WiFi + uptime)
    xTaskCreatePinnedToCore(
        wifiTask,
        "WiFiTask",
        4096,
        NULL,
        1,
        &Task2Handle,
        1              // Core 1
    );
}

void loop() {
    // Nothing here — everything is handled by FreeRTOS tasks
}