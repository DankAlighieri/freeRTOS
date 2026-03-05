#include <Arduino.h>

#if CONFIG_FREERTOS_UNICORE
    const BaseType_t app_core = 0;
#else
    const BaseType_t app_core = 1;
#endif

// Duas tasks: Umas escuta uma mensagem do monitor serial
// Outra envia de volta a mensagem via UART
// 

static bool messageFlag = false;

static char *message = "";

void readSerialTask(void *p) {
    
}

void printToSerialTask(void *p) {
    if(messageFlag) {

    }
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