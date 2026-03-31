#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <Adafruit_BusIO_Register.h>
#include <Adafruit_GFX.h>
#include <HTTPClient.h>

#if CONFIG_FREERTOS_UNICORE
  const int app_core = 0;
#else
  const int app_core = 1;
#endif

static QueueHandle_t apiQueue;
const int queueLength = 10;

typedef struct {
  char *date;
  char *time;
} datetime;

const int sdaWire = 21;
const int sclWire = 22;
const int displayWidth = 128;
const int displayHeight = 64;

static char available = 0;

static SemaphoreHandle_t mutex;

static JsonDocument doc;
static HTTPClient client;

Adafruit_SSD1306 display(displayWidth, displayHeight, &Wire, -1);

// TaskA Sera responsavel pela comunicacao com a API de data e hora
// TaskB ira realizar todas as atualizacoes do display

void taskA(void *p);
void taskB(void *p);

void setup() {
  Serial.begin(115200);

  if(!Wire.begin(sdaWire, sclWire)) {
    Serial.println("Failed to initialize wires");
    for(;;);
  }

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)){
    Serial.println("Failed to initialize display");
    for(;;);
  }

  apiQueue = xQueueCreate(queueLength, sizeof(datetime));

  xTaskCreatePinnedToCore(
                taskA, 
                "Task to consume the API",
                8192,
                NULL,
                1,
                NULL,
                app_core);
}

void loop() {

}

void taskA(void *p) {
  WiFi.begin("Wokwi-GUEST");
  Serial.println("Connecting to wifi");
  vTaskDelay(pdMS_TO_TICKS(300));
  while(!WiFi.isConnected()) {
    Serial.println("Unable to connect to WiFi, try again!");
    vTaskDelay(pdMS_TO_TICKS(200));
  }

  while (1) {
    bool err = client.begin("https://worldtimeapi.org/api/timezone/America/Recife");
    if (!err) {
      Serial.println("Connecting to client...");
      continue;
    }

    int protocolResp = client.GET();

    String resp = client.getString();

    Serial.println(resp);

    client.end();
    vTaskDelay(pdMS_TO_TICKS(300));
  }
}

void taskB(void *p){
  while (1) {
    if(available){
    
    }
  } 
}