/*!
 * @file MAX3010x_multiLed_core.h
 */


#ifndef _MAX3010x_MULTILED_CORE_H
#define _MAX3010x_MULTILED_CORE_H

#include "MAX3010x_core.h"

template<class MAX3010xImpl, class MAX3010xSample> class MAX3010xMultiLed : public MAX3010x<MAX3010xImpl, MAX3010xSample> {
  friend class MAX3010x<MAX3010xImpl, MAX3010xSample>; //!< Friend declaration for access to private members from base class
public:
  /**
   * Mode
   */
  enum Mode {
    MODE_HR_ONLY = 0b010,     //!< Heart Rate Only Mode (Red LED)
    MODE_RED_ONLY = 0b010,    //!< Red LED Only Mode (Red LED)
    MODE_SPO2 = 0b011,        //!< SPO2 Mode (Red + IR LED)
    MODE_RED_IR = 0b011,      //!< Red + IR LED Mode (Red + IR LED)
    MODE_MULTI_LED = 0b111    //!< Multi LED Mode (LEDs configured in multi LED configuration)
  };
protected:
  static const uint8_t MAX3010x_PART_ID = 0x15;   //!< Expected Part ID
  
  static const uint8_t TEMP_CONFIG_REG = 0x21;    //!< Temperature Trigger Register
  static const uint8_t TEMP_CONFIG_BIT = 0;       //!< Temperature Trigger Bit
  
  static const uint8_t TINT_REG = 0x1F;           //!< Temperature Register
  static const uint8_t TFRAC_REG = 0x20;          //!< Fractional Temperature Component Register
    
  static const uint8_t FIFO_BASE = 0x4;           //!< FIFO Base Address
  static const uint8_t SAMPLE_SIZE = 3;           //!< Sample Size
  
  static const uint8_t FIFO_CFG_REG = 0x8;        //!< FIFO Configuration Register
  static const uint8_t FIFO_SMP_AVE_BIT = 5;      //!< Sample Averaging Bit Position
  static const uint8_t FIFO_SMP_AVE_MASK = 0x7;   //!< Sample Averaging Bit Mask
  static const uint8_t FIFO_ROLLOVER_EN_BIT = 4;  //!< FIFO Rollover Bit
  static const uint8_t FIFO_A_FULL_BIT = 0;       //!< FIFO Almost Full Threshold Bit Position
  static const uint8_t FIFO_A_FULL_MASK = 0xF;    //!< FIFO Almost Full Threshold Bit Mask
  
  static const uint8_t MODE_CFG_REG = 0x9;        //!< Mode Configuration Register
  static const uint8_t MODE_SHDN_BIT = 7;         //!< Shutdown Bit
  static const uint8_t MODE_RST_BIT = 6;          //!< Reset Bit
  
  static const uint8_t SPO2_CFG_REG = 0xA;              //!< SpO2 Measurement Configuration Register
  static const uint8_t SPO2_CFG_RESOLUTION_BIT = 0;     //!< Resolution Bit Position
  static const uint8_t SPO2_CFG_RESOLUTION_MASK = 0x3;  //!< Resolution Bit Mask
  static const uint8_t SPO2_CFG_SMP_RATE_BIT = 2;       //!< Sampling Rate Bit Position
  static const uint8_t SPO2_CFG_SMP_RATE_MASK = 0x7;    //!< Sampling Rate Bit Mask
  static const uint8_t SPO2_CFG_ADC_RANGE_BIT = 5;      //!< ADC Range Bit Position
  static const uint8_t SPO2_CFG_ADC_RANGE_MASK = 0x3;   //!< ADC Range Bit Mask
  
  static const uint8_t LED_CFG_REG_BASE = 0xC;          //!< LED Power Configuration Register Base
  static const uint8_t MULTI_LED_CFG_REG_BASE = 0x11;   //!< LED Power Configuration Register Base

  Mode currentMode;                                     //!< Current Mode
  uint8_t nActiveSlots;                                 //!< Number of active LED Slots in FIFO data
  uint8_t nConfiguredSlots;                             //!< Number of configured LED Slots
  
  /**
   * Constructor
   * Initializes a new sensor instance
   * 
   * @param addr Sensor Address
   * @param wire TWI bus instance
   */
  MAX3010xMultiLed(uint8_t addr, TwoWire& wire) : MAX3010x<MAX3010xImpl, MAX3010xSample>(addr, wire) {}
  
  /**
   * Common function for setting the Multi LED Configuration
   * @param activeSlots Number of active slots
   * @param cfg Configuration bytes
   * @return true if successful, otherwise false
   */
  bool setMultiLedConfigurationInternal(uint8_t activeSlots, uint8_t cfg[2]) {
    uint8_t value;
    
    if(!MAX3010x<MAX3010xImpl, MAX3010xSample>::writeBlock(MAX3010xImpl::MULTI_LED_CFG_REG_BASE, sizeof(cfg), cfg)) return false;
    
    nConfiguredSlots = activeSlots;
    if(currentMode == MODE_MULTI_LED) nActiveSlots = nConfiguredSlots;
    
    return MAX3010x<MAX3010xImpl, MAX3010xSample>::clearFIFO();
  }
  
  /**
   * Fill sample with data
   * @param data Raw Data
   * @param sample Sample to fill
   */
  void fillSampleWithData(uint8_t data[MAX3010xImpl::MAX_ACTIVE_LEDS], MAX3010xSample& sample) {
    sample.valid = true;
    
    for(int i = 0; i < static_cast<MAX3010xImpl*>(this)->nActiveSlots; i++) {
      sample.slot[i] = (static_cast<uint32_t>(data[0 + SAMPLE_SIZE*i]) << 16) | (static_cast<uint32_t>(data[1 + SAMPLE_SIZE*i]) << 8) | static_cast<uint32_t>(data[2 + SAMPLE_SIZE*i]);
    }
  }
public:  
  /**
   * Set Measuring Mode and reset FIFO
   * @param mode Mode
   * @return true if successful, otherwise false
   */
  bool setMode(Mode mode) {
    uint8_t activeSlots;
    if(mode == MODE_HR_ONLY) activeSlots = 1;
    else if(mode == MODE_SPO2) activeSlots = 2;
    else if(mode == MODE_MULTI_LED) activeSlots = nConfiguredSlots;
    else return false;
    
    if(!MAX3010x<MAX3010xImpl, MAX3010xSample>::setModeInternal(static_cast<uint8_t>(mode))) return false;
    
    currentMode = mode;
    nActiveSlots = activeSlots;
    
    return true;
  }
  
  /**
   * Sampling Rate
   */
  enum SamplingRate {
    SAMPLING_RATE_50SPS,    //!< 50 samples per second
    SAMPLING_RATE_100SPS,   //!< 100 samples per second
    SAMPLING_RATE_200SPS,   //!< 200 samples per second
    SAMPLING_RATE_400SPS,   //!< 400 samples per second
    SAMPLING_RATE_800SPS,   //!< 800 samples per second
    SAMPLING_RATE_1000SPS,  //!< 1000 samples per second
    SAMPLING_RATE_1600SPS,  //!< 1600 samples per second
    SAMPLING_RATE_3200SPS   //!< 3200 samples per second
  };
  
  /**
   * Set Sampling Rate
   * @param rate Sampling Rate
   * @returns true if successful, otherwise false
   */
  bool setSamplingRate(SamplingRate rate) {
    uint8_t cfg;
    
    if(rate & (~ SPO2_CFG_SMP_RATE_MASK)) return false;
    if(!MAX3010x<MAX3010xImpl, MAX3010xSample>::readByte(SPO2_CFG_REG, cfg)) return false;
    
    cfg &= ~(SPO2_CFG_SMP_RATE_MASK << SPO2_CFG_SMP_RATE_BIT);
    cfg |= (static_cast<uint8_t>(rate) & SPO2_CFG_SMP_RATE_MASK) << SPO2_CFG_SMP_RATE_BIT;
    
    return MAX3010x<MAX3010xImpl, MAX3010xSample>::writeByte(SPO2_CFG_REG, cfg);
  }
  
  /**
   * Sampling Rate
   */
  enum ADCRange {
    ADC_RANGE_2048NA,   //!< 2048nA Range
    ADC_RANGE_4096NA,   //!< 4096nA Range
    ADC_RANGE_8192NA,   //!< 8192nA Range
    ADC_RANGE_16384NA   //!< 16384nA Range
  };
  
  /**
   * Set ADC Range
   * @param range ADC range
   * @returns true if successful, otherwise false
   */
  bool setADCRange(ADCRange range) {
    uint8_t cfg;
    
    if(range & (~ SPO2_CFG_ADC_RANGE_MASK)) return false;
    if(!MAX3010x<MAX3010xImpl, MAX3010xSample>::readByte(SPO2_CFG_REG, cfg)) return false;
    
    cfg &= ~(SPO2_CFG_ADC_RANGE_MASK << SPO2_CFG_ADC_RANGE_BIT);
    cfg |= (static_cast<uint8_t>(range) & SPO2_CFG_ADC_RANGE_MASK) << SPO2_CFG_ADC_RANGE_BIT;
    
    return MAX3010x<MAX3010xImpl, MAX3010xSample>::writeByte(SPO2_CFG_REG, cfg);
  }
  
  /**
   * Measuring resolution and pulse width
   */
  enum Resolution {
    RESOLUTION_15BIT_69US,     //!< 15 bit resolution, 69 us pulse width
    RESOLUTION_16BIT_118US,    //!< 16 bit resolution, 118 us pulse width
    RESOLUTION_17BIT_215US,    //!< 17 bit resolution, 215 us pulse width
    RESOLUTION_18BIT_4110US    //!< 18 bit resolution, 4110 us pulse width
  };
  
  /**
   * Set Resolution
   * @param resolution Resolution and pulse width
   * @returns true if successful, otherwise false
   */
  bool setResolution(Resolution resolution) {
    uint8_t cfg;
    
    if(resolution & (~ SPO2_CFG_RESOLUTION_MASK)) return false;
    if(!MAX3010x<MAX3010xImpl, MAX3010xSample>::readByte(SPO2_CFG_REG, cfg)) return false;
    
    cfg &= ~(SPO2_CFG_RESOLUTION_MASK << SPO2_CFG_RESOLUTION_BIT);
    cfg |= (static_cast<uint8_t>(resolution) & SPO2_CFG_RESOLUTION_MASK) << SPO2_CFG_RESOLUTION_BIT;
    
    return MAX3010x<MAX3010xImpl, MAX3010xSample>::writeByte(SPO2_CFG_REG, cfg);
  }
  
  /**
  * Enable FIFO Rollover
  * @return true if successful, otherwise false
  */
  bool enableFIFORollover() {
    return MAX3010x<MAX3010xImpl, MAX3010xSample>::setBit(FIFO_CFG_REG, FIFO_ROLLOVER_EN_BIT, true);
  }
  
  /**
  * Disable FIFO Rollover
  * @return true if successful, otherwise false
  */
  bool disableFIFORollover() {
    return MAX3010x<MAX3010xImpl, MAX3010xSample>::setBit(FIFO_CFG_REG, FIFO_ROLLOVER_EN_BIT, false);
  }
  
  /**
   * Number of adjacent samples that are averaged for each FIFO SAMPLE
   */
  enum SampleAveraging {
    SMP_AVE_NONE = 0,   //!< No Averaging
    SMP_AVE_1 = 0,      //!< No Averaging
    SMP_AVE_2 = 1,      //!< Average over 2 samples
    SMP_AVE_4 = 2,      //!< Average over 4 samples
    SMP_AVE_8 = 3,      //!< Average over 8 samples
    SMP_AVE_16 = 4,     //!< Average over 16 samples
    SMP_AVE_32 = 5      //!< Average over 32 samples
  };
  
  /**
   * Set Sample Averaging
   * @param averaging Sample averaging
   * @returns true if successful, otherwise false
   */
  bool setSampleAveraging(SampleAveraging averaging) {
    uint8_t cfg;
    
    if(averaging & (~ FIFO_SMP_AVE_MASK)) return false;
    if(!MAX3010x<MAX3010xImpl, MAX3010xSample>::readByte(FIFO_CFG_REG, cfg)) return false;
    
    cfg &= ~(FIFO_SMP_AVE_MASK << FIFO_SMP_AVE_BIT);
    cfg |= (static_cast<uint8_t>(averaging) & FIFO_SMP_AVE_MASK) << FIFO_SMP_AVE_BIT;
    
    return MAX3010x<MAX3010xImpl, MAX3010xSample>::writeByte(FIFO_CFG_REG, cfg);
  }
  
};

#endif
