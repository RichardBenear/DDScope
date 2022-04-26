// =============================================
//============ Display HOME page ===============
//==============================================
// Author: Richard Benear 3/20/21

// Column 1 Home Page
#define COL1_LABELS_X            3
#define COL1_LABELS_Y            168
#define COL1_LABEL_SPACING       18
#define COL1_DATA_BOXSIZE_X      70
#define COL1_DATA_X              84
#define COL1_DATA_Y              COL1_LABELS_Y

// Column 2 Home Page
#define COL2_LABELS_X            170
#define COL2_LABELS_Y            COL1_LABELS_Y
#define COL2_LABEL_SPACING       COL1_LABEL_SPACING
#define COL2_DATA_X              269
#define COL2_DATA_Y              COL1_DATA_Y
#define COL2_DATA_BOXSIZE_X      70
#define COL2_DATA_BOXSIZE_Y      COL1_LABEL_SPACING

// Buttons for actions that are not page selections
#define ACTION_BOXSIZE_X         100 
#define ACTION_BOXSIZE_Y         36 
#define ACTION_COL_1_X           3 
#define ACTION_COL_1_Y           324
#define ACTION_COL_2_X           ACTION_COL_1_X+ACTION_BOXSIZE_X+4
#define ACTION_COL_2_Y           ACTION_COL_1_Y
#define ACTION_COL_3_X           ACTION_COL_2_X+ACTION_BOXSIZE_X+4
#define ACTION_COL_3_Y           ACTION_COL_1_Y
#define ACTION_X_SPACING         7
#define ACTION_Y_SPACING         4
#define ACTION_TEXT_X_OFFSET     10
#define ACTION_TEXT_Y_OFFSET     20

#define AZ                       1 // Odrive Motor 1
#define ALT                      0 // Odrive Motor 0

#define MOTOR_CURRENT_WARNING  2.0  // Warning when over 2 amps....coil heating occuring

float currentBatVoltage   = 0.2; // start with something besides 0 for debug purposes
float lastBatVoltage      = 0.00;

float currentAZEncPos     = 0.00;
float lastAZEncPos        = 11.11;
float currentALTEncPos    = 0.00;
float lastALTEncPos       = 22.22;
float currentAZMotorCur   = 0.11;
float lastAZMotorCur      = 0.11;
float currentALTMotorCur  = 0.11;
float lastALTMotorCur     = 0.11;
float currentALTMotorTemp = 0.00;
float lastALTMotorTemp    = 2.22;
float currentAZMotorTemp  = 0.00;
float lastAZMotorTemp     = 2.22;
char curLatitude[10]      = "";
char curLongitude[10]     = "";
char curTime[10]          = "";
char curLST[10]           = "";
char curTemp[10]          = "";
char curHumidity[10]      = "";
char curDewpoint[10]      = "";
char curAlti[10]          = "";
bool parkWasSet = false;
bool stopButton = false;
bool gotoHome = false;
bool fanOn = false;
bool firstTimeLstT0 = true; // update cat_mgr LST once

// ============================================
// ======= Draw Base content of HOME PAGE =====
// ============================================
void drawHomePage() {
  currentPage = HOME_PAGE;
  tft.setTextSize(1);
  disp.updateColors();
  tft.setTextColor(TEXT_COLOR);
  tft.fillScreen(PAGE_BACKGROUND);
  disp.drawMenuButtons();
  disp.drawTitle(20, 30, "DIRECT-DRIVE SCOPE");
  tft.drawFastVLine(165, 155, 165,TEXT_COLOR);
  tft.setFont(&Inconsolata_Bold8pt7b); 
  disp.drawCommonStatusLabels();
  disp.updateOnstepCmdStatus();
  
  //========== Status Text ===========
  // Draw Status Labels for Real Time data only here, no data displayed
  int y_offset = 0;
  
  // Show Current Local Time
  tft.setCursor(COL1_LABELS_X, COL1_LABELS_Y + y_offset);
  tft.print("Time-----:");

  // Show LST
  y_offset +=COL1_LABEL_SPACING;
  tft.setCursor(COL1_LABELS_X, COL1_LABELS_Y + y_offset);
  tft.print("LST------:");
  
  // Display Latitude
  y_offset +=COL1_LABEL_SPACING;
  tft.setCursor(COL1_LABELS_X, COL1_LABELS_Y + y_offset);
  tft.print("Latitude-:");

  // Display Longitude
  y_offset +=COL1_LABEL_SPACING;
  tft.setCursor(COL1_LABELS_X, COL1_LABELS_Y + y_offset);
  tft.print("Longitude:");

  // Display ambient Temperature
  y_offset +=COL1_LABEL_SPACING;
  tft.setCursor(COL1_LABELS_X, COL1_LABELS_Y + y_offset);
  tft.print("Temperat-:");

  // Display ambient Humidity
  y_offset +=COL1_LABEL_SPACING;
  tft.setCursor(COL1_LABELS_X, COL1_LABELS_Y + y_offset);
  tft.print("Humidity-:");

  // Display Dew Point
  y_offset +=COL1_LABEL_SPACING;
  tft.setCursor(COL1_LABELS_X, COL1_LABELS_Y + y_offset);
  tft.print("Dew Point:");
  
  // Display Altitude
  y_offset +=COL1_LABEL_SPACING;
  tft.setCursor(COL1_LABELS_X, COL1_LABELS_Y + y_offset);
  tft.print("Altitude-:");

  // Battery Voltage
  y_offset +=COL1_LABEL_SPACING;
  tft.setCursor(COL1_LABELS_X, COL1_LABELS_Y + y_offset);
  tft.print("Battery V:");

//======= 2nd Column =======
// Motor encoder positions
  y_offset = 0;
  tft.setCursor(COL2_LABELS_X, COL2_LABELS_Y + y_offset);
  tft.print("AZM enc deg:");

  y_offset +=COL2_LABEL_SPACING;
  tft.setCursor(COL2_LABELS_X, COL2_LABELS_Y + y_offset);
  tft.print("ALT enc deg:");

  // Motor currents
  y_offset +=COL2_LABEL_SPACING;
  tft.setCursor(COL2_LABELS_X, COL2_LABELS_Y + y_offset);
  tft.print("AZM Ibus---:");

  y_offset +=COL2_LABEL_SPACING;
  tft.setCursor(COL2_LABELS_X, COL2_LABELS_Y + y_offset);
  tft.print("ALT Ibus---:");

  // ALT Motor Temperature
  y_offset +=COL2_LABEL_SPACING;
  tft.setCursor(COL2_LABELS_X, COL2_LABELS_Y + y_offset);
  tft.print("ALT MotTemp:");

   // AZ Motor Temperature
  y_offset +=COL2_LABEL_SPACING;
  tft.setCursor(COL2_LABELS_X, COL2_LABELS_Y + y_offset);
  tft.print("AZM MotTemp:");
}

// =================================================
// ============ Update HOME Page Status ============
// =================================================
// Updates here occur on the psuedo-thread timer (updateStatusThread)
// Column 1 
void updateHomeStatus() {
  char xchReply[10]="";
  int y_offset = 0; 

  // Show Local Time
  disp.getLocalCmdTrim(":GL#", xchReply); 
  if (strcmp(curTime, xchReply) !=0 || firstDraw) {
    if (tls.active) {
      disp.canvPrint(COL1_DATA_X, COL1_DATA_Y, y_offset, C_WIDTH-5, C_HEIGHT, BUT_OUTLINE, BUT_BACKGROUND, xchReply);
    } else {
      disp.canvPrint(COL1_DATA_X, COL1_DATA_Y, y_offset, C_WIDTH-5, C_HEIGHT, BUT_OUTLINE, BUT_ON_BGD, xchReply);
    }
    strcpy(curTime, xchReply);
  }

  // Show LST
  y_offset +=COL1_LABEL_SPACING;
  disp.getLocalCmdTrim(":GS#", xchReply); 
  if (strcmp(curLST, xchReply)!=0 || firstDraw) { 
    if (tls.active) {
      disp.canvPrint(COL1_DATA_X, COL1_DATA_Y, y_offset, C_WIDTH-5, C_HEIGHT, BUT_OUTLINE, BUT_BACKGROUND, xchReply);
    } else {
      disp.canvPrint(COL1_DATA_X, COL1_DATA_Y, y_offset, C_WIDTH-5, C_HEIGHT, BUT_OUTLINE, BUT_ON_BGD, xchReply);
    }
    strcpy(curLST, xchReply);
  }

  y_offset +=COL1_LABEL_SPACING;
  // Show Latitude
  disp.getLocalCmdTrim(":Gt#", xchReply); 
  if (strcmp(curLatitude, xchReply)!=0 || firstDraw) {
    if (tls.active) {
      disp.canvPrint(COL1_DATA_X, COL1_DATA_Y, y_offset, C_WIDTH-5, C_HEIGHT, BUT_OUTLINE, BUT_BACKGROUND, xchReply);
    } else {
      disp.canvPrint(COL1_DATA_X, COL1_DATA_Y, y_offset, C_WIDTH-5, C_HEIGHT, BUT_OUTLINE, BUT_ON_BGD, xchReply);
    }
    strcpy(curLatitude, xchReply);
  }

  // Show Longitude
  y_offset +=COL1_LABEL_SPACING;
  disp.getLocalCmdTrim(":Gg#", xchReply); 
  if (strcmp(curLongitude, xchReply)!=0 || firstDraw) {
    if (tls.active) {
      disp.canvPrint(COL1_DATA_X, COL1_DATA_Y, y_offset, C_WIDTH-5, C_HEIGHT, BUT_OUTLINE, BUT_BACKGROUND, xchReply);
    } else {
      disp.canvPrint(COL1_DATA_X, COL1_DATA_Y, y_offset, C_WIDTH-5, C_HEIGHT, BUT_OUTLINE, BUT_ON_BGD, xchReply);
    }
    strcpy(curLongitude, xchReply);
  }

  // Ambient Temperature
  y_offset +=COL1_LABEL_SPACING;
  disp.getLocalCmdTrim(":GX9A#", xchReply); 
  double tempF = ((atof(xchReply)*9)/5) + 32;
  sprintf(xchReply, "%3.1f F", tempF); // convert back to string to right justify
  if (strcmp(curTemp, xchReply)!=0 || firstDraw) {
    disp.canvPrint(COL1_DATA_X, COL1_DATA_Y, y_offset, C_WIDTH-5, C_HEIGHT, BUT_OUTLINE, BUT_BACKGROUND, xchReply);
    strcpy(curTemp, xchReply);
  }
   
  // Ambient Humidity
  y_offset +=COL1_LABEL_SPACING;
  disp.getLocalCmdTrim(":GX9C#", xchReply); 
  if (strcmp(curHumidity, xchReply)!=0 || firstDraw) {
    disp.canvPrint(COL1_DATA_X, COL1_DATA_Y, y_offset, C_WIDTH-5, C_HEIGHT, BUT_OUTLINE, BUT_BACKGROUND, xchReply);
    strcpy(curHumidity, xchReply);
  }
  
  // Ambient Dew Point
  y_offset +=COL1_LABEL_SPACING;
  disp.getLocalCmdTrim(":GX9E#", xchReply); 
  if (strcmp(curDewpoint, xchReply)!=0 || firstDraw) {
    disp.canvPrint(COL1_DATA_X, COL1_DATA_Y, y_offset, C_WIDTH-5, C_HEIGHT, BUT_OUTLINE, BUT_BACKGROUND, xchReply);
    strcpy(curDewpoint, xchReply);
  }  
    
  // Show Altitude
  y_offset +=COL1_LABEL_SPACING;
  disp.getLocalCmdTrim(":GX9D#", xchReply); 
  if (strcmp(curAlti, xchReply)!=0 || firstDraw) {
    disp.canvPrint(COL1_DATA_X, COL1_DATA_Y, y_offset, C_WIDTH-5, C_HEIGHT, BUT_OUTLINE, BUT_BACKGROUND, xchReply);
    strcpy(curAlti, xchReply);
  }

  // Update Battery Voltage
  y_offset +=COL1_LABEL_SPACING;
  currentBatVoltage = disp.getBatteryVoltage();
  if ((currentBatVoltage != lastBatVoltage) || firstDraw) {
    if (currentBatVoltage < BATTERY_LOW_VOLTAGE) {
      disp.canvPrint(COL1_DATA_X, COL1_DATA_Y, y_offset, C_WIDTH-5, C_HEIGHT, BUT_OUTLINE, BUT_ON_BGD, currentBatVoltage);
    } else {
      disp.canvPrint(COL1_DATA_X, COL1_DATA_Y, y_offset, C_WIDTH-5, C_HEIGHT, BUT_OUTLINE, BUT_BACKGROUND, currentBatVoltage);
    }
    lastBatVoltage = currentBatVoltage;
  }

// =================================================
// ============ Update Column 2 Status =============
// =================================================
  // Show ODrive encoder positions
  // AZ encoder
  y_offset =0;
  int bitmap_width_sub = 30;
  currentAZEncPos = getEncoderPositionDeg(AZ);
  if ((currentAZEncPos != lastAZEncPos) || firstDraw) {
    disp.canvPrint(COL2_DATA_X, COL2_DATA_Y, y_offset, C_WIDTH-bitmap_width_sub, C_HEIGHT, BUT_OUTLINE, BUT_BACKGROUND, currentAZEncPos);
    lastAZEncPos = currentAZEncPos;
  }
  
  // ALT encoder
  y_offset +=COL1_LABEL_SPACING;
  currentALTEncPos = getEncoderPositionDeg(ALT);
  if ((currentALTEncPos != lastALTEncPos) || firstDraw) {
    disp.canvPrint(COL2_DATA_X, COL2_DATA_Y, y_offset, C_WIDTH-bitmap_width_sub, C_HEIGHT, BUT_OUTLINE, BUT_BACKGROUND, currentALTEncPos);
    lastALTEncPos = currentALTEncPos;
  }

  // Show ODrive motor currents
  // AZ current
  y_offset +=COL1_LABEL_SPACING;
  currentAZMotorCur = getMotorCurrent(AZ);
  if ((currentAZMotorCur != lastAZMotorCur) || firstDraw) {
    if (lastAZMotorCur > MOTOR_CURRENT_WARNING) {
      disp.canvPrint(COL2_DATA_X, COL2_DATA_Y, y_offset, C_WIDTH-bitmap_width_sub, C_HEIGHT, BUT_OUTLINE, BUT_ON_BGD, currentAZMotorCur);
    } else {
      disp.canvPrint(COL2_DATA_X, COL2_DATA_Y, y_offset, C_WIDTH-bitmap_width_sub, C_HEIGHT, BUT_OUTLINE, BUT_BACKGROUND, currentAZMotorCur);
    }
  lastAZMotorCur = currentAZMotorCur;
  }
  
  // ALT current
  y_offset +=COL1_LABEL_SPACING;
  currentALTMotorCur = getMotorCurrent(ALT);
  if ((currentALTMotorCur != lastALTMotorCur) || firstDraw) {
    if (lastALTMotorCur > MOTOR_CURRENT_WARNING) {
      disp.canvPrint(COL2_DATA_X, COL2_DATA_Y, y_offset, C_WIDTH-bitmap_width_sub, C_HEIGHT, BUT_OUTLINE, BUT_ON_BGD, currentALTMotorCur);
    } else {
      disp.canvPrint(COL2_DATA_X, COL2_DATA_Y, y_offset, C_WIDTH-bitmap_width_sub, C_HEIGHT, BUT_OUTLINE, BUT_BACKGROUND, currentALTMotorCur);
    }
  lastALTMotorCur = currentALTMotorCur;
  }
 
  // ALT Motor Temperature
  y_offset +=COL1_LABEL_SPACING;
  currentALTMotorTemp = getMotorTemp(ALT);
  if ((currentALTMotorTemp != lastALTMotorTemp) || firstDraw) {
    if (currentALTMotorTemp >= 120) { // make box red
      disp.canvPrint(COL2_DATA_X, COL2_DATA_Y, y_offset, C_WIDTH-bitmap_width_sub, C_HEIGHT, BUT_OUTLINE, BUT_ON_BGD, currentALTMotorTemp);
    } else {
      disp.canvPrint(COL2_DATA_X, COL2_DATA_Y, y_offset, C_WIDTH-bitmap_width_sub, C_HEIGHT, BUT_OUTLINE, BUT_BACKGROUND, currentALTMotorTemp);
    }
    lastALTMotorTemp = currentALTMotorTemp;
  }

  // AZ Motor Temperature
  y_offset +=COL1_LABEL_SPACING;
  currentAZMotorTemp = getMotorTemp(AZ);
  if ((currentAZMotorTemp != lastAZMotorTemp) || firstDraw) {
    if (currentAZMotorTemp >= 120) { // make box red
    disp.canvPrint(COL2_DATA_X, COL2_DATA_Y, y_offset, C_WIDTH-bitmap_width_sub, C_HEIGHT, BUT_OUTLINE, BUT_ON_BGD, currentAZMotorTemp);
    } else {
      disp.canvPrint(COL2_DATA_X, COL2_DATA_Y, y_offset, C_WIDTH-bitmap_width_sub, C_HEIGHT, BUT_OUTLINE, BUT_BACKGROUND, currentAZMotorTemp);
    }
    lastAZMotorTemp = currentAZMotorTemp;
  }

  // =================================================
  // ============== Update MOUNT Status ==============
  // =================================================
  int x_offset = 10; // offset this to make easier to pick it up with the eye
  y_offset +=COL1_LABEL_SPACING+1;
  disp.getLocalCmdTrim(":GU#", xchReply); // Get telescope status with reply

  // Parking Status
  if (strstr(xchReply,"P")) {
    disp.canvPrint(COL2_LABELS_X+x_offset, COL2_LABELS_Y, y_offset, C_WIDTH+30, C_HEIGHT, BUT_OUTLINE, BUT_BACKGROUND, " Parked    ");
  } else { 
    disp.canvPrint(COL2_LABELS_X+x_offset, COL2_LABELS_Y, y_offset, C_WIDTH+30, C_HEIGHT, BUT_OUTLINE, BUT_BACKGROUND, " Not Parked");
  } 
  
  // Slewing
  y_offset +=COL1_LABEL_SPACING;
  if (strstr(xchReply,"N")) {
    disp.canvPrint(COL2_LABELS_X+x_offset, COL2_LABELS_Y, y_offset, C_WIDTH+30, C_HEIGHT, BUT_OUTLINE, BUT_BACKGROUND, " Not Slewing");
  } else {
    disp.canvPrint(COL2_LABELS_X+x_offset, COL2_LABELS_Y, y_offset, C_WIDTH+30, C_HEIGHT, BUT_OUTLINE, BUT_BACKGROUND, "  Slewing   ");
  }

  // Homing Status
  y_offset +=COL1_LABEL_SPACING;
  if (strstr(xchReply,"H")) {
    disp.canvPrint(COL2_LABELS_X+x_offset, COL2_LABELS_Y, y_offset, C_WIDTH+30, C_HEIGHT, BUT_OUTLINE, BUT_BACKGROUND, " Homed  ");   
  } else { 
    disp.canvPrint(COL2_LABELS_X+x_offset, COL2_LABELS_Y, y_offset, C_WIDTH+30, C_HEIGHT, BUT_OUTLINE, BUT_BACKGROUND, "Not Home");
  }
  disp.updateCommonStatus();
}

// =================================================
// ============ Update Action Buttons ==============
// =================================================
void updateHomeActionButtons() {
  if (screenTouched || firstDraw || refreshScreen) {
    refreshScreen = false;
    if (screenTouched) refreshScreen = true;
    char xchReply[10]="";
    int x_offset = 0;
    int y_offset = 0;
    tft.setTextColor(TEXT_COLOR);
    
    // ============== Column 1 ===============
    // Enable / Disable Azimuth Motor
    if (odriveAZOff) {
      disp.drawButton(ACTION_COL_1_X + x_offset, ACTION_COL_1_Y + y_offset, ACTION_BOXSIZE_X, ACTION_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, ACTION_TEXT_X_OFFSET, ACTION_TEXT_Y_OFFSET, "  EN AZ   ");
    } else { //motor on
      disp.drawButton(ACTION_COL_1_X + x_offset, ACTION_COL_1_Y + y_offset, ACTION_BOXSIZE_X, ACTION_BOXSIZE_Y, BUT_OUTLINE, BUT_ON_BGD, ACTION_TEXT_X_OFFSET, ACTION_TEXT_Y_OFFSET,   "AZ Enabled");
    }
    // Enable / Disable Altitude Motor
    y_offset +=ACTION_BOXSIZE_Y + ACTION_Y_SPACING;
    if (odriveALTOff) {
      disp.drawButton(ACTION_COL_1_X + x_offset, ACTION_COL_1_Y + y_offset, ACTION_BOXSIZE_X, ACTION_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, ACTION_TEXT_X_OFFSET, ACTION_TEXT_Y_OFFSET, "  EN ALT   ");
    } else { //motor on
      disp.drawButton(ACTION_COL_1_X + x_offset, ACTION_COL_1_Y + y_offset, ACTION_BOXSIZE_X, ACTION_BOXSIZE_Y, BUT_OUTLINE, BUT_ON_BGD, ACTION_TEXT_X_OFFSET-2, ACTION_TEXT_Y_OFFSET,   "ALT Enabled");
    }
    // Stop all movement
    y_offset +=ACTION_BOXSIZE_Y + ACTION_Y_SPACING;
    if (stopButton) {
      disp.drawButton(ACTION_COL_1_X + x_offset, ACTION_COL_1_Y + y_offset, ACTION_BOXSIZE_X, ACTION_BOXSIZE_Y, BUT_OUTLINE, BUT_ON_BGD, ACTION_TEXT_X_OFFSET, ACTION_TEXT_Y_OFFSET,    "AllStopped");
      stopButton = false;
    } else { 
      disp.drawButton(ACTION_COL_1_X + x_offset, ACTION_COL_1_Y + y_offset, ACTION_BOXSIZE_X, ACTION_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, ACTION_TEXT_X_OFFSET+5, ACTION_TEXT_Y_OFFSET, "  STOP!  ");
    }

    // ============== Column 2 ===============
    y_offset = 0;
    disp.getLocalCmdTrim(":GU#", xchReply); // Get telescope status with reply
    // Start / Stop Tracking
    if (strstr(xchReply,"n")) { 
      disp.drawButton(ACTION_COL_2_X + x_offset, ACTION_COL_2_Y + y_offset, ACTION_BOXSIZE_X, ACTION_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, ACTION_TEXT_X_OFFSET-2, ACTION_TEXT_Y_OFFSET, "Start Track");
    } else { 
      disp.drawButton(ACTION_COL_2_X + x_offset, ACTION_COL_2_Y + y_offset, ACTION_BOXSIZE_X, ACTION_BOXSIZE_Y, BUT_OUTLINE, BUT_ON_BGD, ACTION_TEXT_X_OFFSET, ACTION_TEXT_Y_OFFSET,     " Tracking  ");
    }
    
    // Night / Day Mode
    y_offset +=ACTION_BOXSIZE_Y + ACTION_Y_SPACING;
    if (!nightMode) {
      disp.drawButton(ACTION_COL_2_X + x_offset, ACTION_COL_2_Y + y_offset, ACTION_BOXSIZE_X, ACTION_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, ACTION_TEXT_X_OFFSET, ACTION_TEXT_Y_OFFSET, "Night Mode");   
    } else {
      disp.drawButton(ACTION_COL_2_X + x_offset, ACTION_COL_2_Y + y_offset, ACTION_BOXSIZE_X, ACTION_BOXSIZE_Y, BUT_OUTLINE, BUT_ON_BGD, ACTION_TEXT_X_OFFSET, ACTION_TEXT_Y_OFFSET, " Day Mode");          
    }
    // Home Telescope
    y_offset +=ACTION_BOXSIZE_Y + ACTION_Y_SPACING;
    if (gotoHome) {
      disp.drawButton(ACTION_COL_2_X + x_offset, ACTION_COL_2_Y + y_offset, ACTION_BOXSIZE_X, ACTION_BOXSIZE_Y, BUT_OUTLINE, BUT_ON_BGD, ACTION_TEXT_X_OFFSET, ACTION_TEXT_Y_OFFSET,   "  Homing ");
      gotoHome = false;             
    } else {
      disp.drawButton(ACTION_COL_2_X + x_offset, ACTION_COL_2_Y + y_offset, ACTION_BOXSIZE_X, ACTION_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, ACTION_TEXT_X_OFFSET-2, ACTION_TEXT_Y_OFFSET, "  Go Home ");
    }  

    // ============== Column 3 ===============
    // Park / unPark Telescope
    y_offset = 0;
    if (strstr(xchReply,"p")) { 
      disp.drawButton(ACTION_COL_3_X + x_offset, ACTION_COL_3_Y + y_offset, ACTION_BOXSIZE_X, ACTION_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, ACTION_TEXT_X_OFFSET-2, ACTION_TEXT_Y_OFFSET, "  Go Park ");
    } else if (strstr(xchReply,"P")) { 
      disp.drawButton(ACTION_COL_3_X + x_offset, ACTION_COL_3_Y + y_offset, ACTION_BOXSIZE_X, ACTION_BOXSIZE_Y, BUT_OUTLINE, BUT_ON_BGD, ACTION_TEXT_X_OFFSET, ACTION_TEXT_Y_OFFSET,   "  Un Park ");
    }

    // Set Park Position
    y_offset +=ACTION_BOXSIZE_Y + ACTION_Y_SPACING;
    if (parkWasSet) {
      disp.drawButton(ACTION_COL_3_X + x_offset, ACTION_COL_3_Y + y_offset, ACTION_BOXSIZE_X, ACTION_BOXSIZE_Y, BUT_OUTLINE, BUT_ON_BGD, ACTION_TEXT_X_OFFSET, ACTION_TEXT_Y_OFFSET,     "Park WasSet");
      parkWasSet = false;
    } else {
      disp.drawButton(ACTION_COL_3_X + x_offset, ACTION_COL_3_Y + y_offset, ACTION_BOXSIZE_X, ACTION_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, ACTION_TEXT_X_OFFSET-7, ACTION_TEXT_Y_OFFSET, "  Set Park ");
    }
    // Turn ON / OFF Fan
    y_offset +=ACTION_BOXSIZE_Y + ACTION_Y_SPACING;
    if (!fanOn) {
      disp.drawButton(ACTION_COL_3_X + x_offset, ACTION_COL_3_Y + y_offset, ACTION_BOXSIZE_X, ACTION_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, ACTION_TEXT_X_OFFSET-2, ACTION_TEXT_Y_OFFSET, "  Fan Off ");
    } else {
      disp.drawButton(ACTION_COL_3_X + x_offset, ACTION_COL_3_Y + y_offset, ACTION_BOXSIZE_X, ACTION_BOXSIZE_Y, BUT_OUTLINE, BUT_ON_BGD, ACTION_TEXT_X_OFFSET-2, ACTION_TEXT_Y_OFFSET,   "  Fan On  ");
    }
    screenTouched = false;
  }
}

// =================================================
// ============== Update Touchscreen ===============
// =================================================
void touchHomeUpdate() {
  int x_offset = 0;
  int y_offset = 0;
  
  // ======= Column 1 - Leftmost =======
  // Enable Azimuth motor
  if (p.x > ACTION_COL_1_X + x_offset && p.x < ACTION_COL_1_X + x_offset + ACTION_BOXSIZE_X && p.y > ACTION_COL_1_Y + y_offset && p.y <  ACTION_COL_1_Y + y_offset + ACTION_BOXSIZE_Y) {
    soundClick();
    if (odriveAZOff) { // toggle ON
      odriveAZOff = false; // false = NOT off
      digitalWrite(AZ_ENABLED_LED_PIN, LOW); // Turn On AZ LED
      turnOnOdriveMotor(AZ);
    } else { // since already ON, toggle OFF
      odriveAZOff = true;
      digitalWrite(AZ_ENABLED_LED_PIN, HIGH); // Turn Off AZ LED
      idleOdriveMotor(AZ);
    }
  }
            
  y_offset +=ACTION_BOXSIZE_Y + ACTION_Y_SPACING;
  // Enable Altitude motor
  if (p.x > ACTION_COL_1_X + x_offset && p.x < ACTION_COL_1_X + x_offset + ACTION_BOXSIZE_X && p.y > ACTION_COL_1_Y + y_offset && p.y <  ACTION_COL_1_Y + y_offset + ACTION_BOXSIZE_Y) {
    soundClick();
    if (odriveALTOff) { // toggle ON
      odriveALTOff = false; // false = NOT off
      digitalWrite(ALT_ENABLED_LED_PIN, LOW); // Turn On ALT LED
      turnOnOdriveMotor(ALT);
    } else { // toggle OFF
      idleOdriveMotor(ALT); // Idle the Odrive channel
      odriveALTOff = true;
      digitalWrite(ALT_ENABLED_LED_PIN, HIGH); // Turn off ALT LED
    }
  }

  // STOP everthing requested
  y_offset +=ACTION_BOXSIZE_Y + ACTION_Y_SPACING;
  if (p.x > ACTION_COL_1_X + x_offset && p.x < ACTION_COL_1_X + x_offset + ACTION_BOXSIZE_X && p.y > ACTION_COL_1_Y + y_offset && p.y <  ACTION_COL_1_Y + y_offset + ACTION_BOXSIZE_Y) {
    if (!stopButton) {
      soundClick();
      stopButton = true;
      stopMotors();
    }
  }
  // ======= COLUMN 2 of Buttons - Middle =========
  // Start/Stop Tracking
  y_offset = 0;
  if (p.x > ACTION_COL_2_X + x_offset && p.x < ACTION_COL_2_X + x_offset + ACTION_BOXSIZE_X && p.y > ACTION_COL_2_Y + y_offset && p.y <  ACTION_COL_2_Y + y_offset + ACTION_BOXSIZE_Y) {
    soundClick();
    if (trackingState == TrackingNone) {
      disp.setLocalCmd(":Te#"); // Enable Tracking
    } else {
      disp.setLocalCmd(":Td#"); // Disable Tracking
    } 
  }

  // Set Night or Day Mode
  y_offset +=ACTION_BOXSIZE_Y + ACTION_Y_SPACING;
  if (p.x > ACTION_COL_2_X + x_offset && p.x < ACTION_COL_2_X + x_offset + ACTION_BOXSIZE_X && p.y > ACTION_COL_2_Y + y_offset && p.y <  ACTION_COL_2_Y + y_offset + ACTION_BOXSIZE_Y) {
    soundClick();
    if (!nightMode) {
    nightMode = true; // toggle on
    } else {
    nightMode = false; // toggle off
    }
    disp.updateColors();
    firstDraw = true;
    drawHomePage(); return;
  }
  
  // Go to Home Telescope 
  y_offset +=ACTION_BOXSIZE_Y + ACTION_Y_SPACING;
  if (p.x > ACTION_COL_2_X + x_offset && p.x < ACTION_COL_2_X + x_offset + ACTION_BOXSIZE_X && p.y > ACTION_COL_2_Y + y_offset && p.y <  ACTION_COL_2_Y + y_offset + ACTION_BOXSIZE_Y) {
    soundClick();
    disp.setLocalCmd(":hC#"); // go HOME
    gotoHome = true;
  }
  
  // ======== COLUMN 3 of Buttons - Leftmost ========
  // Park and UnPark Telescope
  y_offset = 0;
  if (p.x > ACTION_COL_3_X + x_offset && p.x < ACTION_COL_3_X + x_offset + ACTION_BOXSIZE_X && p.y > ACTION_COL_3_Y + y_offset && p.y <  ACTION_COL_3_Y + y_offset + ACTION_BOXSIZE_Y) {
    soundClick();
    if (parkStatus == NotParked) { 
      disp.setLocalCmd(":hP#"); // go Park
    } else if (parkStatus == Parked) {
      disp.setLocalCmd(":hR#"); // Un park position
    }
  }

  // Set Park Position to Current
  y_offset +=ACTION_BOXSIZE_Y + ACTION_Y_SPACING;
  if (p.x > ACTION_COL_3_X + x_offset && p.x < ACTION_COL_3_X + x_offset + ACTION_BOXSIZE_X && p.y > ACTION_COL_3_Y + y_offset && p.y <  ACTION_COL_3_Y + y_offset + ACTION_BOXSIZE_Y) {
    soundClick();
    disp.setLocalCmd(":hQ#"); // Set Park Position
    parkWasSet = true;
  }

  // Fan Control Action Button
  y_offset +=ACTION_BOXSIZE_Y + ACTION_Y_SPACING;
  if (p.x > ACTION_COL_3_X + x_offset && p.x < ACTION_COL_3_X + x_offset + ACTION_BOXSIZE_X && p.y > ACTION_COL_3_Y + y_offset && p.y <  ACTION_COL_3_Y + y_offset + ACTION_BOXSIZE_Y) {
    soundClick();
    if (!fanOn) {
    digitalWrite(FanOnPin, HIGH);
    fanOn = true;
    } else {
    digitalWrite(FanOnPin, LOW);
    fanOn = false;
    }
  }
}