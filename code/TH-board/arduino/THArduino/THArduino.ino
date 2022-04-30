#include <Arduino.h>
#include <Wire.h>
#include <SoftwareSerial.h>


#define SHT25_ADDR 0x40
#define HUMIDITY_CMD 0xF5
#define TEMPERATURE_CMD 0xF3

#define BLUETOOTH_TRANSMITION_START_CHAR '@'
#define BLUETOOTH_TRANSMITION_END_CHAR '#'
#define BLUETOOTH_TRANSMITION_DEL_CHAR '/'
#define BLUETOOTH_RX 3
#define BLUETOOTH_TX 2
#define BLUETOOTH_BAUD_RATE 9600
#define PWM_MAX 64
#define PWM_MIN 0

String decision;
String bluetooth_buffer = "";
int pwm_counter = 0;
int pwm_velocity = 64;
int dc_pinA = 9;
int dc_pinB = 10;
long double last_tick;

//SoftwareSerial bluetooth(BLUETOOTH_RX ,BLUETOOTH_TX);

void setup() {
  Wire.begin();
  //pinMode(BLUETOOTH_RX,INPUT);
  //pinMode(BLUETOOTH_TX,OUTPUT);
  pinMode(dc_pinA,OUTPUT);
  pinMode(dc_pinB,OUTPUT);
  Serial.setTimeout(0);
  Serial.begin(BLUETOOTH_BAUD_RATE);
  last_tick = millis();
}

void sendCMD(int cmd){
  Wire.beginTransmission(SHT25_ADDR);
  Wire.write(cmd);
  Wire.endTransmission();
}

float receiveData(unsigned int data[2]){
  Wire.requestFrom(SHT25_ADDR,2);
  if (Wire.available() == 2) {
    data[0] = Wire.read();
    data[1] = Wire.read();
  }
}

boolean has_ticked(){
  long double now = millis();
  if (now - last_tick >= 4000){
    last_tick = now;
    return true;
  }
  return false;
}

float getHumidity(){
  unsigned int data[2];
  sendCMD(HUMIDITY_CMD);
  delay(500);
  receiveData(data);
  return (((data[0] * 256.0 + data[1]) * 125.0) / 65536.0) - 6;
}

void handlePWM(){
//  pwm_counter = (pwm_counter + 1) % (PWM_MAX - PWM_MIN);
//  if (pwm_counter < pwm_velocity){
//    digitalWrite(dc_pinA,LOW);
//    digitalWrite(dc_pinB,HIGH);
//  }
//  else {
//    digitalWrite(dc_pinA,LOW);
//    digitalWrite(dc_pinB,LOW);
//  }
    analogWrite(dc_pinA, pwm_velocity);
    digitalWrite(dc_pinB,0);
}



float getCTemperature(){
  unsigned int data[2];
  sendCMD(TEMPERATURE_CMD);
  delay(500);
  receiveData(data);
  return (((data[0] * 256.0 + data[1]) * 175.72) / 65536.0) - 46.85;
}

void sendWithBluetooth(float temperature,float humidity){
  String message = String(humidity) + BLUETOOTH_TRANSMITION_DEL_CHAR + String(temperature) + BLUETOOTH_TRANSMITION_END_CHAR;
  Serial.println(message);
}

void readBluetooth(){
  if (Serial.available() > 1){
    String data = Serial.readString();
    if (data != ""){
      bluetooth_buffer += data;
      if(data.indexOf(BLUETOOTH_TRANSMITION_END_CHAR) != -1){
        String decision = bluetooth_buffer.substring(0, bluetooth_buffer.length() - 1);
        pwm_velocity = atoi(&decision[0]);
        bluetooth_buffer = "";
      }
    }
  }
}

void loop() {
  if(has_ticked()){
    float humidity = getHumidity();
    float temperature = getCTemperature();
    sendWithBluetooth(temperature, humidity);
  }
  readBluetooth();
  handlePWM();
}
