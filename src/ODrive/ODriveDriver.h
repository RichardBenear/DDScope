#ifndef ODriveDriver_h
#define ODriveDriver_h
// ============================================
// ============== Odrive Drivers ==============
// ============================================
// Author: Richard Benear 2020
// ODrive Driver Functions
// ODrive communication via Teensy 4.0
// Uses GitHub ODrive Arduino library by Copyright (c) 2017 Oskar Weigl

#include <HardwareSerial.h>
#include "ODrive/ODriveArduino.h"
#include "pinmaps/Pins.DDT.h"
#include "Display/Display.h"

enum Component
{
    ENCODER,
    MOTOR,
    CONTROLLER,
    AXIS
};

// Forward declarations from OnStep
void stopSlewingAndTracking(StopSlewActions ss);
double LST();
void equToHor(double HA, double Dec, double *Alt, double *Azm);
bool getHor(double *Alt, double *Azm);
bool demoActive = false;

//====================================================
/***** Primitive ascii commands and examples ********
 p motor position velocity_ff torque_ff
    p for position
    motor is the motor number, 0 or 1.
    position is the desired position, in [turns].
    velocity_ff is the velocity feed-forward term, in [turns/s] (optional).
    torque_ff is the torque feed-forward term, in [Nm] (optional).
  Example: p 0 -2 0 0

Reading:
 r [property]
  property name of the property, as seen in ODrive Tool
  response: text representation of the requested value
Example: r vbus_voltage => response: 24.087744 <new line>
  Writing:
    w [property] [value]
    property name of the property, as seen in ODrive Tool
    value text representation of the value to be written
Example: w axis0.controller.input_pos -123.456

Request feedback
  f motor
response:
  pos vel
    f for feedback
    pos is the encoder position in [turns] (float)
    vel is the encoder velocity in [turns/s] (float)

System commands:
ss - Save config
se - Erase config
sr - Reboot
sc - Clear errors
****************************************************/

#define ALT 0 // ODrive Motor 0 - Do not change
#define AZ  1 // ODrive Motor 1 - Do not change
//#define ODRIVE_SLEW_MODE_STEPPER //when defined, slewing is at the Onstep step rate
//    instead of the ODrive trapezoidal max velocity rate

// Printing with stream operator helper functions
template<class T> inline Print& operator <<(Print &obj,     T arg) { obj.print(arg);    return obj; }
template<>        inline Print& operator <<(Print &obj, float arg) { obj.print(arg, 4); return obj; }
unsigned long dumpErrorUpdate;

/**************************************
// Set up serial pins to the ODrive
**************************************/
// Teensy 3 and 4 (all versions) - Serial3
// pin 15: RX - connect to ODrive TX
// pin 14: TX - connect to ODrive RX
// See https://www.pjrc.com/teensy/td_uart.html for other options on Teensy
HardwareSerial& odrive_serial = Serial3;

// ODrive object
ODriveArduino odrive(odrive_serial);

// General Initalization of Odrive
void initOdrive() {
  digitalWrite(ODRIVE_RST, HIGH); // bring ODrive out of Reset
  delay(1000); // allow time for ODrive to boot
  odrive_serial.begin(19200); 
  VLF("MSG: ODrive channel Init");
  demoActive = false;
}
  
// Set Axis # to IDLE
bool idleOdriveMotor(int axis) {
  int requested_state;
  int motornum = axis;
  
  requested_state = AXIS_STATE_IDLE;
  //SerialA << "Axis" << axis << ": Requesting state " << requested_state << '\n';
  
  if(!odrive.run_state(motornum, requested_state, false, 0.01f)) {
    VLF("MSG: Closed loop timeout");
    return false; 
  } else {
      VLF("MSG: Odrive Axis Idle");
      if (axis == AZ) { 
        axis1Enabled = false; 
      } else if (axis == ALT) { 
        axis2Enabled = false;
      }
    return true;
  }
}

// Turn off both axis motors
void stopMotors() {
  stopSlewingAndTracking(SS_LIMIT); // stop slewing, trackin, and goto
  idleOdriveMotor(AZ); // turn off the motors
  idleOdriveMotor(ALT);
  odriveAZOff = true;
  odriveALTOff = true;
  digitalWrite(AZ_ENABLED_LED_PIN, HIGH); // Turn Off AZ LED
  digitalWrite(ALT_ENABLED_LED_PIN, HIGH); // Turn Off ALT LED
}

// Turn Axis # Motor ON
bool turnOnOdriveMotor(int axis) {
  int requested_state;
  int motornum = axis;
  requested_state = AXIS_STATE_CLOSED_LOOP_CONTROL;
  if(!odrive.run_state(motornum, requested_state, false, 0.5f)) {
    VLF("MSG: Closed loop timeout");
  return false; 
  } else {
    VLF("MSG: Odrive Motor in Closed Loop");
    if (axis == AZ) axis1Enabled = true; else axis2Enabled = true;
  return true;
  }
}

// Read bus voltage
float getOdriveBusVoltage() {
  odrive_serial << "r vbus_voltage\n";
  float bat_volt = (float)(odrive.readFloat());
return (float)bat_volt;  
}

// get absolute Encoder positions in degrees
float getEncoderPositionDeg(int axis) {
  float turns;
  odrive_serial << "r axis" << axis << ".encoder.pos_estimate\n"; 
  turns = odrive.readFloat();
  return turns*360;
}  

// get motor positions in turns
float getMotorPositionTurns(int axis) {
  odrive_serial << "r axis" << axis << ".encoder.pos_estimate\n"; 
return odrive.readFloat();
}  

// get motor position in counts
int getMotorPositionCounts(int axis) {
  odrive_serial << "r axis" << axis << ".encoder.pos_estimate_counts\n";
return odrive.readInt();
} 

// get Motor Current
float getMotorCurrent(int axis) {
  odrive_serial << "r axis" << axis << ".motor.I_bus\n";  
return odrive.readFloat();
}  

// read current requested state
int getOdriveRequestedState() {
  odrive_serial << "r axis0.requested_state\n";
  return odrive.readInt();
}

float getMotorPositionDelta(int axis) {
  odrive_serial << "r axis" << axis << ".controller.pos_setpoint\n";
  float reqPos = odrive.readFloat();   
  odrive_serial << "r axis" << axis << ".encoder.pos_estimate\n";
  float posEst = odrive.readFloat();   
  float deltaPos = abs(reqPos - posEst);
  return deltaPos;
}

// Update both ALT and AZ axis positions in turns
// Two slew modes are supported for DDscope
// 1) When DDT_SLEW_MODE_STEPPER defined, then motors are updated at the Onstep step rate
// 2) When DDT_SLEW_MODE_STEPPER not defined, then motors slew at the max limits set in ODrive
// 3) Both modes use the Onstep tracking updates during tracking
// Slewing with ODrive is faster and based on a trapezoidal trajectory
// Odrive slews reach their target faster but must still wait for Onstep to finish slews before
// switching back to tracking.
#ifdef DDT_SLEW_MODE_STEPPER

void updateOdriveMotorPositions() { 
  double alt, az;
  double alt_pos = 0;
  double az_pos = 0;

  // get real-time altitude and azimuth in degrees
  getHor(&alt, &az); // returns degrees
  az = degRange(az); // degRange limits to + or - 360
  
  // ALT position; calculate turns and put on limits
  alt_pos = alt/360.00;
  if (alt_pos < 0.00) alt_pos = 0.00; // does exactly 0 cause CommandError=CE_GOTO_ERR_BELOW_HORIZON?
  if (alt_pos > 90.00) alt_pos = 90.00; // does exactly 90 cause CommandError=CE_GOTO_ERR_ABOVE_OVERHEAD?
  
  // AZ axis only turns 180 deg left or right to keep cables from twisting too much
  if (az > 180) az = az - 360.00; // flip sign too
  az_pos = az/360.00; // convert to turns
  
  // SetPosition is in "turns". Always fractional.
  // Altitude range is between 0.0 and 0.5 turns
  // Azimuth range is between 0.5 and -0.5 turns
  if (!odriveAZOff ) odrive.SetPosition(AZ, az_pos); 
  if (!odriveALTOff ) odrive.SetPosition(ALT, alt_pos);
}

#else // Slew at the ODrive controller trapezoidal vel limit speed, then track with Onstep

void updateOdriveMotorPositions() {
  double alt, az, ot_azm_d, ot_alt_d;
  double alt_pos = 0;
  double az_pos = 0;
  char azmDMS[11] = "";
  char altDMS[12] = "";

  if (trackingState == TrackingMoveTo) {
    disp.getLocalCmdTrim(":GZ#", azmDMS); // DDD*MM'SS# 
    shc.dmsToDouble(&ot_azm_d, azmDMS, false, true);

    disp.getLocalCmdTrim(":GA#", altDMS);	// sDD*MM'SS#
    shc.dmsToDouble(&ot_alt_d, altDMS, true, true);

    // Target AZ in degrees
    if ((az != ot_azm_d)) {
      az = ot_azm_d;
    }
    
    // Target ALT in degrees
    if ((alt != ot_alt_d)) {
      alt = ot_alt_d;
    }
  } else {  // not slewing, but tracking so use real-time updates
      // get real-time altitude and azimuth in degrees
      getHor(&alt, &az); // returns degrees
      az = degRange(az); // degRange limits to + or - 360  
  }

  // ALT position; calculate turns and put on limits
  alt_pos = alt/360.00;
  if (alt_pos < 0.00) alt_pos = 0.00; // does exactly 0 cause CommandError=CE_GOTO_ERR_BELOW_HORIZON?
  if (alt_pos > 90.00) alt_pos = 90.00; // does exactly 90 cause CommandError=CE_GOTO_ERR_ABOVE_OVERHEAD?
  
  // AZ axis only turns 180 deg left or right to keep cables from twisting too much
  if (az > 180) az = az - 360.00; // flip sign too
  az_pos = az/360.00; // convert to turns
  
  // SetPosition is in "turns". Always fractional.
  // Altitude range is between 0.0 and 0.5 turns
  // Azimuth range is between 0.5 and -0.5 turns
  if (!odriveAZOff ) odrive.SetPosition(AZ, az_pos); 
  if (!odriveALTOff ) odrive.SetPosition(ALT, alt_pos);
}
#endif

// Odrive clear ALL errors
void clearAllOdriveErrors() {
  odrive_serial << "w sc\n"; 
} 

// Odrive clear subcategory errors
void clearOdriveErrors(int axis, int comp) {
    switch (comp) {
      case ENCODER:
        odrive_serial << "w axis"<<axis<<".encoder.error 0\n";
        VLF("MSG: Clearing Odrive errors");
        break;
      case MOTOR:
      odrive_serial << "w axis"<<axis<<".motor.error 0\n";
        VLF("MSG: Clearing Odrive errors");
        break;
      case CONTROLLER:
        odrive_serial << "w axis"<<axis<<".controller.error 0\n";
        VLF("MSG: Clearing Odrive errors");
        break;
      case AXIS:
        odrive_serial << "w axis"<<axis<<".error 0\n";
        VLF("MSG: Clearing Odrive errors");
        break;
    }
}

// Dump Error for specific module
int32_t dumpOdriveErrors(int axis, int comp) {   
  switch (comp) { 
    case ENCODER:
      odrive_serial << "r axis"<<axis<<".encoder.error\n";
      return odrive.readInt();
      VLF("MSG: dump Odrive encoder errors");
      break;
    case MOTOR:
    odrive_serial << "r axis"<<axis<<".motor.error\n";
    return odrive.readInt();
      VLF("MSG: dump Odrive motor errors");
      break;
    case CONTROLLER:
      odrive_serial << "r axis"<<axis<<".controller.error\n";
      return odrive.readInt();
      VLF("MSG: dump Odrive controller errors");
      break;
    case AXIS:
      odrive_serial << "r axis"<<axis<<".error\n";
      return odrive.readInt();
      VLF("MSG: dump Odrive axis errors");
      break;
  }
  return odrive.readInt();
}

void setOdriveVelGain(int axis, float level) {
  odrive_serial << "w axis"<<axis<<".controller.config.vel_gain "<<level<<'\n';
}

void setOdriveVelIntGain(int axis, float level) {
  odrive_serial << "w axis"<<axis<<".controller.config.vel_integrator_gain "<<level<<'\n';
}

void setOdrivePosGain(int axis, float level) {
  odrive_serial << "w axis"<<axis<<".controller.config.pos_gain "<<level<<'\n';
}

float getOdriveVelGain(int axis) {
  odrive_serial << "r axis"<<axis<<".controller.config.vel_gain\n";
  return odrive.readFloat();
}

float getOdriveVelIntGain(int axis) {
  odrive_serial << "r axis"<<axis<<".controller.config.vel_integrator_gain\n";
  return odrive.readFloat();
}

float getOdrivePosGain(int axis) {
  odrive_serial << "r axis"<<axis<<".controller.config.pos_gain\n";
  return odrive.readFloat();
}

#endif //ODriveDriver_h