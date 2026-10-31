const int sampleWindow = 50;  // Sample window width in mS (50 mS = 20Hz)
const int AMP_PIN = A0;       // Preamp output pin connected to A0
const int LED_PIN1 = 2;       // OuD pintput LE
const int LED_PIN2 = 3;       // OuD pintput LE
const int LED_PIN3 = 4;       // OuD pintput LE
const int LED_PIN4 = 5;       // OuD pintput LE
const int LED_PIN5 = 6;       // OuD pintput LE
const int LED_PIN6 = 7;       // OuD pintput LE
const int LED_PIN7 = 8;       // OuD pintput LE
const int LED_PIN8 = 9;       // OuD pintput LE
const int LED_PIN9 = 10;       // OuD pintput LE

// Adjust this threshold based on your microphone's sensitivity and room noise
const int THRESHOLD = 370;     

// Variables to handle the clapping logic
bool ledState = false;           // Keeps track of whether the LED is currently ON or OFF
unsigned int clapCount = 0;      // Counts the total number of claps detected
unsigned long lastClapTime = 0;  // Remembers when the last clap happened
const int CLAP_DELAY = 300;      // Milliseconds to wait before listening for the next clap (prevents double-triggering)

unsigned int sample;

void setup()
{
  Serial.begin(9600);
  pinMode(LED_PIN1, OUTPUT);   // Set the LED pin as an output
  pinMode(LED_PIN2, OUTPUT);   // Set the LED pin as an output
  pinMode(LED_PIN3, OUTPUT);   // Set the LED pin as an output
  pinMode(LED_PIN4, OUTPUT);   // Set the LED pin as an output
  pinMode(LED_PIN5, OUTPUT);   // Set the LED pin as an output
  pinMode(LED_PIN6, OUTPUT);   // Set the LED pin as an output
  pinMode(LED_PIN7, OUTPUT);   // Set the LED pin as an output
  pinMode(LED_PIN8, OUTPUT);   // Set the LED pin as an output
  pinMode(LED_PIN9, OUTPUT);   // Set the LED pin as an output
  digitalWrite(LED_PIN1, LOW); // Ensure LED starts OFF
  digitalWrite(LED_PIN2, LOW); // Ensure LED starts OFF
  digitalWrite(LED_PIN3, LOW); // Ensure LED starts OFF
  digitalWrite(LED_PIN4, LOW); // Ensure LED starts OFF
  digitalWrite(LED_PIN5, LOW); // Ensure LED starts OFF
  digitalWrite(LED_PIN6, LOW); // Enstarts OFFsure LED 
  digitalWrite(LED_PIN7, LOW); // Enstarts OFFsure LED
  digitalWrite(LED_PIN8, LOW); // Enstarts OFFsure LED
  digitalWrite(LED_PIN9, LOW); // Enstarts OFFsure LED
}

void loop()
{
  unsigned long startMillis = millis(); // Start of sample window
  unsigned int peakToPeak = 0;          // peak-to-peak level

  unsigned int signalMax = 0;
  unsigned int signalMin = 1024;

  // Collect data for 50 mS
  while (millis() - startMillis < sampleWindow)
  {
    sample = analogRead(AMP_PIN);
    if (sample < 1024)  // toss out spurious readings
    {
      if (sample > signalMax)
      {
        signalMax = sample;  // save just the max levels
      }
      else if (sample < signalMin)
      {
        signalMin = sample;  // save just the min levels
      }
    }
  }
  
  peakToPeak = signalMax - signalMin;  // max - min = peak-peak amplitude
  
  // Print amplitude for debugging (optional, you can comment this out if it's too fast)
  // Serial.print("Amplitude: ");
  // Serial.println(peakToPeak);

  // Check if sound crossed threshold AND if enough time has passed since the last clap
  if (peakToPeak > THRESHOLD && (millis() - lastClapTime > CLAP_DELAY)) 
  {
    clapCount++;                       // Increase the clap counter
    ledState = !ledState;              // Toggle the LED state (if false make true, if true make false)
    
    digitalWrite(LED_PIN1, ledState);   // Update the physical LED
    digitalWrite(LED_PIN2, ledState);   // Update the physical LED
    digitalWrite(LED_PIN3, ledState);   // Update the physical LED
    digitalWrite(LED_PIN4, ledState);   // Update the physical LED
    digitalWrite(LED_PIN5, ledState);   // Update the physical LED
    digitalWrite(LED_PIN6, ledState);   // Update the physical LED
    digitalWrite(LED_PIN7, ledState);   // Update the physical LED
    digitalWrite(LED_PIN8, ledState);   // Update the physical LED
    digitalWrite(LED_PIN9, ledState);   // Update the physical LED
    
    lastClapTime = millis();           // Record the time we detected this clap

    // Print the action to the Serial Monitor
    Serial.print("Clap Detected! Total claps: ");
    Serial.print(clapCount);
    if (ledState) {
      Serial.println(" -> LED is ON");
    } else {
      Serial.println(" -> LED is OFF");
    }
  }
}