#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiServer.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <PZEM004Tv30.h>
#include <HardwareSerial.h>
/*/+++++++////*/
PZEM004Tv30 pzem(Serial2,2,4);  //RX/TX
float voltage,current,pf,energy,frequency,power;
/*///++++++++////*/
const int trigPin = 5;
const int echoPin = 18;
bool CB_status = false;
char* Alart_message = "MPK";
int Alart = 0;
#define gasPin A0
#define gasPin1 A3
//define sound speed in cm/uS
#define SOUND_SPEED 0.03432
//#define CM_TO_INCH 0.393701

long duration;
float distanceCm;
float gaslevel;
float gaslevel1;

#include "DHT.h"
const int DHT_PIN = 15;
const int CB_Pin = 14;
#define DHTTYPE DHT11
int _moisture,sensor_analog;
const int sensor_pin = A6;  /* Soil moisture sensor O/P pin */

#include <Arduino.h>
#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif
#include <Firebase_ESP_Client.h>

DHT dht(DHT_PIN, DHTTYPE);

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
  
//unsigned long sendDataPrevMillis = 0;
//int count = 0;
bool signupOK = false;

void setup(){
  pinMode(CB_Pin, OUTPUT);
  pinMode(DHT_PIN, INPUT);
  dht.begin();
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input
  pinMode(gasPin, INPUT);// sets the H2 gasPin as an Input
  pinMode(gasPin1, INPUT);// sets the C2H2 gasPin as an Input

  Serial.begin(9600);
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
}

void loop(){
   delay(50);
  // Clears the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(50);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  sensor_analog = analogRead(sensor_pin);
  _moisture = ( 100 - ( (sensor_analog/4095.00) * 100 ) );
//  Serial.print("Moisture = ");
//  Serial.print(_moisture);  /* Print Temperature on the serial window */
//  Serial.println("%");
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  // Calculate the distance
  distanceCm = duration * SOUND_SPEED/2;
  gaslevel = analogRead(gasPin);
  gaslevel1 = analogRead(gasPin1);
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  /*////+++++++//////*/
  voltage = pzem.voltage();
  current = pzem.current();
  power = pzem.power();
  energy = pzem.energy();
  frequency = pzem.frequency();
  pf = pzem.pf();
  /*///++++++/////*/
  ///////////////
  if (_moisture>=25||gaslevel>=1000||gaslevel1>=4000||distanceCm<=5||h>=50||t>=32){
    Alart=1;
    if(t>=32){
      Alart_message = "Oil temperature is higher then 70'";
    }
    else if(distanceCm<=5){
      Alart_message = "OIL level is lesser then 60%";
    }
    else if(gaslevel1>=4000){
      Alart_message = "C2H2 gas level is higher then 4000";
    }
    else if(gaslevel>=8000){
      Alart_message = "H2 gas level is higher then 1000";
    }
    else if(h>=50){
      Alart_message = "Himidity level is higher then 10%";
    }
    else if(_moisture>=25){
      Alart_message = "Moisture is higher then 25";
    }
    else{
      Alart_message = "NORMAL";
    }
  }
  else{
    Alart_message = "NORMAL";
    Alart = 0;
  }
  ///////////////
  if (Firebase.ready() && signupOK ) {
    
    if (Firebase.RTDB.setFloat(&fbdo, "Main/DHT/humidity",h)){
       Serial.println("PASSED");
       Serial.print("Humidity: ");
       Serial.println(h);
      
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
    
    
    // Write an Float number on the database path test/float
    if (Firebase.RTDB.setFloat(&fbdo, "Main/DHT/temperature", t)){
       Serial.println("PASSED");
       Serial.print("Temperature: ");
       Serial.println(t);
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
    if(Firebase.RTDB.setFloat(&fbdo, "Main/DHT/Distance", distanceCm)){
          Serial.println("PASSED");
          Serial.print("Distance : ");
          Serial.println(distanceCm);
    }
    
    else{
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());

    }
    if(Firebase.RTDB.setFloat(&fbdo, "Main/DHT/Moisture", _moisture)){
          Serial.println("PASSED");
          Serial.print("Moisture : ");
          Serial.println(_moisture);
    }
    
    else{
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());

    }
    if(Firebase.RTDB.setFloat(&fbdo, "Main/DHT/H2 Gaslevel",gaslevel)){
          Serial.println("PASSED");
          Serial.print("H2 Gas level: ");
          Serial.println(gaslevel);
    }
    
    else{
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());

    }
    if(Firebase.RTDB.setFloat(&fbdo, "Main/DHT/C2H2 Gaslevel",gaslevel1)){
          Serial.println("PASSED");
          Serial.print("C2H2 Gas level: ");
          Serial.println(gaslevel1);
    }
    
    else{
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());

    }
     ///////////////////////////////////////////////////
    //////////////////////////////////////////////////
    if(Firebase.RTDB.setBool(&fbdo, "Main/Bhool/Fault/Alarm",Alart)){
          //Serial.println("PASSED");
          Serial.print("Alart: ");
          Serial.println(Alart);
    }
    else{
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());

    }
    if(Firebase.RTDB.setString(&fbdo, "Main/Bhool/Fault/Alarm_reason",Alart_message)){
          //Serial.println("PASSED");
          Serial.print("Falt Reason: ");
          Serial.println(Alart_message);
    }
    
    else{
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());

    }
  //READING DATA FROM FIREBASE TO OPERATE CB
    if(Firebase.RTDB.getBool(&fbdo, "Main/Bhool/Circuit_breaker")){
      if(fbdo.dataType() == "boolean"){
        CB_status = fbdo.boolData();
        Serial.println("Sucessful READ from" + fbdo.dataPath() + ":" + CB_status + " (" + fbdo.dataType() + ")");
        digitalWrite(CB_Pin, CB_status);
      }}
    else{
      Serial.println("FAILED:" + fbdo.errorReason());
      }
  /*^^^^^^^^^^^^^^^^*/
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
    if (Firebase.RTDB.setFloat(&fbdo, "Main/Electrical/POWER",power)){
       Serial.println("PASSED");
       Serial.print("POWER: ");
       Serial.println(power);
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
    if (Firebase.RTDB.setFloat(&fbdo, "Main/Electrical/ENERGY", energy)){
       Serial.println("PASSED");
       Serial.print("ENERGY: ");
       Serial.println(energy);
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
    if (Firebase.RTDB.setFloat(&fbdo, "Main/Electrical/FREQUENCY", frequency)){
       Serial.println("PASSED");
       Serial.print("FREQUENCY: ");
       Serial.println(frequency);
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
   Serial.println("______________________________");
  }}
  /*-----------------------------*/
  /*
  //THIS BELOW COMMENTED LINE CHECKS THE VALIDITY OF V,C,F,P,PF,E AND PRINTS THEM WITH TERE UNIT IN SERIAL MONITOR
      if(isnan(voltage)){
        Serial.println("Error reading voltage");
      }
      else if (isnan(current)) {
        Serial.println("Error reading current");
      }
      else if (isnan(power)) {
        Serial.println("Error reading power");
      }
      else if (isnan(energy)) {
        Serial.println("Error reading energy");
      }
      else if (isnan(frequency)) {
        Serial.println("Error reading frequency");
      }
      else if (isnan(pf)) {
        Serial.println("Error reading power factor");
      }
      else {

        // Print the values to the Serial console
        Serial.print("Voltage: ");      Serial.print(voltage);      Serial.println("V");
        Serial.print("Current: ");      Serial.print(current);      Serial.println("A");
        Serial.print("Power: ");        Serial.print(power);        Serial.println("W");
        Serial.print("Energy: ");       Serial.print(energy,3);     Serial.println("kWh");
        Serial.print("Frequency: ");    Serial.print(frequency, 1); Serial.println("Hz");
        Serial.print("PF: ");           Serial.println(pf);
      }
}   
*/  
  /*---------------------------*/
  /*^^^^^^^^^^^^^^^^*/
