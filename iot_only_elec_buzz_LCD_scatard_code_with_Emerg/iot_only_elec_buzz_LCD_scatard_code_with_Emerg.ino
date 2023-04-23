#include <WiFi.h>         //library for wifi connection
#include <PZEM004Tv30.h>  //library for Energy meter
#include <HardwareSerial.h>  //library for serial communication with hardware devices
PZEM004Tv30 pzem(Serial2,17,16);  //RX/TX
#include <LiquidCrystal_I2C.h>  //library for I2C LCD display
LiquidCrystal_I2C lcd(0x27, 16, 2);  //declearing I2C location(0x27),and lcd size(16*2)
float voltage, current, pf, energy, frequency, Act_Power, Rec_Power,Apa_Power;
const int CB_Pin = 14; //variable used to operate CB by authority,from web&app
bool CB_status = false;
const int CB_Pin2 = 27; //variable used to operate CB during electrical emeargency, without controle of authority
int Alart2 = 0;  //electrical alart during fault or emergency
char* Alart_message2 = "MPK";
const int buzzer2 = 13;  //buzzer during elec emergency
const int fault_led2 = 33; //pin used to indicate fault by led during fault
#include <Arduino.h>
#include <Firebase_ESP_Client.h>  // library for firebase google realtime facility database
#include "addons/TokenHelper.h"   //Provide the token generation process info.
#include "addons/RTDBHelper.h"    //Provide the RTDB payload printing info and other helper functions.

// Insert your network credentials
#define WIFI_SSID "Jio fiber Bhoom house"
#define WIFI_PASSWORD "bhoom2001"

#define API_KEY "AIzaSyAGunbEfXQpugL8eXjRc5hp3PTXrn5h9iA"  // Insert Firebase project API Key
#define DATABASE_URL "https://temp-3bd23-default-rtdb.firebaseio.com/"   // Insert RTDB URLefine the RTDB URL */
//Define Firebase Data object
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
bool signupOK = false;

void setup() {
  Serial.begin(9600);
  pinMode(CB_Pin, OUTPUT); //fault
  pinMode(CB_Pin2, OUTPUT); //emergency
  pinMode(buzzer2, OUTPUT);// this pin d13 will be used to buzzer when fault occurs
  pinMode(fault_led2, OUTPUT);//LED light to indicate faulty condition
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
  config.api_key = API_KEY;   /* Assign the api key (required) */
  config.database_url = DATABASE_URL;   /* Assign the RTDB URL (required) */
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
  int Apa = round(Apa_Power);
  //////-=====lcd display lcd code scatterd to use natural processing delay of code as lcd delay
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
  delay(500);
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
    if (Firebase.RTDB.setFloat(&fbdo, "Main/Electrical/ACTIVE_POWER",Act_Power)){
       Serial.println("PASSED");
       Serial.print("ACTIVE_POWER: ");
       Serial.println(Act_Power);
    }
    else {
       Serial.println("FAILED");
       Serial.println("REASON: " + fbdo.errorReason());
    }
    ////--====lcd code scatterd to use natural processing delay of code as lcd delay
    lcd.clear();// clear previous values from screen
    lcd.setCursor(0,0);
    lcd.print("Frquency:");
    lcd.setCursor(9,0);  
    lcd.print(frequency);
    lcd.setCursor(13,0);  
    lcd.print("Hz");
    lcd.setCursor(0,1);
    lcd.print("Pf:");
    lcd.setCursor(3,1);  
    lcd.print(pf);
    lcd.setCursor(8,1);
    lcd.print("S:");
    lcd.setCursor(10,1);  
    lcd.print(Apa);
    lcd.setCursor(14,1);  
    lcd.print("VA");
    delay(500); ///////
    
    if (Firebase.RTDB.setFloat(&fbdo, "Main/Electrical/ENERGY", energy)){
       Serial.println("PASSED");
       Serial.print("ENERGY: ");
       Serial.println(energy,3);
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
    //v250/210,c5.434 125%,f48.5,51.5 +-3%,//verification of emergency fault conditions
    if (voltage>=250||voltage<=210||current>=5.434||frequency<=48.50||frequency>=51.5||pf<=0.5){
      Alart2=1;
      Alart_message2 = "Emergency Electrical fault occured: Transformer turned off";
      digitalWrite(CB_Pin2, true);
      digitalWrite(buzzer2, HIGH);
    }
    // verification of normal faulty conditions 
    else if (voltage>=245||voltage<=220||current>=4.347||frequency<=49.50||frequency>=50.2||pf<=0.5){
      Alart2=1;
      digitalWrite(fault_led2, HIGH);
      
      if(voltage>=245){
        Alart_message2 = "--Over voltage -- voltage is greater then 245V";
      }
      else if(voltage<=220){
        Alart_message2 = "--Under voltage -- voltage is lesser then 220V";
      }
      else if(current>=4.347){
        Alart_message2 = "--Over load-- current is higher then 4.34A";
      }
      else if(frequency<=49.50){
        Alart_message2 = "--Under frequency-- frequency is lower then 49.5Hz";
      }
      else if(frequency>=50.2){
        Alart_message2 = "--Over frequency-- frequency is higher then 50.2";
      }
      else if(pf<=0.5){
        Alart_message2 = " --Low pf--pf is lesser then 0.5";
      }
  }
  else{
    Alart2 = 0;
    digitalWrite(buzzer2, LOW);
    digitalWrite(fault_led2, LOW);
    digitalWrite(CB_Pin2, false);
    Alart_message2 = "NORMAL";
  }
  ///////////////lcd code scatterd to use natural processing delay of code as lcd delay
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Act_Power:");
  lcd.setCursor(10,0);  
  lcd.print(Act);
  lcd.setCursor(14,0);  
  lcd.print("W");
  lcd.setCursor(0,1);
  lcd.print("Energy:");
  lcd.setCursor(7,1);  
  lcd.print(energy);
  lcd.setCursor(11,1);
  lcd.print("kWh");
  delay(1000);
  if(Firebase.RTDB.setBool(&fbdo, "Main/Bhool/Ele_Fault/Alarm",Alart2)){
    //Serial.println("PASSED");
    Serial.print("Alart: ");
    Serial.println(Alart2);
    }
  else{
    Serial.println("FAILED");
    Serial.println("REASON: " + fbdo.errorReason());
    }
  if(Firebase.RTDB.setString(&fbdo, "Main/Bhool/Ele_Fault/Alarm_reason",Alart_message2)){
    //Serial.println("PASSED");
    Serial.print("Falt Reason: ");
    Serial.println(Alart_message2);
    }
  else{
    Serial.println("FAILED");
    Serial.println("REASON: " + fbdo.errorReason());
    }
    Serial.println("______________________________");
  }
