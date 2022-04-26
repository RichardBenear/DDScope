
// ============================================
// ============ Display GUIDE Page ============
// ============================================
// Author: Richard Benear
// 8/22/21

// Guide buttons
#define GUIDE_BOXSIZE_X          85
#define GUIDE_BOXSIZE_Y          65 
#define GUIDE_TEXT_X_OFFSET       3
#define GUIDE_TEXT_Y_OFFSET      40 
#define SYNC_OFFSET_X           113 
#define SYNC_OFFSET_Y           290 
#define LEFT_OFFSET_X             7 
#define LEFT_OFFSET_Y           SYNC_OFFSET_Y
#define RIGHT_OFFSET_X          219
#define RIGHT_OFFSET_Y          SYNC_OFFSET_Y
#define UP_OFFSET_X             SYNC_OFFSET_X
#define UP_OFFSET_Y             220 
#define DOWN_OFFSET_X           SYNC_OFFSET_X
#define DOWN_OFFSET_Y           360 

// Guide rates buttons
#define GUIDE_R_X                3
#define GUIDE_R_Y              162
#define GUIDE_R_BOXSIZE_X       74 
#define GUIDE_R_BOXSIZE_Y       28
#define GUIDE_R_SPACER           6 
#define GUIDE_R_TEXT_X_OFFSET    2 
#define GUIDE_R_TEXT_Y_OFFSET   17

// Stop guide button
#define STOP_X                    7
#define STOP_Y                  385 
#define STOP_BOXSIZE_X           86 
#define STOP_BOXSIZE_Y           40 
#define STOP_TEXT_X_OFFSET        4
#define STOP_TEXT_Y_OFFSET       25

// Spiral Search button
#define SPIRAL_X                220
#define SPIRAL_Y                385 
#define SPIRAL_BOXSIZE_X         86 
#define SPIRAL_BOXSIZE_Y         40 
#define SPIRAL_TEXT_X_OFFSET      2
#define SPIRAL_TEXT_Y_OFFSET     25

bool guidingEast = false;
bool guidingWest = false;
bool guidingNorth = false;
bool guidingSouth = false;
bool oneXisOn = true;
bool eightXisOn = false;
bool twentyXisOn = false;
bool HalfMaxisOn = false;
bool syncOn = false;
bool spiralOn = false;
bool stopPressed = false;

// Draw the GUIDE Page
void drawGuidePage() { 
    disp.updateColors();
    tft.setTextColor(TEXT_COLOR);
    tft.fillScreen(PAGE_BACKGROUND);
    currentPage = GUIDE_PAGE;
    disp.drawMenuButtons();
    disp.drawTitle(110, 30, "Guiding");
    disp.drawCommonStatusLabels();
    disp.updateOnstepCmdStatus();
}

// ========== Update Guide Page Status ==========
void updateGuideStatus() {
    
    // Update for buttons only if the screen is touched
    if (screenTouched || firstDraw || refreshScreen) {
        refreshScreen = false;
        if (screenTouched) refreshScreen = true;
        
        // update current status of guide buttons
        tft.setFont(&UbuntuMono_Bold14pt7b);
        if (!guidingEast && !trackingSyncInProgress() && (trackingState != TrackingMoveTo)) {
            disp.drawButton(RIGHT_OFFSET_X, RIGHT_OFFSET_Y, GUIDE_BOXSIZE_X, GUIDE_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, GUIDE_TEXT_X_OFFSET, GUIDE_TEXT_Y_OFFSET, " EAST");
        } else {
            disp.drawButton(RIGHT_OFFSET_X, RIGHT_OFFSET_Y, GUIDE_BOXSIZE_X, GUIDE_BOXSIZE_Y, BUT_OUTLINE, BUT_ON_BGD, GUIDE_TEXT_X_OFFSET, GUIDE_TEXT_Y_OFFSET, " EAST");
        }

        if (!guidingWest && !trackingSyncInProgress() && (trackingState != TrackingMoveTo)) {
            disp.drawButton(LEFT_OFFSET_X, LEFT_OFFSET_Y, GUIDE_BOXSIZE_X, GUIDE_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, GUIDE_TEXT_X_OFFSET, GUIDE_TEXT_Y_OFFSET, " WEST");
        } else {
            disp.drawButton(LEFT_OFFSET_X, LEFT_OFFSET_Y, GUIDE_BOXSIZE_X, GUIDE_BOXSIZE_Y, BUT_OUTLINE, BUT_ON_BGD, GUIDE_TEXT_X_OFFSET, GUIDE_TEXT_Y_OFFSET, " WEST");
        }

        if (!guidingNorth && !trackingSyncInProgress() && (trackingState != TrackingMoveTo)) {
            disp.drawButton(UP_OFFSET_X, UP_OFFSET_Y, GUIDE_BOXSIZE_X, GUIDE_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, GUIDE_TEXT_X_OFFSET, GUIDE_TEXT_Y_OFFSET, "  UP ");
        } else {
            disp.drawButton(UP_OFFSET_X, UP_OFFSET_Y, GUIDE_BOXSIZE_X, GUIDE_BOXSIZE_Y, BUT_OUTLINE, BUT_ON_BGD, GUIDE_TEXT_X_OFFSET, GUIDE_TEXT_Y_OFFSET, "  UP ");
        }

        if (!guidingSouth && !trackingSyncInProgress() && (trackingState != TrackingMoveTo)) {
            disp.drawButton(DOWN_OFFSET_X, DOWN_OFFSET_Y, GUIDE_BOXSIZE_X, GUIDE_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, GUIDE_TEXT_X_OFFSET, GUIDE_TEXT_Y_OFFSET, " DOWN");
        } else {
            disp.drawButton(DOWN_OFFSET_X, DOWN_OFFSET_Y, GUIDE_BOXSIZE_X, GUIDE_BOXSIZE_Y, BUT_OUTLINE, BUT_ON_BGD, GUIDE_TEXT_X_OFFSET, GUIDE_TEXT_Y_OFFSET, " DOWN");
        }
        
        if (!syncOn) {
            disp.drawButton(SYNC_OFFSET_X, SYNC_OFFSET_Y, GUIDE_BOXSIZE_X, GUIDE_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, GUIDE_TEXT_X_OFFSET, GUIDE_TEXT_Y_OFFSET, " SYNC  ");
        } else {
            disp.drawButton(SYNC_OFFSET_X, SYNC_OFFSET_Y, GUIDE_BOXSIZE_X, GUIDE_BOXSIZE_Y, BUT_OUTLINE, BUT_ON_BGD, GUIDE_TEXT_X_OFFSET, GUIDE_TEXT_Y_OFFSET, "SYNCng");
            syncOn = false;
        }
        tft.setFont(&Inconsolata_Bold8pt7b); 

        // Draw Guide Rates Buttons
        int y_offset = 0;
        int x_offset = 0;
        int spacer = GUIDE_R_SPACER;
        if (oneXisOn) {   
            disp.drawButton(GUIDE_R_X+x_offset, GUIDE_R_Y+y_offset, GUIDE_R_BOXSIZE_X, GUIDE_R_BOXSIZE_Y, BUT_OUTLINE, BUT_ON_BGD, GUIDE_R_TEXT_X_OFFSET+4, GUIDE_R_TEXT_Y_OFFSET,     " Guide  ");
            x_offset = x_offset + GUIDE_R_BOXSIZE_X+spacer;
            disp.drawButton(GUIDE_R_X+x_offset, GUIDE_R_Y+y_offset, GUIDE_R_BOXSIZE_X, GUIDE_R_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, GUIDE_R_TEXT_X_OFFSET+2, GUIDE_R_TEXT_Y_OFFSET, " Center "); 
            x_offset = x_offset + GUIDE_R_BOXSIZE_X+spacer;
            disp.drawButton(GUIDE_R_X+x_offset, GUIDE_R_Y+y_offset, GUIDE_R_BOXSIZE_X, GUIDE_R_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, GUIDE_R_TEXT_X_OFFSET, GUIDE_R_TEXT_Y_OFFSET,   "  Find  ");
            x_offset = x_offset + GUIDE_R_BOXSIZE_X+spacer;
            disp.drawButton(GUIDE_R_X+x_offset, GUIDE_R_Y+y_offset, GUIDE_R_BOXSIZE_X, GUIDE_R_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, GUIDE_R_TEXT_X_OFFSET, GUIDE_R_TEXT_Y_OFFSET,   "Half Max"); 
        } 

        if (eightXisOn) {   
            x_offset = 0;
            disp.drawButton(GUIDE_R_X+x_offset, GUIDE_R_Y+y_offset, GUIDE_R_BOXSIZE_X, GUIDE_R_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, GUIDE_R_TEXT_X_OFFSET+4, GUIDE_R_TEXT_Y_OFFSET, " Guide  ");
            x_offset = x_offset + GUIDE_R_BOXSIZE_X+spacer;
            disp.drawButton(GUIDE_R_X+x_offset, GUIDE_R_Y+y_offset, GUIDE_R_BOXSIZE_X, GUIDE_R_BOXSIZE_Y, BUT_OUTLINE, BUT_ON_BGD, GUIDE_R_TEXT_X_OFFSET+2, GUIDE_R_TEXT_Y_OFFSET,     " Center "); 
            x_offset = x_offset + GUIDE_R_BOXSIZE_X+spacer;
            disp.drawButton(GUIDE_R_X+x_offset, GUIDE_R_Y+y_offset, GUIDE_R_BOXSIZE_X, GUIDE_R_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, GUIDE_R_TEXT_X_OFFSET, GUIDE_R_TEXT_Y_OFFSET,   "  Find  ");
            x_offset = x_offset + GUIDE_R_BOXSIZE_X+spacer;
            disp.drawButton(GUIDE_R_X+x_offset, GUIDE_R_Y+y_offset, GUIDE_R_BOXSIZE_X, GUIDE_R_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, GUIDE_R_TEXT_X_OFFSET, GUIDE_R_TEXT_Y_OFFSET,   "Half Max");
        }   

        if (twentyXisOn) {
            x_offset = 0;
            disp.drawButton(GUIDE_R_X+x_offset, GUIDE_R_Y+y_offset, GUIDE_R_BOXSIZE_X, GUIDE_R_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, GUIDE_R_TEXT_X_OFFSET+4, GUIDE_R_TEXT_Y_OFFSET, " Guide  ");
            x_offset = x_offset + GUIDE_R_BOXSIZE_X+spacer;
            disp.drawButton(GUIDE_R_X+x_offset, GUIDE_R_Y+y_offset, GUIDE_R_BOXSIZE_X, GUIDE_R_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, GUIDE_R_TEXT_X_OFFSET+2, GUIDE_R_TEXT_Y_OFFSET, " Center "); 
            x_offset = x_offset + GUIDE_R_BOXSIZE_X+spacer;
            disp.drawButton(GUIDE_R_X+x_offset, GUIDE_R_Y+y_offset, GUIDE_R_BOXSIZE_X, GUIDE_R_BOXSIZE_Y, BUT_OUTLINE, BUT_ON_BGD, GUIDE_R_TEXT_X_OFFSET, GUIDE_R_TEXT_Y_OFFSET,       "  Find  ");
            x_offset = x_offset + GUIDE_R_BOXSIZE_X+spacer; 
            disp.drawButton(GUIDE_R_X+x_offset, GUIDE_R_Y+y_offset, GUIDE_R_BOXSIZE_X, GUIDE_R_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, GUIDE_R_TEXT_X_OFFSET, GUIDE_R_TEXT_Y_OFFSET,   "Half Max");
        }

        if (HalfMaxisOn) {
            x_offset = 0;
            disp.drawButton(GUIDE_R_X+x_offset, GUIDE_R_Y+y_offset, GUIDE_R_BOXSIZE_X, GUIDE_R_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, GUIDE_R_TEXT_X_OFFSET+4, GUIDE_R_TEXT_Y_OFFSET, " Guide  ");
            x_offset = x_offset + GUIDE_R_BOXSIZE_X+spacer;
            disp.drawButton(GUIDE_R_X+x_offset, GUIDE_R_Y+y_offset, GUIDE_R_BOXSIZE_X, GUIDE_R_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, GUIDE_R_TEXT_X_OFFSET+2, GUIDE_R_TEXT_Y_OFFSET, " Center "); 
            x_offset = x_offset + GUIDE_R_BOXSIZE_X+spacer;
            disp.drawButton(GUIDE_R_X+x_offset, GUIDE_R_Y+y_offset, GUIDE_R_BOXSIZE_X, GUIDE_R_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, GUIDE_R_TEXT_X_OFFSET, GUIDE_R_TEXT_Y_OFFSET,   "  Find  ");
            x_offset = x_offset + GUIDE_R_BOXSIZE_X+spacer; 
            disp.drawButton(GUIDE_R_X+x_offset, GUIDE_R_Y+y_offset, GUIDE_R_BOXSIZE_X, GUIDE_R_BOXSIZE_Y, BUT_OUTLINE, BUT_ON_BGD, GUIDE_R_TEXT_X_OFFSET, GUIDE_R_TEXT_Y_OFFSET,       "Half Max");
        }        

        if (spiralOn) {  
            disp.drawButton(SPIRAL_X, SPIRAL_Y, SPIRAL_BOXSIZE_X, SPIRAL_BOXSIZE_Y, BUT_OUTLINE, BUT_ON_BGD, SPIRAL_TEXT_X_OFFSET+2, SPIRAL_TEXT_Y_OFFSET, "Spiral On");    
        } else {
            disp.drawButton(SPIRAL_X, SPIRAL_Y, SPIRAL_BOXSIZE_X, SPIRAL_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, SPIRAL_TEXT_X_OFFSET, SPIRAL_TEXT_Y_OFFSET, "Spiral Off");
        }    

        if (stopPressed) {  
            disp.drawButton(STOP_X, STOP_Y, STOP_BOXSIZE_X, STOP_BOXSIZE_Y, BUT_OUTLINE, BUT_ON_BGD, STOP_TEXT_X_OFFSET, STOP_TEXT_Y_OFFSET,   "Stopping"); 
            stopPressed = false;
        } else {
            disp.drawButton(STOP_X, STOP_Y, STOP_BOXSIZE_X, STOP_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, STOP_TEXT_X_OFFSET+4, STOP_TEXT_Y_OFFSET, "  STOP  ");
        } 
    disp.updateCommonStatus();
    screenTouched = false;
    }  
}

// Manage Touching of Guiding Buttons
void touchGuideUpdate() {
    tft.setFont(&UbuntuMono_Bold11pt7b);
    // SYNC Button 
    if (p.y > SYNC_OFFSET_Y && p.y < (SYNC_OFFSET_Y + GUIDE_BOXSIZE_Y) && p.x > SYNC_OFFSET_X && p.x < (SYNC_OFFSET_X + GUIDE_BOXSIZE_X)) {            
        disp.setLocalCmd(":CS#"); // doesn't have reply
        soundClick();
        syncOn = true;  
    }
                    
    // EAST / RIGHT button
    if (p.y > RIGHT_OFFSET_Y && p.y < (RIGHT_OFFSET_Y + GUIDE_BOXSIZE_Y) && p.x > RIGHT_OFFSET_X && p.x < (RIGHT_OFFSET_X + GUIDE_BOXSIZE_X)) {
        if (!guidingEast) {
            disp.setLocalCmd(":Mw#"); // east west is swapped for DDScope
            guidingEast = true;
        } else {
            disp.setLocalCmd(":Qw#");
            guidingEast = false;
        }
        soundClick();
    }
                    
    // WEST / LEFT button
    if (p.y > LEFT_OFFSET_Y && p.y < (LEFT_OFFSET_Y + GUIDE_BOXSIZE_Y) && p.x > LEFT_OFFSET_X && p.x < (LEFT_OFFSET_X + GUIDE_BOXSIZE_X)) {
        if (!guidingWest) {
            disp.setLocalCmd(":Me#"); // east west is swapped for DDScope
            guidingWest = true;
        } else {
            disp.setLocalCmd(":Qe#");
            guidingWest = false;
        }
        soundClick();
    }
                    
    // NORTH / UP button
    if (p.y > UP_OFFSET_Y && p.y < (UP_OFFSET_Y + GUIDE_BOXSIZE_Y) && p.x > UP_OFFSET_X && p.x < (UP_OFFSET_X + GUIDE_BOXSIZE_X)) {
        if (!guidingNorth) {
            disp.setLocalCmd(":Mn#");
            guidingNorth = true;
        } else {
            disp.setLocalCmd(":Qn#");
            guidingNorth = false;
        }
        soundClick();
    }
                    
    // SOUTH / DOWN button
    if (p.y > DOWN_OFFSET_Y && p.y < (DOWN_OFFSET_Y + GUIDE_BOXSIZE_Y) && p.x > DOWN_OFFSET_X && p.x < (DOWN_OFFSET_X + GUIDE_BOXSIZE_X)) {
        if (!guidingSouth) {
            disp.setLocalCmd(":Ms#");
            guidingSouth = true;
        } else {
            disp.setLocalCmd(":Qs#");
            guidingSouth = false;
        }
        soundClick();
    }

    // Select Guide Rates
    int y_offset = 0;
    int x_offset = 0;  
    int spacer = GUIDE_R_SPACER;
    tft.setFont(&Inconsolata_Bold8pt7b);
    
    // 1x Guide Rate 
    if (p.y > GUIDE_R_Y+y_offset && p.y < (GUIDE_R_Y+y_offset + GUIDE_R_BOXSIZE_Y) && p.x > GUIDE_R_X+x_offset && p.x < (GUIDE_R_X+x_offset + GUIDE_R_BOXSIZE_X)) {
        disp.setLocalCmd(":RG#");
        soundClick();
        oneXisOn = true;
        eightXisOn = false;
        twentyXisOn = false;
        HalfMaxisOn = false;
    }

    // 8x Rate for Centering
    x_offset = x_offset + GUIDE_R_BOXSIZE_X+spacer;
    if (p.y > GUIDE_R_Y+y_offset && p.y < (GUIDE_R_Y+y_offset + GUIDE_R_BOXSIZE_Y) && p.x > GUIDE_R_X+x_offset && p.x < (GUIDE_R_X+x_offset + GUIDE_R_BOXSIZE_X)) {
        disp.setLocalCmd(":RC#");
        soundClick();
        oneXisOn = false;
        eightXisOn = true;
        twentyXisOn = false;
        HalfMaxisOn = false;
    }

    // 24x Rate for Moving
    x_offset = x_offset + GUIDE_R_BOXSIZE_X+spacer;
    if (p.y > GUIDE_R_Y+y_offset && p.y < (GUIDE_R_Y+y_offset + GUIDE_R_BOXSIZE_Y) && p.x > GUIDE_R_X+x_offset && p.x < (GUIDE_R_X+x_offset + GUIDE_R_BOXSIZE_X)) {
        disp.setLocalCmd(":RM#");
        soundClick();
        oneXisOn = false;
        eightXisOn = false;
        twentyXisOn = true;
        HalfMaxisOn = false;
    }

    // Half Max Rate for Slewing
    x_offset = x_offset + GUIDE_R_BOXSIZE_X+spacer;
    if (p.y > GUIDE_R_Y+y_offset && p.y < (GUIDE_R_Y+y_offset + GUIDE_R_BOXSIZE_Y) && p.x > GUIDE_R_X+x_offset && p.x < (GUIDE_R_X+x_offset + GUIDE_R_BOXSIZE_X)) {
        disp.setLocalCmd(":RS#");
        soundClick();
        oneXisOn = false;
        eightXisOn = false;
        twentyXisOn = false;
        HalfMaxisOn = true;
    }

    // Spiral Search
    if (p.y > SPIRAL_Y && p.y < (SPIRAL_Y + SPIRAL_BOXSIZE_Y) && p.x > SPIRAL_X && p.x < (SPIRAL_X + SPIRAL_BOXSIZE_X)) {
        if (!spiralOn) {
            disp.setLocalCmd(":Mp#");
            soundClick();
            spiralOn = true;
        } else {
            disp.setLocalCmd(":Q#");
            stopGuideSpiral();
            soundClick();
            spiralOn = false;
        }
    }
    
    // STOP moving
    if (p.y > STOP_Y && p.y < (STOP_Y + STOP_BOXSIZE_Y) && p.x > STOP_X && p.x < (STOP_X + STOP_BOXSIZE_X)) {
        disp.setLocalCmd(":Q#");
        soundClick();
        stopPressed = true;
        spiralOn = false;
        guidingEast = false;
        guidingWest = false;
        guidingNorth = false;
        guidingSouth = false;
    }
}