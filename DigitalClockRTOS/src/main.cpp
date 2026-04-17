#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <Adafruit_BusIO_Register.h>
#include <Adafruit_GFX.h>
#include <HTTPClient.h>
#include "WifiCreds.h"

#if CONFIG_FREERTOS_UNICORE
  const int app_core = 0;
#else
  const int app_core = 1;
#endif

static QueueHandle_t apiQueue;
const int queueLength = 10;

typedef struct {
  char day[4];
  char month[4];
  char year[10];
  char time[8];
} dateTime_t;

const int sdaWire = 21;
const int sclWire = 22;
const int displayWidth = 128;
const int displayHeight = 64;

Adafruit_SSD1306 display(displayWidth, displayHeight, &Wire, -1);

// TaskA Sera responsavel pela comunicacao com a API de data e hora
// TaskB ira realizar todas as atualizacoes do display

void taskA(void *p);
void taskB(void *p);

void setup() {
  Serial.begin(115200);

  WiFi.begin(WIFISSID, WIFIPASS);
  Serial.println("Iniciando conexão WiFi...");

  while(WiFi.status() != WL_CONNECTED) {
    Serial.println("Aguardando WiFi...");
    vTaskDelay(pdMS_TO_TICKS(1000));
  }

  apiQueue = xQueueCreate(queueLength, sizeof(dateTime_t));

  xTaskCreatePinnedToCore(
                taskA, 
                "Task to consume the API",
                11900,
                NULL,
                1,
                NULL,
                app_core);

  xTaskCreatePinnedToCore(
                taskB, 
                "Task to update screen",
                11900,
                NULL,
                1,
                NULL,
                app_core);    
}

void loop() {
  //exit(0);
}

void parsedateTime_t(String resp) {
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, resp);
  
  if (error) {
    Serial.println("Failed to parse JSON");
    return;
  }

  dateTime_t newdateTime;

  snprintf(newdateTime.day, sizeof(newdateTime.day), "%02d", doc["day"].as<int>());
  snprintf(newdateTime.month, sizeof(newdateTime.month), "%02d", doc["month"].as<int>());
  snprintf(newdateTime.year, sizeof(newdateTime.year), "%d", doc["year"].as<int>());

  const char* timeStr = doc["time"] | "00:00"; 
  strncpy(newdateTime.time, timeStr, sizeof(newdateTime.time) - 1);
  newdateTime.time[sizeof(newdateTime.time) - 1] = '\0';
  
  if(xQueueSend(apiQueue, (void *) &newdateTime, 0) == pdPASS){
    Serial.println("Date-time sent to queue successfully.");
  }
}

void taskA(void *p) {
  WiFiClientSecure connection;
  connection.setInsecure();
  HTTPClient client;
  client.setTimeout(5000);
  while (1) {
    if(WiFi.status() == WL_CONNECTED) {
      Serial.println("Tentando conectar à API...");
      if(client.begin(connection, "https://timeapi.io/api/time/current/zone?timeZone=America/Recife")) {
        Serial.println("DNS resolvido, enviando GET...");
        int protocolResp = client.GET();
            if (protocolResp == HTTP_CODE_OK) {
              parsedateTime_t(client.getString());
            } 
            client.end();
        } else {
            Serial.println("Erro ao iniciar cliente");
            for(;;);
        }
    }
    vTaskDelay(pdMS_TO_TICKS(500));
  }
}


void updateScreenWithdateTime_t(dateTime_t dateTime) {
	display.clearDisplay();

  char date[20];
  snprintf(date, sizeof(date), "%s/%s/%s", dateTime.day, dateTime.month, dateTime.year);

  Serial.print("Date: ");
	Serial.print(date);

	Serial.print("Hour: ");
	Serial.println(dateTime.time);

	display.setCursor(50, 15);
	display.setTextSize(1,2);
	display.setTextColor(SSD1306_WHITE);
	display.print(dateTime.time);

	display.setCursor(35, 35);
	display.setTextSize(1,2);
	display.setTextColor(SSD1306_WHITE);
	display.print(date);

	display.display();
}

void taskB(void *p){
  if(!Wire.begin(sdaWire, sclWire)) {
    Serial.println("Failed to initialize wires");
    for(;;);
  }

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)){
    Serial.println("Failed to initialize display");
    for(;;);
  }

  while (1) {
    dateTime_t dateTime = {"", "", "", ""};
    if(xQueueReceive(apiQueue, (void *) &dateTime, 10) == pdPASS) {
      Serial.println("dateTime_t received from queue sucessfully!");
      updateScreenWithdateTime_t((dateTime_t)dateTime);
    }
    vTaskDelay(pdMS_TO_TICKS(500));
  } 
}