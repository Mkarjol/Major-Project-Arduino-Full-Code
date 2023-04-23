#include <WiFi.h>
#include <Adafruit_GFX.h>
const int trigPin = 5;
const int echoPin = 18;
int Alart = 0;
char* Alart_message = "MPK";
const int buzzer1 = 12;
//define sound speed in cm/uS
#define SOUND_SPEED 0.03432
long duration;
float distanceCm;
#include "DHT.h" 
const int DHT_PIN = 15;
#define DHTTYPE DHT11

#include <Arduino.h>
//#defined(ESP32)
//#include <WiFi.h>
//#elif defined(ESP8266)
//#include <ESP8266WiFi.h>
//#endif
#include <Firebase_ESP_Client.h>

//////mmmmmmmmmmmmmmmmmmmm///////the code for gas sensors calibration////////
//Include the library
#include <MQUnifiedsensor.h>
//Definitions
#define placa "ESP 32"
#define Voltage_Resolution 5
#define type0 "MQ-8" //MQ8
#define type1 "MQ-6" //MQ6
#define type2 "MQ-7" //MQ7
#define type3 "MQ-4" //MQ4
#define ADC_Bit_Resolution 12 // For arduino UNO/MEGA/NANO,12 for esp32
//Edite the below valus for gas sensors calibiration
#define RatioMQ8CleanAir 180    
#define RatioMQ6CleanAir 70   //RS / R0 = 70 ppm 
#define RatioMQ7CleanAir 130
#define RatioMQ4CleanAir 80  

#define gasPin A0
#define gasPin1 A3
#define gasPin2 A7
#define gasPin3 A6
//#define calibration_button 13 //Pin to calibrate your sensor
float gaslevel,gaslevel1,gaslevel2,gaslevel3;    //H2 gas pin MQ8,C2H2 gas pin MQ6,CO gas pin MQ7,CH4 gas pin MQ4
//Declare Sensor
MQUnifiedsensor MQ8(placa, Voltage_Resolution, ADC_Bit_Resolution, gasPin, type0);
MQUnifiedsensor MQ6(placa, Voltage_Resolution, ADC_Bit_Resolution, gasPin1, type1);
MQUnifiedsensor MQ7(placa, Voltage_Resolution, ADC_Bit_Resolution, gasPin2, type2);
MQUnifiedsensor MQ4(placa, Voltage_Resolution, ADC_Bit_Resolution, gasPin3, type3);
/////////mmmmmmmmmmmmmmmmmmm/////////////////

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
  Serial.begin(9600);
  pinMode(DHT_PIN, INPUT);
  dht.begin();
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input
  /*pinMode(gasPin, INPUT);// sets the H2 gasPin as an Input
  pinMode(gasPin1, INPUT);// sets the C2H2 gasPin as an Input
  pinMode(gasPin2, INPUT);// sets the CO gasPin as an Input
  pinMode(gasPin3, INPUT);// sets the CH4 gasPin as an Input
  pinMode(buzzer1, OUTPUT);// this pin d12 will be used to buzzer when fault occurs*/

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
  ///////////mmmmmmmmmmmmmmmmmm////////
 //Set math model to calculate the PPM concentration and the value of constants
  MQ8.setRegressionMethod(1); //_PPM =  a*ratio^b
  MQ8.setA(976.97); MQ8.setB(-0.688); // Configurate the ecuation values to get H2 concentration
  MQ6.setRegressionMethod(1); //_PPM =  a*ratio^b
  MQ6.setA(10000000); MQ6.setB(-3.123); // Configurate the ecuation values to get H2 concentration
  MQ7.setRegressionMethod(1); //_PPM =  a*ratio^b
  MQ7.setA(2000000000000000000); MQ7.setB(-8.074); // Configurate the ecuation values to get H2 concentration
  MQ4.setRegressionMethod(1); //_PPM =  a*ratio^b
  MQ4.setA(80000000000000); MQ4.setB(-6.666); // Configurate the ecuation values to get H2 concentration

  /*
    Exponential regression:
  GAS     | a      | b
  H2      | 976.97  | -0.688
  LPG     | 10000000 | -3.123
  CH4     | 80000000000000 | -6.666
  CO      | 2000000000000000000 | -8.074
  Alcohol | 76101 | -1.86
  */
  
  /*****************************  MQ Init ********************************************/ 
  //Remarks: Configure the pin of arduino as input.
  /************************************************************************************/ 
  MQ8.init();   
  MQ6.init();
  MQ7.init();
  MQ4.init();
  /* 
    //If the RL value is different from 10K please assign your RL value with the following method:
    MQ8.setRL(10);
  */
  /*****************************  MQ CAlibration ********************************************/ 
  // Explanation: 
  // In this routine the sensor will measure the resistance of the sensor supposing before was pre-heated
  // and now is on clean air (Calibration conditions), and it will setup R0 value.
  // We recomend execute this routine only on setup or on the laboratory and save on the eeprom of your arduino
  // This routine not need to execute to every restart, you can load your R0 if you know the value
  // Acknowledgements: https://jayconsystems.com/blog/understanding-a-gas-sensor
  Serial.print("Calibrating please wait.");
  float calcR0 = 0,calcR01 = 0,calcR02 = 0,calcR03 = 0;
  for(int i = 1; i<=10; i ++)
  {
    MQ8.update(); // Update data, the arduino will be read the voltage on the analog pin
    calcR0 += MQ8.calibrate(RatioMQ8CleanAir);
    Serial.print(".");

    MQ6.update(); // Update data, the arduino will be read the voltage on the analog pin
    calcR01 += MQ6.calibrate(RatioMQ6CleanAir);
    Serial.print(".");
    MQ7.update(); // Update data, the arduino will be read the voltage on the analog pin
    calcR02 += MQ7.calibrate(RatioMQ7CleanAir);
    Serial.print(".");
    MQ4.update(); // Update data, the arduino will be read the voltage on the analog pin
    calcR03 += MQ4.calibrate(RatioMQ4CleanAir);
    Serial.print(".");
  }
  MQ8.setR0(calcR0/10);
  MQ6.setR0(calcR01/10);
  MQ7.setR0(calcR02/10);
  MQ4.setR0(calcR03/10);
  Serial.println("  done!.");
  
  //if(isinf(calcR0)) {Serial.println("Warning: Conection issue founded, R0 is infite (Open circuit detected) please check your wiring and supply"); while(1);}
  //if(calcR0 == 0){Serial.println("Warning: Conection issue founded, R0 is zero (Analog pin with short circuit to ground) please check your wiring and supply"); while(1);}
  /*****************************  MQ CAlibration ********************************************/ 
  //MQ8.serialDebug(true);
//////////////mmmmmmmmmmmmmmmmmmmmmmmmmmmm//////////////////
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
//  gaslevel = analogRead(gasPin);
//  H2lvl = map(H2lvl,0,1024,100,10000);
//  gaslevel1 = analogRead(gasPin1);
//  gaslevel2 = analogRead(gasPin2);
//  gaslevel3 = analogRead(gasPin3);
  float h = dht.readHumidity();
  float t = dht.readTemperature() + 1;
  /////////////////mmmmmmmmmmmmmmmmmmmmm/////////////gas sensors
  MQ8.update(); // Update data, the arduino will be read the voltage on the analog pin
  MQ6.update();
  MQ7.update();
  MQ4.update();
  int H2 = MQ8.readSensor(); // Sensor will read PPM concentration using the model and a and b values setted before or in the setu
  int C2H2 = MQ6.readSensor(); // Sensor will read PPM concentration using the model and a and b values setted before or in the setup
  int CO = MQ7.readSensor(); // Sensor will read PPM concentration using the model and a and b values setted before or in the setup
  int CH4 = MQ4.readSensor(); // Sensor will read PPM concentration using the model and a and b values setted before or in the setup
  //////////////////mmmmmmmmmmmmmmmmmmm/////////////
  
  /////////////// himidity RH to moisture in ppm code
  float x111 = 1+pow((t/23772.42),2.456301);
  float x11 = (539495600+(34.38525-539495591)/x111);
  float x1 = h*0.01*x11;
  //float x22 = 1+pow((t/50.13),2.204767);
  //float x2 = 158.9191 + ((-133.063595)/x22);
  float x2 = (h*32/43)*0.01*x11;
  int moisture = x1-x2; //this will give moisture in ppm
  //////////////
  if (moisture>=60||H2>=100||C2H2>=200||CO>=200||CH4>=200||Oil_level<=50||t>=45){
    Alart=1;
    digitalWrite(buzzer1, HIGH);
    if(t>=45){
      Alart_message = "Oil temperature is greater then 45*C '";
    }
    else if(Oil_level<=50){
      Alart_message = "OIL level is lesser then 50%";
    }
    else if(H2>=100){
      Alart_message = "H2 gas level is higher then 100ppm";
    }
    else if(C2H2>=200){
      Alart_message = "C2H2 gas level is higher then 200ppm";
    }
    else if(CO>=200){
      Alart_message = "CO gas level is higher then 200ppm";
    }
    else if(CH4>=200){
      Alart_message = "CH4 gas level is higher then 200ppm";
    }
    else if(moisture>=60){
      Alart_message = "Moisture is higher then 60ppm";
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
    if (Firebase.RTDB.setFloat(&fbdo, "Main/DHT/Oil_Temperature", t)){
       Serial.println("PASSED");
       Serial.print("Temperature(*C): ");
       Serial.println(t);
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
    if(Firebase.RTDB.setFloat(&fbdo, "Main/DHT/OIL_LEVEL", Oil_level)){
          Serial.println("PASSED");
          Serial.print("OIL_LEVEL(%) : ");
          Serial.print(Oil_level);  Serial.println("%");
          Serial.print("DISTANCE(cm): ");
          Serial.println(distanceCm);
    }
    else{
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
    if(Firebase.RTDB.setFloat(&fbdo, "Main/DHT/Oil_Moisture", moisture)){
          Serial.println("PASSED");
          Serial.print("Moisture(ppm) : ");
          Serial.println(moisture);
    }   
    else{
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
    if(Firebase.RTDB.setFloat(&fbdo, "Main/DHT/H2 Gaslevel",H2)){
          Serial.println("PASSED");
          Serial.print("H2 Gas level in ppm: ");
          Serial.println(H2);
    }
    else{
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
    if(Firebase.RTDB.setFloat(&fbdo, "Main/DHT/C2H2 Gaslevel",C2H2)){
          Serial.println("PASSED");
          Serial.print("C2H2 Gas level in ppm: ");
          Serial.println(C2H2);
    }
    else{
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
      }
    if(Firebase.RTDB.setFloat(&fbdo, "Main/DHT/CO Gaslevel",CO)){
          Serial.println("PASSED");
          Serial.print("CO Gas level in ppm: ");
          Serial.println(CO);
    }
    else{
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
    if(Firebase.RTDB.setFloat(&fbdo, "Main/DHT/CH4 Gaslevel",CH4)){
          Serial.println("PASSED");
          Serial.print("CH4 Gas level in ppm: ");
          Serial.println(CH4);
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
