/*
 * Loki
 * (C) 2022 Blackbeard Softworks
 * 
 * This code relies on external hardware to be present.
 * Please refer to the wiring diagram.
 * 
 * The music must be prepared in this way:
 * - The input file must be MP3
 * - Left channel is used exclusively for motor control (meaning you can implement PWM at low frequency with amplitude modulation - from my testing it was <20Hz)
 * - Left channel must ONLY contain motor control signal (no voice, no music, only beeps at about 1024Hz~2048Hz)
 * - The beeps must not taper off in volume (it causes the motor to judder, so it needs to at least sound as if it turns on and off instantly)
 * - Use the DAW or other solutions to generate the beeps on the LEFT CHANNEL ONLY
 * - Leave the right channel alone or adjust its volume (you CANNOT adjust the volume in code. It won't work. Adding a resistor for smaller speakers does work. About 51 Ohms work well)
 * - Left channel MUST be at 0db attenuation (meaning, it peaks and/or clips. Crank it up!)
 */



#include <DFRobotDFPlayerMini.h>
#include <SoftwareSerial.h>

SoftwareSerial mySoftwareSerial(10, 11);
DFRobotDFPlayerMini myDFPlayer;
void printDetail(uint8_t type, int value);

int songIndex = 1;
// Adjust this to the amount of songs including the greeting (the first track must be a greeting, it automatically plays)
const int maxSongCount = 41;

const int button = 2;
const int statusPin = A0;
volatile bool stopped = false;
volatile bool state = false;

void setup() {
  pinMode(button, INPUT_PULLUP);
  pinMode(statusPin, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  mySoftwareSerial.begin(9600);
  Serial.begin(115200);
  Serial.println();
  Serial.println(F("Dancing Hat of Death starting up..."));
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
  // Set this to the length of the greeting track
  delay(7000);
  attachInterrupt(digitalPinToInterrupt(button), playbackCtrl, FALLING);
  Serial.println("Ready");
}

void loop() {
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

// The code below controls the audio playback, though sometimes it eats the button press.
void playbackCtrl() {
  static unsigned long lit = 0;
  unsigned long inttime = millis();
  if (inttime - lit > 1500) {
    digitalWrite(LED_BUILTIN, HIGH);
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
    digitalWrite(LED_BUILTIN, LOW);
    //myDFPlayer.stop();
  }
  lit = inttime;
}
