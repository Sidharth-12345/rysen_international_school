
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>

// I2C LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Ultrasonic sensor
const int trigPin = 6;
const int echoPin = 7;

// Servo motor
const int servoPin = 9;
Servo gateServo;

// LDR module and LED
const int ldrPin = 2;
const int ledPin = 3;

// Servo angles
const int closedAngle = 0;
const int openAngle = 90;

// Ultrasonic detection distance
const int detectionDistance = 10;

// Return servo after 2 seconds outside range
const unsigned long closeDelay = 2000;

bool gateOpen = false;
unsigned long clearStartTime = 0;

float getDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);

  digitalWrite(trigPin, LOW);

  unsigned long duration =
      pulseIn(echoPin, HIGH, 30000UL);

  if (duration == 0) {
    return -1;
  }

  return duration * 0.0343 / 2.0;
}

void openGate() {
  gateServo.write(openAngle);
  gateOpen = true;
  clearStartTime = 0;

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Object Detected");
  lcd.setCursor(0, 1);
  lcd.print("Gate Opening");
}

void closeGate() {
  gateServo.write(closedAngle);
  gateOpen = false;
  clearStartTime = 0;

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("TOLL GATE");
  lcd.setCursor(0, 1);
  lcd.print("Gate Closed");
}

void setup() {
  Serial.begin(9600);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  pinMode(ldrPin, INPUT);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  gateServo.attach(servoPin);
  gateServo.write(closedAngle);

  lcd.init();
  lcd.backlight();

  lcd.setCursor(0, 0);
  lcd.print("AUTO TOLL GATE");
  lcd.setCursor(0, 1);
  lcd.print("System Starting");

  delay(2000);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("TOLL GATE READY");
  lcd.setCursor(0, 1);
  lcd.print("Waiting Vehicle");
}

void loop() {
  // LDR logic: D2 LOW -> D3 LOW
  //             D2 HIGH -> D3 HIGH
  digitalWrite(ledPin, !digitalRead(ldrPin));

  float distance = getDistance();

  Serial.print("Distance: ");
  if (distance >= 0) {
    Serial.print(distance);
    Serial.println(" cm");
  } else {
    Serial.println("No valid echo");
  }

  // Object detected within 10 cm
  if (distance > 0 && distance <= detectionDistance) {
    if (!gateOpen) {
      openGate();
    }

    // Reset timer while object is detected
    clearStartTime = 0;
  }
  else if (gateOpen) {
    // Object is outside the detection range
    if (clearStartTime == 0) {
      clearStartTime = millis();
    }

    // Close after 2 seconds outside the range
    if (millis() - clearStartTime >= closeDelay) {
      closeGate();
    }
  }

  delay(50);
}
