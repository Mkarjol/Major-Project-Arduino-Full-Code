#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
//#include <SD.h>  // libraryes for sd card use
//#include <SPI.h>  //
#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <ArduinoJson.h>
#include <Firebase_ESP_Client.h>
#include <PZEM004Tv30.h>  //library for Energy meter
#include <HardwareSerial.h>  //library for serial communication with hardware devices
PZEM004Tv30 pzem(Serial2,17,16);  //RX/TX  //////////PZEM primary
PZEM004Tv30 pzem2(Serial1,25,26);  //RX/TX //////////PZEM secondary
#include <LiquidCrystal_I2C.h>  //library for I2C LCD display
LiquidCrystal_I2C lcd(0x27, 16, 2);  //declearing I2C location(0x27),and lcd size(16*2)
//sd card
//File myFile;  //veriable for working with our file object
//const int pinCS = 5; //set chipselect to pin GPIO5
//
const int trigPin = 2;
const int echoPin = 4;
const int buzzerB = 12;  //pin used to indicate both elec and non elec Emergency by buzzer
const int fault_ledB = 32; //pin used to indicate both elec and non elec fault by glowing led
int Alart1 = 0;
int CBE=0,CBN=0,CB=0,BzzE=0,BzzN=0,LedE=0,LedN=0;
char* Alart_message1 = "MPK";
int AA1 = 1, AA2 = 0, BB1 = 1, BB2 = 0, BB1E = 1, BB2E = 0; //veriables used for teligram notification AA for elec,BB for non ele,BBE for emergency
float voltage=0, current=0, pf=0, energy=0, frequency=0, Act_Power=0, Rec_Power=0,Apa_Power=0,Act=0,Apa=0,Rec=0,Volt=0; //act, apa,rec are veriables used to get 2 decimal values.Voit is tempervary veriable of voltage
float voltage2=0,current2=0,pf2=0,Act_Power2=0,Efficiency=0,Diffrential=0;
const int CB_Pin = 14; //variable used to operate CB by authority,from web&app
bool CB_status = false;
//const int CB_PinB = 27; //variable used to operate CB during electrical and non elec emeargency in separate code, without controle of authority
int CB_Emerg =0;  //veriable used to deside CB operation
int Alart2 = 0;  //electrical alart during fault or emergency
char* Alart_message2 = "MPK";
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
#define type1 "MQ-135" //MQ135
#define type2 "MQ-7" //MQ7
#define type3 "MQ-4" //MQ4
#define ADC_Bit_Resolution 12 // For arduino UNO/MEGA/NANO,12 for esp32
//Edite the below valus for gas sensors calibiration
#define RatioMQ8CleanAir 180    
#define RatioMQ135CleanAir 70   //RS / R0 = 70 ppm 
#define RatioMQ7CleanAir 120
#define RatioMQ4CleanAir 80  

#define gasPin A0
#define gasPin1 A3
#define gasPin2 A7
#define gasPin3 A6
/////mpk////////
/////mpk/////////
//#define calibration_button 13 //Pin to calibrate your sensor
//float gaslevel,gaslevel1,gaslevel2,gaslevel3;    //H2 gas pin MQ8,C2H2 gas pin MQ6,CO gas pin MQ7,CH4 gas pin MQ4
//Declare Sensor
MQUnifiedsensor MQ8(placa, Voltage_Resolution, ADC_Bit_Resolution, gasPin, type0);
MQUnifiedsensor MQ135(placa, Voltage_Resolution, ADC_Bit_Resolution, gasPin1, type1);
MQUnifiedsensor MQ7(placa, Voltage_Resolution, ADC_Bit_Resolution, gasPin2, type2);
MQUnifiedsensor MQ4(placa, Voltage_Resolution, ADC_Bit_Resolution, gasPin3, type3);
/////////mmmmmmmmmmmmmmmmmmm/////////////////
#include "addons/TokenHelper.h"   //Provide the token generation process info.
#include "addons/RTDBHelper.h"    //Provide the RTDB payload printing info and other helper functions.
// Insert your network credentials
#define WIFI_SSID "Jio fiber Bhoom house"  //WiFi Name and Password
#define WIFI_PASSWORD "bhoom2001"
/////////////////
// Initialize Telegram BOT
#define BOTtoken "6024221882:AAHLqH2Mf0yU2VsBzOwDST9Ji3xaf-XmIPA"  // your Bot Token (Get from Botfather)
// Use @myidbot to find out the chat ID of an individual or a group
// Also note that you need to click "start" on a bot before it can
// message you
String chat_id = "935251637";
WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOTtoken, secured_client);
////////////////
// Insert Firebase project API Key
#define API_KEY "AIzaSyAGunbEfXQpugL8eXjRc5hp3PTXrn5h9iA"  //API key from firebase
#define DATABASE_URL "https://temp-3bd23-default-rtdb.firebaseio.com/"   //firebase url
//Define Firebase Data object
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
bool signupOK = false;   //feedback for sucessfull firebase connection
// the setup function runs once when you press reset or power the board
//String path;
/////////////// sd card read write initialization code
/*//SD card code
void WriteFile(const char * path, const char * message){
  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  myFile = SD.open(path, FILE_WRITE);
  // if the file opened okay, write to it:
  if (myFile) {
    Serial.printf("Writing to %s ", path);
    myFile.println(message);
    myFile.close(); // close the file:
    Serial.println("completed.");
  } 
  // if the file didn't open, print an error:
  else {
    Serial.println("error opening file ");
    Serial.println(path);
  }
}

void ReadFile(const char * path){
  // open the file for reading:
  myFile = SD.open(path);
  if (myFile) {
     Serial.printf("Reading file from %s\n", path);
     // read from the file until there's nothing else in it:
    while (myFile.available()) {
      Serial.write(myFile.read());
    }
    myFile.close(); // close the file:
  } 
  else {
    // if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  }
}
*/
///////////////

void setup() {
  Serial.begin(115200);
  dht.begin();
  //SD.begin(pinCS);
  pinMode(DHT_PIN, INPUT);  //declearing DHT_PIN as input pin 
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input
  pinMode(buzzerB, OUTPUT);// this pin d12 will be used to buzzer when fault occurs
  pinMode(fault_ledB, OUTPUT);//LED light to indicate faulty condition
  pinMode(CB_Pin, OUTPUT); //fault
 // pinMode(CB_PinB, OUTPUT); //emergency

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);  //takes above mentioned id and password of WiFi and starts connecting.
  //client.setInsecure();
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(100);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());   //displays the IP address of WiFi connected
  Serial.println();
  secured_client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Add root certificate for api.telegram.org
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
  if(Firebase.isTokenExpired()){
    Firebase.refreshToken(&config);
  }
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  ////////////
  bot.sendMessage(chat_id, "IoT Transformer Teligram Alerting system started up", ""); //teligram notification about start of processor
  ////////
  /* SD card
  while (!Serial) { ; }  // wait for serial port to connect. Needed for native USB port only
  Serial.println("Initializing SD card...");
  if (!SD.begin(pinCS)) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");

  WriteFile("/test.txt", "IoT Transformer");
  ReadFile("/test.txt");  
  */
  ///////////mmmmmmmmmmmmmmmmmm////////
  //Set math model to calculate the PPM concentration and the value of constants
  MQ8.setRegressionMethod(1); //_PPM =  a*ratio^b
  MQ8.setA(976.97); MQ8.setB(-0.688); // Configurate the ecuation values to get H2 concentration
  MQ135.setRegressionMethod(1); //_PPM =  a*ratio^b
  MQ135.setA(10000000); MQ135.setB(-3.123); // Configurate the ecuation values to get CO2 concentration
  MQ7.setRegressionMethod(1); //_PPM =  a*ratio^b
  MQ7.setA(2000000000000000000); MQ7.setB(-8.074); // Configurate the ecuation values to get CO concentration
  MQ4.setRegressionMethod(1); //_PPM =  a*ratio^b
  MQ4.setA(80000000000000); MQ4.setB(-6.666); // Configurate the ecuation values to get CH4 concentration
  /*
    Exponential regression:
  GAS     | a      | b
  H2      | 976.97  | -0.688
  CO2     | 10000000 | -3.123
  CH4     | 80000000000000 | -6.666
  CO      | 2000000000000000000 | -8.074
  Alcohol | 76101 | -1.86
  */
  //Remarks: Configure the pin of arduino as input.
  MQ8.init();   
  MQ135.init();
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
  /*
   //%%%%%%%%%%%%%%%%%%%%%%uncomment this line for calibration(start)
  float calcR0 = 0,calcR01 = 0,calcR02 = 0,calcR03 = 0;
  for(int i = 1; i<=10; i ++)
  {
    MQ8.update(); // Update data, the arduino will be read the voltage on the analog pin
    calcR0 += MQ8.calibrate(RatioMQ8CleanAir);
    MQ135.update(); // Update data, the arduino will be read the voltage on the analog pin
    calcR01 += MQ135.calibrate(RatioMQ135CleanAir);
    MQ7.update(); // Update data, the arduino will be read the voltage on the analog pin
    calcR02 += MQ7.calibrate(RatioMQ7CleanAir);
    MQ4.update(); // Update data, the arduino will be read the voltage on the analog pin
    calcR03 += MQ4.calibrate(RatioMQ4CleanAir);
    Serial.println(".");
  }
  MQ8.setR0(calcR0/10);
  MQ135.setR0(calcR01/10);
  MQ7.setR0(calcR02/10);
  MQ4.setR0(calcR03/10);
  Serial.print("H2 sensor R0 (in kOhm):");
  Serial.println(calcR0/10);
  Serial.print("CO2 sensor R0 (in kOhm):");
  Serial.println(calcR01/10);
  Serial.print("CO sensor R0 (in kOhm):");
  Serial.println(calcR02/10);
  Serial.print("CH4 sensor R0 (in kOhm):");
  Serial.println(calcR03/10);
  Serial.println("  done!.");
 */
  //%%%%%%%%%%%%%%%%%%%%%% uncomment this line wile calibrating(end)
  //if(isinf(calcR0)) {Serial.println("Warning: Conection issue founded, R0 is infite (Open circuit detected) please check your wiring and supply"); while(1);}
  //if(calcR0 == 0){Serial.println("Warning: Conection issue founded, R0 is zero (Analog pin with short circuit to ground) please check your wiring and supply"); while(1);}
  /*****************************  MQ CAlibration ********************************************/ 
  //uncomment the above MQ calibration code mentioned above,after getting the values of R0, note them and use them to set R0 values of diffrent gas sensors
  MQ8.setR0(0.32);  //this will assign valus to R0, comment this 4 lines while calibrating
  MQ135.setR0(0.99);
  MQ7.setR0(0.42);
  MQ4.setR0(0.44);
//////////////mmmmmmmmmmmmmmmmmmmmmmmmmmmm//////////////////

//////-====lcd display
  lcd.begin();  
  lcd.backlight();
  lcd.print("Transformer_IoT");
  lcd.setCursor(0,1);
  lcd.print("Elec_Parameters");  
  delay(2500);
}

void loop() {
  /////done till adding effici changed CB from 14,27 both to 14//////
  /////////////hii mahesh
  voltage = pzem.voltage();
  voltage2 = pzem2.voltage();
  if(voltage>=0 && voltage<=15000){  //this if else condition is added to filter error due to Dis connection of PZEM like unable to read showing "nan"
    Volt = voltage;
    current = pzem.current();
    frequency = pzem.frequency();
    pf = pzem.pf();
    pf2 = pzem2.pf();
    float a = acos(pf);
    Rec_Power = (voltage*current*sin(a));
    Act_Power = pzem.power();
    energy = pzem.energy();
    Apa_Power = (voltage*current);
    Act = round(Act_Power);   // rounded up to show these systematically
    Rec = round(Rec_Power);
    Apa = round(Apa_Power);
  }
  else{
    voltage = 0.00,Volt = 0.00,current = 0.00,frequency = 0.00,energy =0.00,pf = 0.00,pf2 = 0.00,Act_Power=0.01,Act = 0.00,Rec = 0.00,Apa = 0.00;//the Act_power is considerd 0.01 to avoide error in effeciency calculation
  }
  if((voltage2>=0 && voltage2<=15000)&&(voltage>=0 && voltage<=15000)){
    current2 = pzem2.current();
    pf2 = pzem2.pf();
    Act_Power2 = pzem2.power();
    //efficiency calculation of transformer
    Efficiency = (Act_Power2/Act_Power)*100;   //effi is ratio of usefull input power to output(act power)
    int efficiency = Efficiency*100;   // the below 2 lines are used for displaying efficieny with 2 decimal points
    Efficiency = efficiency/100;
    /*//checking for diffrential relay protection to detect internal faults like earth fault, phase to phase fault, 
    and can detect internal discharges like sparking b/w conducters....*/
    Diffrential = ((current*2.0909) - current2); // checks for diffrential current
  }
  else{
    voltage2=0.00,current2=0.00,pf2=0.01,Act_Power2=0.00,Efficiency=0.00,Diffrential=0.00;
  }
  ////////////////////
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
  ///////////////
  digitalWrite(trigPin, LOW);  //code
  delayMicroseconds(10);
  digitalWrite(trigPin, HIGH); // Sets the trigPin on HIGH state for 10 micro seconds
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH); // Reads the echoPin, returns the sound wave travel time in microseconds
  // Calculate the distance and oil level
  distanceCm = duration * SOUND_SPEED/2;
  float Oil_level = (100*(22.5-distanceCm)/19); //calibration line for Oil Level
  float t = dht.readTemperature() + 1;  //+1 added for sensor calibration u can add required equation if calibration of dht11 needed
  float h = dht.readHumidity()+3;  // +3 because of sensor calibration
  /////////////// 'himidity in RH' to 'moisture in ppm' convertion code, used curve fitting to get relative equation b/w temp,himi,moist 
  float x111 = 1+pow((t/23772.42),2.456301);
  float x11 = (539495600+(34.38525-539495591)/x111);
  float x1 = h*0.0125*x11;
  float x2 = (h*30/43)*0.01*x11;  //edit here for moisture calibration
  int moisture = x1-x2; //this will give moisture in ppm
  //////////////
  /////////////////mmmmmmmmmmmmmmmmmmmmm/////////////gas sensors code start
  MQ8.update(); // Update data, the arduino will be read the voltage on the analog pin
  MQ135.update();
  MQ7.update();
  MQ4.update();
  int H2 = MQ8.readSensor(); // Sensor will read PPM concentration using the model and a and b values setted before or in the setu
  int CO2 = MQ135.readSensor();
  int CO = MQ7.readSensor();
  int CH4 = MQ4.readSensor();
  //////////////////mmmmmmmmmmmmmmmmmmm/////////////end
  /////////// the below line will prevent esp from dumping garbage values like 2147483648 in to firebase
  if(moisture>=0 && moisture<=10000){
    moisture = moisture;
  }
  else{
    moisture = 0.01;
  }
  if(t>=-40 && t<=150){
    t = t;
  }
  else{
    t = 0.01;
  }
  if(h>=5 && h<=100){
    h =h;
  }
  else{
    h = 0.01;
  }
  if(Oil_level>=0 && Oil_level<=110){
    Oil_level = Oil_level;
  }
  else{
    Oil_level = 0.01;
  }
  if(CO>=0 && CO<=10000){
    CO = CO;
  }
  else{
    CO = 0.01;
  }
  if(CO2>=0 && CO2<=10000){
    CO2 = CO2;
  }
  else{
    CO2 = 0.01;
  }
  if(CH4>=0 && CH4<=10000){
    CH4 = CH4;
  }
  else{
    CH4 = 0.01;
  }
  if(H2>=0 && H2<=10000){
    H2 = H2;
  }
  else{
    H2 = 0.01;
  }
  ////////////// non ele////////////^
  // verification of normal faulty conditions
  if (voltage>=250||voltage<=210||current>=4.348||current==0.00||pf<=0.6||frequency>=51.5||frequency<=48.5||Efficiency<=85||Diffrential>=0.01){
    Alart2=1;
    LedE = 1;
    if (voltage==0.00){  //led will not turned on for this condition: NO POWER
      LedE = 0;
      Alart_message2 = "NO POWER SUPPLY";
      if(AA1==1){
      bot.sendMessage(chat_id, "NO POWER SUPPLY");
      }
      AA1 = 0;
      AA2 = 1;
    }
    else if(current==0.00){  ////led will not turned on for this condition: TRANSFORMER OFF
      Alart_message2 = "Transformer Turned OFF";
      LedE = 0;
      Volt = 0;
      if(AA1==1){
      bot.sendMessage(chat_id, "Transformer Turned OFF");
      }
      AA1 = 0;
      AA2 = 1;
    }
    else if(Diffrential>=0.01){  ////if diffrential current higher then 0.01A then fault will be considerd
      Alart_message2 = "Earth Fault Occured";
      LedE = 1;
      if(AA1==1){
      bot.sendMessage(chat_id, "Earth Fault Occured");
      }
      AA1 = 0;
      AA2 = 1;
    }
    else if(voltage>=250){
      Alart_message2 = "Over voltage: voltage is greater then 250V";
      LedE = 1;
      if(AA1==1){
      bot.sendMessage(chat_id, "Over voltage: voltage is greater then 250V");
      }
      AA1 = 0;
      AA2 = 1;
    }
    else if(voltage<=210){
      Alart_message2 = "Under voltage: voltage is lesser then 210V";
      LedE = 1;
      if(AA1==1){
      bot.sendMessage(chat_id, "Under voltage: voltage is lesser then 210V");
      }
      AA1 = 0;
      AA2 = 1;
    }
    else if(current>=4.348){
      Alart_message2 = "Over load: current is higher then 4.34A";
      LedE = 1;
      if(AA1==1){
      bot.sendMessage(chat_id, "Over load: current is higher then 4.34A");
      }
      AA1 = 0;
      AA2 = 1;
    }
    else if(pf!=0.00 && pf<=0.6){
      Alart_message2 = "Low pf: pf is lesser then 0.6";
      LedE = 1;
      if(AA1==1){
      bot.sendMessage(chat_id, "Low pf: pf is lesser then 0.6");
      }
      AA1 = 0;
      AA2 = 1;
    }
    else if(Efficiency<=85){
      Alart_message2 = "Low Efficiency: Efficiency is lesser then 85%";
      if(AA1==1){
      bot.sendMessage(chat_id, "Low Efficiency: Efficiency is lesser then 85%");
      }
      AA1 = 0;
      AA2 = 1;
    }
    else{
      Alart_message2 = "NORMAL";
      LedE = 0;
      if(AA2==1){
      bot.sendMessage(chat_id, "Normal Electrical fault solved");
      }
      AA1==1,AA2 = 0;
      }
  }
  else{
    Alart_message2 = "NORMAL";
    LedE = 0;
    if(AA2==1){
    bot.sendMessage(chat_id, "Normal Electrical fault solved");
    }
    AA1==1,AA2 = 0;
    }
  //////////this below line is used to check over heating of transformar oil
  if(t>=75 ||t<0){
    Alart1=1;
    //digitalWrite(CB_PinB, HIGH);  //Removed pin 27 and made it to operate throw pin 14 using and operation
    CB_Emerg = 1;
    digitalWrite(buzzerB, HIGH);
    LedN = 1;
    Alart_message1 = "EMERGENCY: Transformer Oil temprature Rised above 75*C >>>> Transformer turned off";
    if(BB1E==1){
      bot.sendMessage(chat_id,"EMERGENCY: Transformer temprature incresed above 75*C >>>> Transformer turned off");
    }
    BB1E = 0;
    BB2E = 1;
  }
  //this below line will check for non elec faulty conditions and report fault and reason
  else if(moisture>=35||(Oil_level<=60 && Oil_level>=10)||(t>=65)||(H2>=100&&CO2>=90&&CO>=500&&CH4>=200)||(H2>=49&&CO2>=20&&CO>=300&&CH4>=120&&H2<=70&&CO2<=60)){
    Alart1=1;
    LedN = 1;
    if(t>=65){
      Alart_message1 = "Oil temperature is greater then 65*C";
      if(BB1==1){
        bot.sendMessage(chat_id, "Non_Electrical Alart: Oil temperature is greater then 65*C");
      }
      BB1 = 0;
      BB2 = 1;
    }
    else if(Oil_level<=60 && Oil_level>=10){
      Alart_message1 = "OIL level is lesser then 60%";
      LedE = 1;
      if(BB1==1){
        bot.sendMessage(chat_id, "Non_Electrical Alart: OIL level is lesser then 60%");
      }
      BB1 = 0;
      BB2 = 1;
    }
    else if(moisture>=35){
      Alart_message1 = "Moisture is higher then 35ppm";
      LedE = 1;
      if(BB1==1){
        bot.sendMessage(chat_id, "Non_Electrical Alart: Moisture is higher then 35ppm");
      }
      BB1 = 0;
      BB2 = 1;
    }
    else if(H2>=100&&CO2>=90&&CO>=500&&CH4>=200){  ////the conditions are derived from tryle and error method conducted in HV lab
      Alart_message1 = "Expected Oil Overheating";
      LedE = 1;
      if(BB1==1){
        bot.sendMessage(chat_id, "Non_Electrical Alart: Expected Oil Overheating");
      }
      BB1 = 0;
      BB2 = 1;
    }
    else if(H2>=49&&CO2>=20&&CO>=300&&CH4>=120&&H2<=70&&CO2<=60){ //the conditions are derived from tryle and error method conducted in HV lab
      Alart_message1 = "Expected Sparking in Oil,Paper or Both";
      LedE = 1;
      if(BB1==1){
        bot.sendMessage(chat_id, "Non_Electrical Alart: Expected Sparking in Oil,Paper or Both");
      }
      BB1 = 0;
      BB2 = 1;
    }
    else{
      LedE = 0;
      if(BB2==1){
        bot.sendMessage(chat_id, "Normal Non_Electrical Alart cleard");
      }
      else{}
      BB1 = 1;
      BB2 = 0;
      }
  }
  else{
    Alart1 = 0;
    //digitalWrite(CB_PinB, LOW);
    CB_Emerg = 0;
    digitalWrite(buzzerB, LOW);
    LedN = 0;
    Alart_message1 = "NORMAL";
    if(BB2E==1){
      bot.sendMessage(chat_id, "Oil cooled down (Temp<65*c): Transformer turned ON");
    }
    BB1E = 1;
    BB2E = 0;
  }

  ////////operating conditions for Led
  if(LedE == 1 || LedN == 1){
    digitalWrite(fault_ledB, HIGH);
  }
  else{
    digitalWrite(fault_ledB, LOW);
  }
  ////////////////////////
  /////////////dumping the data to the firebase
  
  if(Firebase.ready() && signupOK ) {
   
    if (Firebase.RTDB.setFloat(&fbdo, "Main/Electrical/VOLTAGE", Volt)){
      Serial.println("PASSED");
      Serial.print("Primary Voltage: ");
      Serial.println(Volt);
      Serial.print("Secondary Voltage: ");
      Serial.println(voltage2 + 0.2);  //0.2 is added to calibrate with PZEM1
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
    if (Firebase.RTDB.setFloat(&fbdo, "Main/Electrical/CURRENT",current)){
       Serial.println("PASSED");
       Serial.print("Primary Current: ");
       Serial.println(current);
       Serial.print("Secondary Current: ");
       Serial.println(current2);
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
       Serial.println(pf2);
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
    if (Firebase.RTDB.setFloat(&fbdo, "Main/Electrical/ACTIVE_POWER",Act)){
       Serial.println("PASSED");
       Serial.print("ACTIVE_POWER: ");
       Serial.println(Act);
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
    //delay(500); ///////
    if (Firebase.RTDB.setFloat(&fbdo, "Main/Electrical/REACTIVE_POWER", Rec)){
       Serial.println("PASSED");
       Serial.print("REACTIVE_POWER: ");
       Serial.println(Rec);
    }
    else {
       Serial.println("FAILED");
       Serial.println("REASON: " + fbdo.errorReason());
    }
    if (Firebase.RTDB.setFloat(&fbdo, "Main/Electrical/APPARANT_POWER", Apa)){
       Serial.println("PASSED");
       Serial.print("APPARANT_POWER: ");
       Serial.println(Apa);  
    }
    else {
       Serial.println("FAILED");
       Serial.println("REASON: " + fbdo.errorReason());
    }
    if (Firebase.RTDB.setFloat(&fbdo, "Main/Electrical/EFFICIENCY", Efficiency)){
       Serial.println("PASSED");
       Serial.print("EFFICIENCY(%): ");
       Serial.println(Efficiency);  
       Serial.print("Diffrential Current(A): ");
       Serial.println(Diffrential);
    }
    else {
       Serial.println("FAILED");
       Serial.println("REASON: " + fbdo.errorReason());
    }
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
    //READING DATA FROM FIREBASE TO OPERATE CB
    if(Firebase.RTDB.getBool(&fbdo, "Main/Bhool/Circuit_breaker")){
      if(fbdo.dataType() == "boolean"){
        CB_status = fbdo.boolData();
        Serial.println("Sucessful READ from" + fbdo.dataPath() + ":" + CB_status + " (" + fbdo.dataType() + ")");
        if(CB_status==1 && CB_Emerg!=1)
        {
          digitalWrite(CB_Pin, HIGH);
        }
        else{
          digitalWrite(CB_Pin, LOW);
        }
      }
    }
    else{
       Serial.println("FAILED:" + fbdo.errorReason());
    }  
    Serial.println("______________________________");
  ///////////////
    if(Firebase.RTDB.setFloat(&fbdo, "Main/DHT/humidity",h)){
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
    //delay(1000);
    //////////////
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
    if(Firebase.RTDB.setFloat(&fbdo, "Main/DHT/CO2_Gaslevel",CO2)){
      Serial.println("PASSED");
      Serial.print("CO2 Gas level in ppm: ");
      Serial.println(CO2);
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
  
  /////////////
  ///////////SD card code
  /*
  myFile = SD.open("/test.txt", FILE_WRITE);
  if (myFile){
    myFile.print(voltage);
    myFile.print(",");
    myFile.print(current);
    myFile.print(",");
    myFile.print(frequency);
    myFile.print(",");
    myFile.print(pf);
    myFile.print(",");
    myFile.print(Act);
    myFile.print(",");
    myFile.print(Rec);
    myFile.print(",");
    myFile.print(Apa);
    myFile.print(",");
    myFile.print(energy);
    myFile.print(",");
    myFile.print(Alart2);
    myFile.print(",");
    myFile.print(Alart_message2);
    myFile.print(",");
    myFile.print(Oil_level);
    myFile.print(",");
    myFile.print(t);
    myFile.print(",");
    myFile.print(h);
    myFile.print(",");
    myFile.print(moisture);
    myFile.print(",");
    myFile.print(H2);
    myFile.print(",");
    myFile.print(C2H2);
    myFile.print(",");
    myFile.print(CO);
    myFile.print(",");
    myFile.print(CH4);
    myFile.print(",");
    myFile.print(Alart1);
    myFile.print(",");
    myFile.println(Alart_message1);
    //myFile.print(",");
    myFile.close(); // close the file
    }
    */
  ///////////
  /////////////
}
