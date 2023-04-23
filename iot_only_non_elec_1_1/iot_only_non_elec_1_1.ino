#include <WiFi.h>
#include <Adafruit_GFX.h>
const int trigPin = 5;
const int echoPin = 18;
char* Alart_message = "MPK";
int Alart = 0;
const int buzzer1 = 12;
#define gasPin A0
#define gasPin1 A3
#define gasPin2 A7
#define gasPin3 A6
//define sound speed in cm/uS
#define SOUND_SPEED 0.03432

long duration;
float distanceCm;
float gaslevel;    //H2 gas pin
int H2lvl;
float gaslevel1;   //C2H2 gas pin
float gaslevel2;   //CO gas pin
float gaslevel3;   //CH4 gas pin

#include "DHT.h" 
const int DHT_PIN = 15;
#define DHTTYPE DHT11

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
  
bool signupOK = false;

void setup(){
  pinMode(DHT_PIN, INPUT);
  dht.begin();
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input
  pinMode(gasPin, INPUT);// sets the H2 gasPin as an Input
  pinMode(gasPin1, INPUT);// sets the C2H2 gasPin as an Input
  pinMode(gasPin2, INPUT);// sets the CO gasPin as an Input
  pinMode(gasPin3, INPUT);// sets the CH4 gasPin as an Input
  pinMode(buzzer1, OUTPUT);// this pin d12 will be used to buzzer when fault occurs

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
  digitalWrite(trigPin, LOW);
  delayMicroseconds(10);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  // Calculate the distance and oil level
  distanceCm = duration * SOUND_SPEED/2;
  float Oil_level = (100*(22.5-distanceCm)/19);//calibration line
  gaslevel = analogRead(gasPin);
  H2lvl = map(H2lvl,0,1024,100,10000);
  gaslevel1 = analogRead(gasPin1);
  gaslevel2 = analogRead(gasPin2);
  gaslevel3 = analogRead(gasPin3);
  float h = dht.readHumidity();
  float t = dht.readTemperature() + 1;
  /////////////// himidity RH to moisture in ppm code
  float x111 = 1+pow((t/23772.42),2.456301);
  float x11 = (539495600+(34.38525-539495591)/x111);
  float x1 = h*0.01*x11;
  //float x22 = 1+pow((t/50.13),2.204767);
  //float x2 = 158.9191 + ((-133.063595)/x22);
  //***//**//
  float x2 = (h*32/43)*0.01*x11;
  //***//**//
  int moisture = x1-x2; //this will give moisture in ppm
  //////////////
  if (moisture>=1000||gaslevel>=1000||gaslevel1>=2000||Oil_level<=50||h>=50||t>=45){
    Alart=1;
    digitalWrite(buzzer1, HIGH);
    if(t>=45){
      Alart_message = "Oil temperature is greater then 45*C '";
    }
    else if(Oil_level<=50){
      Alart_message = "OIL level is lesser then 50%";
    }
    else if(gaslevel1>=4000){
      Alart_message = "C2H2 gas level is higher then 4000";
    }
    else if(gaslevel>=2000){
      Alart_message = "H2 gas level is higher then 1000";
    }
    else if(h>=60){
      Alart_message = "Himidity level is higher then 10%";
    }
    else if(moisture>=1000){
      Alart_message = "Moisture is higher then 1000ppm";
    }
    else{
      Alart = 0;
      digitalWrite(buzzer1, LOW);
      Alart_message = "NORMAL";
    }
  }
  else{
    Alart = 0;
    digitalWrite(buzzer1, LOW);
    Alart_message = "NORMAL";
  }
  ///////////////
  if (Firebase.ready() && signupOK ) {
    if (Firebase.RTDB.setFloat(&fbdo, "Main/DHT/humidity",h)){
       Serial.println("PASSED");
       Serial.print("Humidity(RH%): ");
       Serial.println(h);
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
    // Write an Float number on the database path test/float
    if (Firebase.RTDB.setFloat(&fbdo, "Main/DHT/temperature", t)){
       Serial.println("PASSED");
       Serial.print("Temperature(*C): ");
       Serial.println(t);
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
        if (Firebase.RTDB.setFloat(&fbdo, "Main/DHT/DISTANCE", distanceCm)){
       Serial.println("PASSED");
       Serial.print("DISTANCE(cm): ");
       Serial.println(distanceCm);
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
    if(Firebase.RTDB.setFloat(&fbdo, "Main/DHT/OIL_LEVEL", Oil_level)){
          Serial.println("PASSED");
          Serial.print("OIL_LEVEL(%) : ");
          Serial.print(Oil_level);  Serial.println("%");
    }
    else{
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
    if(Firebase.RTDB.setFloat(&fbdo, "Main/DHT/Moisture", moisture)){
          Serial.println("PASSED");
          Serial.print("Moisture(ppm) : ");
          Serial.println(moisture);
    }   
    else{
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
    if(Firebase.RTDB.setFloat(&fbdo, "Main/DHT/H2 Gaslevel",gaslevel)){
          Serial.println("PASSED");
          Serial.print("H2 Gas level: ");
          Serial.println(gaslevel);
          Serial.print("H2 level in ppm: ");
          Serial.println(H2lvl);
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
    if(Firebase.RTDB.setFloat(&fbdo, "Main/DHT/CO Gaslevel",gaslevel2)){
          Serial.println("PASSED");
          Serial.print("CO Gas level: ");
          Serial.println(gaslevel2);
    }
    else{
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
    if(Firebase.RTDB.setFloat(&fbdo, "Main/DHT/CH4 Gaslevel",gaslevel3)){
          Serial.println("PASSED");
          Serial.print("CH4 Gas level: ");
          Serial.println(gaslevel3);
    }
    else{
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
    if(Firebase.RTDB.setBool(&fbdo, "Main/Bhool/Non-Ele_Fault/Alarm",Alart)){
          //Serial.println("PASSED");
          Serial.print("Alarm: ");
          Serial.println(Alart);
    }
    else{
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
      }
    if(Firebase.RTDB.setString(&fbdo, "Main/Bhool/Non-Ele_Fault/Alarm_reason",Alart_message)){
          //Serial.println("PASSED");
          Serial.print("Falt Reason: ");
          Serial.println(Alart_message);
    }
    else{
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
      }

   Serial.println("______________________________");
  }}
