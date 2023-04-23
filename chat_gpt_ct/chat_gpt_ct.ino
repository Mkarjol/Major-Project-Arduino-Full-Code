const int analogInPin = 25; // Define the analog input pin on the ESP32
float sensitivity = 0.185; // Sensitivity factor for ACS712-20A
float voltage = 0.0; // Variable to hold the analog voltage reading
float current = 0.0; // Variable to hold the corresponding current value

void setup() {
  Serial.begin(9600);
  pinMode(analogInPin, INPUT); // Configure the analog input pin
}

void loop() {
  for(int i=0;i<500;i++){
  voltage = voltage + analogRead(analogInPin) * (3.3 / 4096.0); // Read the analog voltage and scale it to 3.3V
  }
  current = ((voltage/500) - 1.65) / sensitivity; // Convert the voltage to current using the sensitivity factor
  Serial.print("Current: "); // Print the current value to the serial monitor
  Serial.print(current);
  Serial.println("A");
  Serial.print("Digital value");
  Serial.println(voltage/500);
  delay(1000); // Wait for 1 second before taking another reading
}
