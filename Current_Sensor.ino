#include "Adafruit_INA3221.h"
#include <Wire.h>
#include <LiquidCrystal.h>

// Creating an INA3221 object
Adafruit_INA3221 ina3221;

// Initialize the LCD library with the numbers of the interface pins
// LiquidCrystal(rs, enable, d4, d5, d6, d7)
LiquidCrystal lcd(12, 11, 9, 8, 7, 6);

void setup() {

  // Initialize LCD
  lcd.begin(16, 2);
  lcd.print("INA3221 Monitor");
  
  // Initialize serial for debugging
  Serial.begin(115200);
  while (!Serial)
    delay(10); // Wait for serial port to connect on some boards

  Serial.println("Adafruit INA3221 with LCD display");

  // Initialize the INA3221
  if (!ina3221.begin(0x40, &Wire)) { // can use other I2C addresses or buses
    Serial.println("Failed to find INA3221 chip");
    lcd.clear();
    lcd.print("INA3221 Error!");
    while (1)
      delay(10);
  }
  Serial.println("INA3221 Found!");
  
  // Show "Ready" on LCD
  lcd.setCursor(0, 1);
  lcd.print("Ready!");
  delay(1000);

  ina3221.setAveragingMode(INA3221_AVG_16_SAMPLES);

  // Set shunt resistances for all channels to 0.05 ohms
  for (uint8_t i = 0; i < 3; i++) {
    ina3221.setShuntResistance(i, 0.05);
  }

  // Set a power valid alert to tell us if ALL channels are between the two
  // limits:
  ina3221.setPowerValidLimits(3.0 /* lower limit */, 15.0 /* upper limit */);
}

void loop() {
  unsigned long currentTime = millis();
  // Get channel 1 values (channel index 0)
  float voltage_1 = ina3221.getBusVoltage(0);
  float current_1 = ina3221.getCurrentAmps(0) * 1000; // Convert to mA
  float power_1 = voltage_1 * current_1 / 1000; // Power in Watts

  // Display on serial monitor (for debugging)
  //Serial.print("Channel 1: Voltage = ");
  //Serial.print(voltage_1, 2);
  //Serial.print(" V, Current = ");
  //Serial.print(current_1, 2);
  //Serial.print(" mA, Power = ");
  //Serial.print(power_1, 2);
  //Serial.println(" W");
  //Serial.print('\n');

  Serial.println(current_1);

  // Display channel 1 values on LCD
  lcd.clear();
  
  // First row: Voltage and Current
  lcd.setCursor(0, 0);
  lcd.print("V:");
  lcd.print(voltage_1, 1);
  lcd.print("V");
  
  lcd.setCursor(8, 0);
  lcd.print("I:");
  lcd.print(current_1, 0);
  lcd.print("mA");
  
  // Second row: Power and Channel indicator
  lcd.setCursor(0, 1);
  lcd.print("P:");
  lcd.print(power_1, 2);
  lcd.print("W");

  // Delay for 1 second before the next reading
  delay(100);

}
