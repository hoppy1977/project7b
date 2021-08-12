#include <Servo.h>

#define NUM_SERVOS 6

Servo servos[NUM_SERVOS];

int servo_pins[NUM_SERVOS] = {A0, A1, A2, A3, A4, A5};


void setup() {
  Serial.begin(115200);
  for(int row = 0; row < 4; row ++) {
    pinMode(9 + row, OUTPUT);
    digitalWrite(9 + row, HIGH);
  }
  for(int col = 0; col < 3; col ++) {
    pinMode(6 + col, INPUT_PULLUP);
  }

  for(int i = 0; i < NUM_SERVOS; i++) {
    servos[i].attach(servo_pins[i]); 
    servos[i].write(90); 
    delay(100);
  }

  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
}

//Currently selected lolly. -1 if none.
int selected_lolly = -1;

//Variables for controlling the current coin states.
const int coin_threshold = 850;
int coin_credit = 0;
unsigned long last_coin_timestamp = 0;
const int coin_debounce_time = 500;
const int dispensing_speed = 10;

//Load mode variables
const unsigned int hold_time_load_mode = 1500;
unsigned long hold_timestamp = 0;
unsigned long time_load_mode_pressed = 0;
boolean load_mode = false;
int last_loading_rack = -1;
const int loading_speed = 10;

void loop() {
  if(coin_credit > 0) {
    analogWrite(3, 128 + 127*sin(millis()/(float)250));
  } else {
    analogWrite(3, 0);
  }

  
  int coin_value = analogRead(A7);
  //Serial.println(coin_value);
  if(coin_value < coin_threshold && abs(millis() - last_coin_timestamp) > coin_debounce_time) {
    last_coin_timestamp = millis();
    coin_credit++;
    Serial.println("New credit: " + (String)coin_credit);
    detect_coin();
    
  }
  
  // put your main code here, to run repeatedly:
  int last_pressed = get_button_pressed();

  if(load_mode) {
    boolean load_mode_lights = millis() % 600 < 300;
    digitalWrite(2, load_mode_lights);
    digitalWrite(3, !load_mode_lights);
    
    if(last_pressed >= 0 && last_pressed <= 5) {
      last_loading_rack = last_pressed;
      servos[last_loading_rack].write(90 - loading_speed);
    }

    if(last_pressed == -1) {
      servos[last_loading_rack].write(90);
    }
    
  } else {
    if(last_pressed >= 0 && last_pressed <= 5 && selected_lolly != last_pressed) {
    selected_lolly = last_pressed;
    Serial.println("Selecting Lolly " + (String)selected_lolly);
    Serial.println("Checking credit...");
    if(coin_credit > 0) {
        Serial.println("Dispensing Lolly " + (String)selected_lolly);
        coin_credit--;
        Serial.println("New credit: " + (String)coin_credit);
        dispense_lolly(selected_lolly);
        
      } else {
        Serial.println("Not enough credit. Insert more coins.");
        flash_no_coins();
      }
    }
    selected_lolly = -1;
  }
  
  
  if(last_pressed == 9) {
    if(hold_timestamp == 0) {
      hold_timestamp = millis();
    } else {
      if(millis() >= hold_timestamp + hold_time_load_mode) {
        load_mode = !load_mode;
        //Reset lights
        digitalWrite(2, LOW);
        digitalWrite(3, LOW);
        Serial.println("Switched Load Mode to: " + (String)load_mode);
        hold_timestamp = 0;
      }
    }
  } else {
    hold_timestamp = 0;
  }

  //last_pressed = -1;
  delay(5);
}

void flash_no_coins() {
  int flash_time = 120;
  for(int i = 0; i < 4; i++) {
    digitalWrite(3, HIGH);
    delay(flash_time);
    digitalWrite(3, LOW);
    delay(flash_time);
  }
}

int get_button_pressed() {
  int i = -1;
  for(int row = 0; row < 4; row++) {
    digitalWrite(9 + row, LOW);
    delayMicroseconds(10);
    for(int col = 0; col < 3; col++) {
      if(!digitalRead(6 + col)) {
        i = row * 3 + col;
      }
    }
    digitalWrite(9 + row, HIGH);
  }
  return i;
}

void detect_coin() {
  Serial.println("Coin Detected!");
}

void dispense_lolly(int lolly) {
  if(lolly < 0 || lolly > 5) {
    return;
  }
  digitalWrite(3, LOW);
  digitalWrite(2, HIGH);
  //servos[lolly].attach(servo_pins[lolly]);
  //delayMicroseconds(10);
  servos[lolly].write(90 + dispensing_speed);
  
  delay(1600);
  servos[lolly].write(90);
  //delayMicroseconds(10);
  //servos[lolly].detach();
  digitalWrite(2, LOW);
}
