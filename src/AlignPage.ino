// ==============================================
// ============= Alignment Page =================
// ==============================================
// Author: Richard Benear2/18/22

#define BIG_BOX_W           80
#define BIG_BOX_H           40
#define BIG_BOX_T_OFF_W     20
#define BIG_BOX_T_OFF_H     4
#define A_STATUS_X          115
#define A_STATUS_Y          245
#define A_STATUS_Y_SP       19
#define UPDATE_W            200
#define UPDATE_H            18

#define STATE_LABEL_X       105
#define STATE_LABEL_Y       218
#define STATUS_LABEL_X      STATE_LABEL_X
#define STATUS_LABEL_Y      180
#define ERROR_LABEL_X       STATE_LABEL_X
#define ERROR_LABEL_Y       199
#define LABEL_SPACING_Y     2

// Go to Home position button
#define HOME_X              7
#define HOME_Y              170
#define HOME_BOXSIZE_W      BIG_BOX_W
#define HOME_BOXSIZE_H      BIG_BOX_H 
#define HOME_T_OFF_X        BIG_BOX_W/2-BIG_BOX_T_OFF_W
#define HOME_T_OFF_Y        BIG_BOX_H/2+BIG_BOX_T_OFF_H

// Align Num Stars Button(s)
#define NUM_S_X             1
#define NUM_S_Y             HOME_Y+HOME_BOXSIZE_H+5
#define NUM_S_BOXSIZE_W     30
#define NUM_S_BOXSIZE_H     35
#define NUM_S_T_OFF_X       2
#define NUM_S_T_OFF_Y       19
#define NUM_S_SPACING_X     NUM_S_BOXSIZE_W+3 

// Go to Catalog Page Button
#define ACAT_X              HOME_X
#define ACAT_Y              NUM_S_Y+NUM_S_BOXSIZE_H+5
#define CAT_BOXSIZE_W       BIG_BOX_W
#define CAT_BOXSIZE_H       BIG_BOX_H 
#define CAT_T_OFF_X         BIG_BOX_W/2-BIG_BOX_T_OFF_W-10
#define CAT_T_OFF_Y         BIG_BOX_H/2+BIG_BOX_T_OFF_H

// GO TO Target Button
#define GOTO_X              HOME_X
#define GOTO_Y              ACAT_Y+CAT_BOXSIZE_H+5
#define GOTO_BOXSIZE_W      BIG_BOX_W
#define GOTO_BOXSIZE_H      BIG_BOX_H 
#define GOTO_T_OFF_X        BIG_BOX_W/2-BIG_BOX_T_OFF_W
#define GOTO_T_OFF_Y        BIG_BOX_H/2+BIG_BOX_T_OFF_H

// ABORT Button
#define ABORT_X             110
#define ABORT_Y             WRITE_ALIGN_Y

// ALIGN this Star Button
#define ALIGN_X             HOME_X
#define ALIGN_Y             GOTO_Y+GOTO_BOXSIZE_H+5
#define ALIGN_BOXSIZE_W     BIG_BOX_W
#define ALIGN_BOXSIZE_H     BIG_BOX_H 
#define ALIGN_T_OFF_X       BIG_BOX_W/2-BIG_BOX_T_OFF_W
#define ALIGN_T_OFF_Y       BIG_BOX_H/2+BIG_BOX_T_OFF_H

// Write the Alignment Button
#define WRITE_ALIGN_X       HOME_X
#define WRITE_ALIGN_Y       ALIGN_Y+ALIGN_BOXSIZE_H+5
#define SA_BOXSIZE_W        BIG_BOX_W
#define SA_BOXSIZE_H        BIG_BOX_H 
#define SA_T_OFF_X          BIG_BOX_W/2-BIG_BOX_T_OFF_W
#define SA_T_OFF_Y          BIG_BOX_H/2+BIG_BOX_T_OFF_H

// Start/Clear the Alignment Button
#define START_ALIGN_X       225
#define START_ALIGN_Y       WRITE_ALIGN_Y
#define ST_BOXSIZE_W        BIG_BOX_W
#define ST_BOXSIZE_H        BIG_BOX_H 
#define ST_T_OFF_X          BIG_BOX_W/2-BIG_BOX_T_OFF_W
#define ST_T_OFF_Y          BIG_BOX_H/2+BIG_BOX_T_OFF_H

#define STATE_BUT_OUTLINE   TEXT_COLOR

//Different states of Align State machine
typedef enum {
    Idle_State,
    Home_State,
    Num_Stars_State,
    Select_Catalog_State,
    Wait_Catalog_State,
    Goto_State,
    Wait_For_Slewing_State,
    Align_State,
    Write_State,
} AlignStates;

AlignStates Current_State = Idle_State;
AlignStates Next_State = Idle_State;

int numAlignStars = 0;
char numStarsCmd[3] = "";
char acorr[10] = "";
char stateError[20] = "";
char alignStatus[5] = "";
char maxAlign[30];
char curAlign[30];
char lastAlign[30];

bool homeBut = false;
bool catalogBut = false;
bool gotoBut = false;
bool aborted = false;
bool abortBut = false;
bool alignBut = false;
bool saveAlignBut = false;
bool startAlignBut = false;
bool firstLabel = false;

uint16_t homeButtonOutline;
uint16_t numButtonOutline;
uint16_t catButtonOutline;
uint16_t gotoButtonOutline;
uint16_t alignButtonOutline;
uint16_t saveButtonOutline;

// Draw Alignment Page
void drawAlignPage() { 
    currentPage = ALIGN_PAGE;
    disp.updateColors();
    tft.setTextColor(TEXT_COLOR);
    tft.fillScreen(PAGE_BACKGROUND);
    
    disp.drawMenuButtons();
    disp.drawTitle(100, 30, "Alignment");
    disp.drawCommonStatusLabels();
    disp.updateOnstepCmdStatus();
    getAlignStatus();
    showCorrections();
    catSelected =0; // star catalog
}

void getAlignStatus() {
    char _reply[4];
    disp.getLocalCmdTrim(":GW#", _reply); 
    tft.setCursor(2, 165); 
    tft.fillRect(2, 153, 315, 16, BUT_BACKGROUND);
    if (_reply[0] == 'A') tft.printf("AltAzm |");
    if (_reply[0] == 'P') tft.printf(" Fork  |");
    if (_reply[0] == 'G') tft.printf(" GEM   |");
    tft.setCursor(70, 165);
    if (_reply[1] == 'T') tft.printf("  Tracking  |");
    if (_reply[1] == 'N') tft.printf("Not Tracking|");
    tft.setCursor(175, 165);
    if (_reply[2] == '0') tft.printf("  Needs aligned");
    if (_reply[2] == '1') tft.printf("1 star  aligned");
    if (_reply[2] == '2') tft.printf("2 stars aligned");
    if (_reply[2] == '3') tft.printf("3 stars aligned");
}

// ***** Show Calculated Corrections ******
// ax1Cor: align internal index for Axis1
// ax2Cor: align internal index for Axis2
// altCor: for geometric coordinate correction/align, - is below the pole, + above
// azmCor: - is right of the pole, + is left
// doCor: declination/optics orthogonal correction
// pdCor: declination/polar orthogonal correction
// dfCor: fork or declination axis flex
// tfCor: tube flex
void showCorrections() {
    int x_off = 0;
    int y_off = 0;
    int acorr_x = 115;
    int acorr_y = 310;
    char _reply[10];

    tft.drawRect(acorr_x, acorr_y, 200, 5*16, PAGE_BACKGROUND); // clear background
    tft.setCursor(acorr_x, acorr_y); tft.print("Calculated Corrections");

    y_off += 16;
    disp.getLocalCmdTrim(":GX00#", _reply); 
    sprintf(acorr,"ax1Cor=%s", _reply); // ax1Cor
    tft.setCursor(acorr_x+x_off, acorr_y+y_off); tft.print(acorr);

    y_off += 16;
    disp.getLocalCmdTrim(":GX01#", _reply); 
    sprintf(acorr,"ax2Cor=%s", _reply); // ax2Cor
    tft.setCursor(acorr_x+x_off, acorr_y+y_off); tft.print(acorr);

    y_off += 16;
    disp.getLocalCmdTrim(":GX02#", _reply); 
    sprintf(acorr,"altCor=%s", _reply); // altCor
    tft.setCursor(acorr_x+x_off, acorr_y+y_off); tft.print(acorr);

    y_off += 16;
    disp.getLocalCmdTrim(":GX03#", _reply); 
    sprintf(acorr,"azmCor=%s", _reply); // azmCor
    tft.setCursor(acorr_x+x_off, acorr_y+y_off); tft.print(acorr);

    x_off = 100;
    y_off = 16;
    disp.getLocalCmdTrim(":GX04#", _reply); 
    sprintf(acorr," doCor=%s", _reply);  // doCor
    tft.setCursor(acorr_x+x_off, acorr_y+y_off); tft.print(acorr);

    y_off += 16;
    disp.getLocalCmdTrim(":GX05#", _reply); 
    sprintf(acorr," pdCor=%s", _reply);  // pdCor
    tft.setCursor(acorr_x+x_off, acorr_y+y_off); tft.print(acorr);

    y_off += 16;
    disp.getLocalCmdTrim(":GX06#", _reply); 
    sprintf(acorr," ffCor=%s", _reply);  // ffCor
    tft.setCursor(acorr_x+x_off, acorr_y+y_off); tft.print(acorr);

    y_off += 16;
    disp.getLocalCmdTrim(":GX07#", _reply); 
    sprintf(acorr," dfCor=%s", _reply);  // dfCor
    tft.setCursor(acorr_x+x_off, acorr_y+y_off); tft.print(acorr);
}

/**************** Alignment Steps *********************
0) Press the [START] Button to begin Alignment
1) Home the Scope using [HOME] button
2) Select number of stars for Alignment, max=3
3) Press [CATALOG] button - Filter ALL_SKY_ALIGN automatically selected
4) On catalog page, select a suitable star
5) On catalog page, use [BACK] till return to this page
6) Press [GOTO] Button which slews to star
7) If star not centered, go to GUIDE page and center
8) Go back to ALIGN page if on GUIDE page
9) Press [ALIGN] button on the ALIGN page
10) If more stars are to be used, go to step 3, repeat
11) Press the [WRITE] button to save calculations to EEPROM
12) The [ABORT] button resets back to the Start 0) and shuts off motors
********************************************************/

// ***************************************************
// *********** Update Align Page Status **************
// ***************************************************
void updateAlignStatus() {
    // **************************************************
    // Update Buttons - only if the screen is touched
    //          or first time here, or single refresh flash
    // **************************************************
    if (screenTouched || firstDraw || refreshScreen || aborted) { 
        refreshScreen = false;
        if (screenTouched) refreshScreen = true;

        getAlignStatus();

        // Go to Home Position
        if (homeBut) {
            if (!atHome || isSlewing()) {
                disp.drawButton(HOME_X, HOME_Y, HOME_BOXSIZE_W, HOME_BOXSIZE_H, homeButtonOutline, BUT_ON_BGD, HOME_T_OFF_X, HOME_T_OFF_Y, "HOME");  
            }
        } else {
            disp.drawButton(HOME_X, HOME_Y, HOME_BOXSIZE_W, HOME_BOXSIZE_H, homeButtonOutline, BUT_BACKGROUND, HOME_T_OFF_X, HOME_T_OFF_Y, "HOME");           
        }
          
        // Number of Stars for Alignment Buttons
        // Alignment become active here
        int x_offset = 0;
        if (numAlignStars == 1) {  
            disp.drawButton(NUM_S_X, NUM_S_Y, NUM_S_BOXSIZE_W, NUM_S_BOXSIZE_H, numButtonOutline, BUT_ON_BGD, NUM_S_T_OFF_X, NUM_S_T_OFF_Y,   " 1 "); 
        } else {
            disp.drawButton(NUM_S_X, NUM_S_Y, NUM_S_BOXSIZE_W, NUM_S_BOXSIZE_H, numButtonOutline, BUT_BACKGROUND, NUM_S_T_OFF_X, NUM_S_T_OFF_Y, " 1 ");
        } 
        x_offset += NUM_S_SPACING_X;
        if (numAlignStars == 2) {  
            disp.drawButton(NUM_S_X+x_offset, NUM_S_Y, NUM_S_BOXSIZE_W, NUM_S_BOXSIZE_H, numButtonOutline, BUT_ON_BGD, NUM_S_T_OFF_X, NUM_S_T_OFF_Y,   " 2 "); 
        } else {
            disp.drawButton(NUM_S_X+x_offset, NUM_S_Y, NUM_S_BOXSIZE_W, NUM_S_BOXSIZE_H, numButtonOutline, BUT_BACKGROUND, NUM_S_T_OFF_X, NUM_S_T_OFF_Y, " 2 ");
        } 
        x_offset += NUM_S_SPACING_X;
        if (numAlignStars == 3) {  
            disp.drawButton(NUM_S_X+x_offset, NUM_S_Y, NUM_S_BOXSIZE_W, NUM_S_BOXSIZE_H, numButtonOutline, BUT_ON_BGD, NUM_S_T_OFF_X, NUM_S_T_OFF_Y,   " 3 "); 
        } else {
            disp.drawButton(NUM_S_X+x_offset, NUM_S_Y, NUM_S_BOXSIZE_W, NUM_S_BOXSIZE_H, numButtonOutline, BUT_BACKGROUND, NUM_S_T_OFF_X, NUM_S_T_OFF_Y, " 3 ");
        } 

        // go to the Star Catalog
        if (catalogBut) {
            disp.drawButton(ACAT_X, ACAT_Y, CAT_BOXSIZE_W, CAT_BOXSIZE_H, catButtonOutline, BUT_ON_BGD, CAT_T_OFF_X, CAT_T_OFF_Y, "CATALOG");   
        } else {
            disp.drawButton(ACAT_X, ACAT_Y, CAT_BOXSIZE_W, CAT_BOXSIZE_H, catButtonOutline, BUT_BACKGROUND, CAT_T_OFF_X, CAT_T_OFF_Y, "CATALOG");            
        }

        // Go To Coordinates Button
        if (gotoBut || isSlewing()) {
            disp.drawButton( GOTO_X, GOTO_Y,  GOTO_BOXSIZE_W, GOTO_BOXSIZE_H, gotoButtonOutline, BUT_ON_BGD, GOTO_T_OFF_X-2, GOTO_T_OFF_Y, "Going");
        } else {
            disp.drawButton( GOTO_X, GOTO_Y,  GOTO_BOXSIZE_W, GOTO_BOXSIZE_H, gotoButtonOutline, BUT_BACKGROUND, GOTO_T_OFF_X, GOTO_T_OFF_Y, "GOTO"); 
        }
        
        // Abort Alignment Button
        if (abortBut) {
            disp.drawButton(ABORT_X, ABORT_Y, GOTO_BOXSIZE_W, GOTO_BOXSIZE_H, BUT_OUTLINE, BUT_ON_BGD, GOTO_T_OFF_X-5, GOTO_T_OFF_Y, "Abort'g");
            disp.setLocalCmd(":Q#"); // stops move
            stopMotors(); // do this for safety reasons...mount may be colliding with something
            alignNumStars = 0;
            alignThisStar = 0;
            abortBut = false;
            startAlignBut = false;
            numAlignStars=0;
            aborted = true;
            gotoBut = false;
            catalogBut = false;
            homeBut = false;
            alignBut = false;
            Next_State = Idle_State;
        } else {
            disp.drawButton(ABORT_X, ABORT_Y, GOTO_BOXSIZE_W, GOTO_BOXSIZE_H, BUT_OUTLINE, BUT_BACKGROUND, GOTO_T_OFF_X-9, GOTO_T_OFF_Y, " ABORT "); 
            aborted = false;
        }

        // ALIGN button; calculate alignment parameters
        if (alignBut) {
            disp.drawButton(ALIGN_X, ALIGN_Y, ALIGN_BOXSIZE_W, ALIGN_BOXSIZE_H, alignButtonOutline, BUT_ON_BGD, ALIGN_T_OFF_X, ALIGN_T_OFF_Y, "ALIGN");    
        } else {
            disp.drawButton(ALIGN_X, ALIGN_Y, ALIGN_BOXSIZE_W, ALIGN_BOXSIZE_H, alignButtonOutline, BUT_BACKGROUND, ALIGN_T_OFF_X, ALIGN_T_OFF_Y, "ALIGN");            
        }

        // save the alignment calculations to EEPROM
        if (saveAlignBut) {
            disp.drawButton(WRITE_ALIGN_X, WRITE_ALIGN_Y, SA_BOXSIZE_W, SA_BOXSIZE_H, saveButtonOutline, BUT_ON_BGD, SA_T_OFF_X, SA_T_OFF_Y, "SAVEed");
        } else {
            disp.drawButton(WRITE_ALIGN_X, WRITE_ALIGN_Y, SA_BOXSIZE_W, SA_BOXSIZE_H, saveButtonOutline, BUT_BACKGROUND, SA_T_OFF_X, SA_T_OFF_Y, "SAVE");
        }

        // start alingnment
        if (startAlignBut) {
            disp.drawButton(START_ALIGN_X, START_ALIGN_Y, ST_BOXSIZE_W, ST_BOXSIZE_H, BUT_OUTLINE, BUT_ON_BGD, ST_T_OFF_X-10, ST_T_OFF_Y, "STARTed");
        } else {
            disp.drawButton(START_ALIGN_X, START_ALIGN_Y, ST_BOXSIZE_W, ST_BOXSIZE_H, BUT_OUTLINE, BUT_BACKGROUND, ST_T_OFF_X, ST_T_OFF_Y, "START");
        }
    }

    // Display Alignment Status
    if (alignActive()) {
        disp.canvPrint(A_STATUS_X, A_STATUS_Y, 0, 160, UPDATE_H, TEXT_COLOR, BUT_BACKGROUND, " -Align Active-   "); 
    } else if (Current_State == Write_State) { 
        disp.canvPrint(A_STATUS_X, A_STATUS_Y, 0, 160, UPDATE_H, TEXT_COLOR, BUT_BACKGROUND, "  -Align Done-    "); 
    } else {             
        disp.canvPrint(A_STATUS_X, A_STATUS_Y, 0, 160, UPDATE_H, TEXT_COLOR, BUT_BACKGROUND, "-Align Not Active-"); 
    }

    if (alignThisStar > alignNumStars) alignThisStar = alignNumStars;                
    sprintf(curAlign,  "  Current Star = %d", alignThisStar);
    sprintf(lastAlign, " Last Req Star = %d", alignNumStars);
    disp.canvPrint(A_STATUS_X, A_STATUS_Y+  A_STATUS_Y_SP, 0, 160, 18, TEXT_COLOR, BUT_BACKGROUND, curAlign);
    disp.canvPrint(A_STATUS_X, A_STATUS_Y+2*A_STATUS_Y_SP, 0, 160, 18, TEXT_COLOR, BUT_BACKGROUND, lastAlign);
    
    // =======================================================================
    // ==== Align State Machine .. updates at the update-timer-thread rate ===
    // =======================================================================
    switch(Current_State) {
        case 0: disp.canvPrint(STATE_LABEL_X, STATE_LABEL_Y, 0, UPDATE_W, UPDATE_H, TEXT_COLOR, PAGE_BACKGROUND,   "State = Wait_For_Start"); break;
        case 1: {disp.canvPrint(STATE_LABEL_X, STATE_LABEL_Y, 0, UPDATE_W, UPDATE_H, TEXT_COLOR, PAGE_BACKGROUND,  "State = Home"); 
                homeButtonOutline = STATE_BUT_OUTLINE;} break;
        case 2: {disp.canvPrint(STATE_LABEL_X, STATE_LABEL_Y, 0, UPDATE_W, UPDATE_H, TEXT_COLOR, PAGE_BACKGROUND,  "State = Num_Stars");
                numButtonOutline = STATE_BUT_OUTLINE;} break;
        case 3: {disp.canvPrint(STATE_LABEL_X, STATE_LABEL_Y, 0, UPDATE_W, UPDATE_H, TEXT_COLOR, PAGE_BACKGROUND,  "State = Select_Catalog");
                catButtonOutline = STATE_BUT_OUTLINE;} break;
        case 4: disp.canvPrint(STATE_LABEL_X, STATE_LABEL_Y, 0, UPDATE_W, UPDATE_H, TEXT_COLOR, PAGE_BACKGROUND,   "State = Wait_Catalog"); break;
        case 5: {disp.canvPrint(STATE_LABEL_X, STATE_LABEL_Y, 0, UPDATE_W, UPDATE_H, TEXT_COLOR, PAGE_BACKGROUND,  "State = GoTo");
                gotoButtonOutline = STATE_BUT_OUTLINE;} break;
        case 6: disp.canvPrint(STATE_LABEL_X, STATE_LABEL_Y, 0, UPDATE_W, UPDATE_H, TEXT_COLOR, PAGE_BACKGROUND,   "State = Wait_For_Slewing"); break;
        case 7: {disp.canvPrint(STATE_LABEL_X, STATE_LABEL_Y, 0, UPDATE_W, UPDATE_H, TEXT_COLOR, PAGE_BACKGROUND,  "State = Align");
                alignButtonOutline = STATE_BUT_OUTLINE;} break;
        case 8: {disp.canvPrint(STATE_LABEL_X, STATE_LABEL_Y, 0, UPDATE_W, UPDATE_H, TEXT_COLOR, PAGE_BACKGROUND,  "State = Write");
                saveButtonOutline = STATE_BUT_OUTLINE;} break;
        default: disp.canvPrint(STATE_LABEL_X, STATE_LABEL_Y, 0, UPDATE_W, UPDATE_H, TEXT_COLOR, PAGE_BACKGROUND,  "State = Wait_Start"); break;
    }
    firstLabel=true;    // creates a one-time text update in a state to reduce screen flicker
                        // this is necessary in states that looping in their state and are also printing something
    
    // align state machine
    Current_State = Next_State;
    switch(Current_State) {
        case Idle_State: {
            if(!dateWasSet || !timeWasSet) {  // check if date and time have been set
                if (firstLabel) {
                    disp.canvPrint(STATUS_LABEL_X, STATUS_LABEL_Y, LABEL_SPACING_Y, UPDATE_W, UPDATE_H, TEXT_COLOR, BUT_BACKGROUND, "                    ");
                    disp.canvPrint(ERROR_LABEL_X, ERROR_LABEL_Y, LABEL_SPACING_Y, UPDATE_W, UPDATE_H, BUT_ON_BGD, BUT_BACKGROUND, "Date or Time not set");
                    firstLabel=false;
                }
                Next_State = Idle_State;
            } else if (startAlignBut) {
                disp.canvPrint(STATUS_LABEL_X, STATUS_LABEL_Y, LABEL_SPACING_Y, UPDATE_W, UPDATE_H, TEXT_COLOR, BUT_BACKGROUND, "Home Scope to Start");
                disp.canvPrint(ERROR_LABEL_X, ERROR_LABEL_Y, LABEL_SPACING_Y, UPDATE_W, UPDATE_H, BUT_ON_BGD, BUT_BACKGROUND, "No Errors");
                Next_State = Home_State;
            } else {
                if (firstLabel) {
                    disp.canvPrint(STATUS_LABEL_X, STATUS_LABEL_Y, LABEL_SPACING_Y, UPDATE_W, UPDATE_H, TEXT_COLOR, BUT_BACKGROUND, "Press Start to Align");
                    firstLabel = false;  
                }
                Next_State = Idle_State;
            }
            break;
        } 

        case Home_State: {
            if (homeBut) {
                homeBut = false;
                if (trackingState == TrackingNone) { // start tracking if required
                    trackingState = TrackingSidereal;
                }
                axis1Enabled = true;
                CommandErrors e = validateGoto();
                if (e != CE_NONE) {
                    disp.canvPrint(ERROR_LABEL_X, ERROR_LABEL_Y, LABEL_SPACING_Y, UPDATE_W, UPDATE_H, BUT_ON_BGD, BUT_BACKGROUND, "Home Error");
                    Next_State = Idle_State;
                } else if (!atHome) {
                    disp.setLocalCmd(":hC#"); // go HOME
                    sprintf(stateError, "Error %s", commandErrorStr[commandError]);
                    disp.canvPrint(ERROR_LABEL_X, ERROR_LABEL_Y, LABEL_SPACING_Y, UPDATE_W, UPDATE_H, BUT_ON_BGD, BUT_BACKGROUND, stateError);

                    if (firstLabel) { // print only the 1st time, no flicker
                        disp.canvPrint(STATUS_LABEL_X, STATUS_LABEL_Y, LABEL_SPACING_Y, UPDATE_W, UPDATE_H, TEXT_COLOR, BUT_BACKGROUND, "Slewing");
                        firstLabel=false;  
                    }
                    Next_State = Home_State;
                }
            }
            if (atHome) {
                Next_State = Num_Stars_State;
            }
            break;
        } 

        case Num_Stars_State: {
            if (numAlignStars>0) {
                char s[6]; sprintf (s, ":A%d#", numAlignStars); // set number of align stars
                disp.setLocalCmd(s);
                Next_State = Select_Catalog_State; 
            } else {
                if (firstLabel) { 
                    disp.canvPrint(STATUS_LABEL_X, STATUS_LABEL_Y, LABEL_SPACING_Y, UPDATE_W, UPDATE_H, TEXT_COLOR, BUT_BACKGROUND, "Select Number of Stars");
                    firstLabel=false;
                } 
                Next_State = Num_Stars_State;
            }
            break;
        } 
    
        case Select_Catalog_State: {
            if (firstLabel) {
                disp.canvPrint(STATUS_LABEL_X, STATUS_LABEL_Y, LABEL_SPACING_Y, UPDATE_W, UPDATE_H, TEXT_COLOR, BUT_BACKGROUND, "Select Star in Catalog");
                firstLabel=false;
            }
            if (catalogBut) {
                catalogBut = false;
                Next_State = Wait_Catalog_State;
                activeFilter = FM_ALIGN_ALL_SKY;
                cat_mgr.filterAdd(activeFilter); 
                drawCatalogPage(STARS);
                return;
            } else {
                Next_State = Select_Catalog_State;
            }
            break;
        }
            
        case Wait_Catalog_State: {
            if (currentPage == ALIGN_PAGE) { // doesn't change state until Catalog points back to this page
                if (objectSelected) { // a star has been selected from the catalog
                    Next_State = Goto_State;
                } else {
                    Next_State = Select_Catalog_State;
                }
            } else {
                Next_State = Wait_Catalog_State;
            }
            break;
        }   

        case Goto_State: {
            if (firstLabel) {
                disp.canvPrint(STATUS_LABEL_X, STATUS_LABEL_Y, LABEL_SPACING_Y, UPDATE_W, UPDATE_H, TEXT_COLOR, BUT_BACKGROUND, "Press Go To");
                firstLabel=false;
            }
            if (gotoBut) {
                disp.setLocalCmd(":MS#");
                gotoBut = false;
                sprintf(stateError, "Error %s", commandErrorStr[commandError]);
                disp.canvPrint(ERROR_LABEL_X, ERROR_LABEL_Y, LABEL_SPACING_Y, UPDATE_W, UPDATE_H, BUT_ON_BGD, BUT_BACKGROUND, stateError);
                Next_State = Wait_For_Slewing_State;
            } else { // wait for GoTo button press
                Next_State = Goto_State;
            }
            break;
        }

        case Wait_For_Slewing_State: {
            if (isSlewing() || trackingSyncInProgress()) {
                if (firstLabel) {
                    disp.canvPrint(STATUS_LABEL_X, STATUS_LABEL_Y, LABEL_SPACING_Y, UPDATE_W, UPDATE_H, TEXT_COLOR, BUT_BACKGROUND, "Slewing");
                    firstLabel=false;
                }
                Next_State = Wait_For_Slewing_State;
            } else { // not slewing
                disp.canvPrint(STATUS_LABEL_X, STATUS_LABEL_Y, LABEL_SPACING_Y, UPDATE_W, UPDATE_H, TEXT_COLOR, BUT_BACKGROUND, "GoTo Completed");
                Next_State = Align_State;
            }
            break;
        }

        case Align_State: {
            if (!alignBut) {
                if (firstLabel) {
                    disp.canvPrint(STATUS_LABEL_X, STATUS_LABEL_Y, LABEL_SPACING_Y, UPDATE_W, UPDATE_H, TEXT_COLOR, BUT_BACKGROUND, "Press Align");
                    firstLabel=false;
                }
                Next_State = Align_State;
            } else {  // align this star
                disp.setLocalCmd(":A+#"); // add star to alignment
                alignBut = false;
                sprintf(stateError, "Error %s", commandErrorStr[commandError]);
                disp.canvPrint(ERROR_LABEL_X, ERROR_LABEL_Y, LABEL_SPACING_Y, UPDATE_W, UPDATE_H, BUT_ON_BGD, BUT_BACKGROUND, stateError);
            
                if (alignThisStar <= alignNumStars) { // more stars to align? (alignThisStar was incremented by cmd A+)
                    Next_State = Select_Catalog_State;
                } else {
                    Next_State = Write_State;
                }
            } 
            break;
        }

        case Write_State: {
            if (saveAlignBut) {
                disp.canvPrint(STATUS_LABEL_X, STATUS_LABEL_Y, LABEL_SPACING_Y, UPDATE_W, UPDATE_H, TEXT_COLOR, BUT_BACKGROUND, "Writing Align Data");
                disp.setLocalCmd(":AW#");
                sprintf(stateError, "Error %s", commandErrorStr[commandError]);
                disp.canvPrint(ERROR_LABEL_X, ERROR_LABEL_Y, LABEL_SPACING_Y, UPDATE_W, UPDATE_H, BUT_ON_BGD, BUT_BACKGROUND, stateError);
        
                saveAlignBut = false;
                Next_State = Idle_State;
            } else {
                if (firstLabel) {
                    disp.canvPrint(STATUS_LABEL_X, STATUS_LABEL_Y, LABEL_SPACING_Y, UPDATE_W, UPDATE_H, TEXT_COLOR, BUT_BACKGROUND, "Waiting for Write");
                    disp.canvPrint(ERROR_LABEL_X, ERROR_LABEL_Y, LABEL_SPACING_Y, UPDATE_W, UPDATE_H, BUT_ON_BGD, BUT_BACKGROUND, "No Errors");
                    showCorrections(); // show the corrections to determine if we want to Save or Abort
                    firstLabel=false;
                }
                Next_State = Write_State;
            }
            startAlignBut = false;
            getAlignStatus();
            break;
        }
        default: Next_State = Idle_State; 
    }
    if (Current_State != Next_State) refreshScreen=true; else refreshScreen=false;
    if (currentPage != CATALOG_PAGE) disp.updateCommonStatus(); // prevents writing over the Catalog Page due to race condition in timing
    screenTouched = false;
}

// ========================================
// === Manage Touching of Align Buttons ===
// ========================================
void touchAlignUpdate() {
    // Go to Home Telescope Requested
    if (p.x > HOME_X && p.x < HOME_X + HOME_BOXSIZE_W && p.y > HOME_Y  && p.y < HOME_Y + HOME_BOXSIZE_H) {
        if (Current_State==Home_State) {
            soundClick();
            homeBut = true;
        }
    }
    
    // Number of Stars for alignment
    int x_offset = 0;
    if (Current_State==Num_Stars_State) {
        if (p.y > NUM_S_Y && p.y < (NUM_S_Y + NUM_S_BOXSIZE_H) && p.x > NUM_S_X+x_offset && p.x < (NUM_S_X+x_offset + NUM_S_BOXSIZE_W)) {
            soundClick();
            numAlignStars = 1;
        }
        x_offset += NUM_S_SPACING_X;
        if (p.y > NUM_S_Y && p.y < (NUM_S_Y + NUM_S_BOXSIZE_H) && p.x > NUM_S_X+x_offset && p.x < (NUM_S_X+x_offset + NUM_S_BOXSIZE_W)) {
            soundClick();
            numAlignStars = 2;
        }
        x_offset += NUM_S_SPACING_X;
        if (p.y > NUM_S_Y && p.y < (NUM_S_Y + NUM_S_BOXSIZE_H) && p.x > NUM_S_X+x_offset && p.x < (NUM_S_X+x_offset + NUM_S_BOXSIZE_W)) {
            soundClick();
            numAlignStars = 3;
        }
    }

    // Call up the Catalog Button
    if (p.y > ACAT_Y && p.y < (ACAT_Y + CAT_BOXSIZE_H) && p.x > ACAT_X && p.x < (ACAT_X + CAT_BOXSIZE_W)) {
        if (Current_State==Select_Catalog_State ) {
            soundClick();
            catalogBut = true;
        }
    }

    // Go To Target Coordinates
    if (p.y > GOTO_Y && p.y < (GOTO_Y + GOTO_BOXSIZE_H) && p.x > GOTO_X && p.x < (GOTO_X + GOTO_BOXSIZE_W)) {
        if (Current_State==Goto_State) { 
            soundClick();
            gotoBut = true;
        }
    }

    // ==== ABORT GOTO ====
    if (p.y > ABORT_Y && p.y < (ABORT_Y + GOTO_BOXSIZE_H) && p.x > ABORT_X && p.x < (ABORT_X + GOTO_BOXSIZE_W)) {
        soundClick();
        abortBut = true;
    }

    // ALIGN / calculate alignment corrections Button
    if (p.y > ALIGN_Y && p.y < (ALIGN_Y + ALIGN_BOXSIZE_H) && p.x > ALIGN_X && p.x < (ALIGN_X + ALIGN_BOXSIZE_W)) { 
        if (Current_State==Align_State) {
            soundClick();
            alignBut = true;
        }
    }

    // Write Alignment Button
    if (p.y > WRITE_ALIGN_Y && p.y < (WRITE_ALIGN_Y + SA_BOXSIZE_H) && p.x > WRITE_ALIGN_X && p.x < (WRITE_ALIGN_X + SA_BOXSIZE_W)) { 
        if (Current_State==Write_State) {
            soundClick();
            saveAlignBut = true;
        }
    }  

    // START Alignment Button - clear the corrections, reset the state machine
    if (p.y > START_ALIGN_Y && p.y < (START_ALIGN_Y + ST_BOXSIZE_H) && p.x > START_ALIGN_X && p.x < (START_ALIGN_X + ST_BOXSIZE_W)) { 
        soundClick();
        startAlignBut = true;
        disp.setLocalCmd(":SX02#");
        disp.setLocalCmd(":SX03#");
        disp.setLocalCmd(":SX04#");
        disp.setLocalCmd(":SX05#");
        disp.setLocalCmd(":SX06#");
        disp.setLocalCmd(":SX07#");
        disp.setLocalCmd(":SX08#");
        alignNumStars = 0; // number of align stars from Align routines
        alignThisStar = 0;
        numAlignStars = 0; // number of selected align stars from buttons
        Current_State = Idle_State;
        Next_State = Idle_State;

        // Enable the Motors
        odriveAZOff = false; // false = NOT off
        digitalWrite(AZ_ENABLED_LED_PIN, LOW); // Turn On AZ LED
        turnOnOdriveMotor(AZ);
        odriveALTOff = false; // false = NOT off
        digitalWrite(ALT_ENABLED_LED_PIN, LOW); // Turn On ALT LED
        turnOnOdriveMotor(ALT);
    }  
}
