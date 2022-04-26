// ==============================================
// ============ Display Support =================
// ==============================================
// Primary Display Handler
// 3.5" RPi Touchscreen and Display
// SPI Interface
// Author: Richard Benear 3/30/2021
// **************************************************
#include "Display/Display.h"
#include "Odrive/ODriveDriver.h"
         
#define TITLE_BOXSIZE_X         313
#define TITLE_BOXSIZE_Y          40 
#define TITLE_BOX_X               3
#define TITLE_BOX_Y               1 

// Shared common Status 
#define COM_LABEL_Y_SPACE       16

#define COM_COL1_LABELS_X        8
#define COM_COL1_LABELS_Y       98
#define COM_COL1_DATA_X         72
#define COM_COL1_DATA_Y         COM_COL1_LABELS_Y

#define COM_COL2_LABELS_X       179
#define COM_COL2_DATA_X         245

// Page Selection buttons
#define MENU_X                    3
#define MENU_Y                   42
#define MENU_Y_SPACING            0
#define MENU_X_SPACING           80
#define MENU_BOXSIZE_X           72
#define MENU_BOXSIZE_Y           45
#define MENU_TEXT_X_OFFSET        8
#define MENU_TEXT_Y_OFFSET       28

#define BATTERY_LOW_VOLTAGE    21.0  // recommended cutoff for LiPo battery is 19.2V but want some saftey margin

#define C_WIDTH  80
#define C_HEIGHT 14

Adafruit_ILI9486_Teensy tft;
XPT2046_Touchscreen ts(TS_CS, TS_IRQ); // Use Interrupts for touchscreen
TS_Point p;


// ======= Use Local Command Channel ========
void Display::setLocalCmd(char *command) {
  cmdSend(command, true); // true is no Reply
  processCommands();
}

void Display::setLocalCmd(const char *command) {
  setLocalCmd((char *)command);
}

void Display::getLocalCmd(const char *command, char *reply) {
  char s[12]="";
  cmdSend(command, false); // false is give-me-a-reply (!noReply)
  processCommands();
  cmdReply(s);
  strcpy(reply, s); 
}

void Display::getLocalCmdTrim(const char *command, char *reply) {
  char s[15]="";
  cmdSend(command, false); 
  processCommands();
  memset(s, 0, sizeof(*s));
  cmdReply(s);
  if ((strlen(s)>0) && (s[strlen(s)-1]=='#')) s[strlen(s)-1]=0;
  strcpy(reply, s); 
}


// =========================================
// ========= Initialize Display ============
// =========================================
void Display::initDisiplay() {
  SPI.begin(); delay(1);
  tft.begin(); delay(1);

  VLF("MSG: Display started");
  if (!ts.begin()) {
    VLF("MSG: Unable to start touchscreen");
  } else {
    pinMode(TS_IRQ, INPUT_PULLUP); // XPT2046 library doesn't turn on pullup
    ts.setRotation(3); // touchscreen rotation
    VLF("MSG: Touchscreen started");
  }
  
  tft.setRotation(0); // display rotation: Note it is different than touchscreen
  tft.fillScreen(BLACK);
}

void Display::updateColors() {
  if (!nightMode) { // Day mode
    PAGE_BACKGROUND = DEEP_MAROON; 
    BUT_BACKGROUND = BLACK;  // Button Backgound
    BUT_ON_BGD = RED; // button when On background
    TEXT_COLOR = YELLOW; 
    BUT_OUTLINE = YELLOW; 
  } else {    // Night Mode
    PAGE_BACKGROUND = BLACK; 
    BUT_BACKGROUND = DEEP_MAROON; 
    BUT_ON_BGD = BLACK; 
    TEXT_COLOR = ORANGE; 
    BUT_OUTLINE = ORANGE; 
  }
}

// Draw a single button
void Display::drawButton(int x_start, int y_start, int x_size, int y_size, int box_outline_color, int box_fill_color, int text_x_offset, int text_y_offset, const char* label) {
  tft.fillRoundRect(x_start, y_start, x_size, y_size, 7, box_fill_color);
  tft.drawRoundRect(x_start, y_start, x_size, y_size, 7, box_outline_color);
  tft.setCursor(x_start + text_x_offset, y_start + text_y_offset);
  tft.print(label);
}

// Draw a round button
void Display::drawRoundButton(int x, int y, int r, int outline_color, int fill_color, int text_x_offset, int text_y_offset, const char* label) {
  tft.fillCircle(x, y, r, fill_color);
  tft.drawCircle(x, y, r, outline_color);
  tft.setCursor(x + text_x_offset, y + text_y_offset);
  tft.print(label);
}

// Draw the Title block
void Display::drawTitle(int text_x_offset, int text_y_offset, const char* label) {
  tft.setFont(&ARCENA18pt7b);
  tft.fillRect(TITLE_BOX_X, TITLE_BOX_Y, TITLE_BOXSIZE_X, TITLE_BOXSIZE_Y, BUT_BACKGROUND);
  tft.drawRect(TITLE_BOX_X, TITLE_BOX_Y, TITLE_BOXSIZE_X, TITLE_BOXSIZE_Y, BUT_OUTLINE);
  tft.setCursor(TITLE_BOX_X + text_x_offset, TITLE_BOX_Y + text_y_offset);
  tft.print(label);
  tft.setFont(&Inconsolata_Bold8pt7b);
}

// Update Data Field using bitmap canvas
void Display::canvPrint(int x, int y, int y_off, int width, int height, int text_color, int back_color, const char* label) {
  char rjlabel[50];
  int y_box_offset = 10;
  GFXcanvas1 canvas(width, height);
  
  canvas.setFont(&Inconsolata_Bold8pt7b);  
  canvas.setCursor(0, (height-y_box_offset)/2 + y_box_offset); // offset from top left corner of canvas box
  sprintf(rjlabel, "%9s", label);
  canvas.print(rjlabel);
  tft.drawBitmap(x, y - y_box_offset + y_off, canvas.getBuffer(), width, height, text_color, back_color);
}

void Display::canvPrint(int x, int y, int y_off, int width, int height, int text_color, int back_color, double label) {
  char rjlabel[50];
  int y_box_offset = 10;
  int x_fontSize = 10; // pixels width of a character
  GFXcanvas1 canvas(width, height);

  canvas.setFont(&Inconsolata_Bold8pt7b);
  canvas.setCursor(0, (height-y_box_offset)/2 + y_box_offset); // offset from top left corner of canvas box
  dtostrf(label, width/x_fontSize, 1, rjlabel); // right justify text in the bitmap
  canvas.print(rjlabel);
  tft.drawBitmap(x, y - y_box_offset + y_off, canvas.getBuffer(), width, height, text_color, back_color);
}

void Display::canvPrint(int x, int y, int y_off, int width, int height, int text_color, int back_color, int label) {
  char rjlabel[50];
  int y_box_offset = 10;
  int x_fontSize = 10;
  GFXcanvas1 canvas(width, height);
   
  canvas.setFont(&Inconsolata_Bold8pt7b);
  canvas.setCursor(0, (height-y_box_offset)/2 + y_box_offset); // offset from top left corner of canvas box
  sprintf(rjlabel, "%*d", width/x_fontSize, label); // right justify text in the bitmap
  canvas.print(rjlabel);
  tft.drawBitmap(x, y - y_box_offset + y_off, canvas.getBuffer(), width, height, text_color, back_color);
}

// ============ Onstep Command Errors ===============
void Display::updateOnstepCmdStatus() {
  char cmd[40];
  sprintf(cmd, "OnStep Err: %s", commandErrorStr[commandError]);
  if (!dateWasSet || !timeWasSet) {
    disp.canvPrint(2, 454, 0, 319, C_HEIGHT, BUT_OUTLINE, BUT_ON_BGD, " Time and/or Date Not Set");
  } else if (commandError > CE_0) {
    disp.canvPrint(2, 454, 0, 319, C_HEIGHT, BUT_OUTLINE, BUT_ON_BGD, cmd);
  } else {
    disp.canvPrint(2, 454, 0, 319, C_HEIGHT, BUT_OUTLINE, BUT_BACKGROUND, cmd);
  }
}

// Draw the Menu buttons
void Display::drawMenuButtons() {
  int y_offset = 0;
  int x_offset = 0;
  tft.setTextColor(TEXT_COLOR);
  
  // *************** MENU MAP ****************
  // Current Page   |Cur |Col1|Col2|Col3|Col4|
  // Home-----------| Ho | Gu | Fo | GT | Mo |
  // Guide----------| Gu | Ho | Fo | Al | Mo |
  // Focuser--------| Fo | Ho | Gu | GT | Mo |
  // GoTo-----------| GT | Ho | Fo | Gu | Mo |
  // More & (CATs)--| Mo | GT | Se | Od | Al |
  // ODrive---------| Od | Ho | Se | Al | Xs |
  // Extended Status| Xs | Ho | Se | Al | Od |
  // Settings-------| Se | Ho | Fo | Al | Od |
  // Alignment------| Al | Ho | Fo | Gu | Od |
  tft.setFont(&UbuntuMono_Bold11pt7b); 
  switch(currentPage) {
    case HOME_PAGE: 
      x_offset = 0;
      y_offset = 0;
      disp.drawButton(MENU_X + x_offset, MENU_Y + y_offset, MENU_BOXSIZE_X, MENU_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, MENU_TEXT_X_OFFSET, MENU_TEXT_Y_OFFSET, "GUIDE");
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      disp.drawButton(MENU_X + x_offset, MENU_Y + y_offset, MENU_BOXSIZE_X, MENU_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, MENU_TEXT_X_OFFSET, MENU_TEXT_Y_OFFSET, "FOCUS");
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      disp.drawButton(MENU_X + x_offset, MENU_Y + y_offset, MENU_BOXSIZE_X, MENU_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, MENU_TEXT_X_OFFSET, MENU_TEXT_Y_OFFSET, "GO TO");
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      disp.drawButton(MENU_X + x_offset, MENU_Y + y_offset, MENU_BOXSIZE_X, MENU_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, MENU_TEXT_X_OFFSET-4, MENU_TEXT_Y_OFFSET, ".MORE.");
      break;

   case GUIDE_PAGE:
      x_offset = 0;
      y_offset = 0;
      disp.drawButton(MENU_X + x_offset, MENU_Y + y_offset, MENU_BOXSIZE_X, MENU_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, MENU_TEXT_X_OFFSET+3, MENU_TEXT_Y_OFFSET, "HOME");
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      disp.drawButton(MENU_X + x_offset, MENU_Y + y_offset, MENU_BOXSIZE_X, MENU_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, MENU_TEXT_X_OFFSET, MENU_TEXT_Y_OFFSET, "FOCUS");
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      disp.drawButton(MENU_X + x_offset, MENU_Y + y_offset, MENU_BOXSIZE_X, MENU_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, MENU_TEXT_X_OFFSET, MENU_TEXT_Y_OFFSET, "ALIGN");
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      disp.drawButton(MENU_X + x_offset, MENU_Y + y_offset, MENU_BOXSIZE_X, MENU_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, MENU_TEXT_X_OFFSET-4, MENU_TEXT_Y_OFFSET, ".MORE.");
      break;

   case FOCUSER_PAGE:
      x_offset = 0;
      y_offset = 0;
      disp.drawButton(MENU_X + x_offset, MENU_Y + y_offset, MENU_BOXSIZE_X, MENU_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, MENU_TEXT_X_OFFSET+3, MENU_TEXT_Y_OFFSET, "HOME");
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      disp.drawButton(MENU_X + x_offset, MENU_Y + y_offset, MENU_BOXSIZE_X, MENU_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, MENU_TEXT_X_OFFSET, MENU_TEXT_Y_OFFSET, "GUIDE");
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      disp.drawButton(MENU_X + x_offset, MENU_Y + y_offset, MENU_BOXSIZE_X, MENU_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, MENU_TEXT_X_OFFSET, MENU_TEXT_Y_OFFSET, "GO TO");
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      disp.drawButton(MENU_X + x_offset, MENU_Y + y_offset, MENU_BOXSIZE_X, MENU_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, MENU_TEXT_X_OFFSET-4, MENU_TEXT_Y_OFFSET, ".MORE.");
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      break;
    
   case GOTO_PAGE:
      x_offset = 0;
      y_offset = 0;
      disp.drawButton(MENU_X + x_offset, MENU_Y + y_offset, MENU_BOXSIZE_X, MENU_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, MENU_TEXT_X_OFFSET+3, MENU_TEXT_Y_OFFSET, "HOME");
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      disp.drawButton(MENU_X + x_offset, MENU_Y + y_offset, MENU_BOXSIZE_X, MENU_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, MENU_TEXT_X_OFFSET, MENU_TEXT_Y_OFFSET, "FOCUS");
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      disp.drawButton(MENU_X + x_offset, MENU_Y + y_offset, MENU_BOXSIZE_X, MENU_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, MENU_TEXT_X_OFFSET, MENU_TEXT_Y_OFFSET, "GUIDE");
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      disp.drawButton(MENU_X + x_offset, MENU_Y + y_offset, MENU_BOXSIZE_X, MENU_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, MENU_TEXT_X_OFFSET-4, MENU_TEXT_Y_OFFSET, ".MORE.");
      break;
      
   case MORE_PAGE:
      x_offset = 0;
      y_offset = 0;
      disp.drawButton(MENU_X + x_offset, MENU_Y + y_offset, MENU_BOXSIZE_X, MENU_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, MENU_TEXT_X_OFFSET, MENU_TEXT_Y_OFFSET, "GO TO");
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      disp.drawButton(MENU_X + x_offset, MENU_Y + y_offset, MENU_BOXSIZE_X, MENU_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, MENU_TEXT_X_OFFSET, MENU_TEXT_Y_OFFSET, "SETng");
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      disp.drawButton(MENU_X + x_offset, MENU_Y + y_offset, MENU_BOXSIZE_X, MENU_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, MENU_TEXT_X_OFFSET, MENU_TEXT_Y_OFFSET, "ODRIV");
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      disp.drawButton(MENU_X + x_offset, MENU_Y + y_offset, MENU_BOXSIZE_X, MENU_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, MENU_TEXT_X_OFFSET, MENU_TEXT_Y_OFFSET, "ALIGN");
      break;

    case ODRIVE_PAGE: 
      x_offset = 0;
      y_offset = 0;
      disp.drawButton(MENU_X + x_offset, MENU_Y + y_offset, MENU_BOXSIZE_X, MENU_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, MENU_TEXT_X_OFFSET+3, MENU_TEXT_Y_OFFSET, "HOME");
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      disp.drawButton(MENU_X + x_offset, MENU_Y + y_offset, MENU_BOXSIZE_X, MENU_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, MENU_TEXT_X_OFFSET, MENU_TEXT_Y_OFFSET, "SETng");
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      disp.drawButton(MENU_X + x_offset, MENU_Y + y_offset, MENU_BOXSIZE_X, MENU_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, MENU_TEXT_X_OFFSET, MENU_TEXT_Y_OFFSET, "ALIGN");
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      disp.drawButton(MENU_X + x_offset, MENU_Y + y_offset, MENU_BOXSIZE_X, MENU_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, MENU_TEXT_X_OFFSET, MENU_TEXT_Y_OFFSET, "xSTAT");
      break;
    
    case XSTATUS_PAGE: 
      x_offset = 0;
      y_offset = 0;
      disp.drawButton(MENU_X + x_offset, MENU_Y + y_offset, MENU_BOXSIZE_X, MENU_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, MENU_TEXT_X_OFFSET+3, MENU_TEXT_Y_OFFSET, "HOME");
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      disp.drawButton(MENU_X + x_offset, MENU_Y + y_offset, MENU_BOXSIZE_X, MENU_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, MENU_TEXT_X_OFFSET, MENU_TEXT_Y_OFFSET, "SETng");
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      disp.drawButton(MENU_X + x_offset, MENU_Y + y_offset, MENU_BOXSIZE_X, MENU_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, MENU_TEXT_X_OFFSET, MENU_TEXT_Y_OFFSET, "ALIGN");
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      disp.drawButton(MENU_X + x_offset, MENU_Y + y_offset, MENU_BOXSIZE_X, MENU_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, MENU_TEXT_X_OFFSET, MENU_TEXT_Y_OFFSET, "ODRIV");
      break;

    case SETTINGS_PAGE: 
      x_offset = 0;
      y_offset = 0;
      disp.drawButton(MENU_X + x_offset, MENU_Y + y_offset, MENU_BOXSIZE_X, MENU_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, MENU_TEXT_X_OFFSET+3, MENU_TEXT_Y_OFFSET, "HOME");
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      disp.drawButton(MENU_X + x_offset, MENU_Y + y_offset, MENU_BOXSIZE_X, MENU_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, MENU_TEXT_X_OFFSET, MENU_TEXT_Y_OFFSET, "FOCUS");
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      disp.drawButton(MENU_X + x_offset, MENU_Y + y_offset, MENU_BOXSIZE_X, MENU_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, MENU_TEXT_X_OFFSET, MENU_TEXT_Y_OFFSET, "ALIGN");
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      disp.drawButton(MENU_X + x_offset, MENU_Y + y_offset, MENU_BOXSIZE_X, MENU_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, MENU_TEXT_X_OFFSET, MENU_TEXT_Y_OFFSET, "ODRIV");
      break;

    case ALIGN_PAGE: 
      x_offset = 0;
      y_offset = 0;
      disp.drawButton(MENU_X + x_offset, MENU_Y + y_offset, MENU_BOXSIZE_X, MENU_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, MENU_TEXT_X_OFFSET+3, MENU_TEXT_Y_OFFSET, "HOME");
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      disp.drawButton(MENU_X + x_offset, MENU_Y + y_offset, MENU_BOXSIZE_X, MENU_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, MENU_TEXT_X_OFFSET, MENU_TEXT_Y_OFFSET, "FOCUS");
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      disp.drawButton(MENU_X + x_offset, MENU_Y + y_offset, MENU_BOXSIZE_X, MENU_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, MENU_TEXT_X_OFFSET, MENU_TEXT_Y_OFFSET, "GUIDE");
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      disp.drawButton(MENU_X + x_offset, MENU_Y + y_offset, MENU_BOXSIZE_X, MENU_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, MENU_TEXT_X_OFFSET, MENU_TEXT_Y_OFFSET, "ODRIV");
      break;

   default: // HOME Page
      x_offset = 0;
      y_offset = 0;
       disp.drawButton(MENU_X + x_offset, MENU_Y + y_offset, MENU_BOXSIZE_X, MENU_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, MENU_TEXT_X_OFFSET, MENU_TEXT_Y_OFFSET, "GUIDE");
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      disp.drawButton(MENU_X + x_offset, MENU_Y + y_offset, MENU_BOXSIZE_X, MENU_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, MENU_TEXT_X_OFFSET, MENU_TEXT_Y_OFFSET,  "FOCUS");
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      disp.drawButton(MENU_X + x_offset, MENU_Y + y_offset, MENU_BOXSIZE_X, MENU_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, MENU_TEXT_X_OFFSET, MENU_TEXT_Y_OFFSET,  "GO TO");
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      disp.drawButton(MENU_X + x_offset, MENU_Y + y_offset, MENU_BOXSIZE_X, MENU_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, MENU_TEXT_X_OFFSET, MENU_TEXT_Y_OFFSET, ".MORE.");
      x_offset = x_offset + MENU_X_SPACING;
      y_offset +=MENU_Y_SPACING;
      break;
  }  
  tft.setFont(&Inconsolata_Bold8pt7b);
}

//================================================
// ============ Direct the Updates ===============
//================================================
// Update the Display depending on which Page we are on
void Display::updateDisplayPage() { 
  if (lastPage != currentPage) {
    firstDraw = true;
    lastPage = currentPage;
  }
  //unsigned long start = micros(); //uncomment to measure duration time on a page
  
  switch (currentPage) {
    case HOME_PAGE: 
      updateHomeStatus();
      updateHomeActionButtons();
      disp.updateOdriveErrors();
      disp.updateOnstepCmdStatus();
      firstDraw = false;
      break;
    case GUIDE_PAGE:
      updateGuideStatus();
      disp.updateOdriveErrors();
      disp.updateOnstepCmdStatus();
      firstDraw = false;
      break;
    case FOCUSER_PAGE:
      updateFocuserStatus();
      disp.updateOdriveErrors();
      disp.updateOnstepCmdStatus();
      firstDraw = false;
      break;
    case GOTO_PAGE: 
      updateGoToStatus();
      disp.updateOdriveErrors();
      disp.updateOnstepCmdStatus();
      firstDraw = false;
      break;
    case MORE_PAGE:
      updateMoreStatus();
      disp.updateOdriveErrors();
      disp.updateOnstepCmdStatus();
      firstDraw = false;
      break;
    case ODRIVE_PAGE:
      updateOdriveStatus();
      disp.updateOdriveErrors();
      disp.updateOnstepCmdStatus();
      firstDraw = false;
      break;
    case XSTATUS_PAGE:
      updateXStatus();
      disp.updateOdriveErrors();
      disp.updateOnstepCmdStatus();
      firstDraw = false;
      break;
    case SETTINGS_PAGE:
      updateSettingsStatus();
      disp.updateOdriveErrors();
      disp.updateOnstepCmdStatus();
      firstDraw = false;
      break;
    case ALIGN_PAGE:
      updateAlignStatus();
      disp.updateOdriveErrors();
      disp.updateOnstepCmdStatus();
      firstDraw = false;
      break;
    case CATALOG_PAGE:
      updateCatalogStatus();
      firstDraw = false;
      break;
    case PLANETS_PAGE:
      updatePlanetsStatus();
      firstDraw = false;
      break;
    default:
      firstDraw = false;
      break;
  }

  //unsigned long end = micros();
  //unsigned long delta = end - start;
  //VF("DisplayFunctionTimer"); VL(delta);
}

// ==============================================
// ====== Draw multi-page status labels =========
// ==============================================
// These particular status labels are placed near the top of many Pages.
void Display::drawCommonStatusLabels() {
  tft.setFont(&Inconsolata_Bold8pt7b);
  int y_offset = 0;
  // Column 1
  // Display RA Current
  tft.setCursor(COM_COL1_LABELS_X, COM_COL1_LABELS_Y + y_offset);
  tft.print("RA-----:");

  // Display RA Target
  y_offset +=COM_LABEL_Y_SPACE;
  tft.setCursor(COM_COL1_LABELS_X, COM_COL1_LABELS_Y + y_offset);
  tft.print("RA tgt-:");

  // Display DEC Current
  y_offset +=COM_LABEL_Y_SPACE;
  tft.setCursor(COM_COL1_LABELS_X, COM_COL1_LABELS_Y + y_offset);
  tft.print("DEC----:");

  // Display DEC Target
  y_offset +=COM_LABEL_Y_SPACE;
  tft.setCursor(COM_COL1_LABELS_X, COM_COL1_LABELS_Y + y_offset);
  tft.print("DEC tgt:");

  // Column 2
  // Display Current Azimuth
  y_offset =0;
  tft.setCursor(COM_COL2_LABELS_X, COM_COL1_LABELS_Y + y_offset);
  tft.print("AZ-----:");

  // Display Target Azimuth
  y_offset +=COM_LABEL_Y_SPACE;
  tft.setCursor(COM_COL2_LABELS_X, COM_COL1_LABELS_Y + y_offset);
  tft.print("AZ  tgt:");
  
  // Display Current ALT
  y_offset +=COM_LABEL_Y_SPACE;
  tft.setCursor(COM_COL2_LABELS_X, COM_COL1_LABELS_Y + y_offset);
  tft.print("ALT----:"); 

  // Display Target ALT
  y_offset +=COM_LABEL_Y_SPACE;
  tft.setCursor(COM_COL2_LABELS_X, COM_COL1_LABELS_Y + y_offset);
  tft.print("ALT tgt:"); 
  
  tft.drawFastHLine(1, COM_COL1_LABELS_Y+y_offset+6, TFTWIDTH-1,TEXT_COLOR);
}

// Common Status - Real time data update for the particular labels printed above
// This Common Status is found at the top of most pages.
void Display::updateCommonStatus() { 

  // One Time update the SHC LST and Latitude if GPS locked
  if (tls.active && firstGPS) {
    cat_mgr.setLstT0(shc.getLstT0()); 
    cat_mgr.setLat(shc.getLat());
    //VL(shc.getLat()); VL(shc.getLstT0());
    firstGPS = false;
  }

  int y_offset = 0;
  // Column 1
  // Current RA, Returns: HH:MM.T# or HH:MM:SS# (based on precision setting)
  disp.getLocalCmdTrim(":GR#", ra_hms);
  if ((strcmp(currentRA, ra_hms)) || firstDraw) {
    disp.canvPrint(COM_COL1_DATA_X, COM_COL1_DATA_Y, y_offset, C_WIDTH, C_HEIGHT, BUT_OUTLINE, BUT_BACKGROUND, ra_hms);
    strcpy(currentRA, ra_hms);
  }

// Target RA, Returns: HH:MM.T# or HH:MM:SS (based on precision setting)
  y_offset +=COM_LABEL_Y_SPACE; 
  disp.getLocalCmdTrim(":Gr#", tra_hms);
  if ((strcmp(currentTargRA, tra_hms)) || firstDraw) {
    disp.canvPrint(COM_COL1_DATA_X, COM_COL1_DATA_Y, y_offset, C_WIDTH, C_HEIGHT, BUT_OUTLINE, BUT_BACKGROUND, tra_hms);
    strcpy(currentTargRA, tra_hms);
  }

// Current DEC
   y_offset +=COM_LABEL_Y_SPACE; 
  disp.getLocalCmdTrim(":GD#", dec_dms);
  if ((strcmp(currentDEC, dec_dms)) || firstDraw) {
    disp.canvPrint(COM_COL1_DATA_X, COM_COL1_DATA_Y, y_offset, C_WIDTH, C_HEIGHT, BUT_OUTLINE, BUT_BACKGROUND, dec_dms);
    strcpy(currentDEC, dec_dms);
  }

  // Target DEC
  y_offset +=COM_LABEL_Y_SPACE;  
  disp.getLocalCmdTrim(":Gd#", tdec_dms); 
  if ((strcmp(currentTargDEC, tdec_dms)) || firstDraw) {
    disp.canvPrint(COM_COL1_DATA_X, COM_COL1_DATA_Y, y_offset, C_WIDTH, C_HEIGHT, BUT_OUTLINE, BUT_BACKGROUND, tdec_dms);
    strcpy(currentTargDEC, tdec_dms);
  }

  // ============ Column 2 ==============
  y_offset =0;
  char azmDMS[10] = "";
  char altDMS[11] = "";
  
  // === Show ALT and AZM ===

  // Calculate Target AZM / ALT from Column 1 (:Gr#, :Gd#, target RA/DEC)
  // first, convert them to Double
  shc.hmsToDouble(&currentTargRA_d, currentTargRA);
  shc.dmsToDouble(&currentTargDEC_d, currentTargDEC, true, true);
 
  // convert to Horizon
  currentTargRA_d *= 15;
  cat_mgr.EquToHor(currentTargRA_d, currentTargDEC_d, &talt_d, &tazm_d);

  // Get Current ALT and AZ and display them as Double
  disp.getLocalCmdTrim(":GZ#", azmDMS); // DDD*MM'SS# 
  shc.dmsToDouble(&azm_d, azmDMS, false, PM_HIGH);

  disp.getLocalCmdTrim(":GA#", altDMS);	// sDD*MM'SS#
  shc.dmsToDouble(&alt_d, altDMS, true, PM_HIGH);

  //--Current-- AZM float
  if ((current_azm != azm_d) || firstDraw) { 
    disp.canvPrint(COM_COL2_DATA_X, COM_COL1_DATA_Y, y_offset, C_WIDTH-15, C_HEIGHT, BUT_OUTLINE, BUT_BACKGROUND, azm_d);
    current_azm = azm_d;
  }

  // --Target-- AZM in degrees float
  y_offset +=COM_LABEL_Y_SPACE;
  if ((current_tazm != tazm_d) || firstDraw) {
    disp.canvPrint(COM_COL2_DATA_X, COM_COL1_DATA_Y, y_offset, C_WIDTH-15, C_HEIGHT, BUT_OUTLINE, BUT_BACKGROUND, tazm_d);
    current_tazm = tazm_d;
  }

  // --Current-- ALT
  y_offset +=COM_LABEL_Y_SPACE;
  if ((current_alt != alt_d) || firstDraw) {
    disp.canvPrint(COM_COL2_DATA_X, COM_COL1_DATA_Y, y_offset, C_WIDTH-15, C_HEIGHT, BUT_OUTLINE, BUT_BACKGROUND, alt_d);
    current_alt = alt_d;
  }
  
  // --Target-- ALT in degrees
  y_offset +=COM_LABEL_Y_SPACE;
  if ((current_talt != talt_d) || firstDraw) {
    disp.canvPrint(COM_COL2_DATA_X, COM_COL1_DATA_Y, y_offset, C_WIDTH-15, C_HEIGHT, BUT_OUTLINE, BUT_BACKGROUND, talt_d);
    current_talt = talt_d;
  }
}

//==============================================
//======= ODrive Controller Error Status =======
//==============================================
void Display::updateOdriveErrors() {
  int x = 2;
  int y = 473;
  int label_x = 160;
  int data_x = 110;
  tft.setFont(&Inconsolata_Bold8pt7b);
 
// ODrive AZ and ALT CONTROLLER (only) Error Status
  if (firstDraw) {
    tft.setCursor(x, y);
    tft.print("AZ Ctrl err:");
    tft.setCursor(label_x, y);
    tft.print("ALT Ctrl err:");
    current_AZ_ODerr = dumpOdriveErrors(AZ, CONTROLLER);
    current_ALT_ODerr = dumpOdriveErrors(ALT, CONTROLLER);
    disp.canvPrint(        data_x, y, 0, C_WIDTH-40, C_HEIGHT, BUT_OUTLINE, BUT_BACKGROUND, current_AZ_ODerr);
    disp.canvPrint(label_x+data_x, y, 0, C_WIDTH-40, C_HEIGHT, BUT_OUTLINE, BUT_BACKGROUND, current_ALT_ODerr);
  }
  int AZ_ODerr = dumpOdriveErrors(AZ, CONTROLLER);
  if (current_AZ_ODerr != AZ_ODerr) {
      disp.canvPrint(        data_x, y, 0, C_WIDTH-40, C_HEIGHT, BUT_OUTLINE, BUT_BACKGROUND, AZ_ODerr);
      current_AZ_ODerr = AZ_ODerr;
  }
  int ALT_ODerr = dumpOdriveErrors(AZ, CONTROLLER);
  if (current_ALT_ODerr != ALT_ODerr) {
      disp.canvPrint(label_x+data_x, y, 0, C_WIDTH-40, C_HEIGHT, BUT_OUTLINE, BUT_BACKGROUND, ALT_ODerr);
      current_ALT_ODerr = ALT_ODerr;
  }
}

float Display::getBatteryVoltage() {
// ** Low Battery LED **
 float battery_voltage = getOdriveBusVoltage();
//VF("MSG: Battery Voltage="); VL(battery_voltage);
  if (battery_voltage > BATTERY_LOW_VOLTAGE) { // battery ok
    digitalWrite(BATTERY_LOW_LED_PIN,HIGH); // turn off low battery low LED
    batLED = false;
  } else if (battery_voltage > 3) { // don't want beeping when developing code and battery off V=0
    if (batLED) {
      digitalWrite(BATTERY_LOW_LED_PIN,HIGH); // already on, then flash it off
      batLED = false;
      soundAlert();
    } else {
      digitalWrite(BATTERY_LOW_LED_PIN,LOW); // already off, then flash it on
      batLED = true;
    }
  } else { // battery must be off and in development mode
    digitalWrite(BATTERY_LOW_LED_PIN,HIGH); // turn it off
    batLED = false;
  }
  return battery_voltage;
}

// This function is mostly a direct copy from rDUINOScope but with pushColors() changed to drawPixel() with a loop
// rDUINOScope - Arduino based telescope control system (GOTO).
//    Copyright (C) 2016 Dessislav Gouzgounov (Desso)
//    PROJECT Website: http://rduinoscope.byethost24.com
void Display::drawPic(File *StarMaps, uint16_t x, uint16_t y, uint16_t WW, uint16_t HH){
  uint8_t header[14 + 124]; // maximum length of bmp file header
  uint16_t color[320];  
  uint16_t num;   
  uint8_t color_l, color_h;
  uint32_t i,j,k;
  uint32_t width;
  uint32_t height;
  uint16_t bits;
  uint32_t compression;
  uint32_t alpha_mask = 0;
  uint32_t pic_offset;
  char temp[20]="";

  /** read header of the bmp file */
  i=0;
  while (StarMaps->available()) {
    header[i] = StarMaps->read();
    i++;
    if(i==14){
      break;
    }
  }

  pic_offset = (((uint32_t)header[0x0A+3])<<24) + (((uint32_t)header[0x0A+2])<<16) + (((uint32_t)header[0x0A+1])<<8)+(uint32_t)header[0x0A];
  while (StarMaps->available()) {
    header[i] = StarMaps->read();
    i++;
    if(i==pic_offset){
      break;
    }
  }
 
  /** calculate picture width ,length and bit numbers of color */
  width = (((uint32_t)header[0x12+3])<<24) + (((uint32_t)header[0x12+2])<<16) + (((uint32_t)header[0x12+1])<<8)+(uint32_t)header[0x12];
  height = (((uint32_t)header[0x16+3])<<24) + (((uint32_t)header[0x16+2])<<16) + (((uint32_t)header[0x16+1])<<8)+(uint32_t)header[0x16];
  compression = (((uint32_t)header[0x1E + 3])<<24) + (((uint32_t)header[0x1E + 2])<<16) + (((uint32_t)header[0x1E + 1])<<8)+(uint32_t)header[0x1E];
  bits = (((uint16_t)header[0x1C+1])<<8) + (uint16_t)header[0x1C];
  if(pic_offset>0x42){
    alpha_mask = (((uint32_t)header[0x42 + 3])<<24) + (((uint32_t)header[0x42 + 2])<<16) + (((uint32_t)header[0x42 + 1])<<8)+(uint32_t)header[0x42];
  }
  sprintf(temp, "%lu", pic_offset);  //VF("pic_offset=");  VL(temp);
  sprintf(temp, "%lu", width);       //VF("width=");       VL(temp);
  sprintf(temp, "%lu", height);      //VF("height=");      VL(temp);
  sprintf(temp, "%lu", compression); //VF("compression="); VL(temp);
  sprintf(temp, "%d",  bits);        //VF("bits=");        VL(temp);
  sprintf(temp, "%lu", alpha_mask);  //VF("alpha_mask=");  VL(temp);

  /** set position to pixel table */
  StarMaps->seek(pic_offset);
  /** check picture format */
  if(pic_offset == 138 && alpha_mask == 0){
    /** 565 format */
    tft.setRotation(0);
    /** read from SD card, write to TFT LCD */
    for(j=0; j<HH; j++){ // read all lines
      for(k=0; k<WW; k++){ // read two bytes and pack them in int16, continue for a row
          color_l = StarMaps->read();
          color_h = StarMaps->read();
          color[k]=0;
          color[k] += color_h;
          color[k] <<= 8;
          color[k] += color_l;
      }
      num = 0;
    
      //==== original code ====
      // tft.startWrite();
      // tft.setAddrWindow(x, y+j, x+width-1, y+j);
      // tft.pushColors(color, WW, true);
      // tft.endWrite();

      while (num < x + width - 1){  //implementation for DDScope
      //if ((x < 0) || (x >= _width) || (y < 0) || (y >= _height)) return;
        //setAddrWindow(x, y, x + 1, y + 1);
        //pushColor(uint16_t color)

        //while (num < x+width-1){
        tft.drawPixel(x+num, y+j, color[num]); //implementation for DDScope
        num++;
      }
      // dummy read twice to align for 4 
      if(width%2){
        StarMaps->read();StarMaps->read();
      }
    }
  }
}

/*  // not used...pushColors is an older implementation
void pushColors(void * colorBuffer, uint16_t nr_pixels, uint8_t async){
  if (hwSPI) spi_begin();
  
  *dcport |=  dcpinmask;
  *csport &= ~cspinmask;

  if (async==0) {
    SPI.dmaSend(colorBuffer, nr_pixels, 1);
    *csport |= cspinmask;
  } else {
    SPI.dmaSendAsync(colorBuffer, nr_pixels, 1);
  }

  if (hwSPI) spi_end();
}
*/

// Quick set the target to Polaris
void setTargPolaris() {
  // Polaris location RA=02:31:49.09, Dec=+89:15:50.8 (2.5303, 89.2641)
  disp.setLocalCmd(":Sr02:31:49#");
  disp.setLocalCmd(":Sd+89:15:50#"); 
}

// Copied this from Smart Hand Controller LX200.cpp and renamed it
// convert string in format HH:MM:SS to floating point
// (also handles)           HH:MM.M
boolean SHC::hmsToDouble(double *f, char *hms) {
  char h[3],m[5],s[3];
  int  h1,m1,m2=0,s1=0;
  boolean highPrecision=true;
  
  while (*hms==' ') hms++; // strip prefix white-space

  if (highPrecision) { if (strlen(hms)!= 8) return false; } else if (strlen(hms)!= 7) return false;

  h[0]=*hms++; h[1]=*hms++; h[2]=0; atoi2(h,&h1);
  if (highPrecision) {
    if (*hms++!=':') return false; m[0]=*hms++; m[1]=*hms++; m[2]=0; atoi2(m,&m1);
    if (*hms++!=':') return false; s[0]=*hms++; s[1]=*hms++; s[2]=0; atoi2(s,&s1);
  } else {
    if (*hms++!=':') return false; m[0]=*hms++; m[1]=*hms++; m[2]=0; atoi2(m,&m1);
    if (*hms++!='.') return false; m2=(*hms++)-'0';
  }
  if ((h1<0) || (h1>23) || (m1<0) || (m1>59) || (m2<0) || (m2>9) || (s1<0) || (s1>59)) return false;
  
  *f=h1+m1/60.0+m2/600.0+s1/3600.0;
  return true;
}

// DegreesMinutesSeconds to double
// Copied this from Smart Hand Controller LX200.cpp and renamed it
// dmsToDouble - convert string in format sDD:MM:SS to floating point
// (also handles)           DDD:MM:SS
//                          sDD:MM
//                          DDD:MM
//                          sDD*MM
//                          DDD*MM
boolean SHC::dmsToDouble(double *f, char *dms, boolean sign_present, boolean highPrecision) {
  char d[4],m[5],s[3];
  int d1, m1, s1=0;
  int lowLimit=0, highLimit=360;
  int checkLen,checkLen1;
  double sign = 1.0;
  boolean secondsOff = false;

  while (*dms==' ') dms++; // strip prefix white-space

  checkLen1=strlen(dms);

  // determine if the seconds field was used and accept it if so
  if (highPrecision) { 
    checkLen=9;
    if (checkLen1 != checkLen) return false;
  } else {
    checkLen=6;
    if (checkLen1 != checkLen) {
      if (checkLen1==9) { secondsOff=false; checkLen=9; } else return false;
    } else secondsOff = true;
  }

  // determine if the sign was used and accept it if so
  if (sign_present) {
    if (*dms=='-') sign=-1.0; else if (*dms=='+') sign=1.0; else return false; 
    dms++; d[0]=*dms++; d[1]=*dms++; d[2]=0; if (!atoi2(d,&d1)) return false;
  } else {
    d[0]=*dms++; d[1]=*dms++; d[2]=*dms++; d[3]=0; if (!atoi2(d,&d1)) return false;
  }

  // make sure the seperator is an allowed character
  if ((*dms!=':') && (*dms!='*') && (*dms!=char(223))) return false; else dms++;

  m[0]=*dms++; m[1]=*dms++; m[2]=0; if (!atoi2(m,&m1)) return false;

  if ((highPrecision) && (!secondsOff)) {
    // make sure the seperator is an allowed character
    if (*dms++!=':') return false; 
    s[0]=*dms++; s[1]=*dms++; s[2]=0; atoi2(s,&s1);
  }

  if (sign_present) { lowLimit=-90; highLimit=90; }
  if ((d1<lowLimit) || (d1>highLimit) || (m1<0) || (m1>59) || (s1<0) || (s1>59)) return false;
  
  *f=sign*(d1+m1/60.0+s1/3600.0);
  return true;
}

// modified this for xChan format from SHC
double SHC::getLstT0() {
  double f=0;
  disp.getLocalCmdTrim(":GS#", locCmdReply);
  hmsToDouble(&f, locCmdReply);
  return f;
};

// modified this for xChan format from SHC
double SHC::getLat() {
  disp.getLocalCmdTrim(":Gt#", locCmdReply);
  double f=-10000;
  dmsToDouble(&f, locCmdReply, true, false);
  return f;
};

