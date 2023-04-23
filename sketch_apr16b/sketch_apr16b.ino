#include <SdFat.h>
#include <FS.h>

const int chipSelect = 5; // set the chip select pin for your SdFat module

SDFat SD;

void setup() {
  Serial.begin(9600);

  if (!SD.begin(chipSelect)) { // initialize the SdFat card
    Serial.println("SdFat card initialization failed!");
    return;
  }

  // get the current working directory
  File root = SD.openDir("/");
  Serial.print("Current directory: ");
  Serial.println(root.name());
}

void loop() {
  // your code here
}
