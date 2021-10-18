/*!
 * @file MAX30105.h
 */

#ifndef _MAX30105_H
#define _MAX30105_H

#include "MAX3010x_multiLed_core.h"

/**
 * MAX30105 Sample Data
 */
struct MAX30105Sample {
  union {
    struct {
      uint32_t red;     //!< Measurement Value for the Red LED
      uint32_t ir;      //!< Measurement Value for the IR LED
    };                  //!< Measurement Values in Classic Modes
    uint32_t slot[4];   //!< Measurement Values for Slots
  };                    //!< Sample Data
  bool valid;           //!< Indicator whether this sample is valid
};

/**
 * MAX30105 Sensor Driver
 */
class MAX30105 : public MAX3010xMultiLed<MAX30105, MAX30105Sample> {
  friend class MAX3010xMultiLed<MAX30105, MAX30105Sample>; //!< Friend declaration for access to private members from base class
  friend class MAX3010x<MAX30105, MAX30105Sample>; //!< Friend declaration for access to private members from base class
  
  static const uint8_t FIFO_SIZE = 32;            //!< FIFO Size (Number of samples)
  static const uint8_t MAX_ACTIVE_LEDS = 4;       //!< Maximum number of active LEDs
  
  static const uint8_t PILOT_LED_CFG_REG = 0x10;  //!< Proximity Mode LED Power Configuration Register
  static const uint8_t PROX_INT_TRESH_REG = 0x30; //!< Proximity Interrupt Threshold Register
  
  static const uint8_t INT_CNT = 6;
  static const uint8_t INT_CFG_REG[INT_CNT];      //!< Array to map interrupts to the corresponding configuration register
  static const uint8_t INT_CFG_BIT[INT_CNT];      //!< Array to map interrupts to the corresponding configuration bits
  static const uint8_t INT_ST_REG[INT_CNT];       //!< Array to map interrupts to the corresponding status register
  static const uint8_t INT_ST_BIT[INT_CNT];       //!< Array to map interrupts to the corresponding status bits
  
  bool setDefaultConfiguration();
public:
  /*
   * MAX30105 Interrupts
   */
  static const uint8_t INT_A_FULL = 0;            //!< FIFO Almost Full Interrupt
  static const uint8_t INT_TEMP_RDY = 1;          //!< Temperature Ready Interrupt
  static const uint8_t INT_PPG_RDY = 2;           //!< PPG Ready Interrupt
  static const uint8_t INT_ALC_OVF = 3;           //!< Ambient Light Cancellation Overflow Interrupt
  static const uint8_t INT_PROX_RDY = 4;          //!< Proximity Interrupt
  static const uint8_t INT_PWR_RDY = 5;           //!< Power Ready Interrupt
  
  MAX30105(uint8_t addr = MAX3010x_ADDR, TwoWire& wire = Wire);
  
  /**
   * LED
   */
  enum Led {
    LED_RED = 0,          //!< Red LED
    LED_1 = 0,            //!< LED 1 (alias for Red)
    LED_IR = 1,           //!< IR LED
    LED_2 = 1,            //!< LED 2 (alias for IR)
    LED_GREEN = 2,        //!< Green LED
    LED_3 = 2             //!< LED 3 (alias for Green)
  };
  
  
  bool setLedCurrent(Led led, uint8_t current);
  bool setProximityLedCurrent(uint8_t current);
  bool setProximityThreshold(uint8_t threshold);
  
  /**
   * Slot Configuration
   */
  enum SlotConfiguration {
    SLOT_OFF = 0,         //!< Slot is turned off
    SLOT_LED_1 = 1,       //!< LED 1 is active (alias for Red)
    SLOT_RED = 1,         //!< Red LED is active
    SLOT_LED_2 = 2,       //!< LED 2 is active (alias for IR)
    SLOT_IR = 2,          //!< IR LED is active
    SLOT_LED_3 = 3,       //!< LED 3 is active (alias for Green)
    SLOT_GREEN = 3,       //!< Green LED is active
    SLOT_PILOT_OFF = 4,   //!< Slot is turned off
    SLOT_PILOT_LED_1 = 5, //!< LED 1 is active with PILOT_PA (alias for Red)
    SLOT_PILOT_RED = 5,   //!< Red LED is active with PILOT_PA
    SLOT_PILOT_LED_2 = 6, //!< LED 2 is active with PILOT_PA (alias for IR)
    SLOT_PILOT_IR = 6,    //!< IR LED is active with PILOT_PA
    SLOT_PILOT_LED_3 = 7, //!< LED 3 is active with PILOT_PA (alias for Green)
    SLOT_PILOT_GREEN = 7  //!< Green LED is active with PILOT_PA
  };
  
  /**
   * Multi LED Configuration
   */
  struct MultiLedConfiguration {
    SlotConfiguration slot[4]; //!< Slot Configuration
  };
  
  bool setMultiLedConfiguration(const MultiLedConfiguration& cfg);
};


#endif
