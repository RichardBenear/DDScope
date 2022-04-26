// =========================================================
// ================ Touchscreen Handler ====================
// =========================================================
// Author: Richard Benear
// 3/25/21

// This is calibration data for the raw touch data to the screen coordinates
#define TS_MINX                250 
#define TS_MINY                250
#define TS_MAXX               3900
#define TS_MAXY               3900
#define MINPRESSURE             80
#define MAXPRESSURE           1000
#define PENRADIUS                3

void updateTouchInput() {
  if (ts.touched()) {
    if (screenTouched) return; // still processing last button press
    screenTouched = true;

    p = ts.getPoint();      

    // Scale from ~0->4000 to tft.width using the calibration #'s
    //VF("x="); V(p.x); VF(", y="); V(p.y); VF(", z="); VL(p.z); // for calibration
    p.x = map(p.x, TS_MINX, TS_MAXX, 0, tft.width());
    p.y = map(p.y, TS_MINY, TS_MAXY, 0, tft.height());
    //VF("x="); V(p.x); VF(", y="); V(p.y); VF(", z="); VL(p.z); //for calibration

    // check for touchscreen action on selected Page
    switch (currentPage) {
        case HOME_PAGE: touchHomeUpdate(); break;
        case GUIDE_PAGE: touchGuideUpdate(); break;
        case FOCUSER_PAGE: touchFocuserUpdate(); break;
        case GOTO_PAGE: touchGoToUpdate(); break;
        case MORE_PAGE: touchMoreUpdate(); break;
        case ODRIVE_PAGE: touchOdriveUpdate(); break;
        case XSTATUS_PAGE: touchXStatusUpdate(); break;
        case SETTINGS_PAGE: touchSettingsUpdate(); break;
        case ALIGN_PAGE: touchAlignUpdate(); break;
        case CATALOG_PAGE: touchCatalogUpdate(); break;
        case PLANETS_PAGE: touchPlanetsUpdate(); break;
        default: touchHomeUpdate(); break;
    }

    // =============== MENU MAP ================
    // Current Page   |Cur |Col1|Col2|Col3|Col4|
    // Home-----------| Ho | Gu | Fo | GT | Mo |
    // Guide----------| Gu | Ho | Fo | Al | Mo |
    // Focuser--------| Fo | Ho | Gu | GT | Mo |
    // GoTo-----------| GT | Ho | Fo | Gu | Mo |
    // More-CATs------| Mo | GT | Se | Od | Al |
    // ODrive---------| Od | Ho | Se | Al | Xs |
    // Extended Status| Xs | Ho | Se | Al | Od |
    // Settings-------| Se | Ho | Fo | Al | Od |
    // Alignment------| Al | Ho | Fo | Gu | Od |
    
    // Detect which Menu page requested
    // == LeftMost Menu Button ==
    if ((currentPage == CATALOG_PAGE) || 
        (currentPage == PLANETS_PAGE) ||  
        (currentPage == CUST_CAT_PAGE)) return; //skip checking these page menus since they don't exist
    
    if (p.y > MENU_Y && p.y < (MENU_Y + MENU_BOXSIZE_Y) && p.x > (MENU_X                   ) && p.x < (MENU_X                    + MENU_BOXSIZE_X)) {
      soundClick();
      switch(currentPage) {
          case HOME_PAGE: drawGuidePage(); break;
          case GUIDE_PAGE: drawHomePage(); break;
          case FOCUSER_PAGE: drawHomePage(); break;
          case GOTO_PAGE: drawHomePage(); break;
          case MORE_PAGE: drawGoToPage(); break;
          case ODRIVE_PAGE: drawHomePage(); break;
          case XSTATUS_PAGE: drawHomePage(); break;
          case SETTINGS_PAGE: drawHomePage(); break;
          case ALIGN_PAGE: drawHomePage(); break;
          default: drawHomePage(); break;
      }
    }
    // == Center Left Menu - Column 2 ==
    if (p.y > MENU_Y && p.y < (MENU_Y + MENU_BOXSIZE_Y) && p.x > (MENU_X +   MENU_X_SPACING) && p.x < (MENU_X +   MENU_X_SPACING + MENU_BOXSIZE_X)) {
      soundClick();
      switch(currentPage) {
          case HOME_PAGE: drawFocuserPage(); break;
          case GUIDE_PAGE: drawFocuserPage(); break;
          case FOCUSER_PAGE: drawGuidePage(); break;
          case GOTO_PAGE: drawFocuserPage(); break;
          case MORE_PAGE: drawSettingsPage(); break;
          case ODRIVE_PAGE: drawSettingsPage(); break;
          case XSTATUS_PAGE: drawSettingsPage(); break;
          case SETTINGS_PAGE: drawFocuserPage(); break;
          case ALIGN_PAGE: drawFocuserPage(); break;
          default: drawHomePage(); break;
      }
    }
    // == Center Right Menu - Column 3 ==
    if (p.y > MENU_Y && p.y < (MENU_Y + MENU_BOXSIZE_Y) && p.x > (MENU_X + 2*MENU_X_SPACING) && p.x < (MENU_X + 2*MENU_X_SPACING + MENU_BOXSIZE_X)) {
      soundClick();
      switch(currentPage) {
          case HOME_PAGE: drawGoToPage(); break;
          case GUIDE_PAGE: drawAlignPage(); break;
          case FOCUSER_PAGE: drawGoToPage(); break;
          case GOTO_PAGE: drawGuidePage(); break;
          case MORE_PAGE: drawOdrivePage(); break;
          case ODRIVE_PAGE: drawAlignPage(); break;
          case XSTATUS_PAGE: drawAlignPage(); break;
          case SETTINGS_PAGE: drawAlignPage(); break;
          case ALIGN_PAGE: drawGuidePage(); break;
          default: drawHomePage(); break;
      }
    }
    // == Right Menu - Column 4 ==
    if (p.y > MENU_Y && p.y < (MENU_Y + MENU_BOXSIZE_Y) && p.x > (MENU_X + 3*MENU_X_SPACING) && p.x < (MENU_X + 3*MENU_X_SPACING + MENU_BOXSIZE_X)) { 
      soundClick();     
      switch(currentPage) {
          case HOME_PAGE: drawMorePage(); break;
          case GUIDE_PAGE: drawMorePage(); break;
          case FOCUSER_PAGE: drawMorePage(); break;
          case GOTO_PAGE: drawMorePage(); break;
          case MORE_PAGE: drawAlignPage(); break;
          case ODRIVE_PAGE: drawXStatusPage(); break;
          case XSTATUS_PAGE: drawOdrivePage(); break;
          case SETTINGS_PAGE: drawOdrivePage(); break;
          case ALIGN_PAGE: drawOdrivePage(); break;
          default: drawHomePage(); break;
      }
    }
  } 
}