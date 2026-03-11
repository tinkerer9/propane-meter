#include <SevSeg.h>
#include "ADS1X15.h"

#define MODE_SWITCH_PIN A0

#define ADC_ADDRESS 0x48  // I2C address for the ADC
#define ADC_PIN 0         // Analog input pin for the ADC; 0-3

// Constants for conversion formula
const float m = -0.218859;   // slope
const float b = 345.834955;  // intercept

SevSeg sevseg;
ADS1115 ADS(ADC_ADDRESS);

unsigned long lastMillis = 0;

const byte digitPins[] = { 2, 3, 4, 5 };                    // Pins for the 7-segment display digits
const byte segmentPins[] = { 6, 7, 8, 9, 10, 11, 12, 13 };  // Pins for the 7-segment display segments

void setup() {
  Serial.begin(9600);
  pinMode(MODE_SWITCH_PIN, INPUT);

  sevseg.begin(COMMON_CATHODE, 4, digitPins, segmentPins, 0);  // Initialize the 7-segment display

  Wire.begin();
  if (!ADS.begin() || !ADS.isConnected()) {
    sevseg.setChars("----");  // Display error message on connection failure
    Serial.println("Connection Error");

    while (true) {  // Stay in an infinite loop to signal error on display
      sevseg.refreshDisplay();
    }
  }

  ADS.setGain(0);  // Set ADC gain to default value
}

void loop() {
  if (millis() - lastMillis >= 1000) {  // Execute every second
    bool mode = digitalRead(MODE_SWITCH_PIN);
    int16_t rawValue = ADS.readADC(ADC_PIN);  // Read raw ADC value (16 bit)

    if (rawValue < 0) {
      sevseg.setChars("----");  // Display error if reading fails
      Serial.println("Connection Error");
    } else {
      if (mode) {                                                      // If mode switch is in percent/processed mode
        int processedValue = rawValueToProcessedValue(rawValue / 10);  // Convert raw value
        sevseg.setNumber(constrain(processedValue, 0, 100));           // Display processed value (0-100)
        
        Serial.print(processedValue);
        Serial.println("%");
      } else {                            // If mode switch is in raw mode
        sevseg.setNumber(rawValue / 10);  // Display raw value (divided to fit on the display)
        Serial.println(rawValue);         // Print full raw value
      }
    }
    lastMillis = millis();  // Update timing
  }

  sevseg.refreshDisplay();  // Refresh the display continuously
}

int rawValueToProcessedValue(int rawValue) {  // Slope-intercept form
  return m * rawValue + b;                    // Implicitly converted to int when returning
}
