#include <SPI.h>
#include <Wire.h>
#include <ArduCAM.h>
#include <SD.h>

#define CS_CAM 10
#define CS_SD 4
#define BUTTON_PIN 6

ArduCAM myCAM(OV2640, CS_CAM);

File imgFile;

void setup() {
  Serial.begin(115200);
  Wire.begin();
  SPI.begin();

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(CS_SD, OUTPUT);
  pinMode(CS_CAM, OUTPUT);

  digitalWrite(CS_SD, HIGH);
  digitalWrite(CS_CAM, HIGH);

  // Init SD
  if (!SD.begin(CS_SD)) {
    Serial.println("SD failed");
    while (1);
  }

  Serial.println("SD OK");

  // Init camera
  myCAM.set_format(JPEG);
  myCAM.InitCAM();
  myCAM.OV2640_set_JPEG_size(OV2640_320x240);

  Serial.println("Camera ready");
}

void loop() {
  if (digitalRead(BUTTON_PIN) == LOW) {

    Serial.println("Taking picture...");

    myCAM.flush_fifo();
    myCAM.clear_fifo_flag();
    myCAM.start_capture();

    while (!myCAM.get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK));

    uint32_t length = myCAM.read_fifo_length();
    Serial.print("Size: ");
    Serial.println(length);

    imgFile = SD.open("image.jpg", FILE_WRITE);

    myCAM.CS_LOW();
    myCAM.set_fifo_burst();

    for (uint32_t i = 0; i < length; i++) {
      byte b = SPI.transfer(0x00);
      imgFile.write(b);
    }

    myCAM.CS_HIGH();

    imgFile.close();

    Serial.println("Saved to SD!");

    delay(2000); // debounce
  }
}