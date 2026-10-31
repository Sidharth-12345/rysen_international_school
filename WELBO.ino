
#include <Servo.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <math.h>

// Two OLEDs with the same address share the same I2C commands.
// One framebuffer is enough to drive both together.
Adafruit_SH1106G display(128, 64, &Wire, -1);

const int SCREEN_W = 128;
const int SCREEN_H = 64;

// Eye shape and animation
float pupilX = 0;
float pupilY = 0;
float targetX = 0;
float targetY = 0;

int rec = 2;
int play = 3;

unsigned long lastLook = 0;
unsigned long nextBlink = 0;
unsigned long blinkStart = 0;

bool blinking = false;
int blinkDuration = 170;

void drawEye(float px, float py, int lidHeight) {
  display.clearDisplay();

  // Rounded eye outline
  display.drawRoundRect(5, 3, 118, 58, 23, SH110X_WHITE);

  // Eye interior
  display.fillRoundRect(8, 6, 112, 52, 20, SH110X_WHITE);

  // Pupil movement limits
  int cx = 64 + (int)px;
  int cy = 32 + (int)py;

  // Keep pupil inside the eye
  if (cx < 42) cx = 42;
  if (cx > 86) cx = 86;
  if (cy < 23) cy = 23;
  if (cy > 41) cy = 41;

  // Dark pupil
  display.fillCircle(cx, cy, 17, SH110X_BLACK);

  // Iris ring and small texture details
  display.drawCircle(cx, cy, 17, SH110X_WHITE);
  display.drawCircle(cx, cy, 13, SH110X_WHITE);
  display.drawCircle(cx, cy, 9, SH110X_WHITE);

  // Cute eye shine
  display.fillCircle(cx - 6, cy - 6, 4, SH110X_WHITE);
  display.fillCircle(cx + 6, cy + 5, 2, SH110X_WHITE);

  // Eyelid closes from top and bottom while blinking
  if (lidHeight > 0) {
    display.fillRect(0, 0, SCREEN_W, lidHeight, SH110X_BLACK);
    display.fillRect(0, SCREEN_H - lidHeight,
                     SCREEN_W, lidHeight, SH110X_BLACK);
  }

  display.display();
}

Servo servo1;
Servo servo2;

const int trigPin = 7;
const int echoPin = 8;

bool servoActivated = false;
unsigned long activationTime = 0;

const unsigned long waitTime = 5000; // 5 seconds
const int detectionDistance = 15;   // 15 cm

void eye() {
  unsigned long now = millis();

  // Occasionally choose a new gaze direction
  if (now - lastLook > (unsigned long)random(700, 1500)) {
    targetX = random(-17, 18);
    targetY = random(-7, 8);
    lastLook = now;
  }

  // Smoothly move the pupil toward its target
  pupilX += (targetX - pupilX) * 0.12;
  pupilY += (targetY - pupilY) * 0.12;

  int lidHeight = 0;

  // Random natural blink
  if (!blinking && now >= nextBlink) {
    blinking = true;
    blinkStart = now;
    blinkDuration = random(130, 220);
  }

  if (blinking) {
    unsigned long elapsed = now - blinkStart;
    float phase = (float)elapsed / blinkDuration;

    if (phase >= 1.0) {
      blinking = false;
      nextBlink = now + random(1800, 4200);
    } else {
      // Close quickly, reopen smoothly
      float amount;
      if (phase < 0.45) {
        amount = phase / 0.45;
      } else {
        amount = 1.0 - ((phase - 0.45) / 0.55);
      }

      // Smooth animation curve
      amount = amount * amount * (3.0 - 2.0 * amount);
      lidHeight = (int)(amount * 32);
    }
  }

  drawEye(pupilX, pupilY, lidHeight);

  delay(25);
}
void setup() {
  Serial.begin(9600);
Wire.begin();
  Wire.setClock(100000);

  pinMode(rec, OUTPUT);
  pinMode(play, OUTPUT);

  if (!display.begin(0x3C, true)) {
    while (true) {
      delay(100);
    }
  }

  display.setContrast(180);

  // Start with eyes looking forward
  drawEye(0, 0, 0);

  randomSeed(analogRead(A0));
  nextBlink = millis() + random(1800, 4000);
  // Attach servos to pins
  servo1.attach(13);
  servo2.attach(12);

  // Set ultrasonic sensor pins
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // Initial position
  servo1.write(0);
  servo2.write(0);

  Serial.println("System Ready - Servos at 0 degrees");
}

void loop() {
  eye();
  long duration;
  float distance;

  // Trigger ultrasonic sensor
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Read echo with a 30 ms timeout
  duration = pulseIn(echoPin, HIGH, 30000);

  if (duration == 0) {
    distance = -1; // No valid reading
  } else {
    distance = duration * 0.0343 / 2.0;
  }

  Serial.print("Distance: ");
  if (distance > 0) {
    Serial.print(distance);
    Serial.println(" cm");
  } else {
    Serial.println("No object detected");
  }

  // Detect an object within 15 cm
  if (!servoActivated &&
      distance > 0 &&
      distance <= detectionDistance) {

    servo1.write(0);
    servo2.write(90);

    servoActivated = true;
    activationTime = millis();

    Serial.println("Person detected! Servos at 90 degrees");
    digitalWrite(play, HIGH);
    delay(500);
    digitalWrite(play, LOW);
    delay(5000);
  }

  // Return servos to 0 degrees after 5 seconds
  if (servoActivated &&
      millis() - activationTime >= waitTime) {

    servo1.write(90);
    servo2.write(0);

    servoActivated = false;
    Serial.println("5 seconds over! Servos returned to 0 degrees");
  }

  delay(100);
}
