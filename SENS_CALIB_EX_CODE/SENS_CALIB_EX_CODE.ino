/*
  MQUnifiedsensor Library - reading an MQ8

  Demonstrates the use a MQ8 sensor.
  Library originally added 01 may 2019
  by Miguel A Califa, Yersson Carrillo, Ghiordy Contreras, Mario Rodriguez
 
  Added example
  modified 23 May 2019
  by Miguel Califa 

  Updated library usage
  modified 26 March 2020
  by Miguel Califa 

  Wiring:
  https://github.com/miguel5612/MQSensorsLib_Docs/blob/master/static/img/MQ_Arduino.PNG
  Please take care, arduino A0 pin represent the analog input configured on #define pin

 This example code is in the public domain.

*/

//Include the library
#include <MQUnifiedsensor.h>
//Definitions
#define placa "ESP 32"
#define Voltage_Resolution 5
//#define pin A0 //Analog input 0 of your arduino
#define type "MQ-8" //MQ8
#define type1 "MQ-6" //MQ6
#define type2 "MQ-7" //MQ7
#define type3 "MQ-4" //MQ4
#define ADC_Bit_Resolution 12 // For arduino UNO/MEGA/NANO,12 for esp32
#define RatioMQ8CleanAir 180   //RS / R0 = 70 ppm  
#define RatioMQ6CleanAir 70   //RS / R0 = 70 ppm  
#define RatioMQ7CleanAir 130   //RS / R0 = 70 ppm  
#define RatioMQ4CleanAir 80   //RS / R0 = 70 ppm  

#define gasPin A0
#define gasPin1 A3
#define gasPin2 A7
#define gasPin3 A6
//#define calibration_button 13 //Pin to calibrate your sensor
float gaslevel,gaslevel1,gaslevel2,gaslevel3;    //H2 gas pin MQ8,C2H2 gas pin MQ6,CO gas pin MQ7,CH4 gas pin MQ4
//Declare Sensor
MQUnifiedsensor MQ8(placa, Voltage_Resolution, ADC_Bit_Resolution, gasPin, type);
MQUnifiedsensor MQ6(placa, Voltage_Resolution, ADC_Bit_Resolution, gasPin1, type1);
MQUnifiedsensor MQ7(placa, Voltage_Resolution, ADC_Bit_Resolution, gasPin2, type2);
MQUnifiedsensor MQ4(placa, Voltage_Resolution, ADC_Bit_Resolution, gasPin3, type3);
void setup() {
  //Init the serial port communication - to debug the library
  Serial.begin(9600); //Init serial port

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
}

void loop() {
  MQ8.update(); // Update data, the arduino will be read the voltage on the analog pin
  MQ6.update();
  MQ7.update();
  MQ4.update();
  int H2 = MQ8.readSensor(); // Sensor will read PPM concentration using the model and a and b values setted before or in the setup
  Serial.print("H2 PPM : ");
  Serial.println(H2);

  int C2H2 = MQ6.readSensor(); // Sensor will read PPM concentration using the model and a and b values setted before or in the setup
  Serial.print("C2H2 PPM : ");
  Serial.println(C2H2);
  int CO = MQ7.readSensor(); // Sensor will read PPM concentration using the model and a and b values setted before or in the setup
  Serial.print("CO PPM : ");
  Serial.println(CO);
  int CH4 = MQ4.readSensor(); // Sensor will read PPM concentration using the model and a and b values setted before or in the setup
  Serial.print("CH4 PPM : ");
  Serial.println(CH4);
  //MQ8.serialDebug(); // Will print the table on the serial port !!!!!!!!!!!!!!!!! this is the table !!!!!
  delay(1000); //Sampling frequency
}
