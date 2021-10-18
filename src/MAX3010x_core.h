/*!
 * @file MAX3010x_core.h
 */


#ifndef _MAX3010x_CORE_H
#define _MAX3010x_CORE_H

#include "Arduino.h"
#include "Wire.h"

template<class MAX3010xImpl, class MAX3010xSample> class MAX3010x {
protected:
  static const uint8_t MAX3010x_ADDR = 0x57;      //!< I2C Device Address
  static const uint8_t REV_ID_REG = 0xFF;         //!< Revision ID Register
  static const uint8_t PART_ID_REG = 0xFF;        //!< Part ID Register
  static const uint8_t MODE_MASK = 0x7;           //!< Mode Mask
  
  static const uint8_t FIFO_WR_PTR_REG = MAX3010xImpl::FIFO_BASE;         //!< FIFO Write Pointer Register
  static const uint8_t FIFO_OVF_CNT_REG = MAX3010xImpl::FIFO_BASE + 1;    //!< FIFO Overflow Counter Register
  static const uint8_t FIFO_RD_PTR_REG = MAX3010xImpl::FIFO_BASE + 2;     //!< FIFO Read Pointer Register
  static const uint8_t FIFO_DATA_REG = MAX3010xImpl::FIFO_BASE +3;        //!< FIFO Data Register
  
  /**
   * FIFO Registers
   */
  struct FIFORegisters {
    uint8_t write;    //!< Write Pointer
    uint8_t overflow; //!< Overflow Counter
    uint8_t read;     //!< Read Pointer
  };
    
  const uint8_t _addr; //!< I2C Device Address
  TwoWire& _wire;      //!< I2C Bus Implementation
  
  /**
   * Read Block
   * @param reg Register
   * @param count Number of bytes to read
   * @param buffer Buffer for values
   * @return true if successful, otherwise false
   */
  bool readBlock(uint8_t reg, uint8_t count, uint8_t* buffer) {
    _wire.beginTransmission(_addr);
    _wire.write(byte(reg));
    if(_wire.endTransmission(true)) return false; 
    
    if(_wire.requestFrom(_addr, count) != count) return false;
    if(_wire.available() != count) return false;
    
    for(int i = 0; i < count; i++) {
      buffer[i] = _wire.read();
    }
    
    return true;
  }
  
  /**
   * Read Byte
   * @param reg Register
   * @param value Reference to uint8_t variable to store the result in
   * @return true if successful, otherwise false
   */
  bool readByte(uint8_t reg, uint8_t& value) {
    return readBlock(reg, 1, &value);
  }
  
  /**
   * Write Block
   * @param reg Register
   * @param count Number of bytes to write
   * @param buffer Buffer with values
   * @return true if successful, otherwise false
   */
  bool writeBlock(uint8_t reg, uint8_t count, uint8_t* buffer) {
    _wire.beginTransmission(_addr);
    _wire.write(reg);
    for(int i = 0; i < count; i++) {
      _wire.write(buffer[i]);
    }
    
    return _wire.endTransmission(true) == 0;
  }
  
  /**
   * Write Byte
   * @param reg Register
   * @param value Value
   * @return true if successful, otherwise false
   */
  bool writeByte(uint8_t reg, uint8_t value) {    
    return writeBlock(reg, 1, &value);
  }
  
  
  /**
   * Read Bit
   * @param reg Register
   * @param bit Bit Index
   * @param value Reference to bool variable to store the result in
   * @return true if successful, otherwise false
   */
  bool readBit(uint8_t reg, uint8_t bit, bool& value) {
    uint8_t byte;
    
    if(!readByte(reg, byte)) return false;
    value = (byte >> bit) & 0x1;
    
    return true;
  }
  
  /**
   * Wait for Bit
   * @param reg Register
   * @param bit Bit Index
   * @param expectedState Expected State
   * @param timeout Timeout in ms
   * @return true if successful, otherwise false
   */
  bool waitBit(uint8_t reg, uint8_t bit, bool expectedState = true, int timeout = 100) {
    bool bitValue = !expectedState;
    unsigned long startTime = millis();
    while (bitValue != expectedState) {
      
      // Check for bit
      if(!readBit(reg, bit, bitValue)) {
        return false;
      }
      
      // Timeout
      if(millis() - startTime > timeout) {
        return false;
      }
      
      delay(1);
    }
    
    return true;
  }
  
  /**
   * Set Bit
   * @param reg Register
   * @param bit Bit Index
   * @param value Value
   * @return true if successful, otherwise false
   */
  bool setBit(uint8_t reg, uint8_t bit, bool value) {
    uint8_t byte;
    
    if(!readByte(reg, byte)) return false;
    
    byte &= ~(1<<bit);
    
    if(value) {
      byte |= 1<<bit;
    }
    
    return writeByte(reg, byte);
  }
  
  /**
   * Set Mode (internal)
   * @param mode Mode
   * @return true if successful, otherwise false
   */
  bool setModeInternal(uint8_t mode) {
    uint8_t value;
    
    if(mode & (~ MODE_MASK)) return false;
    if(!readByte(MAX3010xImpl::MODE_CFG_REG, value)) return false;
    
    value &= ~ MODE_MASK;
    value |= mode;
    
    if(!writeByte(MAX3010xImpl::MODE_CFG_REG, value)) return false;
    return clearFIFO();
  }
  
  /**
   * Constructor
   * Initializes a new sensor instance
   * 
   * @param addr Sensor Address
   * @param wire TWI bus instance
   */
  MAX3010x(uint8_t addr, TwoWire& wire) : _addr(addr), _wire(wire) {}
public:  
  /**
  * Initializes the I2C transport (Wire.begin()) and resets the sensor
  * @return true if successful, otherwise false
  */
  bool begin() {
    _wire.begin();
    return reset();
  }
    
  /**
  * Resets the sensor to its default settings
  * @return true if successful, otherwise false
  */
  bool reset() {
    
    // Reset
    if(!setBit(MAX3010xImpl::MODE_CFG_REG, MAX3010xImpl::MODE_RST_BIT, true)) return false;
    if(!waitBit(MAX3010xImpl::MODE_CFG_REG, MAX3010xImpl::MODE_RST_BIT, false)) return false;

    // Identify part
    if(readPartId() != MAX3010xImpl::MAX3010x_PART_ID) return false;
    
    // Enable Temperature Interrupt
    if(!enableInterrupt(MAX3010xImpl::INT_TEMP_RDY)) return false;
    
    // Default Config 
    if(!static_cast<MAX3010xImpl*>(this)->setDefaultConfiguration()) return false;
    
    return true;
  }
  
  /**
  * Enable Interrupt
  * @param interrupt Interrupt
  * @return true if successful, otherwise false
  */
  bool enableInterrupt(uint8_t interrupt) {
    if(interrupt >= MAX3010xImpl::INT_CNT) return false;
    if(MAX3010xImpl::INT_CFG_REG[interrupt] == 0xFF||MAX3010xImpl::INT_CFG_BIT[interrupt] >= 8) return false;
    return setBit(MAX3010xImpl::INT_CFG_REG[interrupt], MAX3010xImpl::INT_CFG_BIT[interrupt], true);
  }
  
  /**
  * Disable Interrupt
  * @param interrupt Interrupt
  * @return true if successful, otherwise false
  * @remarks If you disable the temperature interrupt the readTemperature() method will no longer work
  */
  bool disableInterrupt(uint8_t interrupt) {
    if(interrupt >= MAX3010xImpl::INT_CNT) return false;
    if(MAX3010xImpl::INT_CFG_REG[interrupt] == 0xFF||MAX3010xImpl::INT_CFG_BIT[interrupt] >= 8) return false;
    return setBit(MAX3010xImpl::INT_CFG_REG[interrupt], MAX3010xImpl::INT_CFG_BIT[interrupt], false);
  }
  
  /**
  * Check for Interrupt Flag
  * @param interrupt Interrupt
  * @return true if flag is set, otherwise false
  */
  bool checkInterruptFlag(uint8_t interrupt) {
    bool value;
    if(interrupt >= MAX3010xImpl::INT_CNT) return false;
    if(!readBit(MAX3010xImpl::INT_ST_REG[interrupt], MAX3010xImpl::INT_ST_BIT[interrupt], value)) return false;
    return value;
  }
  
  /**
  * Check for Interrupt Flag
  * @param interrupt Interrupt
  * @param timeout Timeout in ms
  * @return true if flag is set, otherwise false
  */
  bool waitForInterrupt(uint8_t interrupt, int timeout = 100) {
    if(interrupt >= MAX3010xImpl::INT_CNT) return false;
    return waitBit(MAX3010xImpl::INT_ST_REG[interrupt], MAX3010xImpl::INT_ST_BIT[interrupt], true, 100);
  }
  
  /**
  * Reads the Part Id
  * @return Part Id or 0xFF on failure
  */
  uint8_t readPartId() {
    uint8_t partId;
    if(!readByte(PART_ID_REG, partId)) return 0xFF;
    return partId;
  }
  
  /**
  * Reads the Revision Id
  * @return Revision Id or 0xFF on failure
  */
  uint8_t readRevisionId() {
    uint8_t revisionId;
    if(!readByte(REV_ID_REG, revisionId)) return 0xFF;
    return revisionId;
  }
  
  /**
  * Put the sensor in power down mode
  * @return true if successful, otherwise false
  */
  bool shutdown() {
    return setBit(MAX3010xImpl::MODE_CFG_REG, MAX3010xImpl::MODE_SHDN_BIT, true);
  }
  
  /**
  * Wake up the sensor from power down mode
  * @return true if successful, otherwise false
  */
  bool wakeUp() {
    return setBit(MAX3010xImpl::MODE_CFG_REG, MAX3010xImpl::MODE_SHDN_BIT, false);
  }
  
  /**
  * Reads the current sensor temperature
  * @remarks 
  * This function will trigger the temperature interrupt once the conversion is finished.
  * Triggering this interrupt is necessary for this method. If you use the interrupt pin
  * of the MAX3010x in your application please be aware of this fact.
  * @return Temperature in °C or NaN
  */
  float readTemperature() {
    uint8_t tInt;
    uint8_t tFrac;
    
    if(!setBit(MAX3010xImpl::TEMP_CONFIG_REG, MAX3010xImpl::TEMP_CONFIG_BIT, true)) return NAN;
    if(!waitForInterrupt(MAX3010xImpl::INT_TEMP_RDY)) return NAN;
    if(!readByte(MAX3010xImpl::TINT_REG, tInt) || !readByte(MAX3010xImpl::TFRAC_REG, tFrac)) return NAN;
    
    return tInt + 0.0625f * tFrac;
  }
  
  /**
  * Reads the number of available samples
  * @return Number of available samples or 0 on failure
  */
  uint8_t available() {
    FIFORegisters fifo;
    if(!readBlock(MAX3010xImpl::FIFO_BASE, sizeof(FIFORegisters), reinterpret_cast<uint8_t*>(&fifo))) return 0;
    
    if(fifo.read == fifo.write) {
      // FIFO is completely full
      if(fifo.overflow) {
        return MAX3010xImpl::FIFO_SIZE;
      }
    }
    return (16+fifo.write-fifo.read) % MAX3010xImpl::FIFO_SIZE;
  }
  
  /**
  * Reads the number lost samples due to FIFO overflow
  * @return Number of lost samples or 0xFF on failure
  */
  uint8_t readOverflowCounter() {
    uint8_t overflowCounter;
    if(!readByte(FIFO_OVF_CNT_REG, overflowCounter)) return 0xFF;
    return overflowCounter;
  }
  
  /**
  * Clears the FIFO
  * @return true if successful, otherwise false
  */
  bool clearFIFO() {
    if(!writeByte(MAX3010xImpl::FIFO_WR_PTR_REG, 0)) return false;
    if(!writeByte(MAX3010xImpl::FIFO_RD_PTR_REG, 0)) return false;
    if(!writeByte(MAX3010xImpl::FIFO_OVF_CNT_REG, 0)) return false;
    return true;
  }
  
  /**
  * Read a sample from the FIFO
  * @return Sample or invalid sample in case of an error
  */
  MAX3010xSample readSample(int timeout = 0) {
    unsigned long startTime = millis();
    MAX3010xSample sample = { 0 };

    FIFORegisters fifo;
       
    // Check if there is any data
    do {
      if(!readBlock(MAX3010xImpl::FIFO_BASE, sizeof(FIFORegisters), reinterpret_cast<uint8_t*>(&fifo))) return sample;
      
      if(fifo.overflow != 0) break;
      if(timeout > 0 && millis()-startTime >= timeout) return sample;
    } while(fifo.write == fifo.read);
    
    uint8_t data[MAX3010xImpl::SAMPLE_SIZE * MAX3010xImpl::MAX_ACTIVE_LEDS] = { 0 };
    if(!readBlock(MAX3010xImpl::FIFO_DATA_REG, MAX3010xImpl::SAMPLE_SIZE * static_cast<MAX3010xImpl*>(this)->nActiveSlots, data)) {
      // Restore read pointer in case of an error to allow a retry
      writeByte(MAX3010xImpl::FIFO_RD_PTR_REG, fifo.read);
      
      return sample;
    }
    
    static_cast<MAX3010xImpl*>(this)->fillSampleWithData(data, sample);
    
    return sample;
  }
};

#endif
