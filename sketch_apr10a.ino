#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include "RTClib.h"
#include <ArduCAM.h>

RTC_PCF8523 rtc;
ArduCAM myCAM(OV2640, 12);

// ---------- PIN SETUP ----------
const int buttonPin = 5;
const int button2Pin = 6;
const int button3Pin = 9;
const int button4Pin = 10;

const int chipSelect = 4;
const int ledPin = 13;
const int camCSPin = 12;

// ---------- STATE VARIABLES ----------
bool lastButtonState = HIGH;
bool lastButton2State = HIGH;
bool lastButton3State = HIGH;
bool lastButton4State = HIGH;

// sound timer
bool soundPlaying = false;
unsigned long soundStartTime = 0;
const unsigned long soundDuration = 5000;


// ---------- PRINT SD FILE ----------
void printFile()
{
  File file = SD.open("log.txt");

  if (file)
  {
    Serial.println("=== File Contents ===");

    while (file.available())
    {
      String line = file.readStringUntil('\n');
      Serial.println(line);
    }

    file.close();
    Serial.println("=== End of File ===\n");
  }
  else
  {
    Serial.println("Error opening file.");
  }
}


// ---------- GET TIME ----------
String getTimeString()
{
  DateTime now = rtc.now();

  String t = "";

  t += String(now.year());
  t += "/";

  if (now.month() < 10) t += "0";
  t += String(now.month());
  t += "/";

  if (now.day() < 10) t += "0";
  t += String(now.day());
  t += " ";

  if (now.hour() < 10) t += "0";
  t += String(now.hour());
  t += ":";

  if (now.minute() < 10) t += "0";
  t += String(now.minute());
  t += ":";

  if (now.second() < 10) t += "0";
  t += String(now.second());

  return t;
}


// ---------- STOP SOUND ----------
void stopSound()
{
  Serial1.print('q');
}


// ---------- TAKE PICTURE ----------
void takePicture()
{
  Serial.println("Taking picture...");

  myCAM.flush_fifo();
  myCAM.clear_fifo_flag();
  myCAM.start_capture();

  while (!myCAM.get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK))
  {
    ;
  }

  uint32_t length = myCAM.read_fifo_length();

  Serial.print("Image size: ");
  Serial.println(length);

  if (length == 0)
  {
    Serial.println("Capture failed.");
    return;
  }

  String fileName = "";
  int fileNumber = 0;

  while (true)
  {
    fileName = "IMG" + String(fileNumber) + ".JPG";
    if (!SD.exists(fileName.c_str())) break;
    fileNumber++;
  }

  File imgFile = SD.open(fileName.c_str(), FILE_WRITE);

  if (!imgFile)
  {
    Serial.println("File error.");
    return;
  }

  digitalWrite(chipSelect, HIGH);
  myCAM.CS_LOW();
  myCAM.set_fifo_burst();

  for (uint32_t i = 0; i < length; i++)
  {
    byte b = SPI.transfer(0x00);
    imgFile.write(b);
  }

  myCAM.CS_HIGH();
  imgFile.close();

  Serial.print("Saved: ");
  Serial.println(fileName);
}


// ---------- SETUP ----------
void setup()
{
  Serial.begin(115200);
  Serial1.begin(9600);
  Wire.begin();
  SPI.begin();

  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(button2Pin, INPUT_PULLUP);
  pinMode(button3Pin, INPUT_PULLUP);
  pinMode(button4Pin, INPUT_PULLUP);

  pinMode(ledPin, OUTPUT);
  pinMode(chipSelect, OUTPUT);
  pinMode(camCSPin, OUTPUT);

  digitalWrite(chipSelect, HIGH);
  digitalWrite(camCSPin, HIGH);

  if (!rtc.begin())
  {
    Serial.println("RTC fail");
    while (1);
  }

  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  if (!SD.begin(chipSelect))
  {
    Serial.println("SD fail");
    while (1);
  }

  myCAM.set_format(JPEG);
  myCAM.InitCAM();
  myCAM.OV2640_set_JPEG_size(OV2640_320x240);

  Serial.println("System ready.\n");
}


// ---------- LOOP ----------
void loop()
{
  // ---------- MAIN BUTTON (Track 2 + camera + log) ----------
  bool currentButtonState = digitalRead(buttonPin);

  if ((lastButtonState == HIGH) && (currentButtonState == LOW))
  {
    delay(50);
    if (digitalRead(buttonPin) == LOW)
    {
      digitalWrite(ledPin, HIGH);

      Serial.println("Playing track 2...");
      Serial1.print("#2\n");

      soundPlaying = true;
      soundStartTime = millis();

      String timestamp = getTimeString();

      Serial.print("Pressed at: ");
      Serial.println(timestamp);

      File file = SD.open("log.txt", FILE_WRITE);

      if (file)
      {
        file.print("Pressed at: ");
        file.println(timestamp);
        file.close();
        Serial.println("Saved to SD.");
      }
      else
      {
        Serial.println("Error writing to file.");
      }

      takePicture();
      printFile();

      delay(300);
      digitalWrite(ledPin, LOW);
    }
  }
  lastButtonState = currentButtonState;


  // ---------- BUTTON 2 (Track 3) ----------
  bool currentButton2State = digitalRead(button2Pin);
  if ((lastButton2State == HIGH) && (currentButton2State == LOW))
  {
    delay(50);
    if (digitalRead(button2Pin) == LOW)
    {
      Serial.println("Playing track 3...");
      Serial1.print("#3\n");
    }
  }
  lastButton2State = currentButton2State;


  // ---------- BUTTON 3 (Track 4) ----------
  bool currentButton3State = digitalRead(button3Pin);
  if ((lastButton3State == HIGH) && (currentButton3State == LOW))
  {
    delay(50);
    if (digitalRead(button3Pin) == LOW)
    {
      Serial.println("Playing track 4...");
      Serial1.print("#4\n");
    }
  }
  lastButton3State = currentButton3State;


  // ---------- BUTTON 4 (Track 5) ----------
  bool currentButton4State = digitalRead(button4Pin);
  if ((lastButton4State == HIGH) && (currentButton4State == LOW))
  {
    delay(50);
    if (digitalRead(button4Pin) == LOW)
    {
      Serial.println("Playing track 5...");
      Serial1.print("#5\n");
    }
  }
  lastButton4State = currentButton4State;


  // ---------- AUTO STOP SOUND ----------
  if (soundPlaying)
  {
    if (millis() - soundStartTime >= soundDuration)
    {
      stopSound();
      Serial.println("Sound stopped (5 sec limit)");
      soundPlaying = false;
    }
  }
}