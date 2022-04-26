#pragma once

// ===== Display.h =====

#ifndef DISPLAY_h
#define DISPLAY_h

class Display
{
public:
    // Local Command Channel
    void setLocalCmd(char *command);
    void setLocalCmd(const char *command);
    void getLocalCmd(const char *command, char *reply);
    void getLocalCmdTrim(const char *command, char *reply);
    
    void initDisiplay();
    void updateColors();
    void drawButton(int x_start, int y_start, int x_size, int y_size, int box_outline_color, int box_fill_color, int text_x_offset, int text_y_offset, const char* label);
    void drawRoundButton(int x, int y, int r, int outline_color, int fill_color, int text_x_offset, int text_y_offset, const char* label);
    void drawTitle(int text_x_offset, int text_y_offset, const char* label);
    void canvPrint(int x, int y, int y_off, int width, int height, int text_color, int back_color, const char* label);
    void canvPrint(int x, int y, int y_off, int width, int height, int text_color, int back_color, double label);
    void canvPrint(int x, int y, int y_off, int width, int height, int text_color, int back_color, int label);
    void updateOnstepCmdStatus();
    void drawMenuButtons();

    void drawCommonStatusLabels();
    void updateCommonStatus();
    void updateOdriveErrors();
    void drawPic(File *StarMaps, uint16_t x, uint16_t y, uint16_t WW, uint16_t HH);
   float getBatteryVoltage();

static void updateDisplayPage();
};
Display disp;

// Leveraged the following from SHC
class SHC
{
public:
    boolean dmsToDouble(double *f, char *dms, boolean sign_present, boolean highPrecision);
    boolean hmsToDouble(double *f, char *hms);
    double getLstT0();
    double getLat();
    void EquToHor(double RA, double Dec, double *Alt, double *Azm);
};
SHC shc;

/* Note: decided not to use this...did drawPic a different way
// add some methods to this class
class Adafruit_SPITFT_Teensy : public Adafruit_SPITFT
{
public:
	Adafruit_SPITFT_Teensy(void);
	void virtual writePixels(uint16_t *colors, uint32_t len, bool block, bool bigEndian);
    void virtual SPI_WRITE16(uint16_t w);
};
*/

// HomePage
void updateHomeStatus();
void updateGuideStatus();
void updateGoToStatus();
void updateFocuserStatus();
void updateOdriveStatus();
void updateXStatus();
void updateSettingsStatus();
void updateAlignStatus();
void updateMoreStatus();
void updateCatalogStatus();
void updatePlanetsStatus();
void updateHomeActionButtons();
void demoModeOn();
void updateOdriveMotorPositions();
void updateTouchInput();
void updateFocPosition();

// OnStep Forward Declarations to make VS Code happy
void cmdSend(const char *s, bool);
void processCommands();
bool cmdReply(char *s);
void soundAlert();

// Display variables
int lastPage = 22; // assign a non page number to get first Home Page to draw
bool batLED = false;
int current_AZ_ODerr = 0;
int current_ALT_ODerr = 0;

char locCmdReply[16]="";
char ra_hms[10], dec_dms[11];
char tra_hms[10], tdec_dms[11];
char currentRA[10] = "";
char currentDEC[11] = ""; 
char currentTargRA[10] = "";
char currentTargDEC[11] = "";
static double currentTargRA_d = 0.0;
static double currentTargDEC_d = 0.0;

double azm_d;
double alt_d;
double tazm_d;
double talt_d;
double current_azm = 0.0;
double current_alt = 0.0;
double tra_d;
double tra_dha = 0.0;
double tdec_d;
double current_tazm = 0.0;
double current_talt = 0.0;
double altitudeFt   = 0.0;

bool firstGPS = true;
bool firstDraw = true;
bool screenTouched = false;

bool ODpositionUpdateEnabled = true;
bool focGoToActive = false;
bool goToErr = false;
bool nightMode = false;
bool refreshScreen = false;
bool objectSelected = false;
bool hasReply = false;
uint8_t catSelected = 0;
uint16_t activeFilter = 0;
const char *activeFilterStr[3] = {"Filt: None", "Filt: Abv Hor", "Filt: All Sky"};

uint16_t PAGE_BACKGROUND = DEEP_MAROON;
uint16_t BUT_BACKGROUND = BLACK;
uint16_t TEXT_COLOR = YELLOW;
uint16_t BUT_OUTLINE = YELLOW;
uint16_t BUT_ON_BGD = RED;

enum CurrentPage
{
    HOME_PAGE,     // 0
    GUIDE_PAGE,    // 1
    FOCUSER_PAGE,  // 2
    GOTO_PAGE,     // 3
    MORE_PAGE,     // 4
    ODRIVE_PAGE,   // 5
    XSTATUS_PAGE,  // 6
    SETTINGS_PAGE, // 7
    ALIGN_PAGE,    // 8
    CATALOG_PAGE,  // 9
    PLANETS_PAGE,  // 10
    CUST_CAT_PAGE  // 11
}; 
CurrentPage currentPage = HOME_PAGE;

enum CAT_PAGE
{
    STARS,
    MESSIER,
    CALDWELL,
    HERSCHEL,
    INDEX,
    PLANETS,
    TREASURE,
    CUSTOM,
};

#endif // DISPLAY_h