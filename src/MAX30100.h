/*!
 * @file MAX30100.h
 */

#ifndef _MAX30100_H
#define _MAX30100_H

#include "MAX3010x_core.h"

/**
 * MAX30100 Sample Data
 */
struct MAX30100Sample {
  union {
    struct {
      uint16_t ir;      //!< Measurement Value for the IR LED
      uint16_t red;     //!< Measurement Value for the Red LED
    };                  //!< Measurement Values in Classic Modes
    uint16_t slot[2];   //!< Measurement Values for Slots
  };                    //!< Sample Data
  bool valid;       //!< Indicator whether this sample is valid
};
  
/**
 * MAX30100 Sensor Driver
 */
class MAX30100 : public MAX3010x<MAX30100, MAX30100Sample> {
  friend class MAX3010x<MAX30100, MAX30100Sample>; //!< Friend declaration for access to private members from base class
private:
  static const uint8_t MAX3010x_PART_ID = 0x11;   //!< Expected Part ID
  
  static const uint8_t TINT_REG = 0x16;           //!< Temperature Register
  static const uint8_t TFRAC_REG = 0x17;          //!< Fractional Temperature Component Register
  
  static const uint8_t SAMPLE_SIZE = 2;           //!< Sample Size
  static const uint8_t MAX_ACTIVE_LEDS = 2;       //!< Maximum number of active LEDs
  static const uint8_t FIFO_SIZE = 16;            //!< FIFO Size (Number of samples)
  static const uint8_t FIFO_BASE = 0x02;          //!< FIFO Register Base
  
  static const uint8_t MODE_CFG_REG = 0x6;        //!< Mode Configuration Register
  static const uint8_t MODE_SHDN_BIT = 7;         //!< Shutdown Bit
  static const uint8_t MODE_RST_BIT = 6;          //!< Reset Bit
  
  static const uint8_t TEMP_CONFIG_REG = 0x06;    //!< Temperature Trigger Register
  static const uint8_t TEMP_CONFIG_BIT = 3;       //!< Temperature Trigger Bit
  
  static const uint8_t SPO2_CFG_REG = 0x7;        //!< SpO2 Measurement Configuration Register
  static const uint8_t LED_CFG_REG = 0x9;         //!< LED Configuration Register
  
  static const uint8_t INT_CNT = 5;
  static const uint8_t INT_CFG_REG[INT_CNT];      //!< Array to map interrupts to the corresponding configuration register
  static const uint8_t INT_CFG_BIT[INT_CNT];      //!< Array to map interrupts to the corresponding configuration bits
  static const uint8_t INT_ST_REG[INT_CNT];       //!< Array to map interrupts to the corresponding status register
  static const uint8_t INT_ST_BIT[INT_CNT];       //!< Array to map interrupts to the corresponding status bits
  
  const uint8_t nActiveSlots = 2;                 //!< Number of active LED Slots in FIFO data (always 2 for MAX30100)
  
  bool setDefaultConfiguration();
  void fillSampleWithData(uint8_t data[SAMPLE_SIZE*MAX_ACTIVE_LEDS], MAX30100Sample& sample);
public:
  /**
   * Mode
   */
  enum Mode {
    MODE_HR_ONLY = 0b010,     //!< Heart Rate Only Mode (IR LED)
    MODE_IR_ONLY = 0b010,     //!< Red LED Only Mode (IR LED)
    MODE_SPO2 = 0b011,        //!< SPO2 Mode (Red + IR LED)
    MODE_RED_IR = 0b011,      //!< Red + IR LED Mode (Red + IR LED)
  };
  
  bool setMode(Mode mode);
  
  /**
   * LED Current in mA
   */
  enum LedCurrent {
    LED_CURRENT_0MA0,         //!< Current of 0 mA
    LED_CURRENT_4MA4,         //!< Current of 4.4 mA
    LED_CURRENT_7MA6,         //!< Current of 7.6mA
    LED_CURRENT_11MA0,        //!< Current of 11 mA
    LED_CURRENT_14MA2,        //!< Current of 14.2 mA
    LED_CURRENT_17MA4,        //!< Current of 17.4 mA
    LED_CURRENT_20MA8,        //!< Current of 20.8 mA
    LED_CURRENT_24MA0,        //!< Current of 24 mA
    LED_CURRENT_27MA1,        //!< Current of 27.1 mA
    LED_CURRENT_30MA6,        //!< Current of 30.6 mA
    LED_CURRENT_33MA8,        //!< Current of 33.8 mA
    LED_CURRENT_37MA0,        //!< Current of 37 mA
    LED_CURRENT_40MA2,        //!< Current of 40.2 mA
    LED_CURRENT_43MA6,        //!< Current of 43.6 mA
    LED_CURRENT_46MA8,        //!< Current of 46.8 mA
    LED_CURRENT_50MA0         //!< Current of 50 mA
  };
  
  /**
   * LED (IR or red)
   */
  enum Led {
    LED_IR = 0,               //!< IR LED
    LED_1 = 0,                //!< LED 1 (Alias for IR)
    LED_RED = 1,              //!< Red LED
    LED_2 = 1                 //!< LED 2 (Alias for Red)
  };
  
  /**
   * Sampling Rate
   */
  enum SamplingRate {
    SAMPLING_RATE_50SPS,      //!< 50 samples per second
    SAMPLING_RATE_100SPS,     //!< 100 samples per second
    SAMPLING_RATE_167SPS,     //!< 167 samples per second
    SAMPLING_RATE_200SPS,     //!< 200 samples per second
    SAMPLING_RATE_400SPS,     //!< 400 samples per second
    SAMPLING_RATE_600SPS,     //!< 600 samples per second
    SAMPLING_RATE_800SPS,     //!< 800 samples per second
    SAMPLING_RATE_1000SPS     //!< 1000 samples per second
  };
  
  /**
   * Measuring resolution and pulse width
   */
  enum Resolution {
    RESOLUTION_13BIT_200US,   //!< 13 bit resolution, 200 us pulse width
    RESOLUTION_14BIT_400US,   //!< 14 bit resolution, 400 us pulse width
    RESOLUTION_15BIT_800US,   //!< 15 bit resolution, 800 us pulse width
    RESOLUTION_16BIT_1600US   //!< 16 bit resolution, 1600 us pulse width
  };
  
  /*
   * MAX30100 Interrupts
   */
  static const uint8_t INT_A_FULL = 0;            //!< FIFO Almost Full Interrupt
  static const uint8_t INT_TEMP_RDY = 1;          //!< Temperature Ready Interrupt
  static const uint8_t INT_HR_RDY = 2;            //!< HR Ready Interrupt
  static const uint8_t INT_SPO2_RDY = 3;          //!< SPO2 Ready Interrupt
  static const uint8_t INT_PWR_RDY = 4;           //!< Power Ready Interrupt
    
  MAX30100(uint8_t addr = MAX3010x_ADDR, TwoWire& wire = Wire);
  bool setLedCurrent(Led led, LedCurrent current);
  bool setSamplingRate(SamplingRate rate);
  bool setResolution(Resolution resolution);
};


#endif
