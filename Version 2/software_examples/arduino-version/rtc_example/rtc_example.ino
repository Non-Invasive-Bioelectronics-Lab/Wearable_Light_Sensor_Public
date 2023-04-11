#include "Time.h"
#include "TimeLib.h"

void setup() {
  // put your setup code here, to run once:
  setTime(12, 20, 2, 4, 3, 2021);
  Serial.begin(115200);

}

void loop() {
  // put your main code here, to run repeatedly:
  uint8_t timeArray[6];
  timeArray[0] = (char)(year() - 2000);
  timeArray[1] = (char)month();
  timeArray[2] = (char)day();
  timeArray[3] = (char)hour();
  timeArray[4] = (char)minute();
  timeArray[5] = (char)second();
  char intHour = (char)hour();
  Serial.print("Time is");
  Serial.print(hour());
  Serial.print(":");
  Serial.print(minute());
  Serial.print(":");
  Serial.println(second());
  delay(1000);

}
