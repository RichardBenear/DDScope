// ==============================================
// ========== Extended Status Page ==============
// ==============================================
// Author: Richard Benear
// 8/30/2021

#define STATUS_BOXSIZE_X         53 
#define STATUS_BOXSIZE_Y         27 
#define STATUS_BOX_X             40 
#define STATUS_BOX_Y            150 
#define STATUS_X                  3 
#define STATUS_Y                101 
#define STATUS_SPACING           16 

char TelStatus[20];
char* stringReply;

// ========== Draw the Extended Status Page ==========
void drawXStatusPage() {
  disp.updateColors();
  tft.setTextColor(TEXT_COLOR);
  tft.fillScreen(PAGE_BACKGROUND);
  currentPage = XSTATUS_PAGE;
  disp.drawMenuButtons();
  disp.drawTitle(48, 30, "Extended Status");
  updateExtStatus(); // this is one-time update
  disp.updateOnstepCmdStatus();
}

// ======== Update Extended Status Page status ========
void updateExtStatus() {
  int y_offset = STATUS_Y;
  int y_spacer = STATUS_SPACING;
  char xchanReply[50];
  char localTime[20];

  tft.setCursor(STATUS_X, STATUS_Y); 
  tft.fillRect(STATUS_X, STATUS_Y, 150, 200, PAGE_BACKGROUND);
  
  disp.getLocalCmdTrim(":GU#", xchanReply); // Get telescope status with reply
  tft.print("Return String: "); tft.print(xchanReply);

  y_offset +=y_spacer; tft.setCursor(STATUS_X, y_offset);  
  char tempBuf[14]; sprintf(tempBuf,"FW ver:%i.%i%s",FirmwareVersionMajor,FirmwareVersionMinor,FirmwareVersionPatch);
  tft.print(tempBuf);
  
  y_offset +=y_spacer; tft.setCursor(STATUS_X, y_offset);   
  if (strstr(xchanReply,"n")) tft.print("Not Tracking"); else tft.print("Tracking    ");

  y_offset +=y_spacer; tft.setCursor(STATUS_X, y_offset);     
  if (strstr(xchanReply,"N")) tft.print("Not Slewing"); else tft.print("Slewing    ");

  y_offset +=y_spacer; tft.setCursor(STATUS_X, y_offset);  
  if      (strstr(xchanReply,"p")) tft.print("Not Parked        ");
  else if (strstr(xchanReply,"I")) tft.print("Parking in process");
  else if (strstr(xchanReply,"P")) tft.print("Parked            ");
  else if (strstr(xchanReply,"F")) tft.print("Parking Failed    ");
  else                             tft.print("ERROR             ");
  
  y_offset +=y_spacer; tft.setCursor(STATUS_X, y_offset);  
  if (strstr(xchanReply,"H")) tft.print("Homed    "); else tft.print("Not Homed");

  y_offset +=y_spacer; tft.setCursor(STATUS_X, y_offset);  
  if (strstr(xchanReply,"S")) tft.print("PPS Synched    "); else tft.print("PPS Not Synched");
  
  y_offset +=y_spacer; tft.setCursor(STATUS_X, y_offset);  
  if (strstr(xchanReply,"G")) tft.print("Pulse Guide Active  "); else tft.print("Pulse Guide Inactive");

  y_offset +=y_spacer; tft.setCursor(STATUS_X, y_offset);  
  if (strstr(xchanReply,"g")) tft.print("Guide Active    "); else tft.print("Guiding Inactive");

  y_offset +=y_spacer; tft.setCursor(STATUS_X, y_offset);  
  if (strstr(xchanReply,"w")) tft.print("Waiting At Home    "); else tft.print("Not Waiting At Home");

  y_offset +=y_spacer; tft.setCursor(STATUS_X, y_offset);  
  if (strstr(xchanReply,"u")) tft.print("Pause At Home Enabled      "); else tft.print("Pausing At Home Not Enabled");

  y_offset +=y_spacer; tft.setCursor(STATUS_X, y_offset);  
  if (strstr(xchanReply,"z")) tft.print("Buzzer Enabled "); else tft.print("Buzzer Disabled");

  y_offset +=y_spacer; tft.setCursor(STATUS_X, y_offset);
  if (strstr(xchanReply,"A")) tft.print("ALT AZ Mount"); else tft.print("Wrong Mount ");

  y_offset +=y_spacer; tft.setCursor(STATUS_X, y_offset);  
  tft.print("Pulse Guide Rate = "); tft.print(getPulseGuideRate());

  y_offset +=y_spacer; tft.setCursor(STATUS_X, y_offset);  
  tft.print("Guide Rate = "); tft.print(getGuideRate());

  y_offset +=y_spacer; tft.setCursor(STATUS_X, y_offset);  
  tft.print("Time Zone = "); tft.print(timeZone);

  y_offset +=y_spacer; tft.setCursor(STATUS_X, y_offset);  
  tft.print("UT1 = "); tft.print(UT1);

  y_offset +=y_spacer; tft.setCursor(STATUS_X, y_offset);  
  tft.print("Local Time = "); 
  disp.getLocalCmdTrim(":GL#", localTime);
  tft.print(localTime);

  y_offset +=y_spacer; tft.setCursor(STATUS_X, y_offset);  
  tft.print("General Error = "); 
  if (generalError == 0) tft.print("ERR_NONE         "); else
  if (generalError == 1) tft.print("ERR_MOTOR_FAULT  "); else
  if (generalError == 2) tft.print("ERR_ALT_MIN      "); else
  if (generalError == 3) tft.print("ERR_LIMIT_SENSE  "); else
  if (generalError == 4) tft.print("ERR_DEC          "); else
  if (generalError == 5) tft.print("ERR_AZM          "); else
  if (generalError == 6) tft.print("ERR_UNDER_POLE   "); else
  if (generalError == 7) tft.print("ERR_MERIDIAN     "); else
  if (generalError == 8) tft.print("ERR_SYNC         "); else
  if (generalError == 9) tft.print("ERR_PARK         "); else
  if (generalError == 10) tft.print("ERR_GOTO_SYNC   "); else
  if (generalError == 11) tft.print("ERR_ALT_MAX     "); else
  if (generalError == 12) tft.print("ERR_UNSPECIFIED "); else
  if (generalError == 13) tft.print("ERR_WEATHER_INIT"); else
  if (generalError == 14) tft.print("ERR_SITE_INIT   "); else
  if (generalError == 15) tft.print("ERR_NV_INIT     ");

  y_offset +=y_spacer; tft.setCursor(STATUS_X, y_offset);  
  tft.print("Command Error = ");
  if (commandError == 0) tft.print("CE_NONE                    "); else
  if (commandError == 1) tft.print("CE_0)                      "); else
  if (commandError == 2) tft.print("CE_CMD_UNKNOWN             "); else
  if (commandError == 3) tft.print("CE_REPLY_UNKNOWN           "); else
  if (commandError == 4) tft.print("CE_PARAM_RANGE             "); else
  if (commandError == 5) tft.print("CE_PARAM_FORM              "); else
  if (commandError == 6) tft.print("CE_ALIGN_FAIL              "); else
  if (commandError == 7) tft.print("CE_ALIGN_NOT_ACTIVE        "); else
  if (commandError == 8) tft.print("CE_NOT_PARKED_OR_AT_HOME   "); else
  if (commandError == 9) tft.print("CE_PARKED                  "); else
  if (commandError == 10) tft.print("CE_PARK_FAILED            "); else
  if (commandError == 11) tft.print("CE_NOT_PARKED             "); else
  if (commandError == 12) tft.print("CE_NO_PARK_POSITION_SET   "); else
  if (commandError == 13) tft.print("CE_GOTO_FAIL              "); else
  if (commandError == 14) tft.print("CE_LIBRARY_FULL           "); else
  if (commandError == 15) tft.print("CE_GOTO_ERR_BELOW_HORIZON "); else
  if (commandError == 16) tft.print("CE_GOTO_ERR_ABOVE_OVERHEAD"); else
  if (commandError == 17) tft.print("CE_SLEW_ERR_IN_STANDBY    "); else
  if (commandError == 18) tft.print("CE_SLEW_ERR_IN_PARK       "); else
  if (commandError == 19) tft.print("CE_GOTO_ERR_GOTO          "); else
  if (commandError == 20) tft.print("CE_SLEW_ERR_OUTSIDE_LIMITS"); else
  if (commandError == 21) tft.print("CE_SLEW_ERR_HARDWARE_FAULT"); else
  if (commandError == 22) tft.print("CE_MOUNT_IN_MOTION        "); else
  if (commandError == 23) tft.print("CE_GOTO_ERR_UNSPECIFIED   "); else
  if (commandError == 24) tft.print("CE_NUL                    ");
  screenTouched = false;
}

void updateXStatus() {
  // placeholder
}

void touchXStatusUpdate() {
  screenTouched = false;
}

  