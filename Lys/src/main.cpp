#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include <Adafruit_TSL2591.h>
#include "pwm_control.h"
#include "sensor_control.h"

// Create PCA9685 object
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();


// Create TSL2591 sensor object
Adafruit_TSL2591 tsl = Adafruit_TSL2591(1);
unsigned long start;
unsigned long loop_timer;
bool up;
int cp;
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

  // Initialize I2C communication
  Wire.begin();
  
  // Initialize TSL2591
  //init_light_sensor();
  Serial.println("I2C Scanner");
  for (byte address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    if (Wire.endTransmission() == 0) {
      Serial.print("Found device at 0x");
      Serial.println(address, HEX);
    }
  }
  
  setup_pwm();
  start = millis();
  up = true;
  Serial.println("PCA9685 initialized and ready!");
  
}

void loop() {
  loop_timer = millis();
  //sensor_control_update();
  //update_selected_preset();  // Add this to check for serial input
  update_pwm_fade();


  //turn_all_lights_off();

  // Turn all lights off if 3 seconds have passed
    if (!is_fade_active(0) && !is_fade_active(1) &&
        !is_fade_active(2) && !is_fade_active(3)){
      fade_led_async(0, get_pwm(0), 0, 500);
      fade_led_async(1, get_pwm(1), 0, 500);
      fade_led_async(2, get_pwm(2), 0, 500);
      fade_led_async(3, get_pwm(3), 0, 500);
      up = false;
    }

  // turn all lights off if 6 seconds have passed
  // and then reset timer.
    if (!is_fade_active(0) && !is_fade_active(1) &&
        !is_fade_active(2) && !is_fade_active(3)){
      fade_led_async(0, get_pwm(0), 3072, 1000);
      fade_led_async(1, get_pwm(1), 3072, 1000);
      fade_led_async(2, get_pwm(2), 3072, 1000);
      fade_led_async(3, get_pwm(3), 3072, 1000);
      up = true;
      start = loop_timer; 
    }
  /* Serial.println("I2C Scanner");
  for (byte address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    if (Wire.endTransmission() == 0) {
      Serial.print("Found device at 0x");
      Serial.println(address, HEX);
    }
  }
  delay(1000); */
  
  

}
  /* 
  TEST 1: 
    FIND ALLE ENHEDERS I2C ADDRESSE - ALLEREDE IMPLEMENTERET
  TEST 2 OG 3: 
    ALLE ENHEDER KAN TÆNDES/SLUKKES INDIVIDUELT
      - VI SLÅR SENSOR FRA OG KALDER HVER FADE PÅ HVER AF DE 4 LED'ER
        INDIVIDUELT, SKRUER OP FRA HØJ TIL LAV. 
        TÆNKER PWM1-> HØJ -> LAV ->PWM2 -> HØJ -> LAV
        M. FADES.
      - KAN EVT. LAVE NOGET SINUS-WAVE SJOV:
      
      // Update every 20 ms (~50 Hz)
        if (now - last_update >= 20) {
          last_update = now;

          float speed = 0.002;   // controls wave speed
          float t = now * speed;

          int max_val = 3072;

          for (int ch = 0; ch < 4; ch++) {

            float phase = ch * (PI / 2);   // offset per LED
            float value = sin(t + phase);  // -1 to +1

            value = value * 0.5 + 0.5;     // 0 to 1
            int target = value * max_val;

            // Fade toward new target
            fade_led_async(ch, pwm.getPWM(ch), target, 20);
          }
        }

  TEST 4:
    SENSOR SLUTTES TIL IGEN OG VI TJEKKER TERA TERM FOR BEREGNET LUX
    OG SER FYSISK OM LED'ERNES PWM ØGES ELLER DÆMPES KORREKT. 
    - Her kører vi bare bare alle de implementerede update-funktioner,
      og undersøger output i tera term (ALLEREDE IMPLEMENTERET.)
  */



