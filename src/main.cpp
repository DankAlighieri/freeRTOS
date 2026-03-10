#include <Arduino.h>

#if CONFIG_FREERTOS_UNICORE
    const BaseType_t app_core = 0;
#else
    const BaseType_t app_core = 1;
#endif

// Duas tasks: 
// Uma escuta uma mensagem do monitor serial
// Outra envia de volta a mensagem via UART
// 

const int maxSize = 120;

static bool messageFlag = false;

static char *message = "";

void readSerialTask(void *p) {
    Serial.println("DEBUG: readSerialTask started");
    message = (char *)pvPortMalloc(sizeof(char) * maxSize);
    Serial.println("DEBUG: memory allocated");
    int i = 0;
    Serial.println("DEBUG: i initialized to 0");
    message[0] = '\0';
    Serial.println("DEBUG: message[0] set to null terminator");
    while (1) {
        Serial.println("DEBUG: waiting for serial input");
        if (Serial.available()) {
            Serial.println("DEBUG: serial data available");
            char c = Serial.read();
            Serial.print("DEBUG: received char: ");
            Serial.println(c);

            if (c == '\n' || c == '\r') {
                Serial.println("DEBUG: newline detected, breaking");
                break;
            }
            Serial.println("DEBUG: char is not newline");

            message[i++] = c;
            Serial.print("DEBUG: char stored at index ");
            Serial.println(i - 1);

            if(i >= 120) {
                Serial.println("DEBUG: max size reached, breaking");
                break;
            }
            Serial.println("DEBUG: size within limits");

            vTaskDelay(200);
            Serial.println("DEBUG: delay completed");
        }
    }
    message[i] = '\0';
    Serial.println("DEBUG: null terminator added");
    Serial.print("DEBUG: final message: ");
    Serial.println(message);
    if(message[0] != '\0') {
        messageFlag = true;
        Serial.println("DEBUG: messageFlag set to true");
    }
    Serial.println("DEBUG: readSerialTask ending");
}

void printToSerialTask(void *p) {
    if(messageFlag) {
        Serial.println(message);
    }
    vTaskDelay(200);
}

void setup() {
    Serial.begin(115200);

    xTaskCreatePinnedToCore(
        readSerialTask,
        "Read Serial",
        4096,
        NULL,
        1,
        NULL,
        app_core
    );

    xTaskCreatePinnedToCore(
        printToSerialTask,
        "Echo to Serial",
        1024,
        NULL,
        1,
        NULL,
        app_core
    );
}

void loop() {
}