/*
 * ESP_CURRENT_PROBOT
 * This sketch sends data to a thingspeak channel via REST.
 * Based on the original template from www.arduinesp.com
*/
#include "EmonLib.h"

#define SAMPLES_TO_AVERAGE 2034
#define CURRENT_CLAMP_CALIBRATION_FACTOR 100

double ave_Irms = 0;

EnergyMonitor ct_sensor1; // Create emon an instance

void setup() {
  Serial.begin(115200);
  ct_sensor1.current(0, 48); // Current: input pin, calibration.
  Serial.println("Setup done");
}

void loop() {
  /*  The Node MCU can read analog values on pin A0 from 0 to 3.3v
      or on one of the reserved pins from 0 to 1V the program syntax is the same.
      The only diference between both is that A0 has a pair of resistor dividers
      The resistor dividers are only present on the nodeMCU
  */
  double Irms = ct_sensor1.calcIrms(SAMPLES_TO_AVERAGE);
  
  ave_Irms = ave_Irms + Irms/20 - ave_Irms/20;

  Serial.print(".");
  Serial.println(Irms);
  Serial.print("...");
  Serial.println(ave_Irms);
 
  delay(1000);
}

