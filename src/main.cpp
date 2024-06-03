
#include <Arduino.h>
#include <SD.h>
#include <string>
#include <Time-master\TimeLib.h>

#define flag_LED 4

#define sensA A0
#define sensB A1
#define sensC A2

#define sd_CS 3
#define sd_SCK 8
#define sd_MOSI 10
#define sd_MISO 9

// SD Card vars
const int chipSelect = sd_CS;
File myFile;
File debugFile;
String fileName = "test3.csv";
String debugFileName = "DebugLog.txt";
String Error = "";

// Other Logic
int set[3];
unsigned long lastTime = 0;
unsigned int delayms = 30*60*1000;
String nextLine;
String currTime;
bool flag = 0;

// Declare Function
int* readSoilADC();

void setup() {
  Serial.begin(9600);

  // Three Moisture Sensors and flag LED
  pinMode(sensA, INPUT);
  pinMode(sensB, INPUT);
  pinMode(sensC, INPUT);
  pinMode(flag_LED, OUTPUT);
  digitalWrite(flag_LED, LOW);

  // Serial SD card connection
  //while (!Serial);
  Serial.print("Initializing SD card...");
  while (!SD.begin(chipSelect)){
    digitalWrite(flag_LED, HIGH);
    delay(100);
  }
  digitalWrite(flag_LED, LOW);
  Serial.println("initialization done.");

  // SD Setup
  myFile = SD.open(fileName, FILE_WRITE); // Main data file
  myFile.println("Time,A,B,C");
  myFile.close(); // close the file:
  setTime(21,52,0,2,6,2024);

  debugFile = SD.open(debugFileName, FILE_WRITE); // Debug Log
  debugFile.println("----------------[Debug Log]----------------");
  debugFile.close(); // close the file
}

void loop() {
  unsigned long nowms = millis();
  Serial.println(nowms);
  if (nowms - lastTime > delayms){
    // Top of the loop reset time and open file
    lastTime = nowms;
    time_t t = now();
    myFile = SD.open(fileName, FILE_WRITE);

    // Check sd conenction
    if (!myFile){
      flag = 1;
      Error += "File could not be opened.";
      if (!SD.begin(chipSelect)) {
        Error += "Re-initialization failed.";
      }
      else {flag = 0; Serial.println("Re-initialization success.");}
    }
    // Set up time
    String fill = "";
    currTime = fill + month(t) + "/" + day(t) + " " + hour(t) + ":" + minute(t) + ":" + second(t);

    // Set up number reports
    int* read = readSoilADC();
    nextLine = currTime + "," + read[0] + "," + read[1] + "," + read[2];
    if (read[0] < 200 || read[0] > 800){
      Error += fill+"Odd measurement from sensor A of " + read[0] + ".";
      flag = 1;
    }
    if (read[1] < 200 || read[1] > 800){
      Error += fill+"Odd measurement from sensor B of " + read[1] + ".";
      flag = 1;
    }
    if (read[2] < 200 || read[2] > 800){
      Error += fill+"Odd measurement from sensor C of " + read[2] + ".";
      flag = 1;
    }
    

    // Print in SD and Serial monitor
    myFile.println(nextLine);
    Serial.println(nextLine); // Show what was written
    myFile.close(); // close the file
    digitalWrite(flag_LED, flag); // Display flag LED
    if (flag){
      Serial.print(Error);
      Serial.println(flag);

      debugFile = SD.open(debugFileName, FILE_WRITE); // Open Debug Log
      nextLine = currTime + " | " + Error;
      debugFile.println(nextLine);  // Print errors at certain time
      debugFile.close(); // close the file

      // Reset 
      Error = "";
      flag = 0;
    }
  }

}

int* readSoilADC()
{
  set[0] = analogRead(sensA); // Read analog value from sensor
  set[1] = analogRead(sensB); // Read analog value from sensor
  set[2] = analogRead(sensC); // Read analog value from sensor
  return set;                    // Return analog moisture value
}



