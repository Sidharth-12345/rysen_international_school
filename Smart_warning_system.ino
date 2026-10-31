
#include <Servo.h>

// Pin configuration
const int firePin = 4;
const int gasPin = A0;
const int rainPin = 2;
const int buzzerPin = 6;
const int servoPin = 3;

// Gas threshold
const int gasThreshold = 200;

// Servo object
Servo rainServo;

void setup() {

  Serial.begin(9600);

  pinMode(firePin, INPUT);
  pinMode(rainPin, INPUT);
  pinMode(buzzerPin, OUTPUT);

  rainServo.attach(servoPin);
  rainServo.write(0);

  digitalWrite(buzzerPin, LOW);

  Serial.println("Fire, Smoke and Rain Detection System");
  Serial.println("System Started");
}

void loop() {

  // Read sensor values
  int fireState = digitalRead(firePin);
  int gasValue = analogRead(gasPin);
  int rainState = digitalRead(rainPin);

  // Display readings
  Serial.print("Fire Sensor: ");
  Serial.print(fireState);

  Serial.print(" | MQ2 Analog Value: ");
  Serial.print(gasValue);

  Serial.print(" | Rain Sensor: ");
  Serial.println(rainState);

  // Rain detection
  if (rainState == LOW) {

    rainServo.write(90);
    Serial.println("RAIN DETECTED");

  }
  else {

    rainServo.write(0);

  }

  // Fire or smoke detection
  if (fireState == LOW || gasValue > gasThreshold) {

    digitalWrite(buzzerPin, HIGH);

    if (fireState == LOW) {
      Serial.println("WARNING: FIRE DETECTED!");
    }

    if (gasValue > 100) {
      Serial.println("WARNING: SMOKE/GAS DETECTED!");
    }

  }
  else {

    digitalWrite(buzzerPin, LOW);

  }

  delay(200);
}
