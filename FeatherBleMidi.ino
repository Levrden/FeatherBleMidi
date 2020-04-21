#include <Wire.h>
#include <SPI.h>

#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_SPI.h"
#include "Adafruit_BLEMIDI.h"
#include "BluefruitConfig.h"

#define FACTORYRESET_ENABLE         1
#define MINIMUM_FIRMWARE_VERSION    "0.7.0"

Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);
Adafruit_BLEMIDI midi(ble);

#define CHANNEL 0  // MIDI channel number 1
const int outPins[] = { 5, 6, 9, 10, 11, 12, 13 }; // Les pins utilisées en sortie (PWM).
int valuePin[] = {0, 0, 0, 0, 0, 0, 0};

bool isConnected = false;

void setup() {
  //pinMode(A0, INPUT);
  for( int i=0 ; i < (sizeof(outPins)/sizeof(int)) ; i++) {
    pinMode(outPins[i], OUTPUT); // Les outPins sont initialisées en OUTPUT.
  }
  Serial.begin(115200);

  if ( !ble.begin(VERBOSE_MODE) ) {
    error(F("Couldn't find Bluefruit, check wiring?"));
  }
  Serial.println( F("OK!") );

  if ( FACTORYRESET_ENABLE ) {
    /* Perform a factory reset to make sure everything is in a known state */
    Serial.println(F("Performing a factory reset: "));
    if ( ! ble.factoryReset() ) {
      error(F("Couldn't factory reset"));
    }
  }
  ble.println("AT+GAPDEVNAME=HapticGloveRight");
  ble.echo(false);

  Serial.println("Requesting Bluefruit info:");
  /* Print Bluefruit information */
  ble.info();
  
  /* Set BLE callbacks */
  ble.setConnectCallback(connected);
  ble.setDisconnectCallback(disconnected);
  Serial.println(F("Enable MIDI: "));
  
  if ( ! midi.begin(true) ) {
    error(F("Could not enable MIDI"));
  }
    
  ble.verbose(false);
  Serial.println(F("Waiting for a connection..."));

  midi.setRxCallback(MIDI_in_callback);
}

void loop() {
  ble.update(1);
  //int val = analogRead(A0);
  //midi.send(0xB0 | CHANNEL, 10, val); // Send the value you read from A0 using controller number 10 on MIDI defined earlier
  decreasePins();
}

void MIDI_in_callback(uint16_t tstamp, uint8_t status, uint8_t CCnumber, uint8_t CCvalue)
{ 
  Serial.print("CTRL received. Controller: ");
  Serial.print(CCnumber);
  Serial.print(", value: ");
  Serial.println(CCvalue);
  if(CCvalue<6 && CCvalue>-1){
      midiWrite(CCvalue);
  }
}



void error(const __FlashStringHelper*err) {
  Serial.println(err);
  while (1);
}

void connected(void) {
  isConnected = true;
  Serial.println(F(" CONNECTED!"));
}

void midiWrite(int midiIn){
  //Serial.println(midiIn);
  valuePin[midiIn] = 254;
  analogWrite(outPins[midiIn], valuePin[midiIn]);
}

void decreasePins(){
  for(int i=0 ; i < (sizeof(outPins)/sizeof(int)) ; i++) {
    if(valuePin[i]>0){
      valuePin[i] -= 2;
      analogWrite(outPins[i], valuePin[i]);
    }
  }
}

void disconnected(void) {
  Serial.println("disconnected");
}
