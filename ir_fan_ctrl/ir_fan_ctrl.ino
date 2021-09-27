#include <IRLibSendBase.h>
#include <IRLib_HashRaw.h>
#include <LiquidCrystal.h>
#include "./data.h"

#define FAN_ON fan_on == true
#define FAN_OFF fan_on == false
#define KEYPAD_PIN A1

int temperature_pin = 0;
int led_pin = 7;
int button_pin = 9;
LiquidCrystal lcd(12, 11, 5, 4, 6, 2);

int led_state = LOW;
int button_state = 0;
IRsendRaw my_sender;

bool fan_on = false;
int thresholds[16] = {0, 0, 0, 0, 
                      0, 0, 0, 0, 
                      0, 0, 0, 0, 
                      0, 0, 0, 0};
char keypad[16] = {'1', '2', '3', 'A', 
                   '4', '5', '6', 'B', 
                   '7', '8', '9', 'C', 
                   '*', '0', '#', 'D'};
int temp_on = 100;
int temp_off = 0;

void calibrate_keypad();
void toggle_fan();
char keypad_read();
int get_keypad_input();

void setup() {
  Serial.begin(9600);
  pinMode(led_pin, OUTPUT);
  pinMode(button_pin, INPUT_PULLUP);
  lcd.begin(16, 2);
  calibrate_keypad();
}

void loop() {
  // get temperature reading
  int temp_reading = analogRead(temperature_pin);
  double tempK = log(10000.0 * ((1024.0 / temp_reading - 1)));
  tempK = 1 / (0.001129148 + (0.000234125 + (0.0000000876741 * tempK * tempK )) * tempK );       //  Temp Kelvin
  float tempC = tempK - 273.15;            // Convert Kelvin to Celcius
  float tempF = (tempC * 9.0)/ 5.0 + 32.0; // Convert Celcius to Fahrenheit

  // display temperature on LCD
  lcd.setCursor(0, 0);
  lcd.print("Temp         F  ");
  lcd.setCursor(6, 0);
  lcd.print(tempF);

  // display control temperatures on LCD
  lcd.setCursor(0, 1);
  lcd.print("    F  /      F ");
  lcd.setCursor(0, 1);
  lcd.print(temp_on);
  lcd.setCursor(10, 1);
  lcd.print(temp_off);

  // set control temperature if A or B was pressed
  // A = set temperature on
  // B = set temperature off
  char keycode = keypad_read();
  if (keycode == 'A' || keycode == 'B') {
    get_keypad_input(keycode);  
  }
  
  // toggle fan based on temperature
  if (tempF >= temp_on) {
    if (FAN_OFF) {
      toggle_fan();
    }
  } else if (tempF <= temp_off) {
    if (FAN_ON) {
      toggle_fan();  
    }
  }

  // toggle fan based on button press
  if (digitalRead(button_pin) == LOW && button_state == 0) {
    button_state = 1;
    toggle_fan();
    delay(500);
  }
  // wait for button to be unpressed before it can be pressed again
  if (digitalRead(button_pin) == HIGH && button_state == 1) {
    button_state = 0;  
  }
}

// Stores the thresholds for all the keypad keys
void calibrate_keypad() {
  lcd.setCursor(0, 0);
  lcd.print(" Calibrating... ");
  for (int i = 0; i < 16; ++i) {
    Serial.print("Press and hold the ");
    Serial.print(keypad[i]);
    Serial.println(" key");
    lcd.setCursor(0, 1);
    lcd.print("Press           ");
    lcd.setCursor(8, 1);
    lcd.print(keypad[i]);
    
    // wait for key to be pressed
    while (analogRead(KEYPAD_PIN) > 1000) {
      delay(100);  
    }

    // store the value of key press
    thresholds[i] = analogRead(KEYPAD_PIN);

    Serial.println("Let go of the key now");

    // wait for key to be let go
    while (analogRead(KEYPAD_PIN) < 1000) {
      delay(100);  
    }
  }

  // print the thresholds for each button
  Serial.print("thresholds = ");
  for (int i = 0; i < 16; ++i) {
    Serial.print(thresholds[i]);
    Serial.print(" ");
    if ((i+1) % 4 == 0) {
      Serial.print("\n             ");
    }
  }
  Serial.println();
}

// Turns the fan and LED on/off
void toggle_fan() {
  // print new fan state
  Serial.print("Turning fan "); 
  if (FAN_ON) {
    Serial.println("off.");  
  } else {
    Serial.println("on.");  
  }

  // toggle LED based on fan state
  if (digitalRead(led_pin) == LOW) {
    digitalWrite(led_pin, HIGH);
  } else {
    digitalWrite(led_pin, LOW);
  }

  // send IR
  my_sender.send(raw_data_power, RAW_DATA_POWER_LEN, 38);

  // change fan state
  fan_on = !fan_on;
}

// reads the keypad and returns the character that was pressed
char keypad_read() {
  // read keypad value
  int keypad_value = analogRead(KEYPAD_PIN);
  char keycode = 0;
  // loop through thresholds to see if value matches any of them
  for (int i = 0; i < 16; ++i) {
    // if value is close enough to a threshold, use that
    if (abs(keypad_value - thresholds[i]) < 5) {
      keycode = keypad[i];

      // wait until key is let go
      while(analogRead(KEYPAD_PIN) < 1000) {
        delay(100);
      }
      
      break;
    }
  }

  /* debug info
  Serial.print(keypad_value);
  Serial.print('\t');
  Serial.println(keycode);
  //*/
  
  return keycode;
}

// reads and sets the control temperatures
void get_keypad_input(char mode) {
  Serial.print("Getting input... Mode: ");
  Serial.println(mode);

  // exit if not a valid mode
  if (mode != 'A' && mode != 'B') {
    return;
  }

  Serial.println("Reading input...");
  char keycode = mode;
  int input = 0;
  short cursor_x = 0;
  lcd.setCursor(0, 0);

  if (mode == 'A') {
    lcd.print("Temp   for   ON ");
    lcd.setCursor(0, 1);
    lcd.print("    F           ");
    lcd.setCursor(0, 1);
  } else if (mode == 'B') {
    lcd.print("Temp   for   OFF");
    lcd.setCursor(0, 1);
    lcd.print("    F           ");
    lcd.setCursor(0, 1);
  }
  
  char input_chars[3] = {'\0', '\0', '\0'};
  while (keycode != 'D') {
    keycode = keypad_read();
    switch(keycode) {
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
      case '0':
        // only accept 3 digits
        if (cursor_x < 3) {
          input_chars[cursor_x] = keycode;
          lcd.print(keycode);
          lcd.setCursor(++cursor_x, 1);
        }
        break;
      // clear the last input
      case 'C':
        // only clear if there is an existing input
        if (cursor_x > 0) {
          --cursor_x;
          input_chars[cursor_x] = '\0';
          lcd.setCursor(cursor_x, 1);
          lcd.print(" ");
          lcd.setCursor(cursor_x, 1);
        }
        break;
    }
  }
  // get the temperature input as a number
  short num_null = 0;
  for (int i = 0; i < 3; ++i) {
    Serial.print("input_chars[");
    Serial.print(i);
    Serial.print("] = ");
    Serial.println(input_chars[i]);
    if (input_chars[i] == '\0') {
      ++num_null;  
    }
    Serial.print("num_null");
    Serial.print(" = ");
    Serial.println(num_null);
  }

  for (int i = cursor_x; i >= 0; --i) {
    if (input_chars[i] == '\0') {
      continue;  
    }
    input += (input_chars[i] - '0') * int(pow(10.0, cursor_x-i-num_null) + 0.5);
    Serial.print("input = ");
    Serial.println(input);
  }

  if (num_null == 3) {
    input = 0;  
  }

  // set the control temperature to the input value
  if (input > 0) {
    if (mode == 'A') {
      if (input > temp_off) {
        temp_on = input;
      }
    } else if (mode == 'B') {
      if (input < temp_on) {
        temp_off = input;
      }
    }
  }
  Serial.print("Control temps: ");
  Serial.print(temp_on);
  Serial.print(" / ");
  Serial.println(temp_off);
}
