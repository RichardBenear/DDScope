// =========================================================
// ====== Display CATALOG and "MORE" Menus Page ============
// =========================================================
// The Catalog-and-More page allows access to More sub menus and Catalogs
// Author: Richard Benear - Dec 2021
//
// Also uses routines from Smart Hand Controller (SHC) 
// Copyright (C) 2018 to 2021 Charles Lemaire, Howard Dutton, and Others
// Author: Charles Lemaire, https://pixelstelescopes.wordpress.com/teenastro/
// Author: Howard Dutton, http://www.stellarjourney.com, hjd1964@gmail.com

// Catalog Selection buttons
#define CAT_SEL_X               5
#define CAT_SEL_Y              175
#define CAT_SEL_BOXSIZE_X      110 
#define CAT_SEL_BOXSIZE_Y       28
#define CAT_SEL_SPACER          CAT_SEL_BOXSIZE_Y + 6
#define CAT_SEL_TEXT_X_OFFSET    7 
#define CAT_SEL_TEXT_Y_OFFSET   17

// Tracking rate buttons
#define TRACK_R_X              125
#define TRACK_R_Y              180
#define TRACK_R_BOXSIZE_X       70 
#define TRACK_R_BOXSIZE_Y       24
#define TRACK_R_SPACER           1 
#define TRACK_R_GROUP_SPACER     5 
#define TRACK_R_TEXT_X_OFFSET    4 
#define TRACK_R_TEXT_Y_OFFSET   16 

// Misc Buttons
#define MISC_X                 212
#define MISC_Y                 160
#define MISC_BOXSIZE_X         100 
#define MISC_BOXSIZE_Y          28
#define MISC_SPACER_Y            8 
#define MISC_TEXT_X_OFFSET       2 
#define MISC_TEXT_Y_OFFSET      18

// Filter Button
#define FM_X                   200
#define FM_Y                   157
#define FM_BOXSIZE_X           120 
#define FM_BOXSIZE_Y            28
#define FM_SPACER_Y              2 
#define FM_TEXT_X_OFFSET         0 
#define FM_TEXT_Y_OFFSET        18

#define GOTO_BUT_X             217
#define GOTO_BUT_Y             260
#define GOTO_TXT_OFF_X          15
#define GOTO_TXT_OFF_Y          25
#define GOTO_M_BOXSIZE_X        90
#define GOTO_M_BOXSIZE_Y        40

#define ABORT_M_BUT_X          218
#define ABORT_M_BUT_Y          310

bool catSelBut1 = false;
bool catSelBut2 = false;
bool catSelBut3 = false;
bool catSelBut4 = false;
bool catSelBut5 = false;
bool clrCustom = false;
bool sideRate = true;
bool lunarRate = false;
bool kingRate = false;
bool incTrackRate = false;
bool decTrackRate = false;
bool rstTrackRate = false;
bool filterBut = false;
bool yesBut = false; 
bool cancelBut = false;
bool yesCancelActive = false;
double catMgrLst;

// ============= Initialize the Catalog & More page ==================
void drawMorePage() {
  currentPage = MORE_PAGE;
  disp.updateColors();
  tft.setTextColor(TEXT_COLOR);
  tft.fillScreen(PAGE_BACKGROUND);
  tft.setFont(&UbuntuMono_Bold8pt7b); 

  tft.setCursor(TRACK_R_X+2, TRACK_R_Y-16);
  tft.print("Tracking");
  tft.setCursor(TRACK_R_X+5, TRACK_R_Y-4);
  tft.print(" Rates");

  disp.drawMenuButtons();
  disp.drawTitle(60, 30, "Catalogs & Misc");

  tft.setCursor(30, 170);
  tft.print("Catalogs");

  // Draw the HOME Icon bitmap
  uint8_t extern black_house_icon[];
  tft.drawBitmap(10, 5, black_house_icon, 39, 31, BUT_BACKGROUND, ORANGE);

  disp.drawCommonStatusLabels();
  disp.updateOnstepCmdStatus();
}

//================== Update the Buttons ======================
void updateMoreStatus() {
  disp.updateCommonStatus();

  if (screenTouched || firstDraw || refreshScreen) { //reduce screen flicker 
    refreshScreen = false;
    if (screenTouched) refreshScreen = true;

    // Show any target object data selected from Catalog
    uint16_t y = 356; uint16_t x = 120;
    if (objectSelected) tft.fillRect(x, y, 199, 5*16, PAGE_BACKGROUND);
    tft.setCursor(x,y+16  ); tft.print(catSelectionStr1);
    tft.setCursor(x,y+16*2); tft.print(catSelectionStr2);
    tft.setCursor(x,y+16*3); tft.print(catSelectionStr3);
    tft.setCursor(x,y+16*4); tft.print(catSelectionStr4); 
    tft.setCursor(x,y+16*5); tft.print(catSelectionStr5); 

    int y_offset = 0;
    int x_offset = 0;
    // Manage Tracking Rate buttons
    if (sideRate) {
        disp.drawButton(TRACK_R_X+x_offset, TRACK_R_Y+y_offset, TRACK_R_BOXSIZE_X, TRACK_R_BOXSIZE_Y, BUT_OUTLINE, BUT_ON_BGD, TRACK_R_TEXT_X_OFFSET, TRACK_R_TEXT_Y_OFFSET,   "Sidereal");
        y_offset += TRACK_R_BOXSIZE_Y+TRACK_R_SPACER;
        disp.drawButton(TRACK_R_X+x_offset, TRACK_R_Y+y_offset, TRACK_R_BOXSIZE_X, TRACK_R_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, TRACK_R_TEXT_X_OFFSET-2, TRACK_R_TEXT_Y_OFFSET, "  Lunar "); 
        y_offset += TRACK_R_BOXSIZE_Y+TRACK_R_SPACER;
        disp.drawButton(TRACK_R_X+x_offset, TRACK_R_Y+y_offset, TRACK_R_BOXSIZE_X, TRACK_R_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, TRACK_R_TEXT_X_OFFSET-2, TRACK_R_TEXT_Y_OFFSET, "  King  ");
    }
    if (lunarRate) {
        y_offset = 0;
        disp.drawButton(TRACK_R_X+x_offset, TRACK_R_Y+y_offset, TRACK_R_BOXSIZE_X, TRACK_R_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, TRACK_R_TEXT_X_OFFSET, TRACK_R_TEXT_Y_OFFSET,   "Sidereal");
        y_offset += TRACK_R_BOXSIZE_Y+TRACK_R_SPACER;
        disp.drawButton(TRACK_R_X+x_offset, TRACK_R_Y+y_offset, TRACK_R_BOXSIZE_X, TRACK_R_BOXSIZE_Y, BUT_OUTLINE, BUT_ON_BGD, TRACK_R_TEXT_X_OFFSET-2, TRACK_R_TEXT_Y_OFFSET, "  Lunar "); 
        y_offset += TRACK_R_BOXSIZE_Y+TRACK_R_SPACER;
        disp.drawButton(TRACK_R_X+x_offset, TRACK_R_Y+y_offset, TRACK_R_BOXSIZE_X, TRACK_R_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, TRACK_R_TEXT_X_OFFSET-2, TRACK_R_TEXT_Y_OFFSET, "  King  ");
    }
    if (kingRate) {
        y_offset = 0;
        disp.drawButton(TRACK_R_X+x_offset, TRACK_R_Y+y_offset, TRACK_R_BOXSIZE_X, TRACK_R_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, TRACK_R_TEXT_X_OFFSET, TRACK_R_TEXT_Y_OFFSET,   "Sidereal");
        y_offset += TRACK_R_BOXSIZE_Y+TRACK_R_SPACER;
        disp.drawButton(TRACK_R_X+x_offset, TRACK_R_Y+y_offset, TRACK_R_BOXSIZE_X, TRACK_R_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, TRACK_R_TEXT_X_OFFSET-2, TRACK_R_TEXT_Y_OFFSET, "  Lunar "); 
        y_offset += TRACK_R_BOXSIZE_Y+TRACK_R_SPACER;
        disp.drawButton(TRACK_R_X+x_offset, TRACK_R_Y+y_offset, TRACK_R_BOXSIZE_X, TRACK_R_BOXSIZE_Y, BUT_OUTLINE, BUT_ON_BGD, TRACK_R_TEXT_X_OFFSET-2, TRACK_R_TEXT_Y_OFFSET, "  King  ");
    }   
    // increment tracking rate by 0.02 Hz
    y_offset += TRACK_R_BOXSIZE_Y+TRACK_R_SPACER+TRACK_R_GROUP_SPACER ; // space between tracking setting fields
    if (incTrackRate) {
        disp.drawButton(TRACK_R_X+x_offset, TRACK_R_Y+y_offset, TRACK_R_BOXSIZE_X, TRACK_R_BOXSIZE_Y, BUT_OUTLINE, BUT_ON_BGD, TRACK_R_TEXT_X_OFFSET, TRACK_R_TEXT_Y_OFFSET, "IncTrack");
        incTrackRate = false;
    } else {
        disp.drawButton(TRACK_R_X+x_offset, TRACK_R_Y+y_offset, TRACK_R_BOXSIZE_X, TRACK_R_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, TRACK_R_TEXT_X_OFFSET, TRACK_R_TEXT_Y_OFFSET, "IncTrack"); 
    }   
    // decrement tracking rate by 0.02 Hz
    y_offset += TRACK_R_BOXSIZE_Y+TRACK_R_SPACER; 
    if (decTrackRate) {
        disp.drawButton(TRACK_R_X+x_offset, TRACK_R_Y+y_offset, TRACK_R_BOXSIZE_X, TRACK_R_BOXSIZE_Y, BUT_OUTLINE, BUT_ON_BGD, TRACK_R_TEXT_X_OFFSET, TRACK_R_TEXT_Y_OFFSET, "DecTrack");
        decTrackRate = false;
    } else {
        disp.drawButton(TRACK_R_X+x_offset, TRACK_R_Y+y_offset, TRACK_R_BOXSIZE_X, TRACK_R_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, TRACK_R_TEXT_X_OFFSET, TRACK_R_TEXT_Y_OFFSET, "DecTrack"); 
    }   

    // Reset Tracking Rate Sidereal
    y_offset += TRACK_R_BOXSIZE_Y+TRACK_R_SPACER+TRACK_R_GROUP_SPACER ; 
    if (rstTrackRate) {
        disp.drawButton(TRACK_R_X+x_offset, TRACK_R_Y+y_offset, TRACK_R_BOXSIZE_X, TRACK_R_BOXSIZE_Y, BUT_OUTLINE, BUT_ON_BGD, TRACK_R_TEXT_X_OFFSET, TRACK_R_TEXT_Y_OFFSET, "Reseting");
        rstTrackRate = false;
    } else {
        disp.drawButton(TRACK_R_X+x_offset, TRACK_R_Y+y_offset, TRACK_R_BOXSIZE_X, TRACK_R_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, TRACK_R_TEXT_X_OFFSET, TRACK_R_TEXT_Y_OFFSET, "RstTrack"); 
    }   

    // Show current Tracking Rate
    // For ALT/AZ this is always shows the default rate
    char _sideRate[12];
    y_offset += TRACK_R_BOXSIZE_Y+TRACK_R_SPACER+12;
    char sideRate[10];
    disp.getLocalCmdTrim(":GT#", sideRate);
    sprintf(_sideRate, "TR=%s", sideRate);
    disp.canvPrint(TRACK_R_X-6, TRACK_R_Y+y_offset, 0, 90, 16, TEXT_COLOR, BUT_BACKGROUND, _sideRate);

    x_offset = 0;
    y_offset = 0;

    // Filter Selection Button - circular selection of 3 values
    if (filterBut || firstDraw) {
      disp.drawButton(FM_X + x_offset, FM_Y + y_offset, FM_BOXSIZE_X, FM_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, FM_TEXT_X_OFFSET+8, FM_TEXT_Y_OFFSET,  activeFilterStr[activeFilter]);
      if (activeFilter == FM_ALIGN_ALL_SKY && !objectSelected) {
      disp.canvPrint(120, 382, 0, 199, 16, TEXT_COLOR, BUT_BACKGROUND, "All Sky For STARS only");
      }
      filterBut = false;
    }

    // Clear Custom Catalog
     y_offset += FM_BOXSIZE_Y + FM_SPACER_Y;
    if (clrCustom) {
      if (!yesCancelActive) {
        yesCancelActive = true;
        disp.drawButton(MISC_X + x_offset, MISC_Y + y_offset, 30, MISC_BOXSIZE_Y, BUT_OUTLINE, BUT_ON_BGD, MISC_TEXT_X_OFFSET, MISC_TEXT_Y_OFFSET,   "Yes");
        disp.drawButton(MISC_X + 40, MISC_Y + y_offset, 60, MISC_BOXSIZE_Y, BUT_OUTLINE, BUT_ON_BGD, MISC_TEXT_X_OFFSET, MISC_TEXT_Y_OFFSET,   "Cancel");
        if (!objectSelected) disp.canvPrint(120, 382, 0, 199, 16, BUT_ON_BGD, BUT_BACKGROUND, "Delete Custom Catalog?!");
      }
      if (yesBut) { // go ahead and clear
        disp.drawButton(MISC_X + x_offset, MISC_Y + y_offset, MISC_BOXSIZE_X, MISC_BOXSIZE_Y, BUT_OUTLINE, BUT_ON_BGD, MISC_TEXT_X_OFFSET+8, MISC_TEXT_Y_OFFSET,   " Clearing ");
        
        File rmFile = SD.open("/custom.csv");
          if (rmFile) {
            SD.remove("/custom.csv");
          }
        rmFile.close(); 

        yesBut = false;
        clrCustom = false;
        yesCancelActive = false;
        refreshScreen = true; 
      }
      if (cancelBut) {
        cancelBut = false;
        clrCustom = false;
        yesCancelActive = false;
        refreshScreen = true; 
        disp.drawButton(MISC_X + x_offset, MISC_Y + y_offset, MISC_BOXSIZE_X, MISC_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, MISC_TEXT_X_OFFSET+5, MISC_TEXT_Y_OFFSET, "Clr Custom");
        if (objectSelected) tft.fillRect(x, y, 199, 5*16, PAGE_BACKGROUND);
      }
    } else {
      disp.drawButton(MISC_X + x_offset, MISC_Y + y_offset, MISC_BOXSIZE_X, MISC_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, MISC_TEXT_X_OFFSET+5, MISC_TEXT_Y_OFFSET, "Clr Custom");
    }

    // Buzzer Enable Button
    y_offset += MISC_BOXSIZE_Y + MISC_SPACER_Y;
    if (soundEnabled) {
      disp.drawButton(MISC_X + x_offset, MISC_Y + y_offset, MISC_BOXSIZE_X, MISC_BOXSIZE_Y, BUT_OUTLINE, BUT_ON_BGD, MISC_TEXT_X_OFFSET+12, MISC_TEXT_Y_OFFSET, "Buzzer On ");
    } else { //off
      disp.drawButton(MISC_X + x_offset, MISC_Y + y_offset, MISC_BOXSIZE_X, MISC_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, MISC_TEXT_X_OFFSET+12, MISC_TEXT_Y_OFFSET, "Buzzer Off");
    }

    // Larger Button Text for GoTo and Abort
    tft.setFont(&UbuntuMono_Bold11pt7b); 

    // Go To Coordinates Button
    if (goToPgBut) {
      disp.drawButton( GOTO_BUT_X, GOTO_BUT_Y,  GOTO_M_BOXSIZE_X, GOTO_M_BOXSIZE_Y, BUT_OUTLINE, BUT_ON_BGD, GOTO_TXT_OFF_X-2, GOTO_TXT_OFF_Y, "Going");
      goToPgBut = false;
    } else {
      if (!isSlewing()) { 
        disp.drawButton( GOTO_BUT_X, GOTO_BUT_Y,  GOTO_M_BOXSIZE_X, GOTO_M_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, GOTO_TXT_OFF_X+5, GOTO_TXT_OFF_Y, "GoTo"); 
      } else {
        refreshScreen = true;
      }
    }
    
    // Abort GoTo Button
    if (abortPgBut) {
      disp.drawButton(ABORT_M_BUT_X, ABORT_M_BUT_Y, GOTO_M_BOXSIZE_X, GOTO_M_BOXSIZE_Y, BUT_OUTLINE, BUT_ON_BGD, GOTO_TXT_OFF_X-5, GOTO_TXT_OFF_Y, "Aborting"); 
      abortPgBut = false;
    } else {
      disp.drawButton(ABORT_M_BUT_X, ABORT_M_BUT_Y, GOTO_M_BOXSIZE_X, GOTO_M_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, GOTO_TXT_OFF_X, GOTO_TXT_OFF_Y, "Abort"); 
    }

    tft.setFont(&UbuntuMono_Bold8pt7b); // Text back to default

    // Draw the Catalog Buttons
    char title[16]="";
    y_offset = 0;
    for (uint16_t i=1; i<=cat_mgr.numCatalogs(); i++) {
      cat_mgr.select(i-1);
      strcpy(title,cat_mgr.catalogTitle());
      disp.drawButton(CAT_SEL_X, CAT_SEL_Y+y_offset, CAT_SEL_BOXSIZE_X, CAT_SEL_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, CAT_SEL_TEXT_X_OFFSET, CAT_SEL_TEXT_Y_OFFSET, title);
      y_offset += CAT_SEL_SPACER;
    }

    // Planet Catalog Button
    disp.drawButton(CAT_SEL_X, CAT_SEL_Y+y_offset, CAT_SEL_BOXSIZE_X, CAT_SEL_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, CAT_SEL_TEXT_X_OFFSET, CAT_SEL_TEXT_Y_OFFSET, "Planets");

    y_offset += CAT_SEL_SPACER;
    // Treasure Catalog
    disp.drawButton(CAT_SEL_X, CAT_SEL_Y+y_offset, CAT_SEL_BOXSIZE_X, CAT_SEL_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, CAT_SEL_TEXT_X_OFFSET, CAT_SEL_TEXT_Y_OFFSET, "Treasure");

    y_offset += CAT_SEL_SPACER;
    // Custom User Catalog Button
    disp.drawButton(CAT_SEL_X, CAT_SEL_Y+y_offset, CAT_SEL_BOXSIZE_X, CAT_SEL_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, CAT_SEL_TEXT_X_OFFSET, CAT_SEL_TEXT_Y_OFFSET, "Custom Cat");
  }
  screenTouched = false;
}

//================================================
// ===== TouchScreen Detect "MORE" page ==========
//================================================
void touchMoreUpdate() {

  // Home Page ICON Button
  if (p.x > 10 && p.x < 50 && p.y > 5 && p.y < 37) {
    soundClick();
    drawHomePage();
    return;
  }

  // Select Tracking Rates
  int x_offset = 0;
  int y_offset = 0;

  // Sidereal Rate 
  if (p.y > TRACK_R_Y+y_offset && p.y < (TRACK_R_Y+y_offset + TRACK_R_BOXSIZE_Y) && p.x > TRACK_R_X && p.x < (TRACK_R_X + TRACK_R_BOXSIZE_X)) {
      disp.setLocalCmd(":TQ#");
      soundClick();
      sideRate = true;
      lunarRate = false;
      kingRate = false;
      return;
  }
  // Lunar Rate
  y_offset += TRACK_R_BOXSIZE_Y+TRACK_R_SPACER;
  if (p.y > TRACK_R_Y+y_offset && p.y < (TRACK_R_Y+y_offset + TRACK_R_BOXSIZE_Y) && p.x > TRACK_R_X && p.x < (TRACK_R_X + TRACK_R_BOXSIZE_X)) {
      disp.setLocalCmd(":TL#");
      soundClick();
      sideRate = false;
      lunarRate = true;
      kingRate = false;
      return;
  }
  // King Rate
  y_offset += TRACK_R_BOXSIZE_Y+TRACK_R_SPACER;
  if (p.y > TRACK_R_Y+y_offset && p.y < (TRACK_R_Y+y_offset + TRACK_R_BOXSIZE_Y) && p.x > TRACK_R_X && p.x < (TRACK_R_X + TRACK_R_BOXSIZE_X)) {
      disp.setLocalCmd(":TK#");
      soundClick();
      sideRate = false;
      lunarRate = false;
      kingRate = true;
      return;
  }
  // Increment tracking rate
  y_offset += TRACK_R_BOXSIZE_Y+TRACK_R_GROUP_SPACER ;
  if (p.y > TRACK_R_Y+y_offset && p.y < (TRACK_R_Y+y_offset + TRACK_R_BOXSIZE_Y) && p.x > TRACK_R_X && p.x < (TRACK_R_X + TRACK_R_BOXSIZE_X)) {
      disp.setLocalCmd(":T+#");
      soundClick();
      incTrackRate = true;
      decTrackRate = false;
      return;
  }
  // Decrement tracking rate
  y_offset += TRACK_R_BOXSIZE_Y+TRACK_R_SPACER;
  if (p.y > TRACK_R_Y+y_offset && p.y < (TRACK_R_Y+y_offset + TRACK_R_BOXSIZE_Y) && p.x > TRACK_R_X && p.x < (TRACK_R_X + TRACK_R_BOXSIZE_X)) {
      disp.setLocalCmd(":T-#");
      soundClick();
      incTrackRate = false;
      decTrackRate = true;
      return;
  }
  // Reset tracking rate
  y_offset += TRACK_R_BOXSIZE_Y+TRACK_R_GROUP_SPACER ;
  if (p.y > TRACK_R_Y+y_offset && p.y < (TRACK_R_Y+y_offset + TRACK_R_BOXSIZE_Y) && p.x > TRACK_R_X && p.x < (TRACK_R_X + TRACK_R_BOXSIZE_X)) {
      disp.setLocalCmd(":TR#");
      soundClick();
      rstTrackRate = true;
      return;
  }

  y_offset = 0;
  // Filter Select Button
  if (p.x > FM_X + x_offset && p.x < FM_X + x_offset + FM_BOXSIZE_X && p.y > FM_Y + y_offset && p.y <  FM_Y + y_offset + FM_BOXSIZE_Y) {
    soundClick();
    filterBut = true;
    // circular selection
    if (activeFilter == FM_NONE) {
      activeFilter = FM_ABOVE_HORIZON; // filter disallows alt < 10 deg
      cat_mgr.filterAdd(activeFilter); 
    } else if (activeFilter == FM_ABOVE_HORIZON) {
      activeFilter = FM_ALIGN_ALL_SKY; // Used for stars only here: filter only allows Mag>=3; Alt>=10; Dec<=80
      cat_mgr.filterAdd(activeFilter); 
    } else if (activeFilter == FM_ALIGN_ALL_SKY) {
      activeFilter = FM_NONE;  // no filter
      cat_mgr.filtersClear();
    }
    return;
  }

  y_offset = 0;
  // Clear Custom Button
  y_offset += FM_BOXSIZE_Y + FM_SPACER_Y;
  if (!yesCancelActive && p.x > MISC_X + x_offset && p.x < MISC_X + x_offset + MISC_BOXSIZE_X && p.y > MISC_Y + y_offset && p.y <  MISC_Y + y_offset + MISC_BOXSIZE_Y) {
    clrCustom = true;
    soundClick();
    return;
  }
  // Clearing Custom Catalog "Yes"
  if (p.x > MISC_X && p.x < MISC_X + 30 && p.y > MISC_Y + y_offset && p.y <  MISC_Y + y_offset + MISC_BOXSIZE_Y) {
    yesBut = true;
    clrCustom = true;
    soundClick();
    return;
  }
  // Clearing Custom catalog "Cancel"
  if (p.x > MISC_X + 40 && p.x < MISC_X + 120 && p.y > MISC_Y + y_offset && p.y <  MISC_Y + y_offset + MISC_BOXSIZE_Y) {
    cancelBut = true;
    clrCustom = true;
    soundClick();
    return;
  }

  // Buzzer Button
  y_offset += MISC_BOXSIZE_Y + MISC_SPACER_Y;
  if (p.x > MISC_X + x_offset && p.x < MISC_X + x_offset + MISC_BOXSIZE_X && p.y > MISC_Y + y_offset && p.y <  MISC_Y + y_offset + MISC_BOXSIZE_Y) {
    soundClick();
    if (!soundEnabled) {
      soundEnabled = true; // turn on
    } else {
      soundEnabled = false; // toggle off
    }
    return;
  }

  // **** Go To Target Coordinates ****
  if (p.y > GOTO_BUT_Y && p.y < (GOTO_BUT_Y + GOTO_M_BOXSIZE_Y) && p.x > GOTO_BUT_X && p.x < (GOTO_BUT_X + GOTO_M_BOXSIZE_X)) {
    soundClick();
    goToPgBut = true;
    if (trackingState == TrackingNone) { // start tracking if required
        trackingState = TrackingSidereal;
    }
    axis1Enabled = true;
    disp.setLocalCmd(":MS#"); // move to
    return;
  }

  // **** ABORT GOTO ****
  if (p.y > ABORT_M_BUT_Y && p.y < (ABORT_M_BUT_Y + GOTO_M_BOXSIZE_Y) && p.x > ABORT_M_BUT_X && p.x < (ABORT_M_BUT_X + GOTO_M_BOXSIZE_X)) {
    soundFreq(1500);
    abortPgBut = true;
    disp.setLocalCmd(":Q#"); // stops move
    stopMotors(); // do this for safety reasons...mount may be colliding with something
    return;
  }

  // CATALOG Array Selection Buttons 
  y_offset = 0;
  for (uint16_t i=1; i<=cat_mgr.numCatalogs(); i++) {
    if (p.x > CAT_SEL_X && p.x < CAT_SEL_X + CAT_SEL_BOXSIZE_X && p.y > CAT_SEL_Y+y_offset  && p.y < CAT_SEL_Y+y_offset + CAT_SEL_BOXSIZE_Y) {
      soundClick();
      
      // disable ALL_SKY filter if any DSO catalog...it's for STARS only
      if (i != 1 && activeFilter == FM_ALIGN_ALL_SKY) { // 1 is STARS
        cat_mgr.filtersClear();
        activeFilter = FM_NONE;
        return;
      } 
      drawCatalogPage(i-1);
      return;
    }
    y_offset += CAT_SEL_SPACER;
  }

  // Planet Catalog Select Button
  if (p.x > CAT_SEL_X && p.x < CAT_SEL_X + CAT_SEL_BOXSIZE_X && p.y > CAT_SEL_Y+y_offset  && p.y < CAT_SEL_Y+y_offset + CAT_SEL_BOXSIZE_Y) {
    soundClick();
    drawPlanetsPage();
    return;
  }

  // Treasure catalog select Button
  y_offset += CAT_SEL_SPACER;
  if (p.x > CAT_SEL_X && p.x < CAT_SEL_X + CAT_SEL_BOXSIZE_X && p.y > CAT_SEL_Y+y_offset  && p.y < CAT_SEL_Y+y_offset + CAT_SEL_BOXSIZE_Y) {
    soundClick();
    drawCatalogPage(cat_mgr.numCatalogs()+1);
    return;
  }

    // User custom catalog select Button
  y_offset += CAT_SEL_SPACER;
  if (p.x > CAT_SEL_X && p.x < CAT_SEL_X + CAT_SEL_BOXSIZE_X && p.y > CAT_SEL_Y+y_offset  && p.y < CAT_SEL_Y+y_offset + CAT_SEL_BOXSIZE_Y) {
    soundClick();
    drawCatalogPage(cat_mgr.numCatalogs()+2);
    return;
  }
}