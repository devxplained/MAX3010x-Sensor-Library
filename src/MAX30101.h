/*!
 * @file MAX30101.h
 */

#ifndef _MAX30101_H
#define _MAX30101_H

#include "MAX3010x_multiLed_core.h"

/**
 * MAX30101 Sample Data
 */
struct MAX30101Sample {
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
 * MAX30101 Sensor Driver
 */
class MAX30101 : public MAX3010xMultiLed<MAX30101, MAX30101Sample> {
  friend class MAX3010xMultiLed<MAX30101, MAX30101Sample>; //!< Friend declaration for access to private members from base class
  friend class MAX3010x<MAX30101, MAX30101Sample>; //!< Friend declaration for access to private members from base class
  
  static const uint8_t FIFO_SIZE = 32;            //!< FIFO Size (Number of samples)
  static const uint8_t MAX_ACTIVE_LEDS = 4;       //!< Maximum number of active LEDs
  
  static const uint8_t INT_CNT = 5;
  static const uint8_t INT_CFG_REG[INT_CNT];      //!< Array to map interrupts to the corresponding configuration register
  static const uint8_t INT_CFG_BIT[INT_CNT];      //!< Array to map interrupts to the corresponding configuration bits
  static const uint8_t INT_ST_REG[INT_CNT];       //!< Array to map interrupts to the corresponding status register
  static const uint8_t INT_ST_BIT[INT_CNT];       //!< Array to map interrupts to the corresponding status bits
  
  bool setDefaultConfiguration();
public:
  /*
   * MAX30101 Interrupts
   */
  static const uint8_t INT_A_FULL = 0;            //!< FIFO Almost Full Interrupt
  static const uint8_t INT_TEMP_RDY = 1;          //!< Temperature Ready Interrupt
  static const uint8_t INT_PPG_RDY = 2;           //!< PPG Ready Interrupt
  static const uint8_t INT_ALC_OVF = 3;           //!< Ambient Light Cancellation Overflow Interrupt
  static const uint8_t INT_PWR_RDY = 4;           //!< Power Ready Interrupt
  
  MAX30101(uint8_t addr = MAX3010x_ADDR, TwoWire& wire = Wire);
  
  /**
   * LED
   */
  enum Led {
    LED_RED = 0,          //!< Red LED
    LED_1 = 0,            //!< LED 1 (alias for Red)
    LED_IR = 1,           //!< IR LED
    LED_2 = 1,            //!< LED 2 (alias for IR)
    LED_GREEN = 2,        //!< Green LED (alias for Green CH 1)
    LED_GREEN_CH1 = 2,    //!< Green LED CH1
    LED_3 = 2,            //!< LED 3 (alias for Green CH 1)
    LED_GREEN_CH2 = 3,    //!< Green LED CH2
    LED_4 = 3             //!< LED 4 (alias for Green CH 2)
  };
  
  bool setLedCurrent(Led led, uint8_t current);
  
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
    SLOT_GREEN = 3        //!< Green LED is active
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
