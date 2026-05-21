#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include <Adafruit_TSL2591.h>
#include "pwm_control.h"
#include "sensor_control.h"
#include "uart_handler.h"
#include "tyveri_sikring.h"

// Create PCA9685 object
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();


// Create TSL2591 sensor object
Adafruit_TSL2591 tsl = Adafruit_TSL2591(1);

unsigned long start;
unsigned long loop_timer;
bool up;
int cp;

static bool triggered_on[4] = {false};
static bool triggered_off[4] = {false};


/*
Eksempel til at scanne efter i2c adresser:

#include <Wire.h>

void setup() {
  Wire.begin();
  Serial.begin(9600);
  
  Serial.println("I2C Scanner");
  for (byte address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    if (Wire.endTransmission() == 0) {
      Serial.print("Found device at 0x");
      Serial.println(address, HEX);
    }
  }
}
-> Vil give os addresser der er forbundet til i2c bussen på mega2560.
*/
void setup() {
  // Initialize serial communication for debugging
  Serial.begin(115200);
  //Serial1.begin(9600); // RX1(19), TX1(18)
  //Serial2.begin(9600); // RX2(17), TX2(16)

  // Initialize I2C communication
  Wire.begin();

  // Initialize TSL2591
  //init_light_sensor();
  setup_pwm();
  start = millis();
  fade_led_async(0, 0, 4095, 500);
  fade_led_async(1, 0, 4095, 500);
  fade_led_async(2, 0, 4095, 500);
  fade_led_async(3, 0, 4095, 500);
  //Serial.println("PCA9685 initialized and ready!");
}

void loop() {

  //uart_handler_update();      // Handle all serial input
  //sensor_control_update();    // Update light sensor
  //update_tyveri_sikring();    // Update burglary state machine
  update_pwm_fade();          // Update LED fades
  //sensor_control_update();

  loop_timer = millis();


  if ((loop_timer - start > 1000) && !triggered_off[0]){
    fade_led_async(0, get_pwm(0), 0, 500);
    triggered_off[0] = true;
  }
  if ((loop_timer - start > 2000) && !triggered_off[1]){
    fade_led_async(1, get_pwm(1), 0, 500);
    triggered_off[1] = true;
  }
  if ((loop_timer - start > 3000) && !triggered_off[2]){
    fade_led_async(2, get_pwm(2), 0, 500);
    triggered_off[2] = true;
  }
  if ((loop_timer - start > 4000) && !triggered_off[3]){
    fade_led_async(3, get_pwm(3), 0, 500);
    start = loop_timer;
    triggered_off[3] = true;
  }
}


