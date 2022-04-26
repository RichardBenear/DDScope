// =========================================================
// =============== Odrive Controller Page ==================
// =========================================================
// Author: Richard Benear 2021

#include <TimedAction.h>

#define OD_ERR_OFFSET_X           4 
#define OD_ERR_OFFSET_Y         190 
#define OD_ERR_SPACING           16 
#define OD_BUTTONS_OFFSET        45 

// Buttons for actions that are not page selections
#define OD_ACT_BOXSIZE_X         100 
#define OD_ACT_BOXSIZE_Y         36 
#define OD_ACT_COL_1_X           3 
#define OD_ACT_COL_1_Y           324
#define OD_ACT_COL_2_X           OD_ACT_COL_1_X+OD_ACT_BOXSIZE_X+4
#define OD_ACT_COL_2_Y           OD_ACT_COL_1_Y
#define OD_ACT_COL_3_X           OD_ACT_COL_2_X+OD_ACT_BOXSIZE_X+4
#define OD_ACT_COL_3_Y           OD_ACT_COL_1_Y
#define OD_ACT_X_SPACING         7
#define OD_ACT_Y_SPACING         4
#define OD_ACT_TEXT_X_OFFSET     10
#define OD_ACT_TEXT_Y_OFFSET     20

uint8_t box_height_adj = 10;

bool clearOdriveErr = false;
bool resetOdriveFlag = false;
bool AZgainHigh = false;
bool AZgainDefault = true;
bool ALTgainHigh = false;
bool ALTgainDefault = true;

//****** Draw Odrive Page ******
void drawOdrivePage() {
  disp.updateColors();
  tft.setTextColor(TEXT_COLOR);
  tft.fillScreen(PAGE_BACKGROUND);
  currentPage = ODRIVE_PAGE;
  disp.drawMenuButtons();
  disp.drawTitle(105, 30, "ODrive");
  disp.drawCommonStatusLabels();
  disp.updateOnstepCmdStatus();
  odrive_serial << "r hw_version_major\n"; 
  uint8_t hwMajor = odrive.readInt();
  odrive_serial << "r hw_version_minor\n"; 
  uint8_t hwMinor = odrive.readInt();
  odrive_serial << "r hw_version_variant\n"; 
  uint8_t hwVar = odrive.readInt();
  odrive_serial << "r fw_version_major\n"; 
  uint8_t fwMajor = odrive.readInt();
  odrive_serial << "r fw_version_minor\n"; 
  uint8_t fwMinor = odrive.readInt();
  odrive_serial << "r fw_version_revision\n"; 
  uint8_t fwRev = odrive.readInt();
  tft.setCursor(12, 165);
  tft.print("*HW Version:"); tft.print(hwMajor); tft.print("."); tft.print(hwMinor); tft.print("."); tft.print(hwVar);
  tft.setCursor(12, 177);
  tft.print("*FW Version:"); tft.print(fwMajor); tft.print("."); tft.print(fwMinor); tft.print("."); tft.print(fwRev);
}

// =========== Motor Thermistor Support =============
float getMotorTemp(int motor) {
  int Ro = 9, B =  3950; //Nominal resistance 10K, Beta constant, 9k at 68 deg
  int Rseries = 10.0;// Series resistor 10K
  float To = 293; // Nominal Temperature 68 deg calibration point
  float Vi = 0;

  /*Read analog output of NTC module,
    i.e the voltage across the thermistor */
  // IMPORTANT: USE 3.3V for Thermistor!! Teensy pins are NOT 5V tolerant!
  if (motor == ALT)
    Vi = analogRead(ALT_THERMISTOR_PIN) * (3.3 / 1023.0);
  else
    Vi = analogRead(AZ_THERMISTOR_PIN) * (3.3 / 1023.0);
  //Convert voltage measured to resistance value
  //All Resistance are in kilo ohms.
  float R = (Vi * Rseries) / (3.3 - Vi);
  /*Use R value in steinhart and hart equation
    Calculate temperature value in kelvin*/
  float T =  1 / ((1 / To) + ((log(R / Ro)) / B));
  float Tc = T - 273.15; // Converting kelvin to celsius
  float Tf = Tc * 9.0 / 5.0 + 32.0; // Converting celsius to Fahrenheit
  return Tf;
}

// ===== Decode all Odrive Errors =====
void decodeOdriveError(uint32_t errorCode) {
  if      (errorCode == ODRIVE_ERROR_NONE)                          tft.println("ODRIVE_ERROR_NONE");
  else if (errorCode == ODRIVE_ERROR_CONTROL_ITERATION_MISSED)      tft.println("ODRIVE_ERROR_CONTROL_ITERATION_MISSED");
  else if (errorCode == ODRIVE_ERROR_DC_BUS_UNDER_VOLTAGE)          tft.println("ODRIVE_ERROR_DC_BUS_UNDER_VOLTAGE");
  else if (errorCode == ODRIVE_ERROR_DC_BUS_OVER_VOLTAGE)           tft.println("ODRIVE_ERROR_DC_BUS_OVER_VOLTAGE");
  else if (errorCode == ODRIVE_ERROR_DC_BUS_OVER_REGEN_CURRENT)     tft.println("ODRIVE_ERROR_DC_BUS_OVER_REGEN_CURRENT");
  else if (errorCode == ODRIVE_ERROR_DC_BUS_OVER_CURRENT)           tft.println("ODRIVE_ERROR_DC_BUS_OVER_CURRENT");
  else if (errorCode == ODRIVE_ERROR_BRAKE_DEADTIME_VIOLATION)      tft.println("ODRIVE_ERROR_BRAKE_DEADTIME_VIOLATION");
  else if (errorCode == ODRIVE_ERROR_BRAKE_DUTY_CYCLE_NAN)          tft.println("ODRIVE_ERROR_BRAKE_DUTY_CYCLE_NAN");
  else if (errorCode == ODRIVE_ERROR_INVALID_BRAKE_RESISTANCE)      tft.println("ODRIVE_ERROR_INVALID_BRAKE_RESISTANCE");
}

void decodeAxisError(int axis, uint32_t errorCode) {
  if      (errorCode == AXIS_ERROR_NONE)                            tft.println("AXIS_ERROR_NONE");
  else if (errorCode == AXIS_ERROR_INVALID_STATE)                   tft.println("AXIS_ERROR_INVALID_STATE");
  else if (errorCode == AXIS_ERROR_WATCHDOG_TIMER_EXPIRED)          tft.println("AXIS_ERROR_WATCHDOG_TIMER_EXPIRED");
  else if (errorCode == AXIS_ERROR_MIN_ENDSTOP_PRESSED)             tft.println("AXIS_ERROR_MIN_ENDSTOP_PRESSED");
  else if (errorCode == AXIS_ERROR_MAX_ENDSTOP_PRESSED)             tft.println("AXIS_ERROR_MAX_ENDSTOP_PRESSED");
  else if (errorCode == AXIS_ERROR_ESTOP_REQUESTED)                 tft.println("AXIS_ERROR_ESTOP_REQUESTED");
  else if (errorCode == AXIS_ERROR_HOMING_WITHOUT_ENDSTOP)          tft.println("AXIS_ERROR_HOMING_WITHOUT_ENDSTOP");
  else if (errorCode == AXIS_ERROR_OVER_TEMP)                       tft.println("AXIS_ERROR_OVER_TEMP");
  else if (errorCode == AXIS_ERROR_UNKNOWN_POSITION)                tft.println("AXIS_ERROR_UNKNOWN_POSITION");
}

void decodeMotorError(int axis, uint32_t errorCode) { 
  if      (errorCode == MOTOR_ERROR_NONE)                           tft.println("MOTOR_ERROR_NONE");
  else if (errorCode == MOTOR_ERROR_PHASE_RESISTANCE_OUT_OF_RANGE)  tft.println("MOTOR_ERROR_PHASE_RESISTANCE_OUT_OF_RANGE");
  else if (errorCode == MOTOR_ERROR_PHASE_INDUCTANCE_OUT_OF_RANGE)  tft.println("MOTOR_ERROR_PHASE_INDUCTANCE_OUT_OF_RANGE");
  else if (errorCode == MOTOR_ERROR_DRV_FAULT)                      tft.println("MOTOR_ERROR_DRV_FAULT");
  else if (errorCode == MOTOR_ERROR_CONTROL_DEADLINE_MISSED)        tft.println("MOTOR_ERROR_CONTROL_DEADLINE_MISSED");
  else if (errorCode == MOTOR_ERROR_MODULATION_MAGNITUDE)           tft.println("MOTOR_ERROR_MODULATION_MAGNITUDE");
  else if (errorCode == MOTOR_ERROR_CURRENT_SENSE_SATURATION)       tft.println("MOTOR_ERROR_CURRENT_SENSE_SATURATION");
  else if (errorCode == MOTOR_ERROR_CURRENT_LIMIT_VIOLATION)        tft.println("MOTOR_ERROR_CURRENT_LIMIT_VIOLATION");
  else if (errorCode == MOTOR_ERROR_MODULATION_IS_NAN)              tft.println("MOTOR_ERROR_MODULATION_IS_NAN");
  else if (errorCode == MOTOR_ERROR_MOTOR_THERMISTOR_OVER_TEMP)     tft.println("MOTOR_ERROR_MOTOR_THERMISTOR_OVER_TEMP");
  else if (errorCode == MOTOR_ERROR_FET_THERMISTOR_OVER_TEMP)       tft.println("MOTOR_ERROR_FET_THERMISTOR_OVER_TEMP");
  else if (errorCode == MOTOR_ERROR_TIMER_UPDATE_MISSED)            tft.println("MOTOR_ERROR_TIMER_UPDATE_MISSED");
  else if (errorCode == MOTOR_ERROR_CURRENT_MEASUREMENT_UNAVAILABLE) tft.println("MOTOR_ERROR_CURRENT_MEASUREMENT_UNAVAIL");
  else if (errorCode == MOTOR_ERROR_CONTROLLER_FAILED)              tft.println("MOTOR_ERROR_CONTROLLER_FAILED");
  else if (errorCode == MOTOR_ERROR_I_BUS_OUT_OF_RANGE)             tft.println("MOTOR_ERROR_I_BUS_OUT_OF_RANGE");
  else if (errorCode == MOTOR_ERROR_BRAKE_RESISTOR_DISARMED)        tft.println("MOTOR_ERROR_BRAKE_RESISTOR_DISARMED"); 
  else if (errorCode == MOTOR_ERROR_SYSTEM_LEVEL)                   tft.println("MOTOR_ERROR_SYSTEM_LEVEL");
  else if (errorCode == MOTOR_ERROR_BAD_TIMING)                     tft.println("MOTOR_ERROR_BAD_TIMING");
  else if (errorCode == MOTOR_ERROR_UNKNOWN_PHASE_ESTIMATE)         tft.println("MOTOR_ERROR_UNKNOWN_PHASE_ESTIMATE");
  else if (errorCode == MOTOR_ERROR_UNKNOWN_PHASE_VEL)              tft.println("MOTOR_ERROR_UNKNOWN_PHASE_VEL");
  else if (errorCode == MOTOR_ERROR_UNKNOWN_TORQUE)                 tft.println("MOTOR_ERROR_UNKNOWN_TORQUE");  
  else if (errorCode == MOTOR_ERROR_UNKNOWN_CURRENT_COMMAND)        tft.println("MOTOR_ERROR_UNKNOWN_CURRENT_COMMAND");
  else if (errorCode == MOTOR_ERROR_UNKNOWN_CURRENT_MEASUREMENT)    tft.println("MOTOR_ERROR_UNKNOWN_CURRENT_MEASUREMENT");
  else if (errorCode == MOTOR_ERROR_UNKNOWN_VBUS_VOLTAGE)           tft.println("MOTOR_ERROR_UNKNOWN_VBUS_VOLTAGE");
  else if (errorCode == MOTOR_ERROR_UNKNOWN_VOLTAGE_COMMAND)        tft.println("MOTOR_ERROR_UNKNOWN_VOLTAGE_COMMAND"); 
  else if (errorCode == MOTOR_ERROR_UNKNOWN_GAINS)                  tft.println("MOTOR_ERROR_UNKNOWN_GAINS");  
  else if (errorCode == MOTOR_ERROR_CONTROLLER_INITIALIZING)        tft.println("MOTOR_ERROR_CONTROLLER_INITIALIZING"); 
  else if (errorCode == MOTOR_ERROR_UNBALANCED_PHASES)              tft.println("MOTOR_ERROR_UNBALANCED_PHASES"); 
}

void decodeControllerError(int axis, uint32_t errorCode) {
  if      (errorCode == CONTROLLER_ERROR_NONE)                      tft.println("CONTROLLER_ERROR_NONE");
  else if (errorCode == CONTROLLER_ERROR_OVERSPEED)                 tft.println("CONTROLLER_ERROR_OVERSPEED");
  else if (errorCode == CONTROLLER_ERROR_INVALID_INPUT_MODE)        tft.println("CONTROLLER_ERROR_INVALID_INPUT_MODE");
  else if (errorCode == CONTROLLER_ERROR_UNSTABLE_GAIN)             tft.println("CONTROLLER_ERROR_UNSTABLE_GAIN"); 
  else if (errorCode == CONTROLLER_ERROR_INVALID_MIRROR_AXIS)       tft.println("CONTROLLER_ERROR_INVALID_MIRROR_AXIS"); 
  else if (errorCode == CONTROLLER_ERROR_INVALID_LOAD_ENCODER)      tft.println("CONTROLLER_ERROR_INVALID_LOAD_ENCODER"); 
  else if (errorCode == CONTROLLER_ERROR_INVALID_ESTIMATE)          tft.println("CONTROLLER_ERROR_INVALID_ESTIMATE");
  else if (errorCode == CONTROLLER_ERROR_INVALID_CIRCULAR_RANGE)    tft.println("CONTROLLER_ERROR_INVALID_CIRCULAR_RANGE"); 
  else if (errorCode == CONTROLLER_ERROR_SPINOUT_DETECTED)          tft.println("CONTROLLER_ERROR_SPINOUT_DETECTED");  
}

void decodeEncoderError(int axis, uint32_t errorCode) {
  if      (errorCode == ENCODER_ERROR_NONE)                         tft.println("ENCODER_ERROR_NONE"); 
  else if (errorCode == ENCODER_ERROR_UNSTABLE_GAIN)                tft.println("ENCODER_ERROR_UNSTABLE_GAIN");
  else if (errorCode == ENCODER_ERROR_CPR_POLEPAIRS_MISMATCH)       tft.println("ENCODER_ERROR_CPR_POLEPAIRS_MISMATCH"); 
  else if (errorCode == ENCODER_ERROR_NO_RESPONSE)                  tft.println("ENCODER_ERROR_NO_RESPONSE");  
  else if (errorCode == ENCODER_ERROR_UNSUPPORTED_ENCODER_MODE)     tft.println("ENCODER_ERROR_UNSUPPORTED_ENCODER_MODE");  
  else if (errorCode == ENCODER_ERROR_ILLEGAL_HALL_STATE)           tft.println("ENCODER_ERROR_ILLEGAL_HALL_STATE");
  else if (errorCode == ENCODER_ERROR_INDEX_NOT_FOUND_YET)          tft.println("ENCODER_ERROR_INDEX_NOT_FOUND_YET"); 
  else if (errorCode == ENCODER_ERROR_ABS_SPI_TIMEOUT)              tft.println("ENCODER_ERROR_ABS_SPI_TIMEOUT");
  else if (errorCode == ENCODER_ERROR_ABS_SPI_COM_FAIL)             tft.println("ENCODER_ERROR_ABS_SPI_COM_FAIL");
  else if (errorCode == ENCODER_ERROR_ABS_SPI_NOT_READY)            tft.println("ENCODER_ERROR_ABS_SPI_NOT_READY");
  else if (errorCode == ENCODER_ERROR_HALL_NOT_CALIBRATED_YET)      tft.println("ENCODER_ERROR_HALL_NOT_CALIBRATED_YET"); 
}

unsigned int lastOdriveErr = 55; // cause initial compare to be false
unsigned int lastALTErr = 55;
unsigned int lastALTCtrlErr = 55;
unsigned int lastALTMotorErr = 55;
unsigned int lastALTEncErr = 55;
unsigned int lastAZErr = 55;
unsigned int lastAZCtrlErr = 55;
unsigned int lastAZMotorErr = 55;
unsigned int lastAZEncErr = 55;

// ========  Update Odrive Page Status ========
void updateOdriveStatus() {
  unsigned int errorCode = 0;
  int y_offset = 0;
  
  disp.updateCommonStatus();
  tft.setCursor(OD_ERR_OFFSET_X, OD_ERR_OFFSET_Y); 

   // Odrive Errors Status
  odrive_serial << "r odrive.error\n";
  errorCode = odrive.readInt();
  if ((lastOdriveErr != errorCode) || firstDraw) { decodeOdriveError(errorCode); lastOdriveErr = errorCode; }

  // ALT Error
  y_offset +=OD_ERR_SPACING + 6;
  tft.setCursor(OD_ERR_OFFSET_X, OD_ERR_OFFSET_Y + y_offset);
  tft.print("----ALT ERRORS----");

  y_offset +=OD_ERR_SPACING;
  tft.setCursor(OD_ERR_OFFSET_X, OD_ERR_OFFSET_Y + y_offset);
  odrive_serial << "r odrive.axis"<<ALT<<".error\n";
  errorCode = odrive.readInt();
  if ((lastALTErr != errorCode) || firstDraw) { decodeAxisError(ALT, errorCode); lastALTErr = errorCode; }

  y_offset +=OD_ERR_SPACING;
  tft.setCursor(OD_ERR_OFFSET_X, OD_ERR_OFFSET_Y + y_offset);
  odrive_serial << "r odrive.axis"<<ALT<<".controller.error\n";
  errorCode = odrive.readInt();
  if ((lastALTCtrlErr != errorCode) || firstDraw) { decodeControllerError(ALT, errorCode); lastALTCtrlErr = errorCode; }

  y_offset +=OD_ERR_SPACING;
  tft.setCursor(OD_ERR_OFFSET_X, OD_ERR_OFFSET_Y + y_offset);
  odrive_serial << "r odrive.axis"<<ALT<<".motor.error\n";
  errorCode = odrive.readInt();
  if ((lastALTMotorErr != errorCode) || firstDraw) { decodeMotorError(ALT, errorCode); lastALTMotorErr = errorCode; }

  y_offset +=OD_ERR_SPACING;
  tft.setCursor(OD_ERR_OFFSET_X, OD_ERR_OFFSET_Y + y_offset);
  odrive_serial << "r odrive.axis"<<ALT<<".encoder.error\n";
  errorCode = odrive.readInt();
  if ((lastALTEncErr != errorCode) || firstDraw) { decodeEncoderError(ALT, errorCode); lastALTEncErr = errorCode; }

  // AZ Errors
  y_offset +=OD_ERR_SPACING;
  tft.setCursor(OD_ERR_OFFSET_X, OD_ERR_OFFSET_Y + y_offset);
  tft.println("----AZ ERRORS----");

  y_offset +=OD_ERR_SPACING;
  tft.setCursor(OD_ERR_OFFSET_X, OD_ERR_OFFSET_Y + y_offset);
  odrive_serial << "r odrive.axis"<<AZ<<".error\n";
  errorCode = odrive.readInt();
  if ((lastAZErr != errorCode) || firstDraw) { decodeAxisError(ALT, errorCode); lastALTErr = errorCode; }

  y_offset +=OD_ERR_SPACING;
  tft.setCursor(OD_ERR_OFFSET_X, OD_ERR_OFFSET_Y + y_offset);
  odrive_serial << "r odrive.axis"<<AZ<<".controller.error\n";
  errorCode = odrive.readInt();
  if ((lastAZCtrlErr != errorCode) || firstDraw) { decodeControllerError(ALT, errorCode); lastALTCtrlErr = errorCode; }

  y_offset +=OD_ERR_SPACING;
  tft.setCursor(OD_ERR_OFFSET_X, OD_ERR_OFFSET_Y + y_offset);
  odrive_serial << "r odrive.axis"<<AZ<<".motor.error\n";
  errorCode = odrive.readInt();
  if ((lastAZMotorErr != errorCode) || firstDraw) { decodeMotorError(AZ, errorCode); lastALTMotorErr = errorCode; }

  y_offset +=OD_ERR_SPACING;
  tft.setCursor(OD_ERR_OFFSET_X, OD_ERR_OFFSET_Y + y_offset);
  odrive_serial << "r odrive.axis"<<ALT<<".encoder.error\n";
  errorCode = odrive.readInt();
  if ((lastAZEncErr != errorCode) || firstDraw) { decodeEncoderError(AZ, errorCode); lastALTEncErr = errorCode; }


  // ***** Button label updates *****
  if (screenTouched || firstDraw || refreshScreen) { // reduce screen flicker
    refreshScreen = false;
    if (screenTouched) refreshScreen = true;

    int x_offset = 0;
    y_offset = 0;
    y_offset +=OD_ACT_BOXSIZE_Y + OD_ACT_Y_SPACING;

    // =========== Column 1 ===========
    if (odriveAZOff) {
      disp.drawButton(OD_ACT_COL_1_X + x_offset, OD_ACT_COL_1_Y + y_offset, OD_ACT_BOXSIZE_X, OD_ACT_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, OD_ACT_TEXT_X_OFFSET, OD_ACT_TEXT_Y_OFFSET, "  EN AZ   ");
    } else { //motor on
      disp.drawButton(OD_ACT_COL_1_X + x_offset, OD_ACT_COL_1_Y + y_offset, OD_ACT_BOXSIZE_X, OD_ACT_BOXSIZE_Y, BUT_OUTLINE, BUT_ON_BGD, OD_ACT_TEXT_X_OFFSET, OD_ACT_TEXT_Y_OFFSET,   "AZ Enabled");
    }

    y_offset += OD_ACT_BOXSIZE_Y + OD_ACT_Y_SPACING;
    if (odriveALTOff) {
      disp.drawButton(OD_ACT_COL_1_X + x_offset, OD_ACT_COL_1_Y + y_offset, OD_ACT_BOXSIZE_X, OD_ACT_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, OD_ACT_TEXT_X_OFFSET, OD_ACT_TEXT_Y_OFFSET, "  EN ALT   ");
    } else { //motor on
      disp.drawButton(OD_ACT_COL_1_X + x_offset, OD_ACT_COL_1_Y + y_offset, OD_ACT_BOXSIZE_X, OD_ACT_BOXSIZE_Y, BUT_OUTLINE, BUT_ON_BGD, OD_ACT_TEXT_X_OFFSET, OD_ACT_TEXT_Y_OFFSET,   "ALT Enabled");
    }

    // Second Column
    y_offset = 0;
    y_offset +=OD_ACT_BOXSIZE_Y + OD_ACT_Y_SPACING;
    if (stopButton) {
      disp.drawButton(OD_ACT_COL_2_X + x_offset, OD_ACT_COL_2_Y + y_offset, OD_ACT_BOXSIZE_X, OD_ACT_BOXSIZE_Y, BUT_OUTLINE, BUT_ON_BGD, OD_ACT_TEXT_X_OFFSET, OD_ACT_TEXT_Y_OFFSET,    "AllStopped");
      stopButton = false;
    } else { 
      disp.drawButton(OD_ACT_COL_2_X + x_offset, OD_ACT_COL_2_Y + y_offset, OD_ACT_BOXSIZE_X, OD_ACT_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, OD_ACT_TEXT_X_OFFSET+5, OD_ACT_TEXT_Y_OFFSET, "  STOP!  ");
    }

    y_offset +=OD_ACT_BOXSIZE_Y + OD_ACT_Y_SPACING;
    // Clear Errors
    if (!clearOdriveErr) {
      disp.drawButton(OD_ACT_COL_2_X + x_offset, OD_ACT_COL_2_Y + y_offset, OD_ACT_BOXSIZE_X, OD_ACT_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, OD_ACT_TEXT_X_OFFSET-6, OD_ACT_TEXT_Y_OFFSET, "Clear Errors");
    } else {
      disp.drawButton(OD_ACT_COL_2_X + x_offset, OD_ACT_COL_2_Y + y_offset, OD_ACT_BOXSIZE_X, OD_ACT_BOXSIZE_Y, BUT_OUTLINE, BUT_ON_BGD, OD_ACT_TEXT_X_OFFSET-6, OD_ACT_TEXT_Y_OFFSET, "Errs Cleared");
      clearOdriveErr = false;
    }

    // 3rd Column
    y_offset = -165;
    
    // AZ Gains High
    if (!AZgainHigh) {
      disp.drawButton(OD_ACT_COL_3_X + x_offset, OD_ACT_COL_3_Y + y_offset, OD_ACT_BOXSIZE_X, OD_ACT_BOXSIZE_Y-box_height_adj, BUT_OUTLINE, BUT_BACKGROUND, OD_ACT_TEXT_X_OFFSET-6, OD_ACT_TEXT_Y_OFFSET, "AZ Gain Hi");
    } else {
      disp.drawButton(OD_ACT_COL_3_X + x_offset, OD_ACT_COL_3_Y + y_offset, OD_ACT_BOXSIZE_X, OD_ACT_BOXSIZE_Y-box_height_adj, BUT_OUTLINE, BUT_ON_BGD, OD_ACT_TEXT_X_OFFSET-6, OD_ACT_TEXT_Y_OFFSET, "AZ Gain Hi");
    }

    // AZ Gains Default
    y_offset +=OD_ACT_BOXSIZE_Y-box_height_adj + OD_ACT_Y_SPACING;
    if (!AZgainDefault) {
      disp.drawButton(OD_ACT_COL_3_X + x_offset, OD_ACT_COL_3_Y + y_offset, OD_ACT_BOXSIZE_X, OD_ACT_BOXSIZE_Y-box_height_adj, BUT_OUTLINE, BUT_BACKGROUND, OD_ACT_TEXT_X_OFFSET-6, OD_ACT_TEXT_Y_OFFSET, "AZ Gain Def");
    } else {
      disp.drawButton(OD_ACT_COL_3_X + x_offset, OD_ACT_COL_3_Y + y_offset, OD_ACT_BOXSIZE_X, OD_ACT_BOXSIZE_Y-box_height_adj, BUT_OUTLINE, BUT_ON_BGD, OD_ACT_TEXT_X_OFFSET-6, OD_ACT_TEXT_Y_OFFSET, "AZ Gain Def");
    }

    // ALT Velocity Gain High
    y_offset +=OD_ACT_BOXSIZE_Y-box_height_adj + OD_ACT_Y_SPACING;
    if (!ALTgainHigh) {
      disp.drawButton(OD_ACT_COL_3_X + x_offset, OD_ACT_COL_3_Y + y_offset, OD_ACT_BOXSIZE_X, OD_ACT_BOXSIZE_Y-box_height_adj, BUT_OUTLINE, BUT_BACKGROUND, OD_ACT_TEXT_X_OFFSET-6, OD_ACT_TEXT_Y_OFFSET, "ALT Gain Hi");
    } else {
      disp.drawButton(OD_ACT_COL_3_X + x_offset, OD_ACT_COL_3_Y + y_offset, OD_ACT_BOXSIZE_X, OD_ACT_BOXSIZE_Y-box_height_adj, BUT_OUTLINE, BUT_ON_BGD, OD_ACT_TEXT_X_OFFSET-6, OD_ACT_TEXT_Y_OFFSET, "ALT Gain Hi");
    }

    // ALT Velocity Gain Default
    y_offset +=OD_ACT_BOXSIZE_Y-box_height_adj + OD_ACT_Y_SPACING;
    if (!ALTgainDefault) {
      disp.drawButton(OD_ACT_COL_3_X + x_offset, OD_ACT_COL_3_Y + y_offset, OD_ACT_BOXSIZE_X, OD_ACT_BOXSIZE_Y-box_height_adj, BUT_OUTLINE, BUT_BACKGROUND, OD_ACT_TEXT_X_OFFSET-6, OD_ACT_TEXT_Y_OFFSET, "ALT Gain Def");
    } else {
      disp.drawButton(OD_ACT_COL_3_X + x_offset, OD_ACT_COL_3_Y + y_offset, OD_ACT_BOXSIZE_X, OD_ACT_BOXSIZE_Y-box_height_adj, BUT_OUTLINE, BUT_ON_BGD, OD_ACT_TEXT_X_OFFSET-6, OD_ACT_TEXT_Y_OFFSET, "ALT Gain Def");
    }

    // ====== Show the Gains ======
    // Show AZM Velocity Gain - AZ is motor 1
    tft.setFont();
    float temp = getOdriveVelGain(AZ);
    tft.setCursor(210,280); tft.print("AZM Vel  Gain:");
    tft.fillRect(295,280, 39, 10, PAGE_BACKGROUND); 
    tft.setCursor(295,280); tft.print(temp);
    
    // Show AZM Velocity Integrator Gain
    temp = getOdriveVelIntGain(AZ);
    tft.setCursor(210,290); tft.print("AZM VelI Gain:");
    tft.fillRect(295,290, 39, 10, PAGE_BACKGROUND); 
    tft.setCursor(295,290); tft.print(temp);

    // Show ALT Velocity Gain - ALT is motor 0
    temp = getOdriveVelGain(ALT);
    tft.setCursor(210,300); tft.print("ALT Vel  Gain:");
    tft.fillRect(295,300, 39, 10, PAGE_BACKGROUND); 
    tft.setCursor(295,300); tft.print(temp);

    // Show ALT Velocity Integrator Gain
    temp = getOdriveVelIntGain(ALT);
    tft.setCursor(210,310); tft.print("ALT VelI Gain:");
    tft.fillRect(295,310, 39, 10, PAGE_BACKGROUND); 
    tft.setCursor(295,310); tft.print(temp);
    tft.setFont(&Inconsolata_Bold8pt7b);
    // ==================================

    y_offset = 0;
    // Demo Button
    if (!demoActive) {
      disp.drawButton(OD_ACT_COL_3_X + x_offset, OD_ACT_COL_3_Y + y_offset, OD_ACT_BOXSIZE_X, OD_ACT_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, OD_ACT_TEXT_X_OFFSET-4, OD_ACT_TEXT_Y_OFFSET, "Demo ODrive");
    } else {
      disp.drawButton(OD_ACT_COL_3_X + x_offset, OD_ACT_COL_3_Y + y_offset, OD_ACT_BOXSIZE_X, OD_ACT_BOXSIZE_Y, BUT_OUTLINE, BUT_ON_BGD, OD_ACT_TEXT_X_OFFSET-4, OD_ACT_TEXT_Y_OFFSET, "Demo Active");
    }

    y_offset +=OD_ACT_BOXSIZE_Y + OD_ACT_Y_SPACING;
    // Reset Odrive Button
    if (!resetOdriveFlag) {
      disp.drawButton(OD_ACT_COL_3_X + x_offset, OD_ACT_COL_3_Y + y_offset, OD_ACT_BOXSIZE_X, OD_ACT_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, OD_ACT_TEXT_X_OFFSET-6, OD_ACT_TEXT_Y_OFFSET, "Reset ODrive");
    } else {
      disp.drawButton(OD_ACT_COL_3_X + x_offset, OD_ACT_COL_3_Y + y_offset, OD_ACT_BOXSIZE_X, OD_ACT_BOXSIZE_Y, BUT_OUTLINE, BUT_ON_BGD, OD_ACT_TEXT_X_OFFSET-6, OD_ACT_TEXT_Y_OFFSET, "  Resetting ");
      resetOdriveFlag = false;
    }

    y_offset +=OD_ACT_BOXSIZE_Y + OD_ACT_Y_SPACING;
    // Enable or Disable the OD position update via UART
    if (ODpositionUpdateEnabled) {
      disp.drawButton(OD_ACT_COL_3_X + x_offset, OD_ACT_COL_3_Y + y_offset, OD_ACT_BOXSIZE_X, OD_ACT_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, OD_ACT_TEXT_X_OFFSET-2, OD_ACT_TEXT_Y_OFFSET, "Dis UpDates");
    } else {
      disp.drawButton(OD_ACT_COL_3_X + x_offset, OD_ACT_COL_3_Y + y_offset, OD_ACT_BOXSIZE_X, OD_ACT_BOXSIZE_Y, BUT_OUTLINE, BUT_ON_BGD, OD_ACT_TEXT_X_OFFSET-2, OD_ACT_TEXT_Y_OFFSET,   "Ena UpDates");
    }
  }
  screenTouched = false;
}

// =========== Odrive touchscreen update ===========
void touchOdriveUpdate() {
  int x_offset = 0;
  int y_offset = 0;
  y_offset +=OD_ACT_BOXSIZE_Y + OD_ACT_Y_SPACING;

  // ===== Column 1 - Leftmost ======
  // Enable Azimuth motor
  if (p.x > OD_ACT_COL_1_X + x_offset && p.x < OD_ACT_COL_1_X + x_offset + OD_ACT_BOXSIZE_X && p.y > OD_ACT_COL_1_Y + y_offset && p.y <  OD_ACT_COL_1_Y + y_offset + OD_ACT_BOXSIZE_Y) {
    soundClick();
    if (odriveAZOff) { // toggle ON
      odriveAZOff = false;
      digitalWrite(AZ_ENABLED_LED_PIN, LOW); // Turn On AZ LED
      turnOnOdriveMotor(AZ);
    } else { // since already ON, toggle OFF
      odriveAZOff = true;
      digitalWrite(AZ_ENABLED_LED_PIN, HIGH); // Turn Off AZ LED
      idleOdriveMotor(AZ);
    }
  }
            
  y_offset +=OD_ACT_BOXSIZE_Y + OD_ACT_Y_SPACING;
  // Enable Altitude motor
  if (p.x > OD_ACT_COL_1_X + x_offset && p.x < OD_ACT_COL_1_X + x_offset + OD_ACT_BOXSIZE_X && p.y > OD_ACT_COL_1_Y + y_offset && p.y <  OD_ACT_COL_1_Y + y_offset + OD_ACT_BOXSIZE_Y) {
    soundClick();
    if (odriveALTOff) { // toggle ON
      odriveALTOff = false;
      digitalWrite(ALT_ENABLED_LED_PIN, LOW); // Turn On ALT LED
      turnOnOdriveMotor(ALT);
    } else { // toggle OFF
      idleOdriveMotor(ALT); // Idle the Odrive channel
      odriveALTOff = true;
      digitalWrite(ALT_ENABLED_LED_PIN, HIGH); // Turn off ALT LED
    }
  }

  // Column 2
  // STOP everthing requested
  y_offset = 0;
  y_offset +=OD_ACT_BOXSIZE_Y + OD_ACT_Y_SPACING;
  if (p.x > OD_ACT_COL_2_X + x_offset && p.x < OD_ACT_COL_2_X + x_offset + OD_ACT_BOXSIZE_X && p.y > OD_ACT_COL_2_Y + y_offset && p.y <  OD_ACT_COL_2_Y + y_offset + OD_ACT_BOXSIZE_Y) {
    if (!stopButton) {
      soundClick();
      stopSlewingAndTracking(SS_LIMIT); // stop slewing, trackin, and goto
      idleOdriveMotor(AZ); // turn off the motors
      idleOdriveMotor(ALT);
      stopButton = true;
      odriveAZOff = true;
      odriveALTOff = true;
      digitalWrite(AZ_ENABLED_LED_PIN, HIGH); // Turn Off AZ LED
      digitalWrite(ALT_ENABLED_LED_PIN, HIGH); // Turn Off ALT LED
    }
  }
  
  y_offset +=OD_ACT_BOXSIZE_Y + OD_ACT_Y_SPACING;
  // Clear ODrive Errors
  if (p.x > OD_ACT_COL_2_X + x_offset && p.x < OD_ACT_COL_2_X + x_offset + OD_ACT_BOXSIZE_X && p.y > OD_ACT_COL_2_Y + y_offset && p.y <  OD_ACT_COL_2_Y + y_offset + OD_ACT_BOXSIZE_Y) {
    VLF("MSG: Clearing ODrive Errors");
    clearOdriveErrors(AZ, ENCODER);
    clearOdriveErrors(AZ, CONTROLLER);
    clearOdriveErrors(AZ, MOTOR);
    clearOdriveErrors(ALT, ENCODER);
    clearOdriveErrors(ALT, CONTROLLER);
    clearOdriveErrors(ALT, MOTOR);
    soundClick();
    clearOdriveErr = true;
  }

  // Column 3
  y_offset = -165;
  // AZ Gain HIGH
  if (p.x > OD_ACT_COL_3_X + x_offset && p.x < OD_ACT_COL_3_X + x_offset + OD_ACT_BOXSIZE_X && p.y > OD_ACT_COL_3_Y + y_offset && p.y <  OD_ACT_COL_3_Y + y_offset + OD_ACT_BOXSIZE_Y-box_height_adj) {
    soundClick();
    AZgainHigh = true;
    AZgainDefault = false;
    setOdriveVelGain(AZ, 1.8); // Set Velocity Gain
    delay(10);
    //setOdriveVelIntGain(AZ, 2.3); // Set Velocity Integrator Gain
  }

    // AZ Gain DEFault
  y_offset +=OD_ACT_BOXSIZE_Y-box_height_adj + OD_ACT_Y_SPACING;
  if (p.x > OD_ACT_COL_3_X + x_offset && p.x < OD_ACT_COL_3_X + x_offset + OD_ACT_BOXSIZE_X && p.y > OD_ACT_COL_3_Y + y_offset && p.y <  OD_ACT_COL_3_Y + y_offset + OD_ACT_BOXSIZE_Y-box_height_adj) {
    soundClick();
    AZgainHigh = false;
    AZgainDefault = true;
    setOdriveVelGain(AZ, 1.5);
    delay(10);
    //setOdriveVelIntGain(AZ, 2.0);
  }

    // ALT Gain HIGH
  y_offset +=OD_ACT_BOXSIZE_Y-box_height_adj + OD_ACT_Y_SPACING;
  if (p.x > OD_ACT_COL_3_X + x_offset && p.x < OD_ACT_COL_3_X + x_offset + OD_ACT_BOXSIZE_X && p.y > OD_ACT_COL_3_Y + y_offset && p.y <  OD_ACT_COL_3_Y + y_offset + OD_ACT_BOXSIZE_Y-box_height_adj) {
    soundClick();
    ALTgainHigh = true;
    ALTgainDefault = false;
    setOdriveVelGain(ALT, 0.5); // Set Velocity Gain
    delay(10);
    //setOdriveVelIntGain(ALT, 0.7); // Set Velocity Integrator Gain
  }

    // ALT Gain DEFault
  y_offset +=OD_ACT_BOXSIZE_Y-box_height_adj + OD_ACT_Y_SPACING;
  if (p.x > OD_ACT_COL_3_X + x_offset && p.x < OD_ACT_COL_3_X + x_offset + OD_ACT_BOXSIZE_X && p.y > OD_ACT_COL_3_Y + y_offset && p.y <  OD_ACT_COL_3_Y + y_offset + OD_ACT_BOXSIZE_Y-box_height_adj) {
    soundClick();
    ALTgainHigh = false;
    ALTgainDefault = true;
    setOdriveVelGain(ALT, 0.3);
    delay(10);
    //setOdriveVelIntGain(ALT, 0.4);
  }

  y_offset = 0;
  // Demo Mode for ODrive
  // Toggle on Demo Mode if button pressed, toggle off if pressed and already on
  // Demo mode relies on a pseudo-thread that fires off the change in positions
  if (p.x > OD_ACT_COL_3_X + x_offset && p.x < OD_ACT_COL_3_X + x_offset + OD_ACT_BOXSIZE_X && p.y > OD_ACT_COL_3_Y + y_offset && p.y <  OD_ACT_COL_3_Y + y_offset + OD_ACT_BOXSIZE_Y) {
    if (!demoActive) {
      VLF("MSG: Demo ODrive");
      soundClick();
      demoActive = true;
      demoThread.enable();
    } else {
      demoActive = false;
      VLF("MSG: Demo OFF ODrive");
      demoModeOff();
      demoThread.disable();
      soundClick();
    }
  }

  y_offset +=OD_ACT_BOXSIZE_Y + OD_ACT_Y_SPACING;
  // Reset ODRIVE
  if (p.x > OD_ACT_COL_3_X + x_offset && p.x < OD_ACT_COL_3_X + x_offset + OD_ACT_BOXSIZE_X && p.y > OD_ACT_COL_3_Y + y_offset && p.y <  OD_ACT_COL_3_Y + y_offset + OD_ACT_BOXSIZE_Y) {
    VLF("MSG: Reseting ODrive");
    digitalWrite(ODRIVE_RST, LOW);
    delay(1);
    digitalWrite(ODRIVE_RST, HIGH);
    soundClick();
    resetOdriveFlag = true;
    AZgainHigh = false;
    AZgainDefault = true;
    ALTgainHigh = false;
    ALTgainDefault = true;
  }

  // Disable / Enable ODrive motor position update - UART between Odrive and Teensy
  // Disable-position-updates so that they don't override ODrive
  // motor positions while tuning ODrive with ODrive USB channel
  y_offset +=OD_ACT_BOXSIZE_Y + OD_ACT_Y_SPACING;
  if (p.x > OD_ACT_COL_3_X + x_offset && p.x < OD_ACT_COL_3_X + x_offset + OD_ACT_BOXSIZE_X && p.y > OD_ACT_COL_3_Y + y_offset && p.y <  OD_ACT_COL_3_Y + y_offset + OD_ACT_BOXSIZE_Y) {
    soundClick();
    if (ODpositionUpdateEnabled) {
      ODpositionUpdateEnabled = false;
    } else {
      ODpositionUpdateEnabled = true;
    }
  }  
}    


int demo_pos = 0;
// ======== Demo Mode ========
void demoModeOn() {
  // choose some AZM and ALT positions in fractional "Turns"
  // ALT position should never be negative in actual use but it "can" go negative in demo
  float pos_one = 0.15;
  float pos_two = 0.3;
  float pos_three = -0.1;
  float pos_four = -0.4;
  float pos_five = 0.4;
  stopSlewingAndTracking(SS_ALL_FAST);
  updateMotorsThread.disable();
  switch(demo_pos) {
    case 0:
      odrive.SetPosition(AZ, pos_one);
      odrive.SetPosition(ALT, pos_one);
      ++demo_pos;
      break;
    case 1:
      odrive.SetPosition(AZ, pos_two);
      odrive.SetPosition(ALT, pos_two);
      ++demo_pos;
      break;
    case 2:
      odrive.SetPosition(AZ, pos_three);
      odrive.SetPosition(ALT, pos_one);
      ++demo_pos;
      break;
    case 3:
      odrive.SetPosition(AZ, pos_four);
      odrive.SetPosition(ALT, pos_five);
      demo_pos = 0;
      break;
    default:
      odrive.SetPosition(AZ, pos_one);
      odrive.SetPosition(ALT, pos_one);
      demo_pos = 0;
      break;
  }
}

void demoModeOff() {
  odrive.SetPosition(AZ, 0);
  odrive.SetPosition(ALT, 0);
  demo_pos = 0;
  updateMotorsThread.enable();
}