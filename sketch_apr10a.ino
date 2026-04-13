#include <SPI.h>
#include <Wire.h>
#include <ArduCAM.h>
#include <SD.h>
#include "memorysaver.h"

#define CS_CAM 10
#define CS_SD 4
#define BUTTON_PIN 6

ArduCAM myCAM(OV2640, CS_CAM);

File imgFile;
int imageCounter = 0;

void setup() {
  Serial.begin(115200);
  Wire.begin();
  SPI.begin();

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(CS_SD, OUTPUT);
  pinMode(CS_CAM, OUTPUT);

  digitalWrite(CS_SD, HIGH);
  digitalWrite(CS_CAM, HIGH);

  // SD INIT
  if (!SD.begin(CS_SD)) {
    Serial.println("SD failed");
    while (1);
  }
  Serial.println("SD OK");

  // CAMERA INIT
  myCAM.set_format(JPEG);
  myCAM.InitCAM();
  myCAM.OV2640_set_JPEG_size(OV2640_320x240);

  // VERIFY CAMERA
  uint8_t vid, pid;
  myCAM.wrSensorReg8_8(0xff, 0x01);
  myCAM.rdSensorReg8_8(OV2640_CHIPID_HIGH, &vid);
  myCAM.rdSensorReg8_8(OV2640_CHIPID_LOW, &pid);

  Serial.print("VID: "); Serial.println(vid, HEX);
  Serial.print("PID: "); Serial.println(pid, HEX);

  if (vid != 0x26 || pid != 0x42) {
    Serial.println("Camera not detected correctly!");
    while (1);
  }

  Serial.println("Camera ready");
}

void loop() {
  if (digitalRead(BUTTON_PIN) == LOW) {

    Serial.println("Taking picture...");

    // Clear FIFO
    myCAM.flush_fifo();
    myCAM.clear_fifo_flag();

    delay(100);

    // Start capture
    myCAM.start_capture();

    // Wait for capture
    while (!myCAM.get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK));

    Serial.println("Capture done");

    uint32_t length = myCAM.read_fifo_length();

    Serial.print("Size: ");
    Serial.println(length);

    // Validate length
    if (length == 0 || length >= 0x7FFFFF) {
      Serial.println("Invalid length");
      return;
    }

    // Create unique filename
    char filename[15];
    sprintf(filename, "IMG_%d.JPG", imageCounter++);

    imgFile = SD.open(filename, FILE_WRITE);

    if (!imgFile) {
      Serial.println("File open failed");
      return;
    }

    Serial.print("Writing ");
    Serial.println(filename);

    myCAM.CS_LOW();
    myCAM.set_fifo_burst();

    byte temp = 0, temp_last = 0;
    bool is_header = false;

    while (length--) {
      temp_last = temp;
      temp = SPI.transfer(0x00);

      // Detect JPEG start (FFD8)
      if (!is_header) {
        if (temp_last == 0xFF && temp == 0xD8) {
          is_header = true;
          imgFile.write(temp_last);
          imgFile.write(temp);
        }
      } 
      else {
        imgFile.write(temp);

        // Detect JPEG end (FFD9)
        if (temp_last == 0xFF && temp == 0xD9) {
          Serial.println("Image complete");
          break;
        }
      }
    }

    myCAM.CS_HIGH();
    imgFile.close();

    Serial.println("Saved to SD!");

    delay(2000); // debounce
  }
}