#include "Arduino.h"

void setup()
{
  // initialize serial communications
  Serial1.begin(115200);
}

void loop()
{
  Serial1.println("hello world");
  delay(1000);
}
