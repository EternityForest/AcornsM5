/*
 M5Stack UI Loop example sketch. See README file for how to put apps on the SD card
*/

#include "acorns.h"


#include "FS.h"
#include "SPIFFS.h"
#include <M5Stack.h>
#include <M5ez.h>
#include <AcornsM5.h>


#define TFT_GREY 0x5AEB

void setup() {
  Serial.begin(115200);
  Serial.println("**starting up**");

  //Give the system file access, which acorns needs.
  SPIFFS.begin();
  Acorns.begin();

  sqDoM5Bindings();


  ez.begin();
  M5.Lcd.fillScreen(TFT_GREY);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setTextWrap(true);

  M5.Lcd.setTextColor(TFT_WHITE, TFT_GREY);  // Adding a background colour erases previous text automatically

  static TaskHandle_t l;
  xTaskCreatePinnedToCore(acorns_UILoop,
                          "UILoop",
                          8192,
                          0,
                          1,
                          &l,
                          1
                         );
}

unsigned long last = millis();


// the loop function runs over and over again forever
void loop() {
  last = millis();
  while (Serial.available())
  {
    Acorns.replChar(Serial.read());
  }
  //Reduce power usage.
  delay(2);
}
