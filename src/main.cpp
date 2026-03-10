#include <Arduino.h>

#if CONFIG_FREERTOS_UNICORE
    const BaseType_t app_core = 0;
#else
    const BaseType_t app_core = 1;
#endif

const int maxSize = 120;

static bool messageFlag = false;

static char *message = "";

void readSerialTask(void *p) {
    while(1) {
        if(!messageFlag){
            message = (char *)pvPortMalloc(sizeof(char) * maxSize);
            int i = 0;
            message[0] = '\0';
            while (1) {
                if (Serial.available()) {
                    char c = Serial.read();
                    if (c == '\n' || c == '\r') break;
                    message[i++] = c;
                    if(i >= 120) break;
                }
                vTaskDelay(100);
            }
            message[i] = '\0';
            if(message[0] != '\0') {
                messageFlag = true;
            }
        }
        vTaskDelay(200);
    }
}

void printToSerialTask(void *p) {
    while (1) {
        if(messageFlag) {
            Serial.println(message);
            vPortFree(message);
            messageFlag = false;
        }
        vTaskDelay(300);
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