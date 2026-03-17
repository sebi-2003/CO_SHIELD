#include "LiquidCrystal_I2C.h"

// Configurare LCD
LiquidCrystal_I2C lcd(0x27, 20, 4);

// Configurare pinii pentru buzzer
int buzzerSignalPin = 8;
int buzzerVccPin = 9;

// Pini senzor
#define TEMP_SENSOR_PIN A1  // Pinul analogic conectat la senzorul LM35
#define GAS_SENSOR_PIN A0   // Pinul analogic conectat la senzorul de gaz

// Variabile pentru calibrare și mediere
float Ro = 10.0;
float A = 60.0;
float B = -0.6;
int numReadings = 10;
float tempReadings[10];
float gasReadings[10];
int readIndex = 0;
float totalTemp = 0;
float totalGas = 0;
float averageTemp = 0;
float averageGas = 0;

bool buzzerState = false;

void setup() {
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(1, 0);
  lcd.print("Gas: ");
  
  pinMode(buzzerSignalPin, OUTPUT);
  pinMode(buzzerVccPin, OUTPUT);
  digitalWrite(buzzerVccPin, LOW);
  digitalWrite(buzzerSignalPin, LOW);

  for (int i = 0; i < numReadings; i++) {
    tempReadings[i] = 0;
    gasReadings[i] = 0;
  }

  delay(5000);
}

void loop() {
  // === CITIRE SENZOR GAZ ===
  int gasValue = analogRead(GAS_SENSOR_PIN);
  float sensorResistance = (float)gasValue;
  float ratio = sensorResistance / Ro;
  float ppm = pow((ratio / A), (1.0 / B));

  // === CITIRE SENZOR TEMPERATURĂ (LM35) ===
  int analogTemp = analogRead(TEMP_SENSOR_PIN);
  float voltage = analogTemp * (5.0 / 1023.0); // Conversie în Volți
  float temperature = voltage * 100.0;         // LM35: 10mV per °C

  // === ACTUALIZARE MEDII ===
  totalTemp -= tempReadings[readIndex];
  totalGas -= gasReadings[readIndex];

  tempReadings[readIndex] = temperature;
  gasReadings[readIndex] = ppm;

  totalTemp += temperature;
  totalGas += ppm;

  averageTemp = totalTemp / numReadings;
  averageGas = totalGas / numReadings;

  readIndex++;
  if (readIndex >= numReadings) readIndex = 0;

  // === AFIȘARE LCD ===
  lcd.setCursor(6, 0);
  lcd.print("       "); // curăță zona
  lcd.setCursor(6, 0);
  lcd.print(averageGas, 1);

  lcd.setCursor(0, 1);
  lcd.print("Temp: ");
  lcd.print(averageTemp, 1);
  lcd.print(" C   ");

  // === ALARMA ===
  if (averageGas > 50 || averageTemp > 40) {
    if (!buzzerState) {
      buzzerState = true;
      Serial.println("Buzzer ON");
    }
    digitalWrite(buzzerVccPin, HIGH);
  } else {
    if (buzzerState) {
      buzzerState = false;
      Serial.println("Buzzer OFF");
    }
    digitalWrite(buzzerVccPin, LOW);
  }

  delay(1000);
}
