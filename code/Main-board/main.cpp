#include <Arduino.h>
#include <LiquidCrystal.h>


#define BLUETOOTH_TRANSMITION_START_CHAR '@'
#define BLUETOOTH_TRANSMITION_END_CHAR '#'
#define BLUETOOTH_TRANSMITION_DEL_CHAR '/'
#define BLUETOOTH_BAUD_RATE 9600
#define PWM_MAX 256
#define PWM_MIN 0

String decision;
int pwm_counter = 0;
int pwm_velocity = 64;
int dc_pinA = 9;
int dc_pinB = 10;
String bluetooth_buffer = "";
float globalHumidity = 0;
float globalTemperature = 0;
int rs = 12, en = 11;
int d4 = 7,d5 = 6,d6 = 5,d7 = 4;
bool stateUpdated = false;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);


void setup() {
  lcd.begin(20,4);
  pinMode(dc_pinA,OUTPUT);
  pinMode(dc_pinB,OUTPUT);
  pinMode(0, INPUT);
  Serial.setTimeout(0);
  Serial.begin(BLUETOOTH_BAUD_RATE);
}

void updateVelocity(){
  if (globalHumidity > 50){
    pwm_velocity = 0;
    decision = "0/100DC H>50";
  }
  else if (globalHumidity < 20){
    pwm_velocity = PWM_MAX/4;
    decision = "25/100DC H<20";
  }
  else if (globalHumidity >= 20 && globalHumidity <= 50){
    if (globalTemperature >= 25){
      pwm_velocity = PWM_MAX/10;
      decision = "1/10DC 20<H<50&T>25";
    }
    else {
      decision = "0/100DC 20<H<50&T<25";
      pwm_velocity = 0;
    }
  }
}

void updateState(float humidity,float temperature){
  globalHumidity = humidity;
  globalTemperature = temperature;
  updateVelocity();
  stateUpdated = true;
}

void get_humidity_and_temp_from_b_data(String b_buffer){
  int index_del = b_buffer.indexOf(BLUETOOTH_TRANSMITION_DEL_CHAR);
  String humidity_str = b_buffer.substring(0, index_del);
  String temp_str = b_buffer.substring(index_del + 1, b_buffer.length() - 1);
  updateState(humidity_str.toFloat(), temp_str.toFloat());
}

void readBluetooth(){
  if (Serial.available() > 1){
    String data = Serial.readString();
    if (data != ""){
      bluetooth_buffer += data;
      if(data.indexOf(BLUETOOTH_TRANSMITION_END_CHAR) != -1){
        get_humidity_and_temp_from_b_data(bluetooth_buffer);
        bluetooth_buffer = "";
      }
    }
  }
}

void printLCD(){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.println(("T: " + String(globalTemperature)).c_str());
  lcd.println(("H: " + String(globalHumidity)).c_str());
  lcd.setCursor(0, 1);
  lcd.println(decision.c_str());
  stateUpdated = false;
}

void sendWithBluetooth(int velocity){
  String message = String(velocity) + BLUETOOTH_TRANSMITION_END_CHAR;
  Serial.write(&message[0]);
}

void loop() {
  readBluetooth();
  if (stateUpdated) {
      printLCD();
      sendWithBluetooth(pwm_velocity);
  }
}
