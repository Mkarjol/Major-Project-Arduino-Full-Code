#include <WiFi.h>
#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Firebase_ESP_Client.h>
#include <PZEM004Tv30.h>  //library for Energy meter
#include <HardwareSerial.h>  //library for serial communication with hardware devices
PZEM004Tv30 pzem(Serial2,17,16);  //RX/TX
#include <LiquidCrystal_I2C.h>  //library for I2C LCD display
LiquidCrystal_I2C lcd(0x27, 16, 2);  //declearing I2C location(0x27),and lcd size(16*2)
const int trigPin = 2;
const int echoPin = 4;
const int buzzer1 = 12;
const int CB_Pin1 = 26; //variable used to operate CB during non electrical emeargency, without controle of authority
const int fault_led1 = 32; //pin used to indicate non elec fault by glowing led
int Alart1 = 0;
char* Alart_message1 = "MPK";
float voltage, current, pf, energy, frequency, Act_Power, Rec_Power,Apa_Power;
const int CB_Pin = 14; //variable used to operate CB by authority,from web&app
bool CB_status = false;
const int CB_Pin2 = 27; //variable used to operate CB during electrical emeargency, without controle of authority
int Alart2 = 0;  //electrical alart during fault or emergency
char* Alart_message2 = "MPK";
const int buzzer2 = 13;  //buzzer during elec emergency
const int fault_led2 = 33; //pin used to indicate fault by led during fault
#define SOUND_SPEED 0.03432   //define sound speed in cm/uS
long duration;
float distanceCm;
#include "DHT.h" 
#define DHTTYPE DHT11
const int DHT_PIN = 15;
DHT dht(DHT_PIN, DHTTYPE);
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
#define RatioMQ7CleanAir 120
#define RatioMQ4CleanAir 80  

#define gasPin A0
#define gasPin1 A3
#define gasPin2 A7
#define gasPin3 A6
/////mpk/////////
TaskHandle_t Task2;
/////mpk/////////
//#define calibration_button 13 //Pin to calibrate your sensor
//float gaslevel,gaslevel1,gaslevel2,gaslevel3;    //H2 gas pin MQ8,C2H2 gas pin MQ6,CO gas pin MQ7,CH4 gas pin MQ4
//Declare Sensor
MQUnifiedsensor MQ8(placa, Voltage_Resolution, ADC_Bit_Resolution, gasPin, type0);
MQUnifiedsensor MQ6(placa, Voltage_Resolution, ADC_Bit_Resolution, gasPin1, type1);
MQUnifiedsensor MQ7(placa, Voltage_Resolution, ADC_Bit_Resolution, gasPin2, type2);
MQUnifiedsensor MQ4(placa, Voltage_Resolution, ADC_Bit_Resolution, gasPin3, type3);
/////////mmmmmmmmmmmmmmmmmmm/////////////////
#include "addons/TokenHelper.h"   //Provide the token generation process info.
#include "addons/RTDBHelper.h"    //Provide the RTDB payload printing info and other helper functions.
// Insert your network credentials
#define WIFI_SSID "Jio fiber Bhoom house"  //WiFi Name and Password
#define WIFI_PASSWORD "bhoom2001"
// Insert Firebase project API Key
#define API_KEY "AIzaSyAGunbEfXQpugL8eXjRc5hp3PTXrn5h9iA"  //API key from firebase
#define DATABASE_URL "https://temp-3bd23-default-rtdb.firebaseio.com/"   //firebase url
//Define Firebase Data object
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
bool signupOK = false;   //feedback for sucessfull firebase connection

///////mpk////////
void codeForTask2( void * parameter )
{
  for (;;) {
  digitalWrite(trigPin, LOW);  //code
  delayMicroseconds(10);
  digitalWrite(trigPin, HIGH); // Sets the trigPin on HIGH state for 10 micro seconds
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH); // Reads the echoPin, returns the sound wave travel time in microseconds
  // Calculate the distance and oil level
  distanceCm = duration * SOUND_SPEED/2;
  float Oil_level = (100*(22.5-distanceCm)/19); //calibration line for Oil Level
  float h = dht.readHumidity()+3;  // +3 because of sensor calibration
  float t = dht.readTemperature() + 1;  //+1 added for sensor calibration u can add required equation if calibration of dht11 needed
  /////////////// 'himidity in RH' to 'moisture in ppm' convertion code, used curve fitting to get relative equation b/w temp,himi,moist 
  float x111 = 1+pow((t/23772.42),2.456301);
  float x11 = (539495600+(34.38525-539495591)/x111);
  float x1 = h*0.0125*x11;
  float x2 = (h*30/43)*0.01*x11;  //edit here for moisture calibration
  int moisture = x1-x2; //this will give moisture in ppm
  //////////////
  /////////////////mmmmmmmmmmmmmmmmmmmmm/////////////gas sensors code start
  MQ8.update(); // Update data, the arduino will be read the voltage on the analog pin
  MQ6.update();
  MQ7.update();
  MQ4.update();
  int H2 = MQ8.readSensor(); // Sensor will read PPM concentration using the model and a and b values setted before or in the setu
  int C2H2 = MQ6.readSensor();
  int CO = MQ7.readSensor();
  int CH4 = MQ4.readSensor();
  //////////////////mmmmmmmmmmmmmmmmmmm/////////////end
  if(moisture>=57||Oil_level<=50||t>=75){
      Alart1=1;
      Alart_message1 = "Emergency Non-Electrical fault occured: Transformer turned off";
      digitalWrite(CB_Pin1, true);
      digitalWrite(buzzer1, HIGH);
  }
  //this below line will check for faulty conditions and report fault and reason
  else if(moisture>=35||H2>=100||C2H2>=100||CO>=60||CH4>=60||Oil_level<=60||t>=70){
    Alart1=1;
    digitalWrite(fault_led1, HIGH);
    if(t>=70){
      Alart_message1 = "Oil temperature is greater then 70*C '";
    }
    else if(Oil_level<=60){
      Alart_message1 = "OIL level is lesser then 60%";
    }
    else if(H2>=100){
      Alart_message1 = "H2 gas level is higher then 100ppm";
    }
    else if(C2H2>=100){
      Alart_message1 = "C2H2 gas level is higher then 100ppm";
    }
    else if(CO>=60){
      Alart_message1 = "CO gas level is higher then 200ppm";
    }
    else if(CH4>=60){
      Alart_message1 = "CH4 gas level is higher then 200ppm";
    }
    else if(moisture>=35){
      Alart_message1 = "Moisture is higher then 35ppm";
    }
    else{}
  }
  else{
    Alart1 = 0;
    digitalWrite(buzzer1, LOW);
    digitalWrite(fault_led1, LOW);
    digitalWrite(CB_Pin1, false);
    Alart_message1 = "NORMAL";
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
    if(Firebase.RTDB.setFloat(&fbdo, "Main/DHT/H2_Gaslevel",H2)){
          Serial.println("PASSED");
          Serial.print("H2 Gas level in ppm: ");
          Serial.println(H2);
    }
    else{
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
    if(Firebase.RTDB.setFloat(&fbdo, "Main/DHT/C2H2_Gaslevel",C2H2)){
          Serial.println("PASSED");
          Serial.print("C2H2 Gas level in ppm: ");
          Serial.println(C2H2);
    }
    else{
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
      }
    if(Firebase.RTDB.setFloat(&fbdo, "Main/DHT/CO_Gaslevel",CO)){
          Serial.println("PASSED");
          Serial.print("CO Gas level in ppm: ");
          Serial.println(CO);
    }
    else{
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
    if(Firebase.RTDB.setFloat(&fbdo, "Main/DHT/CH4_Gaslevel",CH4)){
          Serial.println("PASSED");
          Serial.print("CH4 Gas level in ppm: ");
          Serial.println(CH4);
    }
    else{
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
    if(Firebase.RTDB.setBool(&fbdo, "Main/Bhool/Non-Ele_Fault/Alarm",Alart1)){
          //Serial.println("PASSED");
          Serial.print("Alarm: ");
          Serial.println(Alart1);
    }
    else{
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
      }
    if(Firebase.RTDB.setString(&fbdo, "Main/Bhool/Non-Ele_Fault/Alarm_reason",Alart_message1)){
          //Serial.println("PASSED");
          Serial.print("Falt Reason: ");
          Serial.println(Alart_message1);
    }
    else{
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
      }

   Serial.println("______________________________");
  }
  }
}
//////mpk////////
void setup(){
  Serial.begin(9600);
  dht.begin();
  pinMode(DHT_PIN, INPUT);  //declearing DHT_PIN as input pin 
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input
  pinMode(CB_Pin1, OUTPUT); //emergency
  pinMode(buzzer1, OUTPUT);// this pin d12 will be used to buzzer when fault occurs
  pinMode(fault_led1, OUTPUT);//LED light to indicate faulty condition
  
  pinMode(CB_Pin, OUTPUT); //fault
  pinMode(CB_Pin2, OUTPUT); //emergency
  pinMode(buzzer2, OUTPUT);// this pin d13 will be used to buzzer when fault occurs
  pinMode(fault_led2, OUTPUT);//LED light to indicate faulty condition

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);  //takes above mentioned id and password of WiFi and starts connecting
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(100);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());   //displays the IP address of WiFi connected
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
   // Serial.print("%s\n", config.signer.signupError.message.c_str(str));
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
  //Remarks: Configure the pin of arduino as input.
  MQ8.init();   
  MQ6.init();
  MQ7.init();
  MQ4.init();
  //If the RL value is different from 10K please assign your RL value with the following method:MQ8.setRL(10);
  //here in this project we used sensors with 10k  load resistance
  /*****************************  MQ CAlibration ********************************************/ 
  /*Explanation: keep the sensors in clean air while calibrating.use this calibration code once to calibrate ur sensors.
  get the values of R0 note them 
  In this routine the sensor will measure the resistance of the sensor supposing before was pre-heated
  and now is on clean air (Calibration conditions), and it will setup R0 value.
  We recomend execute this routine only on setup or on the laboratory and save on the eeprom of your arduino
  This routine not need to execute to every restart, you can load your R0 if you know the value using function: MQ6.SetR0(value in kOhm)
  Acknowledgements: https://jayconsystems.com/blog/understanding-a-gas-sensor
  Serial.print("Calibrating please wait.");*/
  /*   //%%%%%%%%%%%%%%%%%%%%%%uncomment this line for calibration(start)
  float calcR0 = 0,calcR01 = 0,calcR02 = 0,calcR03 = 0;
  for(int i = 1; i<=10; i ++)
  {
    MQ8.update(); // Update data, the arduino will be read the voltage on the analog pin
    calcR0 += MQ8.calibrate(RatioMQ8CleanAir);
    MQ6.update(); // Update data, the arduino will be read the voltage on the analog pin
    calcR01 += MQ6.calibrate(RatioMQ6CleanAir);
    MQ7.update(); // Update data, the arduino will be read the voltage on the analog pin
    calcR02 += MQ7.calibrate(RatioMQ7CleanAir);
    MQ4.update(); // Update data, the arduino will be read the voltage on the analog pin
    calcR03 += MQ4.calibrate(RatioMQ4CleanAir);
    Serial.println(".");
  }
  MQ8.setR0(calcR0/10);
  MQ6.setR0(calcR01/10);
  MQ7.setR0(calcR02/10);
  MQ4.setR0(calcR03/10);
  Serial.print("H2 sensor R0 (in kOhm):");
  Serial.println(calcR0/10);
  Serial.print("C2H2 sensor R0 (in kOhm):");
  Serial.println(calcR01/10);
  Serial.print("CO sensor R0 (in kOhm):");
  Serial.println(calcR02/10);
  Serial.print("CH4 sensor R0 (in kOhm):");
  Serial.println(calcR03/10);
  Serial.println("  done!.");
  *///%%%%%%%%%%%%%%%%%%%%%% uncomment this line wile calibrating(end)
  //if(isinf(calcR0)) {Serial.println("Warning: Conection issue founded, R0 is infite (Open circuit detected) please check your wiring and supply"); while(1);}
  //if(calcR0 == 0){Serial.println("Warning: Conection issue founded, R0 is zero (Analog pin with short circuit to ground) please check your wiring and supply"); while(1);}
  /*****************************  MQ CAlibration ********************************************/ 
  //uncomment the above MQ calibration code mentioned above,after getting the values of R0, note them and use them to set R0 values of diffrent gas sensors
  MQ8.setR0(0.13);  //this will assign valus to R0, comment this 4 lines while calibrating
  MQ6.setR0(0.23);
  MQ7.setR0(0.30);
  MQ4.setR0(0.33);
//////////////mmmmmmmmmmmmmmmmmmmmmmmmmmmm//////////////////

//////-====lcd display
  lcd.begin();  
  lcd.backlight();
  lcd.print("Transformer_IoT");
  lcd.setCursor(0,1);
  lcd.print("Elec_Parameters");  
  delay(2000); 
/////-======
  /////////mpk///////
  xTaskCreatePinnedToCore(
    codeForTask2,      /* Task function. */
    "Task_2",                /* name of task. */
    32768,                    /* Stack size of task */
    NULL,                    /* parameter of the task */
    2,                       /* priority of the task */
    &Task2,                  /* Task handle to keep track of created task */
    0);                      /* Core */
    
  /////////mpk///////
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
