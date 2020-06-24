#include "Canbus.h"
#include "Kombi.h"
#include "Bluetooth.h"
#include <SPI.h>
#include "helper.h"
#include "ICDisplay.h"
#include "Music.h"

// DO NOT TOUCH THIS!
// In doing so you may accidently trigger a lights show
// whilst driving, which would be illegal :)
bool safeForShowTime = false;

#ifdef DEBUG
int freeRam () {
  extern int __heap_start, *__brkval;
  int v;
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}
unsigned long lastMemTime = millis();
#endif


// For printf
static FILE uartout = { 0 };   // FILE struct
static int uart_putchar(char c, FILE *stream);


Bluetooth::Payload btp{};
can_frame read{};

// uart_putchar - char output function for printf
static int uart_putchar (char c, FILE *stream)
{
  if( c == '\n' )
    Serial.print(F("\r"));
  Serial.write(c) ;
  return 0;
}

Music *music = nullptr;
void setup() {
  // Printf stuff
  fdev_setup_stream(&uartout, uart_putchar, NULL, _FDEV_SETUP_WRITE);
  stdout = &uartout;
  
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  Serial.begin(115200);
  SPI.begin();
  Bluetooth::begin(9600); // Baud rate on HC-06 module
  canB = new Canbus(4, CAN_83K3BPS, MCP_16MHZ);
  canC = new Canbus(5, CAN_500KBPS, MCP_16MHZ);
  kombi = new Kombi();
  audioDisplay = new AudioDisplay();
  telDisplay = new TelDisplay();
  LOG_INFO(F_TC("Setup complete!\n"));
  // By default, enable the audio page
  audioDisplay->enablePage();
}

void nextTrack() {
  LOG_DEBG(F_TC("Asking for next track\n"));
  btp.cmd = BT_CMD_MUSIC_CTRL;
  btp.argSize = 1;
  btp.args[0] = BT_NEXT;
  Bluetooth::sendPayload(&btp);
}

void prevTrack() {
  LOG_DEBG(F_TC("Asking for previous track\n"));
  btp.cmd = BT_CMD_MUSIC_CTRL;
  btp.argSize = 1;
  btp.args[0] = BT_PREV;
  Bluetooth::sendPayload(&btp);
}

void keyInputAudio() {
  switch(kombi->getKeyPress()) {
    case BUTTON_PAGE_NEXT:
      nextTrack();
      break;
    case BUTTON_PAGE_PREV:
      prevTrack();
      break;
    default:
      break;
  }
}

void keyInputTel() {
  switch(kombi->getKeyPress()) {
    default:
      break;
  }
}

void keyInputOther() {
  switch(kombi->getKeyPress()) {
    default:
      break;
  }
}

void handleBMessage() {
  kombi->processKombiFrame(&read);
  ICDisplay::processIncommingFrame(&read);
}

void handleCMessage() {

}

void loop() {
  if (!Bluetooth::isConnected() && music) {
    LOG_INFO(F_TC("Phone disconnected. Unloading music\n"));
    delete music;
    music = nullptr;
    audioDisplay->removeMusic();
  }
  if (Bluetooth::getPayload(&btp)) {
    switch (btp.cmd)
    {
    case BT_CMD_TRACK_NAME:
      delete music;
      music = new Music((char*)btp.args, btp.argSize);
      audioDisplay->setMusicData(music);
      break;
    case BT_CMD_TRACK_SEEK:
      if (music) music->setPosition(btp.args[0] | btp.args[1] << 8);
      break;
    case BT_CMD_TRACK_LEN:
      if (music) music->setDuration(btp.args[0] | btp.args[1] << 8);
      break;
    case BT_CMD_MUSIC_CTRL:
      if (music) music->setPlayStatus(btp.args[0]);
      break;
    default:
      break;
    }
  }


  // Poll for new can frames on both busses
  if (canB->readMessage(&read)) {
    handleBMessage();
  }
  if (canC->readMessage(&read)) {
    handleCMessage();
  }
  switch(kombi->getICPage()) {
    case KOMBI_PAGE_AUDIO:
      keyInputAudio();
      break;
    case KOMBI_PAGE_PHONE:
      keyInputTel();
      break;
    default:
      keyInputOther();
      break;
  }
  /*
  if (kombi->getICPage() == KOMBI_PAGE_AUDIO) { // Only let audio page write to CAN
    audioDisplay->enablePage();
    telDisplay->disablePage();
  } else if (kombi->getICPage() == KOMBI_PAGE_PHONE) { // Only let tele page write to CAN
    audioDisplay->disablePage();
    telDisplay->enablePage();
  } else { // Disable both pages from writing to CAN
    audioDisplay->disablePage();
    telDisplay->disablePage();
  }
  */
  // The update functions are still needed, even if we are not in the page
  audioDisplay->update();
  telDisplay->update();
  if (music) music->update();

  #ifdef DEBUG
  if (millis() - lastMemTime >= 5000) {
    lastMemTime = millis();
    Serial.println(freeRam(), DEC);
  }
  #endif
}
