#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ---------- YOUR PCB CONFIGURATION ----------

// Replace -1 with the actual Arduino pin identifiers.
// Do not guess these from the Fusion model.
constexpr int BUTTON_1_PIN = -1;
constexpr int BUTTON_2_PIN = -1;
constexpr int BUTTON_3_PIN = -1;

constexpr int LED_1_PIN = -1;
constexpr int LED_2_PIN = -1;
constexpr int LED_3_PIN = -1;

// Set false if your LEDs turn ON when the pin is LOW.
constexpr bool LED_ACTIVE_HIGH = true;

// Verify these against your actual OLED.
constexpr int SCREEN_WIDTH = 128;
constexpr int SCREEN_HEIGHT = 64;
constexpr uint8_t OLED_ADDRESS = 0x3C;

// -1 assumes no dedicated OLED reset pin.
constexpr int OLED_RESET_PIN = -1;

// --------------------------------------------

static_assert(
  BUTTON_1_PIN >= 0 &&
  BUTTON_2_PIN >= 0 &&
  BUTTON_3_PIN >= 0 &&
  LED_1_PIN >= 0 &&
  LED_2_PIN >= 0 &&
  LED_3_PIN >= 0,
  "Replace all six -1 pin values with your PCB connections."
);

constexpr int CHANNELS = 3;
constexpr unsigned long DEBOUNCE_MS = 30;

const int buttonPins[CHANNELS] = {
  BUTTON_1_PIN, BUTTON_2_PIN, BUTTON_3_PIN
};

const int ledPins[CHANNELS] = {
  LED_1_PIN, LED_2_PIN, LED_3_PIN
};

bool ledOn[CHANNELS] = {false, false, false};
bool lastRaw[CHANNELS] = {HIGH, HIGH, HIGH};
bool stableState[CHANNELS] = {HIGH, HIGH, HIGH};

unsigned long lastChange[CHANNELS] = {0, 0, 0};

bool displayReady = false;

Adafruit_SSD1306 display(
  SCREEN_WIDTH,
  SCREEN_HEIGHT,
  &Wire,
  OLED_RESET_PIN
);

// Set the output voltage needed for this LED's wiring.
void updateLed(int channel) {
  bool outputHigh = LED_ACTIVE_HIGH
                      ? ledOn[channel]
                      : !ledOn[channel];

  digitalWrite(ledPins[channel], outputHigh ? HIGH : LOW);
}

// Draw the current state of all three LEDs.
void updateDisplay() {
  if (!displayReady) return;

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);

  display.println("LED CONTROLLER");
  display.println();

  for (int i = 0; i < CHANNELS; i++) {
    display.print("LED ");
    display.print(i + 1);
    display.print(": ");
    display.println(ledOn[i] ? "ON" : "OFF");
  }

  display.display();
}

void setup() {
  Serial.begin(115200);

  for (int i = 0; i < CHANNELS; i++) {
    // Button must connect the input to GND when pressed.
    pinMode(buttonPins[i], INPUT_PULLUP);

    lastRaw[i] = digitalRead(buttonPins[i]);
    stableState[i] = lastRaw[i];
    lastChange[i] = millis();

    // Prepare the OFF level before enabling the output.
    updateLed(i);
    pinMode(ledPins[i], OUTPUT);
    updateLed(i);
  }

  // Uses the selected board's default SDA and SCL pins.
  Wire.begin();

  // Check whether something responds at the OLED address.
  Wire.beginTransmission(OLED_ADDRESS);
  bool addressResponds = Wire.endTransmission() == 0;

  if (addressResponds) {
    displayReady = display.begin(
      SSD1306_SWITCHCAPVCC,
      OLED_ADDRESS
    );
  }

  Serial.println("Three-button LED controller started.");
  Serial.println(
    displayReady
      ? "OLED initialized."
      : "OLED unavailable. Check wiring and configuration."
  );

  updateDisplay();
}

void loop() {
  unsigned long now = millis();
  bool screenChanged = false;

  for (int i = 0; i < CHANNELS; i++) {
    bool reading = digitalRead(buttonPins[i]);

    // Restart the debounce timer whenever the raw input changes.
    if (reading != lastRaw[i]) {
      lastRaw[i] = reading;
      lastChange[i] = now;
    }

    // Accept the new state after it remains stable.
    if (now - lastChange[i] >= DEBOUNCE_MS &&
        reading != stableState[i]) {

      stableState[i] = reading;

      // LOW means a new press. Release does not toggle the LED.
      if (stableState[i] == LOW) {
        ledOn[i] = !ledOn[i];
        updateLed(i);
        screenChanged = true;

        Serial.print("Button ");
        Serial.print(i + 1);
        Serial.print(" -> LED ");
        Serial.println(ledOn[i] ? "ON" : "OFF");
      }
    }
  }

  if (screenChanged) {
    updateDisplay();
  }
}void setup() {
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:

}
