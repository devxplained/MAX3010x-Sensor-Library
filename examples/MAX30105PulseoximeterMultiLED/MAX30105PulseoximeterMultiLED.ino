#include <MAX3010x.h>

MAX30105::MultiLedConfiguration cfg = {
  MAX30105::SLOT_RED, 
  MAX30105::SLOT_IR, 
  MAX30105::SLOT_GREEN, 
  MAX30105::SLOT_PILOT_IR
};
MAX30105 sensor;

void setup() {
  Serial.begin(115200);

  if(sensor.begin()) { 
    sensor.setMultiLedConfiguration(cfg);
    sensor.setMode(MAX30105::MODE_MULTI_LED);
    Serial.println("Slot1,Slot2,Slot3,Slot4");
  }
  else {
    Serial.println("Sensor not found");  
    while(1);
  }  
}

void loop() {
  auto sample = sensor.readSample(1000);
  Serial.print(sample.slot[0]);
  Serial.print(",");
  Serial.print(sample.slot[1]);
  Serial.print(",");
  Serial.print(sample.slot[2]);
  Serial.print(",");
  Serial.print(sample.slot[3]);
  Serial.println();
}
