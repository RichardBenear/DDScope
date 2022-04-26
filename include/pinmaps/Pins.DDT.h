// -------------------------------------------------------------------------------------------------
// Pin map for OnStep Direct Drive Telescope (Teensy4.0)

#if defined(__IMXRT1062__)

#define AZ_ENABLED_LED_PIN     20
#define ALT_ENABLED_LED_PIN    21
#define BATTERY_LOW_LED_PIN    22      // 24V battery sense
#define ALT_THERMISTOR_PIN     23      // Analog input
#define AZ_THERMISTOR_PIN      24      // Analog input 

// IL9341_t3 2.8" Display and Touchscreen pins
#define TFT_DC                  9
#define TFT_CS                 10
//#define TFT_RST               255     // 255 = unused, connect to 3.3V
#define TFT_MOSI               11
#define TFT_SCLK               13
#define TFT_MISO               12
#define TS_CS                   8
#define TS_IRQ                  6
#define TFT_LED                 5
#define ODRIVE_RST              3
#define LEDnegPin               7     // Drain Voltage
#define FanOnPin               25     // was pin 32 for Ver1
//#define LEDneg2Pin                  // Drain 
//#define ReticlePin         Aux4     // Drain
#define TonePin                 4     // Piezo Buzzer

// The PPS pin is a 3.3V logic input, OnStep measures time between rising edges and adjusts the internal sidereal clock frequency
#define PpsPin                  2     // PPS time source, GPS for example

// Axis1 RA/Azm step/dir driver
#define Axis1_EN              OFF     // Enable
//#define Axis1_M0             15     // Microstep Mode 0 or SPI MOSI
//#define Axis1_M1             16     // Microstep Mode 1 or SPI SCK
//#define Axis1_M2             17     // Microstep Mode 2 or SPI CS or Decay Mode
//#define Axis1_M3           Aux1     // ESP8266 GPIO0 (option on MiniPCB) or SPI MISO/Fault
#define Axis1_STEP             32     // Step - Mapped but NOT CONNECTED
#define Axis1_DIR              33     // Dir - Mapped but NOT CONNECTED
//#define Axis1_DECAY    Axis1_M2     // Decay mode
//#define Axis1_FAULT        Aux1     // SPI MISO/Fault
//#define Axis1_HOME         Aux3     // Sense home position

// Axis2 Dec/Alt step/dir driver
#define Axis2_EN              OFF     // Enable
//#define Axis2_M0              8     // Microstep Mode 0 or SPI MOSI
//#define Axis2_M1              7     // Microstep Mode 1 or SPI SCK
//#define Axis2_M2              6     // Microstep Mode 2 or SPI CS or Decay Mode
//#if PINMAP == MiniPCB13
//  #define Axis2_M3         Aux1     // SPI MISO/Fault or I2C SDA
//#else
//  #define Axis2_M3         Aux2     // ESP8266 RST or SPI MISO/Fault
//#endif
#define Axis2_STEP             30     // Step Mapped but NOT CONNECTED
#define Axis2_DIR              31     // Dir Mapped but NOT CONNECTED
//#define Axis2_DECAY    Axis2_M2     // Decay mode
//#define Axis2_FAULT    Axis2_M3
//#define Axis2_HOME         Aux4     // Sense home position

#define Axis4_STEP             28     // Focuser Step
#define Axis4_DIR              29     // Focuser Dir
#define Axis4_EN               27     // Focuser Enable
#define Axis4_SLEEP            26     // Sleep Motor Controller

#else
#error "Wrong processor for this configuration!"

#endif
