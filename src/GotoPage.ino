// ==============================================
// ================ GoTo Page ===================
// ==============================================
// Author: Richard Benear 2021

#define NUM_BUTTON_X         2
#define NUM_BUTTON_Y         252
#define NUM_BUTTON_W         50
#define NUM_BUTTON_H         45
#define NUM_BUTTON_SPACING_X 1
#define NUM_BUTTON_SPACING_Y 1
#define NUM_T_OFF_X          NUM_BUTTON_W/2 - 10/2
#define NUM_T_OFF_Y          NUM_BUTTON_H/2 + 3

#define TEXT_LABEL_X         5
#define TEXT_LABEL_Y         185
#define TEXT_FIELD_X         135
#define TEXT_FIELD_Y         TEXT_LABEL_Y
#define TEXT_FIELD_WIDTH     65
#define TEXT_FIELD_HEIGHT    26
#define TEXT_SPACING_X       10
#define TEXT_SPACING_Y       TEXT_FIELD_HEIGHT

#define RA_SELECT_X          205
#define RA_SELECT_Y          166
#define RA_CLEAR_X           265
#define RA_CLEAR_Y           RA_SELECT_Y
#define RA_T_OFF_X           6
#define RA_T_OFF_Y           15
#define CO_BOXSIZE_X         50
#define CO_BOXSIZE_Y         24

#define DEC_SELECT_X         RA_SELECT_X
#define DEC_SELECT_Y         192
#define DEC_CLEAR_X          RA_CLEAR_X
#define DEC_CLEAR_Y          DEC_SELECT_Y
#define DEC_T_OFF_X          RA_T_OFF_X
#define DEC_T_OFF_Y          RA_T_OFF_Y

#define SEND_BUTTON_X        215
#define SEND_BUTTON_Y        220
#define S_T_OFF_X            19
#define S_T_OFF_Y            23
#define SEND_BOXSIZE_X       70
#define SEND_BOXSIZE_Y       40

#define POL_BUTTON_X         40
#define POL_BUTTON_Y         219
#define P_T_OFF_X            6
#define P_T_OFF_Y            18
#define POL_BOXSIZE_X        70
#define POL_BOXSIZE_Y        28

#define GOTO_BUTTON_X        200
#define GOTO_BUTTON_Y        300
#define GTA_T_OFF_X          23
#define GTA_T_OFF_Y          30
#define GOTO_BOXSIZE_X       100
#define GOTO_BOXSIZE_Y       50

#define ABORT_BUTTON_X       GOTO_BUTTON_X
#define ABORT_BUTTON_Y       360

#define RA_CMD_ERR_X         5
#define RA_CMD_ERR_Y         229
#define DEC_CMD_ERR_X        5
#define DEC_CMD_ERR_Y        246
#define CMD_ERR_W            180
#define CMD_ERR_H            19

#define CUSTOM_FONT_OFFSET   -15

char numLabels[12][3] = {"9", "8", "7", "6", "5", "4", "3", "2", "1", "-", "0", "+"};
char RAtext[8] = "";
char DECtext[8] = "";
char cmd[10] = "";

int buttonPosition = 12;
uint8_t RAtextIndex = 0;
uint8_t DECtextIndex = 0;

bool RAselect = true;
bool RAclear = false;
bool DECselect = false;
bool DECclear = false;
bool sendOn = false;
bool setPolOn = false;
bool timeOn = false;
bool numDetected = false;
bool goToPgBut = false;
bool abortPgBut = false;

// Draw the Go To Page
void drawGoToPage() {
  disp.updateColors();
  tft.setTextColor(TEXT_COLOR);
  tft.fillScreen(PAGE_BACKGROUND);
  currentPage = GOTO_PAGE;
  disp.drawTitle(115, 30, "Go To");
  disp.drawMenuButtons();
  disp.drawCommonStatusLabels(); // status common to many pages
  disp.updateOnstepCmdStatus();

  RAtextIndex = 0; 
  DECtextIndex = 0; 

  // Draw RA and DEC Coordinate Labels
  tft.setCursor(160, 430);
  tft.print("Assumes Epoch J2000");
  tft.setCursor(TEXT_LABEL_X, TEXT_LABEL_Y);
  tft.print(" RA  (hhmm[ss]):");
  tft.setCursor(TEXT_LABEL_X, TEXT_LABEL_Y+TEXT_SPACING_Y);
  tft.print("DEC(sddmm[sec]):");

  // Draw Key Pad
  int z=0;
  for(int i=0; i<4; i++) { 
    for(int j=0; j<3; j++) {
      int row=i; int col=j; 
      disp.drawButton(NUM_BUTTON_X+col*(NUM_BUTTON_W+NUM_BUTTON_SPACING_X), 
              NUM_BUTTON_Y+row*(NUM_BUTTON_H+NUM_BUTTON_SPACING_Y), 
              NUM_BUTTON_W, NUM_BUTTON_H, BUT_OUTLINE, BUT_BACKGROUND, NUM_T_OFF_X, NUM_T_OFF_Y, numLabels[z]);
      z++;
    }
  }

  // Initialize RA and DEC Enter/Accept buttons
  disp.drawButton( RA_SELECT_X,  RA_SELECT_Y, CO_BOXSIZE_X, CO_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, RA_T_OFF_X,  RA_T_OFF_Y,  "RASel");
  disp.drawButton(  RA_CLEAR_X,   RA_CLEAR_Y, CO_BOXSIZE_X, CO_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, RA_T_OFF_X,  RA_T_OFF_Y,  "RAClr"); 
  disp.drawButton(DEC_SELECT_X, DEC_SELECT_Y, CO_BOXSIZE_X, CO_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, DEC_T_OFF_X, DEC_T_OFF_Y, "DESel");
  disp.drawButton( DEC_CLEAR_X,  DEC_CLEAR_Y, CO_BOXSIZE_X, CO_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, DEC_T_OFF_X, DEC_T_OFF_Y, "DEClr"); 
  
  // Send data button
  disp.drawButton(SEND_BUTTON_X, SEND_BUTTON_Y, SEND_BOXSIZE_X, SEND_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, S_T_OFF_X, S_T_OFF_Y, "Send"); 

  // Initialize GOTO and ABORT buttons
  disp.drawButton( GOTO_BUTTON_X, GOTO_BUTTON_Y,  GOTO_BOXSIZE_X, GOTO_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, GTA_T_OFF_X+2, GTA_T_OFF_Y, "GoTo"); 
  disp.drawButton(ABORT_BUTTON_X, ABORT_BUTTON_Y, GOTO_BOXSIZE_X, GOTO_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, GTA_T_OFF_X, GTA_T_OFF_Y, "Abort"); 

  tft.fillRect(TEXT_FIELD_X, TEXT_FIELD_Y+CUSTOM_FONT_OFFSET, TEXT_FIELD_WIDTH, TEXT_FIELD_HEIGHT-9, BUT_BACKGROUND);
  tft.fillRect(TEXT_FIELD_X, TEXT_FIELD_Y+TEXT_SPACING_Y+CUSTOM_FONT_OFFSET, TEXT_FIELD_WIDTH, TEXT_FIELD_HEIGHT-9, BUT_BACKGROUND);
} // end initialize

// assign label to pressed button field
void processNumPadButton() {
  if (numDetected) {
    if (RAselect && (buttonPosition >= 0 && (buttonPosition < 9 || buttonPosition == 10)) && RAtextIndex < 6) {
      RAtext[RAtextIndex] = numLabels[buttonPosition][0];
      tft.fillRect(TEXT_FIELD_X, TEXT_FIELD_Y+CUSTOM_FONT_OFFSET, TEXT_FIELD_WIDTH, TEXT_FIELD_HEIGHT-9, BUT_BACKGROUND);
      tft.setCursor(TEXT_FIELD_X, TEXT_FIELD_Y);
      tft.print(RAtext); 
      RAtextIndex++;
    }

    if (DECselect && (((DECtextIndex == 0 && (buttonPosition == 9 || buttonPosition == 11)) 
      || (DECtextIndex>0 && (buttonPosition!=9||buttonPosition!=11)))) && DECtextIndex < 7) {
      DECtext[DECtextIndex] = numLabels[buttonPosition][0]; 
      tft.fillRect(TEXT_FIELD_X, TEXT_FIELD_Y+TEXT_SPACING_Y+CUSTOM_FONT_OFFSET, TEXT_FIELD_WIDTH, TEXT_FIELD_HEIGHT-9, BUT_BACKGROUND);
      tft.setCursor(TEXT_FIELD_X, TEXT_FIELD_Y+TEXT_SPACING_Y);
      tft.print(DECtext);
      DECtextIndex++;
    }
    numDetected = false;
  }
}

// ==== Update any changing Status for GO TO Page ====
void updateGoToStatus() {
  disp.updateCommonStatus(); // status common to many pages
  
  if (screenTouched || firstDraw || refreshScreen) { 
        refreshScreen = false;
        if (screenTouched) refreshScreen = true;
    
    // Get button and print label
    switch (buttonPosition) {
      case 0:  processNumPadButton(); break;
      case 1:  processNumPadButton(); break;
      case 2:  processNumPadButton(); break;
      case 3:  processNumPadButton(); break;
      case 4:  processNumPadButton(); break;
      case 5:  processNumPadButton(); break;
      case 6:  processNumPadButton(); break;
      case 7:  processNumPadButton(); break;
      case 8:  processNumPadButton(); break;
      case 9:  processNumPadButton(); break;
      case 10: processNumPadButton(); break;
      case 11: processNumPadButton(); break;
      default: break;
    }

    // RA Select Button
    if (RAselect) {
      disp.drawButton( RA_SELECT_X,  RA_SELECT_Y, CO_BOXSIZE_X, CO_BOXSIZE_Y, BUT_OUTLINE, BUT_ON_BGD, RA_T_OFF_X,  RA_T_OFF_Y,  "RASel");
    } else {
      disp.drawButton( RA_SELECT_X,  RA_SELECT_Y, CO_BOXSIZE_X, CO_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, RA_T_OFF_X,  RA_T_OFF_Y,  "RASel");
    }

    // RA Clear button
    if (RAclear) {
      disp.drawButton(  RA_CLEAR_X,   RA_CLEAR_Y, CO_BOXSIZE_X, CO_BOXSIZE_Y, BUT_OUTLINE, BUT_ON_BGD, RA_T_OFF_X,  RA_T_OFF_Y,  "RAClr");
      tft.fillRect(TEXT_FIELD_X, TEXT_FIELD_Y+CUSTOM_FONT_OFFSET, TEXT_FIELD_WIDTH, TEXT_FIELD_HEIGHT-9, BUT_ON_BGD);
      memset(RAtext,0,sizeof(RAtext)); // clear RA buffer
      //VF("RAtext="); VL(RAtext);
      tft.fillRect(RA_CMD_ERR_X, RA_CMD_ERR_Y+CUSTOM_FONT_OFFSET, CMD_ERR_W, CMD_ERR_H, PAGE_BACKGROUND);
      RAtextIndex = 0;
      buttonPosition = 12;
      RAclear = false;
    } else {
      disp.drawButton(RA_CLEAR_X, RA_CLEAR_Y, CO_BOXSIZE_X, CO_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, RA_T_OFF_X,  RA_T_OFF_Y,  "RAClr");
    }
    
    // DEC Select button
    if (DECselect) {
      disp.drawButton(DEC_SELECT_X, DEC_SELECT_Y, CO_BOXSIZE_X, CO_BOXSIZE_Y, BUT_OUTLINE, BUT_ON_BGD, DEC_T_OFF_X, DEC_T_OFF_Y, "DESel");
    } else {
      disp.drawButton(DEC_SELECT_X, DEC_SELECT_Y, CO_BOXSIZE_X, CO_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, DEC_T_OFF_X, DEC_T_OFF_Y, "DESel"); 
    }

    // DEC Clear Button
    if (DECclear) {
      disp.drawButton( DEC_CLEAR_X,  DEC_CLEAR_Y, CO_BOXSIZE_X, CO_BOXSIZE_Y, BUT_OUTLINE, BUT_ON_BGD, DEC_T_OFF_X, DEC_T_OFF_Y, "DEClr"); 
      tft.fillRect(TEXT_FIELD_X, TEXT_FIELD_Y+CUSTOM_FONT_OFFSET+TEXT_FIELD_HEIGHT, TEXT_FIELD_WIDTH, TEXT_FIELD_HEIGHT-9, BUT_ON_BGD);
      memset(DECtext,0,sizeof(DECtext)); // clear DEC buffer
      //VF("DECtext="); VL(DECtext);
      tft.fillRect(DEC_CMD_ERR_X, DEC_CMD_ERR_Y+CUSTOM_FONT_OFFSET, CMD_ERR_W, CMD_ERR_H, PAGE_BACKGROUND);
      DECtextIndex = 0;
      buttonPosition = 12;
      DECclear = false;
    } else {
      disp.drawButton( DEC_CLEAR_X,  DEC_CLEAR_Y, CO_BOXSIZE_X, CO_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, DEC_T_OFF_X, DEC_T_OFF_Y, "DEClr"); 
    }
    
    // Send Coordinates Button
    if (sendOn) {
      disp.drawButton(SEND_BUTTON_X, SEND_BUTTON_Y, SEND_BOXSIZE_X, SEND_BOXSIZE_Y, BUT_OUTLINE, BUT_ON_BGD, S_T_OFF_X, S_T_OFF_Y, "Sent");
      sendOn = false; 
    } else {
      disp.drawButton(SEND_BUTTON_X, SEND_BUTTON_Y, SEND_BOXSIZE_X, SEND_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, S_T_OFF_X, S_T_OFF_Y, "Send"); 
    }

    // Quick Set Polaris Target Button
    if (setPolOn) {
      disp.drawButton(POL_BUTTON_X, POL_BUTTON_Y, POL_BOXSIZE_X, POL_BOXSIZE_Y, BUT_OUTLINE, BUT_ON_BGD, P_T_OFF_X, P_T_OFF_Y, "Setting");
      setPolOn = false; 
    } else {
      disp.drawButton(POL_BUTTON_X, POL_BUTTON_Y, POL_BOXSIZE_X, POL_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, P_T_OFF_X, P_T_OFF_Y, "Polaris"); 
    }

    tft.setFont(&UbuntuMono_Bold11pt7b);    
    // Go To Coordinates Button
    if (goToPgBut) {
      disp.drawButton( GOTO_BUTTON_X, GOTO_BUTTON_Y,  GOTO_BOXSIZE_X, GOTO_BOXSIZE_Y, BUT_OUTLINE, BUT_ON_BGD, GTA_T_OFF_X, GTA_T_OFF_Y, "Going");
      goToPgBut = false;
    } else {
      if (!isSlewing()) { 
        disp.drawButton( GOTO_BUTTON_X, GOTO_BUTTON_Y,  GOTO_BOXSIZE_X, GOTO_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, GTA_T_OFF_X+2, GTA_T_OFF_Y, "GoTo"); 
      } else {
        refreshScreen = true;
      }
    }
    
    // Abort GoTo Button
    if (abortPgBut) {
      disp.drawButton(ABORT_BUTTON_X, ABORT_BUTTON_Y, GOTO_BOXSIZE_X, GOTO_BOXSIZE_Y, BUT_OUTLINE, BUT_ON_BGD, GTA_T_OFF_X-5, GTA_T_OFF_Y, "Aborting"); 
      abortPgBut = false;
    } else {
      disp.drawButton(ABORT_BUTTON_X, ABORT_BUTTON_Y, GOTO_BOXSIZE_X, GOTO_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, GTA_T_OFF_X, GTA_T_OFF_Y, "Abort"); 
    }
    tft.setFont(&UbuntuMono_Bold8pt7b); 
    screenTouched = false;
  }
}

// ==== TouchScreen was touched, determine which button ====
void touchGoToUpdate() {
  //were number Pad buttons pressed?
  for(int i=0; i<4; i++) { 
    for(int j=0; j<3; j++) {
      int row=i; int col=j; 
      if (p.y >   NUM_BUTTON_Y+row*(NUM_BUTTON_H+NUM_BUTTON_SPACING_Y) 
       && p.y <  (NUM_BUTTON_Y+row*(NUM_BUTTON_H+NUM_BUTTON_SPACING_Y)) + NUM_BUTTON_H 
       && p.x >   NUM_BUTTON_X+col*(NUM_BUTTON_W+NUM_BUTTON_SPACING_X) 
       && p.x <  (NUM_BUTTON_X+col*(NUM_BUTTON_W+NUM_BUTTON_SPACING_X) + NUM_BUTTON_W)) {
        soundClick();
        buttonPosition=row*3+col;
        //VF("buttonPosition="); VL(buttonPosition);
        numDetected = true;
      }
    }
  }

  // Select RA field
  if (p.y > RA_SELECT_Y && p.y < (RA_SELECT_Y + CO_BOXSIZE_Y) && p.x > RA_SELECT_X && p.x < (RA_SELECT_X + CO_BOXSIZE_X)) {
    soundClick();
    RAselect = true; 
    DECselect = false; 
  }

  // Clear RA field
  if (p.y > RA_CLEAR_Y && p.y < (RA_CLEAR_Y + CO_BOXSIZE_Y) && p.x > RA_CLEAR_X && p.x < (RA_CLEAR_X + CO_BOXSIZE_X)) {
    soundClick();
    RAclear = true; 
    RAtextIndex = 0;
    buttonPosition = 12; 
  }

  // Select DEC field
  if (p.y > DEC_SELECT_Y && p.y < (DEC_SELECT_Y + CO_BOXSIZE_Y) && p.x > DEC_SELECT_X && p.x < (DEC_SELECT_X + CO_BOXSIZE_X)) {
    soundClick();
    DECselect = true;
    RAselect = false;
  }

  // Clear DEC field
  if (p.y > DEC_CLEAR_Y && p.y < (DEC_CLEAR_Y + CO_BOXSIZE_Y) && p.x > DEC_CLEAR_X && p.x < (DEC_CLEAR_X + CO_BOXSIZE_X)) {
    soundClick();
    DECclear = true; 
    DECtextIndex = 0;
    buttonPosition = 12; 
  }

  // SEND Coordinates
  if (p.y > SEND_BUTTON_Y && p.y < (SEND_BUTTON_Y + SEND_BOXSIZE_Y) && p.x > SEND_BUTTON_X && p.x < (SEND_BUTTON_X + SEND_BOXSIZE_X)) {
    soundClick();
    sendOn = true; 
    RAtextIndex = 0;
    DECtextIndex = 0;
    buttonPosition = 12; 
    
    if (RAselect) {
      //:Sr[HH:MM.T]# or :Sr[HH:MM:SS]# 
      sprintf(cmd, ":Sr%c%c:%c%c:%c%c#", RAtext[0], RAtext[1], RAtext[2], RAtext[3], RAtext[4], RAtext[5]);
      disp.setLocalCmd(cmd);
    } else if (DECselect) {
      //:Sd[sDD*MM]# or :Sd[sDD*MM:SS]# 
      sprintf(cmd, ":Sd%c%c%c:%c%c:%c%c#", DECtext[0], DECtext[1], DECtext[2], DECtext[3], DECtext[4], DECtext[5], DECtext[6]);
      disp.setLocalCmd(cmd);
    }
  }

  // Quick set Polaris Target
  if (p.y > POL_BUTTON_Y && p.y < (POL_BUTTON_Y + POL_BOXSIZE_Y) && p.x > POL_BUTTON_X && p.x < (POL_BUTTON_X + POL_BOXSIZE_X)) {
    soundClick();
    setPolOn = true;
    setTargPolaris(); 
  }

  // ==== Go To Target Coordinates ====
  if (p.y > GOTO_BUTTON_Y && p.y < (GOTO_BUTTON_Y + GOTO_BOXSIZE_Y) && p.x > GOTO_BUTTON_X && p.x < (GOTO_BUTTON_X + GOTO_BOXSIZE_X)) {
    soundClick();
    goToPgBut = true;
    axis1Enabled = true;
    disp.setLocalCmd(":MS#"); // move to
  }

  // ==== ABORT GOTO ====
  if (p.y > ABORT_BUTTON_Y && p.y < (ABORT_BUTTON_Y + GOTO_BOXSIZE_Y) && p.x > ABORT_BUTTON_X && p.x < (ABORT_BUTTON_X + GOTO_BOXSIZE_X)) {
    soundClick();
    abortPgBut = true;
    disp.setLocalCmd(":Q#"); // stops move
    stopMotors(); // do this for safety reasons...mount may be colliding with something
  }
}

 