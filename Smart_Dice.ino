
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

// SH1106 I2C OLED, 128x64
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define OLED_ADDRESS 0x3C

Adafruit_SH1106G display(
  SCREEN_WIDTH,
  SCREEN_HEIGHT,
  &Wire,
  OLED_RESET
);

const int buttonPin = 2;
const int buzzerPin = 8;

int diceNumber = 1;

void showWelcome() {
  display.clearDisplay();
  display.setTextColor(SH110X_WHITE);

  display.setTextSize(2);
  display.setCursor(12, 8);
  display.println("SMART DICE");

  display.setTextSize(1);
  display.setCursor(20, 38);
  display.println("Press button to roll");

  display.display();
}

void showDice(int number, bool rolling) {
  display.clearDisplay();
  display.setTextColor(SH110X_WHITE);

  display.setTextSize(1);
  display.setCursor(30, 0);
  display.println("SMART DICE");

  display.drawLine(0, 12, 127, 12, SH110X_WHITE);

  display.setCursor(35, 17);
  if (rolling) {
    display.println("Rolling...");
  } else {
    display.println("Your number:");
  }

  // Dice outline
  display.drawRoundRect(42, 29, 44, 34, 5, SH110X_WHITE);

  // Large dice number
  display.setTextSize(3);
  display.setCursor(54, 34);
  display.print(number);

  display.display();
}

void rollDice() {
  for (int i = 0; i < 15; i++) {
    diceNumber = random(1, 7);

    showDice(diceNumber, true);

    tone(buzzerPin, 700 + (i * 60), 40);
    delay(80);
  }

  // Final result
  diceNumber = random(1, 7);
  showDice(diceNumber, false);

  tone(buzzerPin, 1500, 250);

  Serial.print("Dice result: ");
  Serial.println(diceNumber);
}

void setup() {
  Serial.begin(9600);

  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(buzzerPin, OUTPUT);

  if (!display.begin(OLED_ADDRESS, true)) {
    Serial.println("SH1106 OLED not detected!");
    while (true) {
      delay(10);
    }
  }

  randomSeed(analogRead(A0));

  showWelcome();
  tone(buzzerPin, 1200, 100);
}

void loop() {
  if (digitalRead(buttonPin) == LOW) {
    delay(30);  // Button debounce

    if (digitalRead(buttonPin) == LOW) {
      rollDice();

      // Wait for button release
      while (digitalRead(buttonPin) == LOW) {
        delay(10);
      }

      delay(30);
    }
  }
}
