/*
 * Loki (Revision 1006)
 * (C) 2022 Blackbeard Softworks
 * 
 * This code relies on external hardware to be present.
 * Please refer to the wiring diagram.
 * 
 * The music must be prepared in this way:
 * - The input file must be MP3
 * - Right channel is used exclusively for motor control (meaning you can implement PWM at low frequency with amplitude modulation - from my testing it was <20Hz)
 * - Right channel must ONLY contain motor control signal (no voice, no music, only beeps at about 1024Hz~2048Hz)
 * - The beeps must not taper off in volume (it causes the motor to judder, so it needs to at least sound as if it turns on and off instantly)
 * - Use the DAW or other solutions to generate the beeps on the RIGHT CHANNEL ONLY
 * - Leave the left channel alone or adjust its volume (you CANNOT adjust the volume in code. It won't work. Adding a resistor for smaller speakers does work. About 51 Ohms work well)
 * - Right channel MUST be at 0db attenuation (meaning, it peaks and/or clips. Crank it up!)
 */

#include <DFRobotDFPlayerMini.h>
#include <SoftwareSerial.h>

SoftwareSerial mySoftwareSerial(10, 11);
DFRobotDFPlayerMini myDFPlayer;
void printDetail(uint8_t type, int value);

int songIndex = 1;

// This is the amount of songs on your SD card, greeting inclusive.
const int maxSongCount = 41;

// This will be the playback button
const int button = 2;
// If you want to use other pins for status detection, change this.
const int statusPin = A0;
volatile bool stopped = false;
volatile bool state = false;
volatile bool led = false;

// If you want to use other pins, change this.
const int ledPin = LED_BUILTIN;

unsigned long pm = 0;

// Change this to change how frequently the motor changes the direction
const long interval = 1000;

void setup() {
  pinMode(button, INPUT_PULLUP);
  pinMode(statusPin, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  mySoftwareSerial.begin(9600);
  Serial.begin(115200);
  Serial.println();
  Serial.println(F("Loki v1.1 starting up..."));
  if (!myDFPlayer.begin(mySoftwareSerial)) {  //Use softwareSerial to communicate with mp3.
    Serial.println(F("Unable to begin:"));
    Serial.println(F("1.Please recheck the connection!"));
    Serial.println(F("2.Please insert the SD card!"));
    while (true) {
      delay(0); // Code to compatible with ESP8266 watch dog.
    }
  }
  Serial.println(F("DFPlayer Mini online."));
  myDFPlayer.setTimeOut(1000);
  myDFPlayer.volume(30);  //Set volume value. From 0 to 30
  myDFPlayer.playMp3Folder(1);  //Play the first mp3
  delay(250);
  while (digitalRead(statusPin) == LOW) {
    // Wait while the track finishes playing
  }
  EIFR = 1;
  delay(8000);
  attachInterrupt(digitalPinToInterrupt(button), playbackCtrl, FALLING);
  Serial.println("Ready");
}

void loop() {

  unsigned long cm = millis();
  if (cm - pm >= interval) {
    pm = cm;
    led = !led;
  }
  digitalWrite(ledPin, led);
  if (state == true) {
    Serial.print("Attempting to play index ");
    Serial.print(songIndex);
    Serial.println(" file");
    myDFPlayer.playMp3Folder(songIndex);
    delay(50);
    while (digitalRead(statusPin) == LOW) {
      Serial.println("playing ");
    }
    state = false;
  }
  if (state == false) {
    delay(50);
    if (stopped != true) {
      Serial.println("Finished, stopping");
      myDFPlayer.stop();
      stopped = true;
    }
  }

}

void playbackCtrl() {
  static unsigned long lit = 0;
  unsigned long inttime = millis();
  if (inttime - lit > 1500) {
    if (state == true) {
      myDFPlayer.stop();
      state = false;
    }
    if (state == false) {
      myDFPlayer.stop();
      state = true;
    }
    songIndex++;
    if (songIndex > maxSongCount || songIndex < 2) {
      songIndex = 2;
    }
  }
  lit = inttime;
}
