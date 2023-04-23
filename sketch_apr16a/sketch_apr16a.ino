/*
 *  Arduino SD Card Tutorial Example
 *  
 *  by Dejan Nedelkovski, www.HowToMechatronics.com
 */

#include <SD.h>
#include <SPI.h>

File myFile;

int pinCS = 5; // Pin 10 on Arduino Uno

void setup() {
    
  Serial.begin(9600);
  pinMode(pinCS, OUTPUT);
  
  // SD Card Initialization
  if (SD.begin())
  {
    Serial.println("SD card is ready to use.");
  } else
  {
    Serial.println("SD card initialization failed");
    return;
  }
  
  // Create/Open file 
  myFile = SD.open("/test.txt", FILE_WRITE);
  //appendFile(SD, "/helloworld.txt", "Hello World!\n");
  // if the file opened okay, write to it:
  if (myFile) {
    Serial.println("Writing to file...");
    // Write to file
    myFile.println("Testing text 1, 2 ,3...");
    myFile.close(); // close the file
    Serial.println("Done.");
  }
  // if the file didn't open, print an error:
  else {
    Serial.println("error opening test.txt");
  }

  // Reading the file
  myFile = SD.open("/test.txt");
  if (myFile) {
    Serial.println("Read:");
    // Reading the whole file
    while (myFile.available()) {
      Serial.write(myFile.read());
   }
    myFile.close();
  }
  else {
    Serial.println("error opening test.txt");
  }
  
}
void loop() {
  myFile = SD.open("/test.txt", FILE_WRITE);
  if (myFile){
    myFile.print("voltage");
    myFile.print(",");
    myFile.print("current");
    myFile.print(",");
    myFile.print("frequency");
    myFile.print("CO");
    myFile.print(",");
    myFile.print("CH4");
    myFile.print(",");
    myFile.print("Alart1");
    myFile.print(",");
    myFile.println("Alart_message1");
    //myFile.print(",");
    myFile.close(); // close the file
    Serial.println("mahesh karjol");
    delay(3000);
  }
  }
