#include <arduino.h>
#include <OneButton.h>

#define BUTTON_PIN 15
#define LED_PIN 18

// variables will change:
int led_state = HIGH;  // the current state of LED
int button_state;      // the current state of button
int last_button_state; // the previous state of button

OneButton button(BUTTON_PIN, true, true);

void singleClick()
{

  Serial.println("The button is pressed");

  // toggle state of LED
  led_state = !led_state;

  // control LED arccoding to the toggled state
  digitalWrite(LED_PIN, led_state);
}

void setup_button()
{
  button.attachClick(singleClick);
}

void setup()
{
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT); // set ESP32 pin to output mode

  setup_button();
}

void loop()
{
  button.tick();
  delay(10);
}