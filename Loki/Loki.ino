/*
 * Loki (Revision 1445)
 * (C) 2022 Blackbeard Softworks
 * 
 * This code relies on external hardware to be present.
 * Please refer to the wiring diagram.
 * 
 * The music must be prepared in this way:
 * - The input file must be MP3
 * - Right channel is used exclusively for motor control (meaning you can implement PWM at low frequency with amplitude modulation - from my testing it was <20Hz)
 * - Right channel must ONLY contain motor control signal (no voice, no music, only beeps at about 1024Hz~2048Hz. DC signal doesn't work as capacitor blocks DC signal)
 * - The beeps must not taper off in volume (it causes the motor to judder, so it needs to at least sound as if it turns on and off instantly)
 * - Use the DAW or other solutions to generate the beeps on the RIGHT CHANNEL ONLY
 * - Leave the left channel alone or adjust its volume 
 * (you CANNOT adjust the volume in code. It won't work. Adding a resistor for smaller speakers does work. About 51 Ohms work well, but taping over the speaker hole works too.)
 * - Right channel MUST be at 0db attenuation (meaning, it peaks and/or clips. Crank it up!)
 */

// DFRobot's own library is used here. We could use lightweight library, but let's keep it simple.
#include <DFRobotDFPlayerMini.h>
#include <SoftwareSerial.h>

SoftwareSerial mySoftwareSerial(10, 11);
DFRobotDFPlayerMini myDFPlayer;
void printDetail(uint8_t type, int value);

// Do not change this value.
int songIndex = 1;

// This is the number of songs on your SD card, greeting inclusive.
// Supports up to 65535, but honestly with one button it would be a VERY big chore to go through all 65533 tracks for accessing the last track.
// Can be LESS than the actual song count, but CANNOT BE MORE than the actual song count. This causes the function to completely fail.
const int maxSongCount = 41;

// This will be the playback button
const int button = 2;
// The statusPin is used only for legacy purposes.
const int statusPin = A0;

// Do not change these values.
volatile bool stopped = false;
volatile bool state = false;
volatile bool led = false;

// This is the pin that reverses the motor when combined with the Logic Reverser circuit.
// If you want to use other pins, change this.
const int ledPin = LED_BUILTIN;

// Do not change this value.
unsigned long pm = 0;

// Change this to change how frequently the motor changes the direction
// Changing this to too small of a value may cause the motor, back EMF or power supply issues.
const long interval = 1000;

void setup() {
  // Setup pin modes
  pinMode(button, INPUT_PULLUP);
  pinMode(statusPin, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  mySoftwareSerial.begin(9600);
  Serial.begin(115200);
  Serial.println();
  Serial.println(F("Loki v1.4 starting up..."));
  // Delay to let DFPlayer Mini initialize
  delay(2000);
  if (!myDFPlayer.begin(mySoftwareSerial)) {
    Serial.println(F("Unable to begin:"));
    Serial.println(F("1.Please recheck the connection!"));
    Serial.println(F("2.Please insert the SD card!"));
    while (true) {
      delay(0); // Code to compatible with ESP8266 watchdog.
    }
  }
  Serial.println(F("DFPlayer Mini online."));
  // This value can be adjusted to let the SD card catch up, not resulting in Time Out error
  myDFPlayer.setTimeOut(1500); 
  
  // Must be at 30. Lower volumes just don't have the gusto to push through. 
  // Technically you can add another transistor stage or Darlington transistor for this, but this is the cheapest option.
  myDFPlayer.volume(30);  
  // Play the first MP3 file, which must be the greeting audio
  myDFPlayer.playMp3Folder(1);  
  // Small delay to prevent malfunctions
  delay(250);
  // Legacy code start
  while (digitalRead(statusPin) == LOW) {
    // Wait while the track finishes playing
  }
  // Legacy code end
  // Set register
  EIFR = 1;
  delay(7000); // Adjust for the prompt length
  // Attach an interrupt routine to the button
  attachInterrupt(digitalPinToInterrupt(button), playbackCtrl, FALLING);
  Serial.println("Ready");
}

void loop() {
  // This part until next comment deals with the reversing of the motor.
  unsigned long cm = millis();
  if (cm - pm >= interval) {
    pm = cm;
    led = !led;
  }
  digitalWrite(ledPin, led);
  // End of section

  
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
    printDetail(myDFPlayer.readType(), myDFPlayer.read());
  }
}

// Interrupt routine
void playbackCtrl() {
  static unsigned long lit = 0;
  // We do not want to use delay() here.
  unsigned long inttime = millis();
  // Debouncing routine, as well as toggle
  if (inttime - lit > 500) {
    if (state == true) {
      state = false;
      stopped = true;
    }
    if (state == false) {
      state = true;
      stopped = true;
    }
    // Advance the song index by one
    songIndex++;
    // If we are exceeding the maximum song index or if it's below 2 (meaning, the beginning)
    // We want to set it to 2 so the greeting message doesn't play again.
    if (songIndex > maxSongCount || songIndex < 2) {
      songIndex = 2;
    }
  }
  lit = inttime;
}

// Default DFPlayer debug function
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
