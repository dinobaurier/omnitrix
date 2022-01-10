#include <Arduino.h>

#include <Arduino_GFX.h>
#include <Arduino_GC9A01.h>
#include <Arduino_ESP32SPI.h>
#include <Adafruit_I2CDevice.h>
#include <Adafruit_SPIDevice.h>

#include <SPIFFS.h>
#include <pngle.h>
#include "DFRobotDFPlayerMini.h"

#define GIF_FILENAME "/animation.gif"
#include "pins.h"


#include "GifClass.h"

const int pullUps[]={14,15,25,32,34,35,36,39};
static GifClass gifClass;

const int freq = 10000;
const int ledChannel = 13;
const int resolution = 8;

Arduino_DataBus *bus = new Arduino_ESP32SPI(TFT_DC, TFT_CS, TFT_SCK, TFT_MOSI, TFT_MISO, VSPI /* spi_num */);
Arduino_GC9A01 *gfx = new Arduino_GC9A01(bus, TFT_RST, 0 /* rotation */, true /* IPS */);

HardwareSerial myHardwareSerial(1); // use HardwareSerial UART1
DFRobotDFPlayerMini myDFPlayer;

// Parameter
const byte volume = 25, startSound = 1, nextSound = 3, selectSound = 4, endSound = 6, introSound = 8;
const int attacSeconds = 60; // Dauer Kampfmodus in Sekunden 
int transLoop = 0;
bool turn = false;
byte alienNo = 0, mode =1; // 1 = idle , 2 = alien Select , 3 = Kampf

int16_t xOffsetDefault = 65;
int16_t yOffsetDefault = 25;

int16_t xOffset = xOffsetDefault;
int16_t yOffset = yOffsetDefault;

#include "graphic.h"

// *************************************
// FUNTIONEN
// *************************************

void drawStart()
{
  gfx->fillScreen(DARKGREY);
  xOffset = 0;
  yOffset = 0;
  showImage(99);

  xOffset = xOffsetDefault;
  yOffset = yOffsetDefault;
}

void endTransformation()
{
  myDFPlayer.play(endSound);

  gfx->fillScreen(BLACK);
  delay(2000);
  // Offset ändern damit ganzer Bildschirm gefüllt wird.
  xOffset = 0;
  yOffset = 0;
  showImage(98);


  // und wieder auf Standard
  xOffset = xOffsetDefault;
  yOffset = yOffsetDefault;
}

void selectAlien(){
  if (alienNo == 11)  {
    // INTRO abspielen
    myDFPlayer.play(introSound);
  }
  else{
    myDFPlayer.play(selectSound);
  }
}

void nextAlien(){
  myDFPlayer.play(nextSound);
  alienNo++;
  if (alienNo == 12){
    alienNo = 1;
  }
  showImage(alienNo);
}

// *************************************
// FUNTIONEN ENDE 
// *************************************


void playStartAnimation(){
  myDFPlayer.play(startSound);
  File gifFile = SPIFFS.open(GIF_FILENAME, "r");

  if (!gifFile || gifFile.isDirectory()){
    // Serial.println(F("ERROR: open gifFile Failed!"));
    // gfx->println(F("ERROR: open gifFile Failed!"));
  }
  else{
    // read GIF file header
    gd_GIF *gif = gifClass.gd_open_gif(&gifFile);
    if (!gif){
      // Serial.println(F("gd_open_gif() failed!"));
    }
    else{
      int32_t s = gif->width * gif->height;
      uint8_t *buf = (uint8_t *)malloc(s);
      if (!buf){
        // Serial.println(F("buf malloc failed!"));
      }
      else{
        // Serial.println(F("GIF video start"));
        gfx->setAddrWindow((gfx->width() - gif->width) / 2, (gfx->height() - gif->height) / 2, gif->width, gif->height);
        int t_fstart, t_delay = 0, t_real_delay, res, delay_until;
        int duration = 0, remain = 0;
        while (1){
          t_fstart = millis();
          t_delay = gif->gce.delay * 10;
          res = gifClass.gd_get_frame(gif, buf);
          if (res < 0){
            // Serial.println(F("ERROR: gd_get_frame() failed!"));
            break;
          }
          else if (res == 0){
            // Serial.print(F("rewind, duration: "));
            // Serial.print(duration);
            // Serial.print(F(", remain: "));
            // Serial.print(remain);
            // Serial.print(F(" ("));
            // Serial.print(100.0 * remain / duration);
            // Serial.println(F("%)"));
            duration = 0;
            remain = 0;
            //gifClass.gd_rewind(gif);
            continue;
          }

          gfx->startWrite();
          gfx->writeIndexedPixels(buf, gif->palette->colors, s);
          gfx->endWrite();

          t_real_delay = t_delay - (millis() - t_fstart);
          duration += t_delay;
          remain += t_real_delay;
          delay_until = millis() + t_real_delay;
          do{
            delay(1);
          } while (millis() < delay_until);
        }
        // Serial.println(F("GIF video end"));
        // Serial.print(F("duration: "));
        // Serial.print(duration);
        // Serial.print(F(", remain: "));
        // Serial.print(remain);
        // Serial.print(F(" ("));
        // Serial.print(100.0 * remain / duration);
        // Serial.println(F("%)"));

        gifClass.gd_close_gif(gif);
        free(buf);
      }
    }
  }
}



void setup()
{

  // PULLUPS
  for (byte pin = 0; pin < sizeof(pullUps); pin++) {
    pinMode(pin, INPUT_PULLUP);
  }

  pinMode(TFT_LED, OUTPUT);
  // delay(1000);
  Serial2.begin(9600, SERIAL_8N1, RX2, TX2); // RX, TX
  Serial.begin(115200);

  ledcSetup(ledChannel, freq, resolution);
  ledcAttachPin(TFT_LED, ledChannel);
  ledcWrite(ledChannel, 255);

  // Serial.println();
  // Serial.println(F("DFRobot DFPlayer Mini Demo"));
  // Serial.println(F("Initializing DFPlayer ... (May take 3~5 seconds)"));
  //if (!myDFPlayer.begin(mySoftwareSerial)) {  //Use softwareSerial to communicate with mp3.
  myDFPlayer.setTimeOut(2000);

  if (!myDFPlayer.begin(Serial2, false)){ //Use HardwareSerial to communicate with mp3.
    // Serial.println(F("Unable to begin:"));
    // Serial.println(F("1.Please recheck the connection!"));
    // Serial.println(F("2.Please insert the SD card!"));
    while (true){
      delay(0); // Code to compatible with ESP8266 watch dog.
    }
  }
  // Serial.println(F("DFPlayer Mini online."));

  myDFPlayer.volume(volume); //Set volume value. From 0 to 30
  myDFPlayer.EQ(DFPLAYER_EQ_BASS);

  pinMode(BTN_RELEASE, INPUT);
  pinMode(BTN_RING, INPUT);

  // Init Display
  gfx->begin(40000000);
  gfx->fillScreen(DARKGREY);

/* Wio Terminal */
#if defined(ARDUINO_ARCH_SAMD) && defined(SEEED_GROVE_UI_WIRELESS)
  if (!SD.begin(SDCARD_SS_PIN, SDCARD_SPI, 4000000UL))
#elif defined(ESP32) || defined(ESP8266)
  if (!SPIFFS.begin())
  // if (!SD.begin())
#else
  if (!SD.begin())
#endif
  {
    // Serial.println(F("ERROR: File System Mount Failed!"));
    // gfx->println(F("ERROR: File System Mount Failed!"));
  }

  drawStart();
}

void loop(){

  // Startmodus - es wird nur auf Öffnung der Omnitrix reagiert
  if (mode == 1){
    int Button_RELEASE_state = digitalRead(BTN_RELEASE);
    if (Button_RELEASE_state == LOW){
      delay(200);
      mode = 2;
      playStartAnimation();
    }
  }

  // Auswahlmodus - reagiert auf Drehring und Reindrücken
  else if (mode == 2){
    int Button_RING_state = digitalRead(BTN_RING);
    int Button_RELEASE_state = digitalRead(BTN_RELEASE);

    // Drehring hat den Kontakt berührt
    if (Button_RING_state == HIGH && turn == false){
      turn = true;
    }

     // Wechsel erst ausführen wenn Kontakt am Drehring wieder weg ist
    else if (Button_RING_state == LOW && turn == 1){
      turn = false;
      nextAlien();
    }

    // Herabgedrückt - Alien ausgewählt. 
    if (Button_RELEASE_state == HIGH){
      delay(200);
      if (alienNo > 0){
        mode = 3;
        // Serial.println(F("Alien Selected"));
        selectAlien();
      }
      // Wenn kein Alien ausgewählt, wieder auf Start
      else{
        mode = 1;
        drawStart();
      }
    }
  }

  // Kampfmodus
  else if (mode == 3){
    transLoop++;
    delay(500);

    int Button_RELEASE_state = digitalRead(BTN_RELEASE);

    // Wenn wieder geöffnet wird, Kampfmodus abbrechen
    if (Button_RELEASE_state == LOW){
      mode = 2;
      transLoop = 0;
      playStartAnimation();
    }

    // Nach Dauer attacSeconds wieder zurück in Startmodus wechseln
    if (transLoop > attacSeconds * 2){
      transLoop = 0;
      mode = 1;
      endTransformation();
    }
  }
}
