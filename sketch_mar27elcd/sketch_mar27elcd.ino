/*
 * Using LCD screen with ESP32 
 * Written by Ahmad Shamshiri on Sep 04, 2019
 * at 17:49 in Ajax, Ontario, Canada
 * for Robojax.com
 * Watch video instruction for this code: 
 https://youtu.be/COssWn4Pcm4
 * 
Get this code and other Arduono codes from Robojax.com
You can get the wiring diagram from my Arduino Course at Udemy.com
Learn Arduino step by step with all library, codes, wiring diagram all in one place
visit my course now http://robojax.com/L/?id=62

If you found this tutorial helpful, please support me so I can continue creating 
content like this. You can support me on Patreon http://robojax.com/L/?id=63
or make donation using PayPal http://robojax.com/L/?id=64

 * Code is available at http://robojax.com/learn/arduino
 * This code is "AS IS" without warranty or liability. Free to be used as long as you keep this note intact.* 
 * This code has been download from Robojax.com
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>. 

*/

// start of settings for LCD1602 with I2C
#include <LiquidCrystal_I2C.h>
// Set the LCD address to 0x27 for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x27, 16, 2);
// watch video for details of 0x3F I2C address
// end of settings for LCD1602 with I2C

int count =0;
void setup() {
  lcd.begin();  
  lcd.backlight();
  lcd.print("Robojax ESP32");
  lcd.setCursor(0,1);
  lcd.print("Demo");  
  delay(2000); 
}


void loop() {
 // Robojax ESP32 LCD-1602-I2C 
  lcd.clear();// clear previous values from screen
  lcd.print("Robojax ESP32");
  lcd.setCursor(0,1);
  lcd.print("Counting:");
  lcd.setCursor(11,1);  
  lcd.print(count);
  count++;
  delay(200); 
}
