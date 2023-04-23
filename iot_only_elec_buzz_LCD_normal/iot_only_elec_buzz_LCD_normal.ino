#include <WiFi.h>
#include <PZEM004Tv30.h>
#include <HardwareSerial.h>
PZEM004Tv30 pzem(Serial2,2,4);  //RX/TX
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);
float voltage, current, pf, energy, frequency, Act_Power, Rec_Power,Apa_Power;
const int CB_Pin = 14;
bool CB_status = false;
char* Alart_message_ = "MPK";
int Alart_ = 0;
const int buzzer2 = 13;
#include <Arduino.h>
#if defined(ESP32)
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif
#include <Firebase_ESP_Client.h>
//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// Insert your network credentials
#define WIFI_SSID "Jio fiber Bhoom house"
#define WIFI_PASSWORD "bhoom2001"

// Insert Firebase project API Key
#define API_KEY "AIzaSyAGunbEfXQpugL8eXjRc5hp3PTXrn5h9iA"

// Insert RTDB URLefine the RTDB URL */
#define DATABASE_URL "https://temp-3bd23-default-rtdb.firebaseio.com/" 

//Define Firebase Data object
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
bool signupOK = false;

void setup() {
  Serial.begin(9600);
  pinMode(CB_Pin, OUTPUT);
  pinMode(buzzer2, OUTPUT);// this pin d13 will be used to buzzer when fault occurs
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(100);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();
  /* Assign the api key (required) */
  config.api_key = API_KEY;
  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;
  /* Sign up */
  if (Firebase.signUp(&config, &auth, "", "")){
    Serial.println("ok");
    signupOK = true;
  } 
  else{
    //Serial.print("%s\n", config.signer.signupError.message.c_str(str));
  }
  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  // put your setup code here, to run once:
//////-====lcd display
  lcd.begin();  
  lcd.backlight();
  lcd.print("Transformer_IoT");
  lcd.setCursor(0,1);
  lcd.print("Elec_Parameters");  
  delay(2000); 
/////-======
}

void loop() {
  // put your main code here, to run repeatedly:
  frequency = pzem.frequency();
  pf = pzem.pf();
  voltage = pzem.voltage();
  current = pzem.current();
  float a = acos(pf);
  Rec_Power = (voltage*current*sin(a));
  Act_Power = pzem.power();
  energy = pzem.energy();
  Apa_Power = (voltage*current);
  int Act = round(Act_Power);
  //////-=====lcd display
  lcd.clear();// clear previous values from screen
  lcd.setCursor(0,0);
  lcd.print("Voltage:");
  lcd.setCursor(8,0);  
  lcd.print(voltage);
  lcd.setCursor(14,0);  
  lcd.print("V");
  lcd.setCursor(0,1);
  lcd.print("Current:");
  lcd.setCursor(8,1);  
  lcd.print(current);
  lcd.setCursor(12,1);  
  lcd.print("A");
  delay(2000);
  lcd.clear();// clear previous values from screen
  lcd.setCursor(0,0);
  lcd.print("Frq:");
  lcd.setCursor(4,0);  
  lcd.print(frequency);
  lcd.setCursor(8,0);  
  lcd.print(" ");
  lcd.setCursor(9,0);
  lcd.print("Pf:");
  lcd.setCursor(12,0);  
  lcd.print(pf);
  lcd.setCursor(0,1);
  lcd.print("P:");
  lcd.setCursor(2,1);  
  lcd.print(Act);
  lcd.setCursor(6,1);
  lcd.print("W");
  lcd.setCursor(8,1);
  lcd.print("E:");
  lcd.setCursor(10,1);  
  lcd.print(energy);
  lcd.setCursor(14,1);
  lcd.print("kWh");
  /////-======
    if (Firebase.RTDB.setFloat(&fbdo, "Main/Electrical/VOLTAGE", voltage)){
      Serial.println("PASSED");
      Serial.print("VOLTAGE: ");
      Serial.println(voltage);
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
    if (Firebase.RTDB.setFloat(&fbdo, "Main/Electrical/CURRENT",current)){
       Serial.println("PASSED");
       Serial.print("CURRENT: ");
       Serial.println(current);
    } 
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
    if (Firebase.RTDB.setFloat(&fbdo, "Main/Electrical/ACTIVE_POWER",Act_Power)){
       Serial.println("PASSED");
       Serial.print("ACTIVE_POWER: ");
       Serial.println(Act_Power);
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
    if (Firebase.RTDB.setFloat(&fbdo, "Main/Electrical/ENERGY", energy)){
       Serial.println("PASSED");
       Serial.print("ENERGY: ");
       Serial.println(energy,3);
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
    if (Firebase.RTDB.setFloat(&fbdo, "Main/Electrical/FREQUENCY", frequency)){
       Serial.println("PASSED");
       Serial.print("FREQUENCY: ");
       Serial.println(frequency,1);
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
    if (Firebase.RTDB.setFloat(&fbdo, "Main/Electrical/POWER_FACTOR", pf)){
       Serial.println("PASSED");
       Serial.print("POWER_FACTOR: ");
       Serial.println(pf);
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
    if (Firebase.RTDB.setFloat(&fbdo, "Main/Electrical/APPARANT_POWER", Apa_Power)){
       Serial.println("PASSED");
       Serial.print("APPARANT_POWER: ");
       Serial.println(Apa_Power);
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
    if (Firebase.RTDB.setFloat(&fbdo, "Main/Electrical/REACTIVE_POWER", Rec_Power)){
       Serial.println("PASSED");
       Serial.print("REACTIVE_POWER: ");
       Serial.println(Rec_Power);
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
      //READING DATA FROM FIREBASE TO OPERATE CB
    if(Firebase.RTDB.getBool(&fbdo, "Main/Bhool/Circuit_breaker"))
    {
      if(fbdo.dataType() == "boolean")
      {
        CB_status = fbdo.boolData();
        Serial.println("Sucessful READ from" + fbdo.dataPath() + ":" + CB_status + " (" + fbdo.dataType() + ")");
        digitalWrite(CB_Pin, CB_status);
      }
    }
    else{
      Serial.println("FAILED:" + fbdo.errorReason());
      }
    /////////////
    if (voltage>=240||voltage<=210||current>=4.347||frequency<=49.70||frequency>=50.3||pf<=0.5){
    Alart_=1;
    digitalWrite(buzzer2, HIGH);
    if(voltage>=240){
      Alart_message_ = "--Over voltage -- voltage is greater then 240V";
    }
    else if(voltage<=210){
      Alart_message_ = "--Under voltage -- voltage is lesser then 210V";
    }
    else if(current>=4.347){
      Alart_message_ = "--Over load-- current is higher then 4.34A";
    }
    else if(frequency<=49.70){
      Alart_message_ = "--Under frequency-- frequency is lower then 49.5Hz";
    }
    else if(frequency>=50.3){
      Alart_message_ = "--Over frequency-- frequency is higher then 50.5";
    }
    else if(pf<=0.5){
      Alart_message_ = " --Low pf--pf is lesser then 0.5";
    }
    else{
      Alart_ = 0;
      digitalWrite(buzzer2, LOW);
      Alart_message_ = "NORMAL";
    }
  }
  else{
    Alart_ = 0;
    digitalWrite(buzzer2, LOW);
    Alart_message_ = "NORMAL";
  }
  ///////////////
  if(Firebase.RTDB.setBool(&fbdo, "Main/Bhool/Ele_Fault/Alarm",Alart_)){
          //Serial.println("PASSED");
          Serial.print("Alart: ");
          Serial.println(Alart_);
    }
    else{
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
      }
    if(Firebase.RTDB.setString(&fbdo, "Main/Bhool/Ele_Fault/Alarm_reason",Alart_message_)){
          //Serial.println("PASSED");
          Serial.print("Falt Reason: ");
          Serial.println(Alart_message_);
    }
    else{
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
      }
   Serial.println("______________________________");
  }
