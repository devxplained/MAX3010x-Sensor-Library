#include <MAX3010x.h>

MAX30100 sensor;

void setup() {
  Serial.begin(115200);

  if(sensor.begin()) { 
    Serial.println("IR,Red");
  }
  else {
    Serial.println("Sensor not found");  
    while(1);
  }  
}

void loop() {
  auto sample = sensor.readSample(1000);
  Serial.print(sample.ir);
  Serial.print(",");
  Serial.println(sample.red);
}
