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

const int ledPin = 12;

static byte ledState = LOW;

// Ler o terminal serial, aguardar o usuario enviar o tempo de delay, enviar para a fila B
void taskA(void *p) {
  char buffer[20];
  int index = 0;
  buffer[index] = '\0';
  char queue2Msg[10];

  while (1) {
    char queue2Msg[10];
    // Caso led pisque 100x, printar a mensagem da filaB
    if(xQueueReceive(queue2, queue2Msg, 5) == pdPASS) {
      queue2Msg[9] = '\0';
      Serial.print("Recebido");
      Serial.println(queue2Msg);
    }

    while (1) {
      if(Serial.available()){
        char c = Serial.read();
        if (c == '\n' || c == '\r') break;
        buffer[index++] = c;
      }
      if(index >= 20 - 1) break;
      vTaskDelay(10/portTICK_PERIOD_MS); // p desbloquear o core
    }
    buffer[index] = '\0';
    index = 0;

    char *t = strtok(buffer, " ");

    if (!strcmp(t, "delay") && t != NULL) {
      t = strtok(NULL, " ");

      int value = atoi(t);

      if(xQueueSend(queue1, (void *) &(value), 0) == pdPASS){
        Serial.println("Delay value added to queue1");
      }
    }
    vTaskDelay(10/portTICK_PERIOD_MS);
  }
  
}

// Ler delay da fila A, aplicar o delay para o blink do led, enviar msg caso o led pisque 100x
void taskB(void *p) {
  int del = 0, c = 0;
  while (1) {
    if (xQueueReceive(queue1, (void *) &del, 0) == pdPASS) {
      Serial.print("Delay received from queue1: ");
      Serial.println(del);
    }

    if (del) {
      Serial.println(c++);
      ledState = !ledState;
      digitalWrite(ledPin, ledState);
      vTaskDelay(((int) del)/portTICK_PERIOD_MS);
      ledState = !ledState;
      digitalWrite(ledPin, ledState);
      vTaskDelay(((int) del)/portTICK_PERIOD_MS);

      const char response[10] = "Blinked!";
      if(c >= 100) {
        if(xQueueSend(queue2, response, 5) == pdPASS) {
          Serial.print(response);
          Serial.println(" sent to queue");
          c = 0;
        }
      }
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

  pinMode(ledPin, OUTPUT);
}

void loop() {
}