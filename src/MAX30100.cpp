/*!
 * @file MAX30100.cpp
 */

#include "MAX30100.h"

const uint8_t MAX30100::INT_CFG_REG[] = {
  0x01,                                     //!< FIFO Almost Full Interrupt Enable Register
  0x01,                                     //!< Temperature Ready Interrupt Enable Register
  0x01,                                     //!< HR Ready Interrupt Enable Register    
  0x01,                                     //!< SPO2 Ready Interrupt Enable Register
  0xFF                                      //!< Power Ready Status Interrupt Enable Register (not existing)
};

const uint8_t MAX30100::INT_CFG_BIT[] = {
  7,                                        //!< FIFO Almost Full Interrupt Enable Bit
  6,                                        //!< Temperature Ready Interrupt Enable Bit
  5,                                        //!< HR Ready Interrupt Enable Bit    
  4,                                        //!< SPO2 Ready Interrupt Enable Bit
  255                                       //!< Power Ready Interrupt Enable Bit (not existing)
};

const uint8_t MAX30100::INT_ST_REG[] = {
  0x00,                                     //!< FIFO Almost Full Status Register
  0x00,                                     //!< Temperature Ready Status Register
  0x00,                                     //!< HR Ready Status Register    
  0x00,                                     //!< SPO2 Ready Status Register
  0x00                                      //!< Power Ready Status Register
};

const uint8_t MAX30100::INT_ST_BIT[] = {
  7,                                        //!< FIFO Almost Full Status Bit
  6,                                        //!< Temperature Ready Status Bit
  5,                                        //!< HR Ready Status Bit    
  4,                                        //!< SPO2 Ready Status Bit
  0                                         //!< Power Ready Status Bit
};
  
/**
 * Constructor
 * Initializes a new sensor instance
 * 
 * @param addr Sensor Address (default 0x57)
 * @param wire TWI bus instance (default Wire)
 */
MAX30100::MAX30100(uint8_t addr, TwoWire& wire) : MAX3010x(addr, wire) {
  
}

/**
 * Set Measuring Mode and reset FIFO
 * @param mode Mode
 * @return true if successful, otherwise false
 */
bool MAX30100::setMode(MAX30100::Mode mode) {
  return setModeInternal(static_cast<uint8_t>(mode));
}
  
/**
 * Set Default Configuration
 * @returns true if successful, otherwise false
 */
bool MAX30100::setDefaultConfiguration() {
  if(!setLedCurrent(LED_RED, LED_CURRENT_14MA2)) return false;
  if(!setLedCurrent(LED_IR, LED_CURRENT_20MA8)) return false;
  if(!setResolution(RESOLUTION_16BIT_1600US)) return false;
  if(!setSamplingRate(SAMPLING_RATE_50SPS)) return false;
  if(!setMode(MODE_SPO2)) return false;
  
  return true;
}

/**
 * Fill sample with data
 * @param data Raw Data
 * @param sample Sample to fill
 */
void MAX30100::fillSampleWithData(uint8_t data[MAX30100::SAMPLE_SIZE*MAX30100::MAX_ACTIVE_LEDS], MAX30100Sample& sample) {
  sample.valid = true;
  sample.ir = (static_cast<uint16_t>(data[0]) << 8) | static_cast<uint16_t>(data[1]);
  sample.red = (static_cast<uint16_t>(data[2]) << 8) | static_cast<uint16_t>(data[3]);
}

/**
 * Set LED Current
 * @param led LED
 * @param current Current
 * @returns true if successful, otherwise false
 */
bool MAX30100::setLedCurrent(MAX30100::Led led, MAX30100::LedCurrent current) {
  uint8_t cfg;
  
  if(!readByte(LED_CFG_REG, cfg)) return false;
  
  cfg &= ~(0xf << (4*static_cast<uint8_t>(led)));
  cfg |= static_cast<uint8_t>(current) << (4*static_cast<uint8_t>(led));
  
  return writeByte(LED_CFG_REG, cfg);
}

/**
 * Set Sampling Rate
 * @param rate Sampling Rate
 * @returns true if successful, otherwise false
 */
bool MAX30100::setSamplingRate(MAX30100::SamplingRate rate) {
  uint8_t cfg;
  
  if(!readByte(SPO2_CFG_REG, cfg)) return false;
  
  cfg &= ~(0x7 << 2);
  cfg |= static_cast<uint8_t>(rate) << 2;
  
  return writeByte(SPO2_CFG_REG, cfg);
}

/**
 * Set Resolution
 * @param resolution Resolution and pulse width
 * @returns true if successful, otherwise false
 */
bool MAX30100::setResolution(MAX30100::Resolution resolution) {
  uint8_t cfg;
  
  if(!readByte(SPO2_CFG_REG, cfg)) return false;
  
  cfg &= ~0x3;
  cfg |= static_cast<uint8_t>(resolution);
  
  return writeByte(SPO2_CFG_REG, cfg);
}
