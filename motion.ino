/*
 * Simple motion sensor sketch.
 * Apr '19
 * Brent Williams (TK-81113)
 * Becauseinterwebs.com/tktalkie.com
 * 
 * This is a simple sketch that uses a PIR (Passive Infrared Resistor) 
 * motion sensor to trigger an .mp3 file to play user a DFPlayer mini 
 * mp3 player.
 * 
 * The DFPlayer has a built-in amplifier so you can hook speakers up 
 * directly to it, but in a loud, crowded room it may not be enough, 
 * so you can run the DAC channels out from the DFPlayer to an amplifier
 * circuit/board such as a PAM8403 to get much louder volumes.
 * 
 * This sketch is basically a combination of an Adafruit sample for the PIR and  
 * of the sample sketch provided by DFRobot and then slightly modified, 
 * so I'm not taking credit for it :)
 * 
 * https://www.dfrobot.com/wiki/index.php/DFPlayer_Mini_SKU:DFR0299?gclid=CjwKCAjw-ZvlBRBbEiwANw9UWk0FDywBGz_ngc-lFUCtihJJHgaZcOVHAfy_BC2OBnvwC9r4t_ifXRoCt0cQAvD_BwE
 */

#include "Arduino.h"
#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"

SoftwareSerial DFSerial(10, 11);            // RX, TX; Communicate with the DFPlayer
DFRobotDFPlayerMini DFPlayer;               // DFPlayer library
void checkState(uint8_t type, int value);   // Used to reset the play flag and output status messages

int ledPin = 13;                // choose the pin for the LED (use built-in by default)
int inputPin = 2;               // choose the input pin (for PIR sensor)
int val = 0;                    // variable for reading the pin status
int pirState = LOW;             // track state of PIR module
boolean canPlay = true;         // flag to let us know when the next mp3 can be played since the PIR module will output
                                // HIGH for several seconds, but we only want the first notice to trigger an mp3 playback.
byte  lastRnd;                  // holds last random number so that the same file is not played twice in a row

// SETTINGS YOU SHOULD CHANGE
byte  maxRnd = 20;              // Set this to the max number of mp3 files you want to be played
byte  volume = 30;              // Set this to the volume level you want for the DFPlayer.  This affects BOTH the 
                                // speaker out AND the DAC lines!  Values from 0 - 30.  You could also hook up a 
                                // potentiometer to control volume and modify the sketch to read the pot.

void setup() {
  pinMode(ledPin, OUTPUT);      // declare LED as output
  pinMode(inputPin, INPUT);     // declare sensor as input
  digitalWrite (inputPin, LOW);
  DFSerial.begin(9600);
  Serial.begin(9600);
  // Use softwareSerial to communicate with mp3.
  if (!DFPlayer.begin(DFSerial)) {  
    Serial.println(F("Unable to begin:"));
    Serial.println(F("1.Please recheck the connection!"));
    Serial.println(F("2.Please insert the SD card!"));
    while(true);
  }
  Serial.println(F("DFPlayer Mini online."));
  
  DFPlayer.volume(volume);  //Set volume value. From 0 to 30. This affects the DAC output as well!

  // short delay between commands...
  delay(30);
  
  // Here you can set a different EQ on the DFPlayer to one that sounds best with your speaker setup.
  // DFPlayer.EQ(DFPLAYER_EQ_NORMAL);
  // DFPlayer.EQ(DFPLAYER_EQ_POP);
  // DFPlayer.EQ(DFPLAYER_EQ_ROCK);
  // DFPlayer.EQ(DFPLAYER_EQ_JAZZ);
  // DFPlayer.EQ(DFPLAYER_EQ_CLASSIC);
  // DFPlayer.EQ(DFPLAYER_EQ_BASS);

  delay(30);
  
  canPlay = false;
  
  playTrack(1);     //Play the first mp3

  delay(30);
  
}

void loop(){

  val = digitalRead(inputPin);   // read input value from PIR

  switch (val) {
    case HIGH:
      {
        digitalWrite(ledPin, HIGH);  // turn LED ON
        if (pirState == LOW && canPlay == true) {
          // we have just turned on
          Serial.println("Motion detected!");
          canPlay = false;
          // Begin Random -----------------------------
          byte rnd = 0;
          byte count = 0;
          rnd = lastRnd;
          while (rnd == lastRnd && count < 50) { 
            rnd = random(1, maxRnd);
            count++;
          }
          lastRnd = rnd;
          playTrack(rnd);
          // End Random -------------------------------
          // Comment out the random section above and uncomment the line 
          // below to just loop through the sounds in sequential order.
          //DFPlayer.next();
          pirState = HIGH;
        }
      }
      break;
    case LOW:
      {
        if (pirState == HIGH){
          digitalWrite(ledPin, LOW); // turn LED OFF
          Serial.println("Motion ended!");
          pirState = LOW;
        }
      }
      break;
  }

  // Check state of DFPlayer 
  // The 'canPlay' flag is reset when the current mp3 ends so that the next one can be played when motion is detected.
  if (DFPlayer.available()) {
    checkState(DFPlayer.readType(), DFPlayer.read()); 
  }
  
}

// Play the specified track
void playTrack(uint8_t track) {
   DFPlayer.play(track);
   delay(30);
   int file = DFPlayer.readCurrentFileNumber();
   Serial.print("Track:");Serial.println(track);
   Serial.print("File:");Serial.println(file);
}

// Check the current state of DFPlayer
void checkState(uint8_t type, int value){
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
    case DFPlayerPlayFinished:
      Serial.print(F("Number:"));
      Serial.print(value);
      Serial.println(F(" Play Finished!"));
      canPlay = true;
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

