/*
 * ESP_CURRENT_PROBOT
 * This sketch sends data to a thingspeak channel via REST.
 * Based on the original template from www.arduinesp.com
*/

#include <ESP8266WiFi.h>
#include "EmonLib.h"
#include "secrets.h"

/* ADC takes about 49.15 microseconds per measurment,
 * so this is about 6 periods of the 6-Hz AC line */
#define SAMPLES_TO_AVERAGE 2034

#define MOVING_AVERAGE_SIZE 20

#define MIN_CURRENT_THRESHOLD 0.5

/* About two minutes */
#define STATUS_FILTER_COUNTER 1200

/* Server period is in milliseconds
 * the REST api needs a minimum 15 sec delay between updates */
#define SERVER_UPDATE_PERIOD 20000

WiFiClient client;

EnergyMonitor ct_sensor1; 

//Global variables
double ave_Irms = 0;
uint32_t laundrybot_status = 0;
uint32_t last_update_time = 0;
uint32_t state_counter = 0;
const char* server = "api.thingspeak.com";
uint32_t server_port = 80;

void setup() {
  /* Console setup */
  Serial.begin(115200);

  /* Wifi Setup */
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  WiFi.begin(ssid, password);

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  /* Current sensor setup */
  ct_sensor1.current(0, 51);
  last_update_time = millis();

  Serial.println("Setup done");
}

void loop() {
  /*  The Node MCU can read analog values on pin A0 from 0 to 3.3v
   *  or on one of the reserved pins from 0 to 1V the program syntax is the same.
   *  The only diference between both is that A0 has a pair of resistor dividers
   *  The resistor dividers are only present on the nodeMCU */
  double Irms = ct_sensor1.calcIrms(SAMPLES_TO_AVERAGE);

  /* Moving average for current */
  ave_Irms = ave_Irms + Irms / MOVING_AVERAGE_SIZE - ave_Irms / MOVING_AVERAGE_SIZE;

  /* This is a filer that prevents false negatives
   * THe washing machine makes short pauses between cycle steps */
  if (ave_Irms > MIN_CURRENT_THRESHOLD) {
    laundrybot_status = 1;
    state_counter = 0;
  } else {
    if (laundrybot_status == 1) {
      state_counter++;
    }
    if (state_counter >= STATUS_FILTER_COUNTER){
      laundrybot_status = 0;
      state_counter = 0;
    }
  }
  delay(100);

  if (last_update_time + SERVER_UPDATE_PERIOD < millis()) {
    delay(1000);
    last_update_time = millis();
    //Serial.print(".");
    //Serial.println(millis());
    if (!client.connect(server, server_port)) {
      Serial.println("connection failed");
    } else {
      String postStr = apiKey;
      postStr += "&field1=";
      postStr += String(laundrybot_status);
      postStr += "&field2=";
      postStr += String(ave_Irms);
      postStr += "\r\n\r\n";

      client.print("POST /update HTTP/1.1\n");
      client.print("Host: api.thingspeak.com\n");
      client.print("Connection: close\n");
      client.print("X-THINGSPEAKAPIKEY: " + apiKey + "\n");
      client.print("Content-Type: application/x-www-form-urlencoded\n");
      client.print("Content-Length: ");
      client.print(postStr.length());
      client.print("\n\n");
      client.print(postStr);

      Serial.print("Status is: ");
      Serial.println(laundrybot_status);
      Serial.print("Average_Irms is: ");
      Serial.println(ave_Irms);
      Serial.println("% send to Thingspeak");
    }
    client.stop();
  }
}
