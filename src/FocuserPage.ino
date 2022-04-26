/// =============================================
// ============ Display Focuser Page ============
// ==============================================
// Author: Richard Benear
// 12/20/21
// Initially, this Focuser Page tried to use the FocuserDC.h
// and StepperDC.h implementations done by Howard Dutton to 
// drive a DC Motor Focuser by using the local command channel (cmdX).
// The hardware used was an A4988 stepper driver. This mostly worked
// but there were problems with inconsistent moves where the DC motor would
// not respond after multiple successive moves. I suspect there are problems 
// with the length of the Enable high/low pulse width and inductive kick from 
// the coils with a long cable and coupling this noise into the Step signal
// which moves the phase to the wrong phase. Therefore, I resorted to building 
// my own A4988 driver for the DC Motor control where I could tweak the 
// parameters more directly. These functions are found near the end 
// of this file.

// For IN and OUT Buttons
#define FOC_INOUT_X             206 
#define FOC_INOUT_Y             173 
#define FOC_INOUT_BOXSIZE_X     109 
#define FOC_INOUT_BOXSIZE_Y      75 
#define FOC_INOUT_X_SPACING       0 
#define FOC_INOUT_Y_SPACING      85 
#define FOC_INOUT_TEXT_X_OFFSET  15 
#define FOC_INOUT_TEXT_Y_OFFSET  45

// For Focuser Status
#define FOC_LABEL_X               3 
#define FOC_LABEL_Y             176 
#define FOC_LABEL_Y_SPACING      16 
#define FOC_LABEL_OFFSET_X      115
#define FOC_LABEL_OFFSET_Y        6 

// For Focuser Speed Selection
#define SPEED_X                    3 
#define SPEED_Y                  270 
#define SPEED_BOXSIZE_X          104 
#define SPEED_BOXSIZE_Y           30 
#define SPEED_TEXT_X_OFFSET        8
#define SPEED_TEXT_Y_OFFSET       17 

// Stop Focuser position
#define CALIB_FOC_X              215 
#define CALIB_FOC_Y              390 
#define CALIB_FOC_BOXSIZE_X       90 
#define CALIB_FOC_BOXSIZE_Y       45 
#define CALIB_FOC_TEXT_X_OFFSET    8 
#define CALIB_FOC_TEXT_Y_OFFSET   26 

// For Focuser Middle Buttons Selection
#define MID_X                    116 
#define MID_Y                   SPEED_Y 
#define MID_BOXSIZE_X             80 
#define MID_BOXSIZE_Y             30 
#define MID_TEXT_X_OFFSET          6 
#define MID_TEXT_Y_OFFSET         17 

#define MTR_PWR_INC_SIZE           5
#define FOC_MOVE_DISTANCE 5 // default to 5 pulses
#define FOC_SPEED_INC 100 // default to inc/dec of 100 microsec
#define EN_OFF_TIME 2000 // microseconds

bool focMovingIn = true;
bool gotoSetpoint = false;
bool focGoToHalf = false;
bool setPoint = false;
bool decSpeed = false;
bool incSpeed = false;
bool stepPhasePressed = false;
bool incMoveCt = false;
bool decMoveCt = false;
bool setZero = false;
bool setMax = false;
bool revFocuser = false;
bool inwardCalState = true; // start with inward calibration
bool focReset = false;
bool calibActive = false;
int focMoveSpeed = 500; // pulse width in microsec
int focMoveDistance = 30; // probably need to start with 30 after powering up
int _moveDistance = 0;
int current_focMoveDistance = 0;
int current_focMoveSpeed = 0;
int current_focMaxPos = 0;
int current_focMinPos = 0;
int current_focPos = 0;
int current_focDeltaMove = 0;
int focPosition = 0;
int focTarget = 0;
int focDeltaMove = 0;
int focMaxPosition = 0;
int focMinPosition = 0;
int setPointTarget = 0;


// Draw the initial content for Focuser Page
void drawFocuserPage() {
    disp.updateColors();
    tft.setTextColor(TEXT_COLOR);
    tft.fillScreen(PAGE_BACKGROUND);
    currentPage = FOCUSER_PAGE;
    disp.drawMenuButtons();
    disp.drawTitle(110, 30, "Focuser");
    disp.drawCommonStatusLabels();
    disp.updateOnstepCmdStatus();
    
    int y_offset = 0;

    // Maximum out position
    tft.setCursor(FOC_LABEL_X, FOC_LABEL_Y + y_offset);
    tft.print("          Max:");
    
    // Minimum in position
    y_offset +=FOC_LABEL_Y_SPACING;
    tft.setCursor(FOC_LABEL_X, FOC_LABEL_Y + y_offset);
    tft.print("          Min:");
    
    // Move Speed - actually is the width of motor enable pulse low
    y_offset +=FOC_LABEL_Y_SPACING;
    tft.setCursor(FOC_LABEL_X, FOC_LABEL_Y + y_offset);
    tft.print("   Move Speed:");

    // Move Distance - actually is the number of enable pulses in a move
    y_offset +=FOC_LABEL_Y_SPACING;
    tft.setCursor(FOC_LABEL_X, FOC_LABEL_Y + y_offset);
    tft.print("Move Distance:");

    // Current position
    y_offset +=FOC_LABEL_Y_SPACING;
    tft.setCursor(FOC_LABEL_X, FOC_LABEL_Y + y_offset);
    tft.print("Curr Position:");

    // Delta from target position
    y_offset +=FOC_LABEL_Y_SPACING;
    tft.setCursor(FOC_LABEL_X, FOC_LABEL_Y + y_offset);
    tft.print(" Target Delta:");
}

// Update the following on timer tick
void updateFocuserStatus()
{
    disp.updateCommonStatus();
    
    int y_offset = 0;
    if ((current_focMaxPos != focMaxPosition) || firstDraw) {
        disp.canvPrint(FOC_LABEL_X+FOC_LABEL_OFFSET_X, FOC_LABEL_Y, y_offset, C_WIDTH, C_HEIGHT, BUT_OUTLINE, BUT_BACKGROUND, focMaxPosition);
        current_focMaxPos = focMaxPosition;
    }

    y_offset +=FOC_LABEL_Y_SPACING;
    if ((current_focMinPos != focMinPosition) || firstDraw) {
        disp.canvPrint(FOC_LABEL_X+FOC_LABEL_OFFSET_X, FOC_LABEL_Y, y_offset, C_WIDTH, C_HEIGHT, BUT_OUTLINE, BUT_BACKGROUND, focMinPosition);
        current_focMinPos = focMinPosition;
    }

    // Focuser Speed
    y_offset +=FOC_LABEL_Y_SPACING;
    if ((current_focMoveSpeed != focMoveSpeed) || firstDraw) {
        disp.canvPrint(FOC_LABEL_X+FOC_LABEL_OFFSET_X, FOC_LABEL_Y, y_offset, C_WIDTH, C_HEIGHT, BUT_OUTLINE, BUT_BACKGROUND, focMoveSpeed);
        current_focMoveSpeed = focMoveSpeed;
    }

    // Focuser move distance
    y_offset +=FOC_LABEL_Y_SPACING;
    if ((current_focMoveDistance != focMoveDistance) || firstDraw) {
        disp.canvPrint(FOC_LABEL_X+FOC_LABEL_OFFSET_X, FOC_LABEL_Y, y_offset, C_WIDTH, C_HEIGHT, BUT_OUTLINE, BUT_BACKGROUND, focMoveDistance);
        current_focMoveDistance = focMoveDistance;
    }
   
    // Update Current Focuser Position
    y_offset +=FOC_LABEL_Y_SPACING;
    if ((current_focPos != focPosition) || firstDraw) {
        disp.canvPrint(FOC_LABEL_X+FOC_LABEL_OFFSET_X, FOC_LABEL_Y, y_offset, C_WIDTH, C_HEIGHT, BUT_OUTLINE, BUT_BACKGROUND, focPosition);
        current_focPos = focPosition;
    }

    // Update Delta Focuser Position
    y_offset +=FOC_LABEL_Y_SPACING;
    if ((current_focDeltaMove != focDeltaMove) || firstDraw) {
        disp.canvPrint(FOC_LABEL_X+FOC_LABEL_OFFSET_X, FOC_LABEL_Y, y_offset, C_WIDTH, C_HEIGHT, BUT_OUTLINE, BUT_BACKGROUND, focDeltaMove);
        current_focDeltaMove = focDeltaMove;
    }

    //***** Update Label Status' for Buttons ******
    if (screenTouched || firstDraw || refreshScreen) {
        refreshScreen = false;
        if (screenTouched) refreshScreen = true;
         
        // Update IN and OUT focuser status
        tft.setFont(&UbuntuMono_Bold16pt7b);
        if (focMovingIn) {
            disp.drawButton(FOC_INOUT_X, FOC_INOUT_Y, FOC_INOUT_BOXSIZE_X, FOC_INOUT_BOXSIZE_Y, BUT_OUTLINE, BUT_ON_BGD, FOC_INOUT_TEXT_X_OFFSET+5, FOC_INOUT_TEXT_Y_OFFSET, " IN ");
        } else {
            disp.drawButton(FOC_INOUT_X, FOC_INOUT_Y, FOC_INOUT_BOXSIZE_X, FOC_INOUT_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, FOC_INOUT_TEXT_X_OFFSET+5, FOC_INOUT_TEXT_Y_OFFSET, " IN ");
        }

        if (!focMovingIn) {
            disp.drawButton(FOC_INOUT_X, FOC_INOUT_Y + FOC_INOUT_Y_SPACING, FOC_INOUT_BOXSIZE_X, FOC_INOUT_BOXSIZE_Y, BUT_OUTLINE, BUT_ON_BGD, FOC_INOUT_TEXT_X_OFFSET, FOC_INOUT_TEXT_Y_OFFSET, " OUT ");
        } else {
            disp.drawButton(FOC_INOUT_X, FOC_INOUT_Y + FOC_INOUT_Y_SPACING, FOC_INOUT_BOXSIZE_X, FOC_INOUT_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, FOC_INOUT_TEXT_X_OFFSET, FOC_INOUT_TEXT_Y_OFFSET, " OUT ");
        }
        tft.setFont(&Inconsolata_Bold8pt7b);
        
        // *** Left Column Buttons ***
        // update speed selection
        y_offset = 0;
        // Increment Speed
        if (incSpeed) {
            disp.drawButton(SPEED_X, SPEED_Y + y_offset, SPEED_BOXSIZE_X, SPEED_BOXSIZE_Y, BUT_OUTLINE, BUT_ON_BGD, SPEED_TEXT_X_OFFSET, SPEED_TEXT_Y_OFFSET,   "  Inc'ing ");
            incSpeed = false;
        } else {
            disp.drawButton(SPEED_X, SPEED_Y + y_offset, SPEED_BOXSIZE_X, SPEED_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, SPEED_TEXT_X_OFFSET, SPEED_TEXT_Y_OFFSET, " Inc Speed");
        }

        // Decrement Speed
        y_offset +=SPEED_BOXSIZE_Y + 2;
        if (decSpeed) {
            disp.drawButton(SPEED_X, SPEED_Y + y_offset, SPEED_BOXSIZE_X, SPEED_BOXSIZE_Y, BUT_OUTLINE, BUT_ON_BGD, SPEED_TEXT_X_OFFSET, SPEED_TEXT_Y_OFFSET,   "  Dec'ing ");
            decSpeed = false;
        } else {
            disp.drawButton(SPEED_X, SPEED_Y + y_offset, SPEED_BOXSIZE_X, SPEED_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, SPEED_TEXT_X_OFFSET, SPEED_TEXT_Y_OFFSET, " Dec Speed");
        }

        // Set a GoTo setpoint
        y_offset +=SPEED_BOXSIZE_Y + 2; 
        if (setPoint) {
            disp.drawButton(SPEED_X, SPEED_Y + y_offset, SPEED_BOXSIZE_X, SPEED_BOXSIZE_Y, BUT_OUTLINE, BUT_ON_BGD, SPEED_TEXT_X_OFFSET, SPEED_TEXT_Y_OFFSET, "  Setting ");
            setPoint = false;
        } else {
            disp.drawButton(SPEED_X, SPEED_Y + y_offset, SPEED_BOXSIZE_X, SPEED_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, SPEED_TEXT_X_OFFSET, SPEED_TEXT_Y_OFFSET, "Set Goto Pt");
        }

        // Goto the setpoint
        y_offset +=SPEED_BOXSIZE_Y + 2;
        if (gotoSetpoint) {
            disp.drawButton(SPEED_X, SPEED_Y + y_offset, SPEED_BOXSIZE_X, SPEED_BOXSIZE_Y, BUT_OUTLINE, BUT_ON_BGD, SPEED_TEXT_X_OFFSET, SPEED_TEXT_Y_OFFSET, "Going to SP");
            gotoSetpoint = false;
        } else {
            disp.drawButton(SPEED_X, SPEED_Y + y_offset, SPEED_BOXSIZE_X, SPEED_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, SPEED_TEXT_X_OFFSET, SPEED_TEXT_Y_OFFSET, "Goto Set Pt");
        }

        // Goto Halfway point
        y_offset +=SPEED_BOXSIZE_Y + 2;
        if (focGoToHalf) {
            disp.drawButton(SPEED_X, SPEED_Y + y_offset, SPEED_BOXSIZE_X, SPEED_BOXSIZE_Y, BUT_OUTLINE, BUT_ON_BGD, SPEED_TEXT_X_OFFSET, SPEED_TEXT_Y_OFFSET, "GoingTo Half");
            focGoToHalf = false;
        } else {
            disp.drawButton(SPEED_X, SPEED_Y + y_offset, SPEED_BOXSIZE_X, SPEED_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, SPEED_TEXT_X_OFFSET, SPEED_TEXT_Y_OFFSET, " GoTo Half  ");
        }

        // Calibrate Min And Max
        y_offset = 0;
        if (!calibActive) {
            disp.drawButton(CALIB_FOC_X, CALIB_FOC_Y, CALIB_FOC_BOXSIZE_X, CALIB_FOC_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, CALIB_FOC_TEXT_X_OFFSET, CALIB_FOC_TEXT_Y_OFFSET, "Calibrate");
        } else {
            if (inwardCalState && calibActive) {
                disp.drawButton(CALIB_FOC_X, CALIB_FOC_Y, CALIB_FOC_BOXSIZE_X, CALIB_FOC_BOXSIZE_Y, BUT_OUTLINE, BUT_ON_BGD, CALIB_FOC_TEXT_X_OFFSET, CALIB_FOC_TEXT_Y_OFFSET, " Min Calib");
            } else if (!inwardCalState && calibActive) {
                disp.drawButton(CALIB_FOC_X, CALIB_FOC_Y, CALIB_FOC_BOXSIZE_X, CALIB_FOC_BOXSIZE_Y, BUT_OUTLINE, BUT_ON_BGD, CALIB_FOC_TEXT_X_OFFSET, CALIB_FOC_TEXT_Y_OFFSET, " Max Calib");
            }
        }

        // *** Center column Buttons ***
        y_offset = 0;
        // Increment Motor Power
        if (incMoveCt) {
            disp.drawButton(MID_X, MID_Y + y_offset, MID_BOXSIZE_X, MID_BOXSIZE_Y, BUT_OUTLINE, BUT_ON_BGD, MID_TEXT_X_OFFSET, MID_TEXT_Y_OFFSET, " Inc'ing ");
            incMoveCt = false;
        } else {
            disp.drawButton(MID_X, MID_Y + y_offset, MID_BOXSIZE_X, MID_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, MID_TEXT_X_OFFSET, MID_TEXT_Y_OFFSET, "Inc Cnt");
        }

        y_offset +=SPEED_BOXSIZE_Y + 2;
        // Decrement Motor Power
        if (decMoveCt) {
            disp.drawButton(MID_X, MID_Y + y_offset, MID_BOXSIZE_X, MID_BOXSIZE_Y, BUT_OUTLINE, BUT_ON_BGD, MID_TEXT_X_OFFSET, MID_TEXT_Y_OFFSET, "Dec'ing ");
            decMoveCt = false;
        } else {
            disp.drawButton(MID_X, MID_Y + y_offset, MID_BOXSIZE_X, MID_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, MID_TEXT_X_OFFSET, MID_TEXT_Y_OFFSET, "Dec Cnt");
        }

        y_offset +=SPEED_BOXSIZE_Y + 2;
        // Set Zero Position
        if (setZero) {
            disp.drawButton(MID_X, MID_Y + y_offset, MID_BOXSIZE_X, MID_BOXSIZE_Y, BUT_OUTLINE, BUT_ON_BGD, MID_TEXT_X_OFFSET, MID_TEXT_Y_OFFSET, "Setting");
            setZero = false;
        } else {
            disp.drawButton(MID_X, MID_Y + y_offset, MID_BOXSIZE_X, MID_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, MID_TEXT_X_OFFSET, MID_TEXT_Y_OFFSET, "Set Zero");
        }

        y_offset +=SPEED_BOXSIZE_Y + 2;
        // Set Maximum position
        if (setMax) {
            disp.drawButton(MID_X, MID_Y + y_offset, MID_BOXSIZE_X, MID_BOXSIZE_Y, BUT_OUTLINE, BUT_ON_BGD, MID_TEXT_X_OFFSET, MID_TEXT_Y_OFFSET, "Setting");
            setMax = false;
        } else {
            disp.drawButton(MID_X, MID_Y + y_offset, MID_BOXSIZE_X, MID_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, MID_TEXT_X_OFFSET, MID_TEXT_Y_OFFSET, "Set Max ");
        }

        y_offset +=SPEED_BOXSIZE_Y + 2;
        // Reset focuser
        if (focReset) {
            disp.drawButton(MID_X, MID_Y + y_offset, MID_BOXSIZE_X, MID_BOXSIZE_Y, BUT_OUTLINE, BUT_ON_BGD, MID_TEXT_X_OFFSET, MID_TEXT_Y_OFFSET,   "Reseting");
            focReset = false;
        } else {
            disp.drawButton(MID_X, MID_Y + y_offset, MID_BOXSIZE_X, MID_BOXSIZE_Y, BUT_OUTLINE, BUT_BACKGROUND, MID_TEXT_X_OFFSET+5, MID_TEXT_Y_OFFSET, " RESET  ");
        }
    screenTouched = false;
    }
}

// Update buttons when touched
void touchFocuserUpdate()
{   
    // IN button
    if (p.y > FOC_INOUT_Y && p.y < (FOC_INOUT_Y + FOC_INOUT_BOXSIZE_Y) && p.x > FOC_INOUT_X && p.x < (FOC_INOUT_X + FOC_INOUT_BOXSIZE_X))
    {
        soundFreq(2000);
        if (!focMovingIn) { //was moving out, change direction
            focChangeDirection();
        }   
        focMove(focMoveDistance, focMoveSpeed);
        focGoToActive = false;
    }

    int y_offset = FOC_INOUT_Y_SPACING;
    // OUT button
    if (p.y > FOC_INOUT_Y + y_offset && p.y < (FOC_INOUT_Y + y_offset + FOC_INOUT_BOXSIZE_Y) && p.x > FOC_INOUT_X && p.x < (FOC_INOUT_X + FOC_INOUT_BOXSIZE_X))
    {
        soundFreq(2100);
        if (focMovingIn) { //was moving in, change direction
            focChangeDirection();
        }
        focMove(focMoveDistance, focMoveSpeed);
        focGoToActive = false;
    }

    // ========= LEFT Column Buttons ========
    // Select Focuser Speed 
    y_offset = 0;
    // Increment Speed
    if (p.y > SPEED_Y + y_offset && p.y < (SPEED_Y + y_offset + SPEED_BOXSIZE_Y) && p.x > SPEED_X && p.x < (SPEED_X + SPEED_BOXSIZE_X))
    {
        focMoveSpeed += FOC_SPEED_INC; // microseconds
        if (focMoveSpeed > 900) focMoveSpeed = 900;
        soundClick();
        incSpeed = true;
    }

    // Decrement Speed
    y_offset +=SPEED_BOXSIZE_Y + 2;
    if (p.y > SPEED_Y + y_offset && p.y < (SPEED_Y + y_offset + SPEED_BOXSIZE_Y) && p.x > SPEED_X && p.x < (SPEED_X + SPEED_BOXSIZE_X))
    {
        focMoveSpeed -= FOC_SPEED_INC; // microseconds
        if (focMoveSpeed < 100) focMoveSpeed = 100;
        soundClick();
        decSpeed = false;
    }

    // ======== Set GoTo points ========
    // Set GoTo point
    y_offset +=SPEED_BOXSIZE_Y + 2;
    if (p.y > SPEED_Y + y_offset && p.y < (SPEED_Y + y_offset + SPEED_BOXSIZE_Y) && p.x > SPEED_X && p.x < (SPEED_X + SPEED_BOXSIZE_X))
    {
        setPointTarget = focPosition;
        setPoint = true;
        soundClick();
    }

    // GoTo Setpoint
    y_offset +=SPEED_BOXSIZE_Y + 2;
    if (p.y > SPEED_Y + y_offset && p.y < (SPEED_Y + y_offset + SPEED_BOXSIZE_Y) && p.x > SPEED_X && p.x < (SPEED_X + SPEED_BOXSIZE_X))
    {
        focTarget = setPointTarget;
        gotoSetpoint = true; 
        soundClick();
        focGoToActive = true;
    }

    // GoTo Halfway 
    y_offset +=SPEED_BOXSIZE_Y + 2;
    if (p.y > SPEED_Y + y_offset && p.y < (SPEED_Y + y_offset + SPEED_BOXSIZE_Y) && p.x > SPEED_X && p.x < (SPEED_X + SPEED_BOXSIZE_X))
    {
        focTarget = (focMaxPosition - focMinPosition) / 2;
        focGoToHalf = true;
        soundClick();
        focGoToActive = true;
    }
    
    // ======== Bottom Right Corner Button ========
    // ** Calibration Button Procedure **
    // 1st button press moves focuser inward
    // 2nd button press stops inward move and sets as Minimum position
    // 3rd button press moves focuser outward
    // 4th button press stops outward move and sets as Maximum position
    if (p.y > CALIB_FOC_Y && p.y < (CALIB_FOC_Y + CALIB_FOC_BOXSIZE_Y) && p.x > CALIB_FOC_X && p.x < (CALIB_FOC_X + CALIB_FOC_BOXSIZE_X))
    {  
        soundClick();
        if (inwardCalState) {
            if (!focGoToActive) { // then we are starting calibration
                if (!focMovingIn) focChangeDirection(); // go inward
                focMovingIn = true;
                focTarget -= 12000;
                focGoToActive = true;
                calibActive = true;
            } else { // then we have been told to stop move in and are at Minimum
                focGoToActive = false;
                focMinPosition = 0;
                focPosition = 0;
                inwardCalState = false;
            }
        } else { // now go out and calibrate max position
            if (!focGoToActive) { // then we are starting OUT MAX calibration
                if (focMovingIn) focChangeDirection();
                focMovingIn = false;
                focTarget += 12000;
                focGoToActive = true;
            } else { // then we have been told to stop move OUT and are at Maximum
                focGoToActive = false;
                focMaxPosition = focPosition;
                inwardCalState = true; // reset this in case want to calibrate again
                calibActive = false;
            }
        } 
       
    }

    // ****** Center Column Buttons ******
    y_offset = 0;
    // move distance increment
    if (p.y > MID_Y + y_offset && p.y < (MID_Y + y_offset + MID_BOXSIZE_Y) && p.x > MID_X && p.x < (MID_X + MID_BOXSIZE_X))
    {
        focMoveDistance += MTR_PWR_INC_SIZE;
        if (focMoveDistance >= 100) focMoveDistance = 100;
        soundClick();
        incMoveCt = true;
        decMoveCt = false;
    }

    // move distance decrement
    y_offset +=SPEED_BOXSIZE_Y + 2;
    if (p.y > MID_Y + y_offset && p.y < (MID_Y + y_offset + MID_BOXSIZE_Y) && p.x > MID_X && p.x < (MID_X + MID_BOXSIZE_X))
    {
        focMoveDistance -= MTR_PWR_INC_SIZE;
        if (focMoveDistance <= 0) focMoveDistance = 5;
        soundClick();
        incMoveCt = false;
        decMoveCt = true;
    }

    // Set Zero point of Focuser
    y_offset +=SPEED_BOXSIZE_Y + 2;
    if (p.y > MID_Y + y_offset && p.y < (MID_Y + y_offset + MID_BOXSIZE_Y) && p.x > MID_X && p.x < (MID_X + MID_BOXSIZE_X))
    {
        focMinPosition = 0;
        focPosition = 0;
        soundClick();
        setZero = true;
    }

    // Set Max Position of focuser
    y_offset +=SPEED_BOXSIZE_Y + 2;
    if (p.y > MID_Y + y_offset && p.y < (MID_Y + y_offset + MID_BOXSIZE_Y) && p.x > MID_X && p.x < (MID_X + MID_BOXSIZE_X))
    {
        focMaxPosition = focPosition;
        soundClick();
        setMax = true;
    }

    // Reset focuser
    y_offset +=SPEED_BOXSIZE_Y + 2;
    if (p.y > MID_Y + y_offset && p.y < (MID_Y + y_offset + MID_BOXSIZE_Y) && p.x > MID_X && p.x < (MID_X + MID_BOXSIZE_X))
    {
        soundClick();
        digitalWrite(Axis4_SLEEP,LOW); 
        delay(2);
        digitalWrite(Axis4_SLEEP,HIGH); 
        focReset = true;
    }
}

// ======= A4988 Driver Functions =======
// Initialize DC Focuser
void focInit() {
    // set phase power from 70% Home position backward to 100% power position
    digitalWrite(Axis4_DIR,LOW);
    delayMicroseconds(2);
    digitalWrite(Axis4_EN,LOW);
    delayMicroseconds(2);
    digitalWrite(Axis4_STEP,HIGH); delayMicroseconds(5); digitalWrite(Axis4_STEP,LOW); delayMicroseconds(5);
    digitalWrite(Axis4_EN,HIGH);
    delayMicroseconds(2);
    digitalWrite(Axis4_DIR,HIGH);
}

// step phases to opposite current direction, 4 steps required in half step mode
void focChangeDirection() {
    digitalWrite(Axis4_EN,LOW);
    delayMicroseconds(10);
    digitalWrite(Axis4_STEP,HIGH); delayMicroseconds(5); digitalWrite(Axis4_STEP,LOW); delayMicroseconds(5);
    digitalWrite(Axis4_STEP,HIGH); delayMicroseconds(5); digitalWrite(Axis4_STEP,LOW); delayMicroseconds(5);
    digitalWrite(Axis4_STEP,HIGH); delayMicroseconds(5); digitalWrite(Axis4_STEP,LOW); delayMicroseconds(5);
    digitalWrite(Axis4_STEP,HIGH); delayMicroseconds(5); digitalWrite(Axis4_STEP,LOW); delayMicroseconds(5);
    digitalWrite(Axis4_EN,HIGH);
    delayMicroseconds(10);
    if (focMovingIn) focMovingIn = false; else focMovingIn = true;
}

// Move focuser; numPulses equivalent to move distance; pulseWidth equivalent to move speed
void focMove(int numPulses, int pulseWidth) {   
    for (int i=0; i<numPulses; i++) {
        digitalWrite(Axis4_EN,LOW); delayMicroseconds(pulseWidth); 
        digitalWrite(Axis4_EN,HIGH); delayMicroseconds(EN_OFF_TIME); // arbitrarily selected 1 ms off time
    }  
    if (focMovingIn) focPosition -= numPulses; else focPosition += numPulses;
}

// Focuser GoTo a target position. Updated via main loop timer. Target set elsewhere.
void updateFocPosition() {
    if (!focGoToActive) return;
    focDeltaMove = focTarget - focPosition;
    if (abs(focDeltaMove) <= focMoveDistance) _moveDistance = abs(focDeltaMove); else _moveDistance = focMoveDistance;
    if (focDeltaMove < 0) { // negative move, go In
        if (focMovingIn) { // already moving In?
            focMove(_moveDistance, focMoveSpeed);
        } else { // moving Out, so change direction
            focChangeDirection();
            focMove(_moveDistance, focMoveSpeed);
        }
    } else if (focDeltaMove > 0) {  // positive move, go Out
        if (focMovingIn) { // moving In? then change direction
            focChangeDirection(); 
            focMove(_moveDistance, focMoveSpeed);
        } else { 
            focMove(_moveDistance, focMoveSpeed);
        }
    } else { // else positions equal, do nothing
        focGoToActive = false;
    }
}