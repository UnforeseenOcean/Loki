/*
 * Loki (Revision 1441)
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
  delay(2000);
  if (!myDFPlayer.begin(mySoftwareSerial)) {  //Use softwareSerial to communicate with mp3.
    Serial.println(F("Unable to begin:"));
    Serial.println(F("1.Please recheck the connection!"));
    Serial.println(F("2.Please insert the SD card!"));
    while (true) {
      delay(0); // Code to compatible with ESP8266 watch dog.
    }
  }
  Serial.println(F("DFPlayer Mini online."));
  myDFPlayer.setTimeOut(1500);
  myDFPlayer.volume(30);  //Set volume value. From 0 to 30
  myDFPlayer.playMp3Folder(1);  //Play the first mp3
  delay(250);
  while (digitalRead(statusPin) == LOW) {
    // Wait while the track finishes playing
  }
  EIFR = 1;
  delay(7000);
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
    while (stopped == false) {
      Serial.println("playing");
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
  if (myDFPlayer.available()) {
    printDetail(myDFPlayer.readType(), myDFPlayer.read()); //Print the detail message from DFPlayer to handle different errors and states.
  }
}

void playbackCtrl() {
  static unsigned long lit = 0;
  unsigned long inttime = millis();
  if (inttime - lit > 1500) {
    if (state == true) {
      state = false;
      stopped = true;
    }
    if (state == false) {
      state = true;
      stopped = true;
    }
    songIndex++;
    if (songIndex > maxSongCount || songIndex < 2) {
      songIndex = 2;
    }
  }
  lit = inttime;
}

void printDetail(uint8_t type, int value) {

  switch (type) {
    case TimeOut:
      Serial.println(F("Time Out!"));
      break;
    case WrongStack:
      Serial.println(F("Stack Wrong!"));
      break;
    case DFPlayerCardInserted:
      Serial.println(F("Card Inserted!"));
      break;
    case DFPlayerCardRemoved:
      Serial.println(F("Card Removed!"));
      break;
    case DFPlayerCardOnline:
      Serial.println(F("Card Online!"));
      break;
    case DFPlayerUSBInserted:
      Serial.println("USB Inserted!");
      break;
    case DFPlayerUSBRemoved:
      Serial.println("USB Removed!");
      break;
    case DFPlayerPlayFinished:
      Serial.print(F("Number:"));
      Serial.print(value);
      Serial.println(F(" Play Finished!"));
      break;
    case DFPlayerError:
      Serial.print(F("DFPlayerError:"));
      switch (value) {
        case Busy:
          Serial.println(F("Card not found"));
          break;
        case Sleeping:
          Serial.println(F("Sleeping"));
          break;
        case SerialWrongStack:
          Serial.println(F("Get Wrong Stack"));
          break;
        case CheckSumNotMatch:
          Serial.println(F("Check Sum Not Match"));
          break;
        case FileIndexOut:
          Serial.println(F("File Index Out of Bound"));
          break;
        case FileMismatch:
          Serial.println(F("Cannot Find File"));
          break;
        case Advertise:
          Serial.println(F("In Advertise"));
          break;
        default:
          break;
      }
      break;
    default:
      break;
  }
}
