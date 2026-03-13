#include <Arduino.h>
#include <string.h>

#if CONFIG_FREERTOS_UNICORE
  const BaseType_t app_core = 0;
#else
  const BaseType_t app_core = 1;
#endif

static const UBaseType_t queueLength = 10;

static QueueHandle_t queue1;
static QueueHandle_t queue2;

// Task A
void taskA(void *p) {
  while (1)
  {
    char buffer[20];
    int index = 0;
    buffer[index] = '\0';

    while (1) {
      if(Serial.available()){
        char c = Serial.read();
        if (c == '\n' || c == '\r') break;
        buffer[index++] = c;
      }
      if(index >= 20 - 1) break;
    }
    buffer[index] = '\0';

    char *t = strtok(buffer, " ");

    if (!strcmp(t, "delay")) {
      while (t) {
        t = strtok(NULL, " ");
        break;
      }

      static int value = atoi(t);

      if(xQueueSend(queue1, (void *) &(value), 1) == pdPASS){
        Serial.println("Delay value added to queue");
      }
    }
    
    vTaskDelay(500/portTICK_PERIOD_MS);
  }
  
}
// Task B
void taskB(void *p) {
  int del = 0;
  while (1) {
    if (xQueueReceive(queue1, (void *) &del, 10) == pdPASS) {
      Serial.print("Delay received from queue: ");
      Serial.println(del);
    }
    
  }
  
}

void setup() {
  Serial.begin(115200);

  vTaskDelay(1000/portTICK_PERIOD_MS);

  queue1 = xQueueCreate(queueLength, sizeof(int));
  queue2 = xQueueCreate(queueLength, sizeof(char) * 10);

  xTaskCreatePinnedToCore(taskA, "task A", 4096, NULL, 1, NULL, app_core);
  xTaskCreatePinnedToCore(taskB, "task B", 4096, NULL, 1, NULL, app_core);

}

void loop() {

}