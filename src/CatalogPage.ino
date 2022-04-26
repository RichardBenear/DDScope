// ******* Catalog Handler Page *********/
// Used much of the Catalog Manager from the Smart Hand Controller Code by xx

#include "DDcatalog/Strings_en.h"
#include "Display/Display.h"
#include "DDCatalog/Catalog.h"

// Catalog Selection buttons
#define SD_CARD_LINE_LEN    110 // Length of line stored to SD card for Custom Catalog
#define MAX_TREASURE_PAGES  8   // more than needed with current settings
#define MAX_TREASURE_ROWS   129 // full number of rows in catlog; starts with 1, not 0
#define MAX_CUSTOM_PAGES    10
#define MAX_CUSTOM_ROWS     MAX_CUSTOM_PAGES*NUM_CUS_ROWS_PER_SCREEN // this is arbitrary selection..could be more if needed
#define MAX_SHC_PAGES       30 // used by paging indexer affay for SHC
                               
#define NUM_CAT_ROWS_PER_SCREEN 16 //(370/CAT_H+CAT_Y_SPACING)
#define NUM_CUS_ROWS_PER_SCREEN 14 //(370/CUS_H+CUS_Y_SPACING)

#define CAT_X               1
#define CAT_Y               43
#define CAT_W               112
#define CAT_H               22
#define CAT_Y_SPACING       1
#define CAT_TEXT_X_OFF      3
#define CAT_TEXT_Y_OFF      10

// have a separate set of values for Custom Catalog for better spacing/visibility since it will be used more frequently
#define CUS_X               1
#define CUS_Y               43
#define CUS_W               112
#define CUS_H               24
#define CUS_Y_SPACING       2
#define CUS_TEXT_X_OFF      3
#define CUS_TEXT_Y_OFF      12

#define BACK_X              5
#define BACK_Y              445
#define BACK_W              60
#define BACK_H              35
#define BACK_T_X_OFF        15
#define BACK_T_Y_OFF        20

#define NEXT_X              255
#define NEXT_Y              BACK_Y

#define RETURN_X            165
#define RETURN_Y            BACK_Y
#define RETURN_W            80

#define SAVE_LIB_X          75
#define SAVE_LIB_Y          BACK_Y
#define SAVE_LIB_W          80
#define SAVE_LIB_H          BACK_H
#define SAVE_LIB_T_X_OFF    6
#define SAVE_LIB_T_Y_OFF    BACK_T_Y_OFF

#define SUB_STR_X_OFF       2
#define MAG_X_OFF           30
#define RA_X_OFF            70
#define DEC_X_OFF           140
#define FONT_Y_OFF          7
#define STATUS_STR_X        3
#define STATUS_STR_Y        430
#define STATUS_STR_W        150
#define STATUS_STR_H        16

bool treasureCatalog = false;
bool customCatalog = false;
bool shcCatalog = false;

bool objSel = false;
bool backSel = false;
bool nextSel = false;
bool saveTouched = false;
bool delSelected = false;

bool tEndOfList = false;
bool cEndOfList = false;
bool shcEndOfList = false;

uint16_t catButSelPos = 0;
uint16_t catButDetected = 0;

uint16_t tCurrentPage = 0;
uint16_t cCurrentPage = 0;
uint16_t shcCurrentPage = 0;

uint16_t tPrevPage = 0;
uint16_t cPrevPage = 0;
uint16_t returnToPage = 0;

uint16_t pre_tAbsIndex = 0;
uint16_t pre_tRelIndex = 0;
uint16_t pre_cAbsIndex = 0;
uint16_t pre_cRelIndex = 0;
uint16_t pre_shcIndex = 0;

uint16_t curSelTIndex = 0;
uint16_t curSelCIndex = 0;
uint16_t curSelSIndex = 0;

uint16_t tPrevRowIndex = 0;
uint16_t cPrevRowIndex = 0;
uint16_t shcPrevRowIndex = 0;

uint16_t tAbsRow = 0;
uint16_t cAbsRow = 0;

uint16_t tLastPage = 0;
uint16_t cLastPage = 0;

uint16_t shcLastPage = 0;
uint16_t shcLastRow = 0;

uint16_t tRow = 0;
uint16_t cRow = 0;
uint16_t shcRow = 0;

uint16_t width_off = 40; // negative width offset

uint16_t cusRowEntries = 0;
uint16_t treRowEntries = 0;

bool customItemSelected = false;

// ======== Arrays ==========
// Smart Hand Controller (4) Catalogs
char     shcObjName[NUM_CAT_ROWS_PER_SCREEN][20]; //19 + NULL
char        shcCons[NUM_CAT_ROWS_PER_SCREEN][7];
char          bayer[NUM_CAT_ROWS_PER_SCREEN][3];
char       shcSubId[NUM_CAT_ROWS_PER_SCREEN][18];
char     objTypeStr[NUM_CAT_ROWS_PER_SCREEN][15];
float        shcMag[NUM_CAT_ROWS_PER_SCREEN];
char  shcRACustLine[NUM_CAT_ROWS_PER_SCREEN][10];
char shcDECCustLine[NUM_CAT_ROWS_PER_SCREEN][11];
char     shcRaSrCmd[NUM_CAT_ROWS_PER_SCREEN][17]; // has ":Sr...#" cmd channel chars
char    shcDecSrCmd[NUM_CAT_ROWS_PER_SCREEN][18]; // has ":Sd...#" cmd channel chars
uint8_t    shcRaHrs[NUM_CAT_ROWS_PER_SCREEN][3]; 
uint8_t    shcRaMin[NUM_CAT_ROWS_PER_SCREEN][3];
uint8_t    shcRaSec[NUM_CAT_ROWS_PER_SCREEN][3];
short     shcDecDeg[NUM_CAT_ROWS_PER_SCREEN][5]; 
uint8_t   shcDecMin[NUM_CAT_ROWS_PER_SCREEN][3];
uint8_t   shcDecSec[NUM_CAT_ROWS_PER_SCREEN][3];
double       shcAlt[NUM_CAT_ROWS_PER_SCREEN];
double       shcAzm[NUM_CAT_ROWS_PER_SCREEN];
char    shcCustWrSD[SD_CARD_LINE_LEN] = "";

// Treasure Catalog
typedef struct {
    char* tObjName;
    char* tRAhRAm;
    char* tsDECdDECm;
    char* tCons;
    char* tObjType;
    char* tMag;
    char* tSize;
    char* tSubId;
} treasure_t; 
treasure_t _tArray[MAX_TREASURE_ROWS];

char       tRaSrCmd[MAX_TREASURE_ROWS][13]; 
char      tDecSrCmd[MAX_TREASURE_ROWS][14];
char      tRAhhmmss[MAX_TREASURE_ROWS][9];
char    tDECsddmmss[MAX_TREASURE_ROWS][10];
char Treasure_Array[MAX_TREASURE_ROWS][SD_CARD_LINE_LEN];
char   treaCustWrSD[SD_CARD_LINE_LEN] = "";
uint16_t tFiltArray[MAX_TREASURE_ROWS];

// Custom User Catalog
typedef struct {
    char* cObjName;
    char* cRAhhmmss;
    char* cDECsddmmss;
    char* cCons;
    char* cObjType;
    char* cMag;
    char* cSize;
    char* cSubId;
} custom_t; 
custom_t _cArray[MAX_CUSTOM_ROWS];

char      Custom_Array[MAX_CUSTOM_ROWS][SD_CARD_LINE_LEN];
char Copy_Custom_Array[MAX_CUSTOM_ROWS][SD_CARD_LINE_LEN]; // save a copy for row deletion purposes
char          cRaSrCmd[MAX_CUSTOM_ROWS][18]; 
char         cDecSrCmd[MAX_CUSTOM_ROWS][18];
uint16_t         cFiltArray[MAX_CUSTOM_ROWS];

// Other Arrays
double dtAlt[MAX_TREASURE_ROWS];
double dtAzm[MAX_TREASURE_ROWS];
double dcAlt[MAX_CUSTOM_ROWS];
double dcAzm[MAX_CUSTOM_ROWS];
uint16_t tPagingArrayIndex[MAX_TREASURE_PAGES];
uint16_t cPagingArrayIndex[MAX_CUSTOM_PAGES];
uint16_t shcPagingArrayIndex[MAX_SHC_PAGES];
char herschObjName[7]="";
char prefix[5]="";
char title[14]="";
char catSelectionStr1[26]="";
char catSelectionStr2[11]="";
char catSelectionStr3[11]="";
char catSelectionStr4[16]="";
char catSelectionStr5[15]="";
char truncObjType[5]="";
// Bayer designation, the Greek letter for each star within a constellation
const char* Txt_Bayer[25] = {
  "Alp","Bet","Gam","Del","Eps","Zet","Eta","The","Iot","Kap","Lam","Mu","Nu","Xi","Omi","Pi","Rho","Sig","Tau","Ups","Phi","Chi","Psi","Ome","?"
};

//==================================================================
//============ Draw the Initial Catalog Screen =====================
//==================================================================
void drawCatalogPage(int catSel) { 
    returnToPage = currentPage; // save page from where this function was called so we can return
    currentPage = CATALOG_PAGE;
    objectSelected = false;
    catSelected = catSel;
    tCurrentPage = 0; 
    cCurrentPage = 0;
    shcCurrentPage = 0;
    tPrevPage = 0;
    cPrevPage = 0;
    tEndOfList = false;
    cEndOfList = false;
    shcEndOfList = false;
    
    disp.updateColors();
    tft.setTextColor(TEXT_COLOR);
    tft.fillScreen(PAGE_BACKGROUND);

    // Check which Catalog is active
    if (catSelected == cat_mgr.numCatalogs()+1) { //This is Treasure catalog
        disp.drawTitle(90, 30, "Treasure");
        treasureCatalog = true;
        customCatalog = false;
        shcCatalog = false;
        if (!loadTreasureArray()) {
            disp.canvPrint(STATUS_STR_X, STATUS_STR_Y, -6, STATUS_STR_W, STATUS_STR_H, TEXT_COLOR, BUT_BACKGROUND, "ERR:Loading Treasure");
            drawMorePage();
            return;
        } else { 
            parseTcatIntoArray();
        }
        tAbsRow = 0; // initialize the absolute index pointer into total array

    } else if (catSelected == cat_mgr.numCatalogs()+2) { //This is Custom User catalog
        disp.drawTitle(80, 30, "User Catalog");
        customCatalog = true;
        treasureCatalog = false;
        shcCatalog = false;
        if (!loadCustomArray()) {
            disp.canvPrint(STATUS_STR_X, STATUS_STR_Y, -6, STATUS_STR_W, STATUS_STR_H, TEXT_COLOR, BUT_BACKGROUND, "ERR:Loading Custom"); 
            drawMorePage();
            return;
        } else {
            parseCcatIntoArray();
        }
        cAbsRow = 0; // initialize the absolute index into total array
        
        // Draw the Trash can/Delete Icon bitmap; used to delete an entry in only the CUSTOM USER catalog
        uint8_t extern trash_icon[];
        tft.drawBitmap(270, 5, trash_icon, 28, 32, BUT_BACKGROUND, ORANGE);

    } else if (catSelected <= cat_mgr.numCatalogs()) {// process the catalogs that come with the Onstep Smart Hand Controller
        shcCatalog = true;
        treasureCatalog = false;
        customCatalog = false;

        for (int i=0; i<MAX_SHC_PAGES; i++) shcPagingArrayIndex[i] = 0; // initialize paging array indexes

        shcPrevRowIndex = cat_mgr.getIndex();
        cat_mgr.select(catSelected);
        cat_mgr.setIndex(0); // initialize row index for entire catalog array at zero
        strcpy(prefix, cat_mgr.catalogPrefix()); // prefix for catalog e.g. Star, M, N, I etc

        // Show Title and number of Entries of this catalog
        strcpy(title, cat_mgr.catalogTitle());
        if (catSelected == HERSCHEL) strcpy(title, "Herschel"); else // shorten title
        if (catSelected == STARS) strcpy(title, "Stars"); // shorten title
        disp.drawTitle(110, 32, title); 
        tft.setFont(); tft.setTextSize(1);
        tft.setCursor(9,25); strcpy(title,cat_mgr.catalogSubMenu()); tft.print(title); 

        tft.setCursor(235, 25); tft.printf("Entries="); 
        if (activeFilter) tft.print("?Filt"); else tft.print(cat_mgr.getMaxIndex());
        tft.setCursor(235, 9); tft.print(activeFilterStr[activeFilter]);
    }

    tft.setFont(&Inconsolata_Bold8pt7b);
    disp.drawButton(BACK_X, BACK_Y, BACK_W, BACK_H, BUT_OUTLINE, BUT_BACKGROUND, BACK_T_X_OFF, BACK_T_Y_OFF, "BACK");
    disp.drawButton(NEXT_X, NEXT_Y, BACK_W, BACK_H, BUT_OUTLINE, BUT_BACKGROUND, BACK_T_X_OFF, BACK_T_Y_OFF, "NEXT");
    disp.drawButton(RETURN_X, RETURN_Y, RETURN_W, BACK_H, BUT_OUTLINE, BUT_BACKGROUND, BACK_T_X_OFF, BACK_T_Y_OFF, "RETURN");
    disp.drawButton(SAVE_LIB_X, SAVE_LIB_Y, SAVE_LIB_W, SAVE_LIB_H, BUT_OUTLINE, BUT_BACKGROUND, SAVE_LIB_T_X_OFF, SAVE_LIB_T_Y_OFF, "SAVE LIB");

    drawACatPage(); // draw first page of the selected catalog
}

// The Treasure catalog is a compilation of popular objects from :rDUINO Scope-http://www.rduinoscope.tk/index.html
// Load the treasure.csv file from SD to an array of text lines
// success = 1, fail = 0
bool loadTreasureArray() {
    //============== Begin Load from File ==========================
    // Load RAM array from SD catalog file
    char rdChar;
    char rowString[SD_CARD_LINE_LEN]=""; // temp row buffer
    int rowNum=0;
    int charNum=0;
    File rdFile = SD.open("mod1_treasure.csv");

    if (rdFile) {
        // load data from SD into array
        while (rdFile.available()) {
            rdChar = rdFile.read();
            rowString[charNum] = rdChar;
            charNum++;
            
            if (rdChar == '\n') {
                strcpy(Treasure_Array[rowNum], rowString);
                rowNum++;
                memset(&rowString, 0, SD_CARD_LINE_LEN);
                charNum = 0;
            }
        }
        treRowEntries = rowNum-1;
        //VF("treEntries="); VL(treRowEntries);
    } else {
        VLF("SD Treas read error");
        return false;
    }
    rdFile.close();
    //VLF("SD Tres read done");
    return true;
}

// The Custom catalog is a selection of User objects that have been saved on the SD card.
//      When using any of the catalogs, the SaveToCat button will store the objects info
//      in the Custom Catalog.
// Load the custom.csv file from SD into an RAM array of text lines
// success = 1, fail = 0
bool loadCustomArray() {
    //============== Load from Custom File ==========================
    // Load RAM array from SD catalog file
    char rdChar;
    char rowString[SD_CARD_LINE_LEN]=""; // temp row buffer
    int rowNum=0;
    int charNum=0;
    File rdFile = SD.open("custom.csv");

    if (rdFile) {
        // load data from SD into array
        while (rdFile.available()) {
            rdChar = rdFile.read();
            rowString[charNum] = rdChar;
            charNum++;
            if (rdChar == '\n') {
                strcpy(Custom_Array[rowNum], rowString);
                strcpy(Copy_Custom_Array[rowNum], rowString);
                rowNum++;
                memset(&rowString, 0, SD_CARD_LINE_LEN);
                charNum = 0;
            }
        }
        cusRowEntries = rowNum-1; // actual number since rowNum was already incremented; start at 0
    } else {
        VLF("SD Cust read error");
        return false;
    }
    rdFile.close();
    //VLF("SD Cust read done");
    return true;
}

// Parse the treasure.csv line data into individual fields in an array
// mod1_treasure.csv file format = ObjName;RAhRAm;SignDECdDECm;Cons;ObjType;Mag;Size;SubId\n
// mod1_treasure.csv field spacing: 7;8;7;4;9;4;9;18 = 66 total w/out semicolons, 73 with
// 7 - objName
// 8 - RA
// 7 - DEC
// 4 - Cons
// 9 - ObjType
// 4 - Mag
// 9 - Size
// 18 - SubId
void parseTcatIntoArray() {
    for (int i=0; i<MAX_TREASURE_ROWS; i++) {
        _tArray[i].tObjName      = strtok(Treasure_Array[i], ";"); //VL(_tArray[i].tObjName);
        _tArray[i].tRAhRAm       = strtok(NULL, ";");              //VL(_tArray[i].tRAhRAm);
        _tArray[i].tsDECdDECm    = strtok(NULL, ";");              //VL(_tArray[i].tsDECdDECm);
        _tArray[i].tCons         = strtok(NULL, ";");              //VL(_tArray[i].tCons);
        _tArray[i].tObjType      = strtok(NULL, ";");              //VL(_tArray[i].tObjType);
        _tArray[i].tMag          = strtok(NULL, ";");              //VL(_tArray[i].tMag);
        _tArray[i].tSize         = strtok(NULL, ";");              //VL(_tArray[i].tSize);
        _tArray[i].tSubId        = strtok(NULL, "\r");             //VL(_tArray[i].tSubId);
    }
    //VLF("finished parsing treasure");
}

// Parse the custom.csv line data into individual fields in an array
// custom.csv file format = ObjName;Mag;Cons;ObjType;SubId;cRahhmmss;cDecsddmmss\n
void parseCcatIntoArray() {
    for (int i=0; i<=cusRowEntries; i++) {
        _cArray[i].cObjName      = strtok(Custom_Array[i], ";"); //VL(_cArray[i].cObjName);
        _cArray[i].cMag          = strtok(NULL, ";");            //VL(_cArray[i].cMag);
        _cArray[i].cCons         = strtok(NULL, ";");            //VL(_cArray[i].cCons);
        _cArray[i].cObjType      = strtok(NULL, ";");            //VL(_cArray[i].cObjType);
        _cArray[i].cSubId        = strtok(NULL, ";");            //VL(_cArray[i].cSubId);
        _cArray[i].cRAhhmmss     = strtok(NULL, ";");            //VL(_cArray[i].cRAhhmmss)
        _cArray[i].cDECsddmmss   = strtok(NULL, "\n");           //VL(_cArray[i].cDECsddmmss);
    }
    //VLF("finished parsing custom");
}

// =====================================================
// ============ Draw a page of Catalog Data ============
// =====================================================
void drawACatPage() {
    char catLine[47]=""; //hold the string that is displayed beside the button on each page
    tRow = 0;
    cRow = 0;
    shcRow = 0;
    pre_tAbsIndex=0;
    pre_cAbsIndex=0;
    pre_shcIndex=0;
    
    tft.setFont(); tft.setTextSize(1);
    tft.fillRect(0,60,319,358,PAGE_BACKGROUND);
    
    // ============= TREASURE catalog =================
    // TREASURE Catalog takes different processing than SmartHandController Catalogs
    // since it is stored on the SD card in a different format
    if (treasureCatalog) {  
        tAbsRow = (tPagingArrayIndex[tCurrentPage]); // array of page 1st row indexes
        tLastPage = ((MAX_TREASURE_ROWS / NUM_CAT_ROWS_PER_SCREEN)+1);
        
        // Show Page number and total Pages
        tft.fillRect(6, 9, 70, 12, BUT_BACKGROUND);
        tft.setCursor(6, 9); tft.printf("Page "); tft.print(tCurrentPage+1);
        tft.printf(" of "); if (activeFilter == FM_ABOVE_HORIZON) tft.print("??"); else tft.print(tLastPage); 
        tft.setCursor(6, 25); tft.print(activeFilterStr[activeFilter]);
        
        // ========== draw TREASURE page of catalog data ========
        while ((tRow < NUM_CAT_ROWS_PER_SCREEN) && (tAbsRow != MAX_TREASURE_ROWS)) {  

            // ======== convert RA/DEC ===========
            char  *_end;
            char  *nextStr;
            double raH  = 0.0;
            int    raM  = 0;
            int    raS  = 0;
            double decD = 0.0;
            int    decM = 0;
            int    decS = 0;
            double tRAdouble;
            double tDECdouble;

            // Format RA in Hrs:Min:Sec and form string to be sent as target
            raH = strtod(_tArray[tAbsRow].tRAhRAm, &_end); // convert first hour chars
            nextStr = strchr(_tArray[tAbsRow].tRAhRAm,'h'); // get ptr to minute chars
            if (nextStr != NULL) {
                raM = atoi(nextStr+1);
            } else {
                VLF("found RA==NULL in treasure");
            }
            snprintf(tRAhhmmss[tAbsRow], 9, "%02d:%02d:00", (int)raH, raM);
            snprintf(tRaSrCmd[tAbsRow], 13, ":Sr%02d:%02d:%02d#", (int)raH, raM, raS); //Note: this is in HOURS
        
            // Format DEC in Deg:Min:Sec and form string to be sent as target
            decD = strtod(_tArray[tAbsRow].tsDECdDECm, &_end);
            nextStr = strchr(_tArray[tAbsRow].tsDECdDECm, 'd');
            if (nextStr != NULL) {
                decM = atoi(nextStr+1);
            } else {
                VLF("found DEC==NULL in treasure");
            }
            snprintf(tDECsddmmss[tAbsRow], 10, "%+03d*%02d:00", (int)decD, decM);
            snprintf(tDecSrCmd[tAbsRow], 14, ":Sd%+03d*%02d:%02d#", (int)decD, decM, decS);
    
            // to get Altitude, first convert RAh and DEC to double
            shc.hmsToDouble(&tRAdouble, tRAhhmmss[tAbsRow]);
            shc.dmsToDouble(&tDECdouble, tDECsddmmss[tAbsRow], true, PM_HIGH);

            // dtAlt[tAbsRow] is used by filter to check if above Horizon
            cat_mgr.EquToHor(tRAdouble*15, tDECdouble, &dtAlt[tAbsRow], &dtAzm[tAbsRow]);
            //VF("dtAlt="); VL(dtAlt[tAbsRow]);
            // filter out elements below 10 deg if filter enabled
            if (((activeFilter == FM_ABOVE_HORIZON) && (dtAlt[tAbsRow] > 10.0)) || activeFilter == FM_NONE) { 
            
                // Erase text background
                tft.setCursor(CAT_X+CAT_W-width_off+2, CAT_Y+tRow*(CAT_H+CAT_Y_SPACING));
                tft.fillRect(CAT_X+CAT_W-width_off+2, CAT_Y+tRow*(CAT_H+CAT_Y_SPACING), 215+width_off, 17, BUT_BACKGROUND);
        
                // get object names and put them on the buttons
                disp.drawButton(CAT_X, CAT_Y+tRow*(CAT_H+CAT_Y_SPACING), CAT_W-width_off, CAT_H, BUT_OUTLINE, BUT_BACKGROUND, CAT_TEXT_X_OFF, CAT_TEXT_Y_OFF, _tArray[tAbsRow].tObjName);
                            
                // format and print the text field for this row next to the button
                // FYI: mod1_treasure.csv file field order and spacing: 7;8;7;4;9;4;9;18 = 66 total w/out semicolons, 73 with ;'s
                // 7 - objName
                // 8 - RA
                // 7 - DEC
                // 4 - Cons
                // 9 - ObjType
                // 4 - Mag
                // 9 - Size ( Not used )
                // 18 - SubId
                // select some Treasure fields to show beside button
                snprintf(catLine, 42, "%-4s |%-4s |%-9s |%-18s",  //35 + 6 + NULL = 42
                                                    _tArray[tAbsRow].tMag, 
                                                    _tArray[tAbsRow].tCons, 
                                                    _tArray[tAbsRow].tObjType, 
                                                    _tArray[tAbsRow].tSubId);
                tft.setCursor(CAT_X+CAT_W+SUB_STR_X_OFF-width_off+2, CAT_Y+tRow*(CAT_H+CAT_Y_SPACING)+FONT_Y_OFF); 
                tft.print(catLine);
                tFiltArray[tRow] = tAbsRow;
                //VF("tFiltArray[tRow]="); VL(tFiltArray[tRow]);
                tRow++; // increments only through the number of lines displayed on screen per page
                //VF("tRow="); VL(tRow);
            } 
            tPrevRowIndex = tAbsRow;
            tAbsRow++; // increments through all lines in the catalog
            
            // stop printing data if last row on the last page
            if (tAbsRow == MAX_TREASURE_ROWS) {
                tEndOfList = true; 
                if (tRow == 0) {disp.canvPrint(STATUS_STR_X, STATUS_STR_Y, 0, STATUS_STR_W, STATUS_STR_H, TEXT_COLOR, BUT_BACKGROUND, "None above 10 deg");}
                return; 
            }
        }
        tPagingArrayIndex[tCurrentPage+1] = tAbsRow; // tPagingArrayIndex holds index of first element of page to help with NEXT and BACK paging
        //VF("tPagingArrayIndex+1="); VL(tPagingArrayIndex[tCurrentPage+1]);

    } else if (customCatalog) {//====== draw CUSTOM USER page of catalog data ======
        cAbsRow = (cPagingArrayIndex[cCurrentPage]); // array of page 1st row indexes
        cLastPage = (cusRowEntries / NUM_CUS_ROWS_PER_SCREEN)+1;
       
        //VF("cusRowEntries="); VL(cusRowEntries);
        //VF("cLastPage="); VL(cLastPage);
        //VF("cCurPage="); VL(cCurrentPage);

        // Show Page number and total Pages
        tft.fillRect(6, 9, 70, 12, BUT_BACKGROUND);
        tft.setCursor(6, 9); tft.printf("Page "); tft.print(cCurrentPage+1);
        tft.printf(" of "); if (activeFilter == FM_ABOVE_HORIZON) tft.print("??"); else tft.print(cLastPage+1);
        tft.setCursor(6, 25); tft.print(activeFilterStr[activeFilter]);

        while ((cRow < NUM_CUS_ROWS_PER_SCREEN) && (cAbsRow != MAX_CUSTOM_ROWS)) {  
            //VF("cAbsRow="); VL(cAbsRow);
            //VF("cRow="); VL(cRow);
            
            // ======== process RA/DEC ===========
            double cRAdouble;
            double cDECdouble;

            // RA in Hrs:Min:Sec
            snprintf(cRaSrCmd[cAbsRow], 16, ":Sr%11s#", _cArray[cAbsRow].cRAhhmmss);
            snprintf(cDecSrCmd[cAbsRow], 17, ":Sd%12s#", _cArray[cAbsRow].cDECsddmmss);
    
            // to get Altitude, first convert RAh and DEC to double
            shc.hmsToDouble(&cRAdouble, _cArray[cAbsRow].cRAhhmmss);
            shc.dmsToDouble(&cDECdouble, _cArray[cAbsRow].cDECsddmmss, true, PM_HIGH);
            
            //double cHAdouble=haRange(LST()*15.0-cRAdouble);
            cat_mgr.EquToHor(cRAdouble*15, cDECdouble, &dcAlt[cAbsRow], &dcAzm[cAbsRow]);

            //VF("activeFilter="); VL(activeFilter);
            if (((activeFilter == FM_ABOVE_HORIZON) && (dcAlt[cAbsRow] > 10.0)) || activeFilter == FM_NONE) { // filter out elements below 10 deg if filter enabled
                //VF("printing row="); VL(cAbsRow);
                // Erase text background
                tft.setCursor(CUS_X+CUS_W+2, CUS_Y+cRow*(CUS_H+CUS_Y_SPACING));
                tft.fillRect(CUS_X+CUS_W+2, CUS_Y+cRow*(CUS_H+CUS_Y_SPACING), 215, 17, BUT_BACKGROUND);
        
                // get object names and put them on the buttons
                disp.drawButton(CUS_X, CUS_Y+cRow*(CUS_H+CUS_Y_SPACING), CUS_W, CUS_H, BUT_OUTLINE, BUT_BACKGROUND, CUS_TEXT_X_OFF, CUS_TEXT_Y_OFF, _cArray[cAbsRow].cObjName);
                            
                // format and print the text field for this row next to the button
                snprintf(catLine, 42, "%-4s |%-4s |%-9s |%-18s",  // 35 + 6 + NULL = 42
                                                        _cArray[cAbsRow].cMag, 
                                                        _cArray[cAbsRow].cCons, 
                                                        _cArray[cAbsRow].cObjType, 
                                                        _cArray[cAbsRow].cSubId);
                tft.setCursor(CUS_X+CUS_W+SUB_STR_X_OFF+2, CUS_Y+cRow*(CUS_H+CUS_Y_SPACING)+FONT_Y_OFF); 
                tft.print(catLine);
                cFiltArray[cRow] = cAbsRow;
                //VF("cFiltArray[cRow]="); VL(cFiltArray[cRow]);
                cRow++; // increments only through the number of lines displayed on screen per page
                //VF("ceRow="); VL(cRow);
            } 
            cPrevRowIndex = cAbsRow;
            cAbsRow++; // increments through all lines in the catalog

            // stop printing data if last row on the last page
            if (cAbsRow == cusRowEntries+1) {
                cEndOfList = true; 
                if (cRow == 0) {disp.canvPrint(STATUS_STR_X, STATUS_STR_Y, 0, STATUS_STR_W, STATUS_STR_H, TEXT_COLOR, BUT_BACKGROUND, "None above 10 deg");}
                //VF("endOfList");
                return; 
            }
        }
        cPagingArrayIndex[cCurrentPage+1] = cAbsRow; // cPagingArrayIndex holds index of first element of page to help with NEXT and BACK paging
        //VF("PagingArrayIndex+1="); VL(cPagingArrayIndex[cCurrentPage+1]);

    } else {  //=================== draw page of SHC catalog data =======================
        
        cat_mgr.setIndex(shcPagingArrayIndex[shcCurrentPage]); // array of page 1st row indexes
        if (cat_mgr.hasActiveFilter()) {
            shcLastPage = shcPrevRowIndex >= cat_mgr.getIndex();
        } else {
            shcLastPage = ((cat_mgr.getMaxIndex() / NUM_CAT_ROWS_PER_SCREEN)+1);
        }
        shcLastRow = (cat_mgr.getMaxIndex() % NUM_CAT_ROWS_PER_SCREEN);

        while ((shcRow < NUM_CAT_ROWS_PER_SCREEN) || ((shcRow < shcLastRow) && shcLastPage)) { 
            // Page number and total Pages
            tft.fillRect(8, 9, 70, 12, BUT_BACKGROUND);
            tft.setCursor(8, 9); tft.printf("Page "); tft.print(shcCurrentPage+1);
            tft.printf(" of ");
            if (activeFilter) tft.print("??"); else tft.print((cat_mgr.getMaxIndex()/NUM_CAT_ROWS_PER_SCREEN)+1);

            // erase any previous data
            tft.setCursor(CAT_X+CAT_W+2, CAT_Y+shcRow*(CAT_H+CAT_Y_SPACING));
            tft.fillRect(CAT_X+CAT_W+2, CAT_Y+shcRow*(CAT_H+CAT_Y_SPACING), 215, 17, BUT_BACKGROUND);

            //fill the button with some identifying text which varies depending on catalog chosen
            memset(shcObjName[shcRow], '\0', 20); //NULL out row first
            if (catSelected == HERSCHEL || catSelected == INDEX) { //use prefix and primaryId for these 2 catalogs
                snprintf(shcObjName[shcRow], 9, "%2s%4ld", prefix, cat_mgr.primaryId());
                //VF("priId="); VL(cat_mgr.primaryId());
            } else if (cat_mgr.objectName()!= -1) { // does it have a name
                strcpy(shcObjName[shcRow], cat_mgr.objectNameStr());
            } else if (cat_mgr.subId()!=-1) { // does it have a subId
                strcpy(shcObjName[shcRow], cat_mgr.subIdStr()); 
                //VF("subId="); VL(cat_mgr.subIdStr());
            } else {
                strcpy(shcObjName[shcRow], "Unknown");
            }
            disp.drawButton(CAT_X, CAT_Y+shcRow*(CAT_H+CAT_Y_SPACING), CAT_W, CAT_H, BUT_OUTLINE, BUT_BACKGROUND, CAT_TEXT_X_OFF, CAT_TEXT_Y_OFF, shcObjName[shcRow]);
        
            // Object type e.g. Star, Galaxy, etc
            memset(objTypeStr[shcRow], '\0', sizeof(objTypeStr[shcRow]));
            strcpy(objTypeStr[shcRow], cat_mgr.objectTypeStr());
            
            // Object SubId, e.g. N147, N7243
            memset(shcSubId[shcRow], '\0', sizeof(shcSubId[shcRow]));
            strcpy(shcSubId[shcRow], cat_mgr.subIdStr());
            
            // Constellation
            memset(shcCons[shcRow], '\0', sizeof(shcCons[shcRow]));
            strcpy(shcCons[shcRow], cat_mgr.constellationStr());
            
            // magnitude column
            shcMag[shcRow] = cat_mgr.magnitude();
    
            // bayer and flamsteen
            memset(bayer[shcRow], '\0', sizeof(bayer[shcRow]));
            if (!cat_mgr.isDsoCatalog()) { // stars catalog
                if (cat_mgr.bayerFlam() < 24) { // show greek letter names
                    strcpy(bayer[shcRow], Txt_Bayer[cat_mgr.bayerFlam()]);
                } else { // just show Flamsteen number
                    strcpy(bayer[shcRow], cat_mgr.bayerFlamStr());
                }
                snprintf(catLine, 34, "%4.1f |%3s%4s |%-18s", // 29 + 4 + NULL = 34
                                                shcMag[shcRow], 
                                                bayer[shcRow], 
                                                shcCons[shcRow], 
                                                objTypeStr[shcRow]);
            } else { // not a star 
                snprintf(catLine, 47, "%4.1f |%-4s|%-15s |%-18s", //41 + 5 + NULL = 47
                                                shcMag[shcRow], 
                                                shcCons[shcRow], 
                                                objTypeStr[shcRow], 
                                                shcSubId[shcRow]);
            }
            // print out a line of data to the right of the object's button
            tft.setCursor(CAT_X+CAT_W+SUB_STR_X_OFF, CAT_Y+shcRow*(CAT_H+CAT_Y_SPACING)+FONT_Y_OFF); 
            tft.print(catLine);
        
            // fill the RA array for this row on the current page
            // RA in Hrs:Min:Sec
            cat_mgr.raHMS(*shcRaHrs[shcRow], *shcRaMin[shcRow], *shcRaSec[shcRow]);
            // shcRACustLine is used by the "Save to custom" catalog feature
            snprintf(shcRACustLine[shcRow], 9, "%02u:%02u:%02u", (unsigned int)*shcRaHrs[shcRow], (unsigned int)*shcRaMin[shcRow], (unsigned int)*shcRaSec[shcRow]);
            snprintf(shcRaSrCmd[shcRow], 14, ":Sr%s#", shcRACustLine[shcRow]); // written to the controller for GoTo coordinates
            
            // fill the DEC array for this Row on the current page
            // DEC in Deg:Min:Sec
            cat_mgr.decDMS(*shcDecDeg[shcRow], *shcDecMin[shcRow], *shcDecSec[shcRow]);

            double osALT, osAZM;
            //save the Alt and Azm for use later
            cat_mgr.EquToHor(cat_mgr.ra(), cat_mgr.dec(), &shcAlt[shcRow], &shcAzm[shcRow]);
            equToHor(cat_mgr.ra(), cat_mgr.dec(), &osALT, &osAZM);

            // shcDECCustLine is used later by the "Save to custom catalog" feature
            snprintf(shcDECCustLine[shcRow], 10, "%+03d*%02u:%02u", (int)*shcDecDeg[shcRow], (unsigned int)*shcDecMin[shcRow], (unsigned int)*shcDecSec[shcRow]);
            snprintf(shcDecSrCmd[shcRow], 15, ":Sd%s#", shcDECCustLine[shcRow]); // written to the controller for GoTo coordinates
            
            shcPrevRowIndex = cat_mgr.getIndex();
            shcRow++; // increments through the number of lines on screen
            cat_mgr.incIndex(); // increment the index that keeps track of the entire catalog's contents
            
            // stop printing data if last field on the last page
            //VF("shcIndex="); VL(cat_mgr.getIndex());
            //VF("shcPrevRowIndex="); VL(shcPrevRowIndex);
            if ((shcPrevRowIndex >= cat_mgr.getIndex()) || (cat_mgr.getIndex() == cat_mgr.getMaxIndex())) {
                shcEndOfList = true; 
                return; 
            }
        }
        shcPagingArrayIndex[shcCurrentPage+1] = cat_mgr.getIndex(); // shcPagingArrayIndex holds index of first element of page to help with NEXT and BACK paging
        //VF("shcPagingArrayIndex+1="); VL(shcPagingArrayIndex[shcCurrentPage+1]);
    }
}

//======================================================
// =====  Update Screen for buttons and text ===========
//======================================================
void updateCatalogStatus() { 
    uint16_t tRelIndex = 0;
    uint16_t tAbsIndex = 0;
    uint16_t cRelIndex = 0;
    uint16_t cAbsIndex = 0;

    if (screenTouched || refreshScreen) { 
        refreshScreen = false;
        if (screenTouched) refreshScreen = true;
        
        // ====== TREASURE Catalog =========
        if (catButDetected && treasureCatalog) { 
            tRelIndex = catButSelPos; // save the relative-to-this-screen-index of button pressed
            tAbsIndex = tFiltArray[catButSelPos]; // this is absolute full array index

            if (tPrevPage == tCurrentPage) { //erase previous selection
                disp.drawButton(CAT_X, CAT_Y+pre_tRelIndex*(CAT_H+CAT_Y_SPACING), 
                    CAT_W-width_off, CAT_H, BUT_OUTLINE, BUT_BACKGROUND, CAT_TEXT_X_OFF, CAT_TEXT_Y_OFF, _tArray[pre_tAbsIndex].tObjName); 
            }
            // highlight selected by settting background ON color 
            disp.drawButton(CAT_X, CAT_Y+tRelIndex*(CAT_H+CAT_Y_SPACING), 
                CAT_W-width_off, CAT_H, BUT_OUTLINE, BUT_ON_BGD, CAT_TEXT_X_OFF, CAT_TEXT_Y_OFF, _tArray[tAbsIndex].tObjName); 
            
            // the following 5 lines are displayed on the Catalog/More page
            // Note: ObjName and SubId are swapped here relative to other catalogs since the Treasure catalog is formatted differently
            snprintf(catSelectionStr1, 26, "Name:%-19s", _tArray[tAbsIndex].tSubId);   //VF("t_subID=");   VL(_tArray[tAbsIndex].tSubId);
            snprintf(catSelectionStr2, 11, "Mag-:%-4s",  _tArray[tAbsIndex].tMag);     //VF("t_mag=");     VL(_tArray[tAbsIndex].tMag);
            snprintf(catSelectionStr3, 11, "Cons:%-4s",  _tArray[tAbsIndex].tCons);    //VF("t_constel="); VL(_tArray[tAbsIndex].tCons);
            snprintf(catSelectionStr4, 16, "Type:%-9s",  _tArray[tAbsIndex].tObjType); //VF("t_objType="); VL(_tArray[tAbsIndex].tObjType);
            snprintf(catSelectionStr5, 15, "Id--:%-7s",  _tArray[tAbsIndex].tObjName); //VF("t_objName="); VL(_tArray[tAbsIndex].tObjName);
           
            // show if we are above and below visible limits
            if (dtAlt[tAbsIndex] > 10.0) {   // show minimum 10 degrees altitude, use dtAlt[tAbsIndex] previously calculated in drawCatPage()
                disp.canvPrint(STATUS_STR_X, STATUS_STR_Y, 0, STATUS_STR_W, STATUS_STR_H, TEXT_COLOR, BUT_BACKGROUND, "Above +10 deg");
            } else {
                disp.canvPrint(STATUS_STR_X, STATUS_STR_Y, 0, STATUS_STR_W, STATUS_STR_H, TEXT_COLOR, BUT_BACKGROUND, "Below +10 deg");
            }
            writeTarget(tAbsIndex); // write RA and DEC as target for GoTo
            showTargetCoords(); // display the target coordinates that were just written

            pre_tRelIndex = tRelIndex;
            pre_tAbsIndex = tAbsIndex;
            curSelTIndex = tAbsIndex;
            tPrevPage = tCurrentPage;
            catButDetected = false;
        }

        // ====== CUSTOM USER catalog ... uses different spacing =====
        if (catButDetected && customCatalog) { 

            cRelIndex = catButSelPos; // save the "screen/page" index of button pressed
            cAbsIndex = cFiltArray[catButSelPos];

            if (cPrevPage == cCurrentPage) { //erase previous selection
                disp.drawButton(CUS_X, CUS_Y+pre_cRelIndex*(CUS_H+CUS_Y_SPACING), 
                    CUS_W, CUS_H, BUT_OUTLINE, BUT_BACKGROUND, CUS_TEXT_X_OFF, CUS_TEXT_Y_OFF, _cArray[pre_cAbsIndex].cObjName); 
            }
            // highlight selected by settting background ON color 
            disp.drawButton(CUS_X, CUS_Y+cRelIndex*(CUS_H+CUS_Y_SPACING), 
                CUS_W, CUS_H, BUT_OUTLINE, BUT_ON_BGD, CUS_TEXT_X_OFF, CUS_TEXT_Y_OFF, _cArray[cAbsIndex].cObjName); 

            // the following 5 lines are displayed on the Catalog/More page
            snprintf(catSelectionStr1, 26, "Name-:%-19s", _cArray[cAbsIndex].cObjName);  //VF("c_objName="); //VL(_cArray[cAbsIndex].cObjName);
            snprintf(catSelectionStr2, 11, "Mag--:%-4s",  _cArray[cAbsIndex].cMag);      //VF("c_Mag=");     //VL(_cArray[cAbsIndex].cMag);
            snprintf(catSelectionStr3, 11, "Const:%-4s",  _cArray[cAbsIndex].cCons);     //VF("c_constel="); //VL(_cArray[cAbsIndex].cCons);
            snprintf(catSelectionStr4, 16, "Type-:%-9s",  _cArray[cAbsIndex].cObjType);  //VF("c_objType="); //VL(_cArray[cAbsIndex].cObjType);
            snprintf(catSelectionStr5, 15, "Id---:%-7s",  _cArray[cAbsIndex].cSubId);    //VF("c_subID=");   //VL(_cArray[cAbsIndex].cSubId);
            
            // show if we are above and below visible limits
            if (dcAlt[cAbsIndex] > 10.0) {      // minimum 10 degrees altitude
                disp.canvPrint(STATUS_STR_X, STATUS_STR_Y, 0, STATUS_STR_W, STATUS_STR_H, TEXT_COLOR, BUT_BACKGROUND, "Above +10 deg");
            } else {
                disp.canvPrint(STATUS_STR_X, STATUS_STR_Y, 0, STATUS_STR_W, STATUS_STR_H, TEXT_COLOR, BUT_BACKGROUND, "Below +10 deg");
            }
            writeTarget(cAbsIndex); // write RA and DEC as target for GoTo
            showTargetCoords(); // display the target coordinates that were just written

            pre_cRelIndex = cRelIndex;
            pre_cAbsIndex = cAbsIndex;
            curSelCIndex = cAbsIndex;
            cPrevPage = cCurrentPage;
            catButDetected = false;
            customItemSelected = true;
        }

        // DELETE CUSTOM USER library ROW check
        // Delete the row and shift other rows up, write back to storage media
        if (delSelected && customItemSelected) {
            delSelected = false;

            // delIndex is Row to delete in full array
            cAbsIndex = cFiltArray[catButSelPos];
            uint16_t delIndex = cAbsIndex;
            if (cusRowEntries == 0) { // check if delete of only one entry in catalog, if so, just delete catalog
                File rmFile = SD.open("/custom.csv");
                    if (rmFile) {
                        SD.remove("/custom.csv");
                    }
                rmFile.close(); 
                return;
            } else { // copy rows after the one to be deleted over rows -1
                //VF("cAbsIndex="); VL(cAbsIndex);
                while (delIndex <= cusRowEntries-1) {
                    strcpy(Copy_Custom_Array[delIndex], Copy_Custom_Array[delIndex+1]);
                    delIndex++;
                }
                memset(Copy_Custom_Array[cusRowEntries], '\0', SD_CARD_LINE_LEN); // null terminate row left over at the end
                //for (int i = 0; i<=cusRowEntries-1; i++) { VL(Copy_Custom_Array[i]); }
            }

            // delete old SD file
            File rmFile = SD.open("/custom.csv");
                if (rmFile) {
                    SD.remove("/custom.csv");
                }
            rmFile.close(); 

            // write new array into SD File
            File cWrFile;
            if ((cWrFile = SD.open("custom.csv", FILE_WRITE)) == 0) {
                disp.canvPrint(STATUS_STR_X, STATUS_STR_Y, 0, STATUS_STR_W, STATUS_STR_H, TEXT_COLOR, BUT_BACKGROUND, "SD open ERROR");
                return;
            } else {
                if (cWrFile) {
                    for(uint16_t i=0; i<=cusRowEntries-1; i++) {
                        cWrFile.print(Copy_Custom_Array[i]);
                        delay(5);
                    }
                    //VF("cWrsize="); VL(cWrFile.size());
                    cWrFile.close();
                }
            }
            currentPage = MORE_PAGE; // tell the RETURN button where to return to
            drawCatalogPage(cat_mgr.numCatalogs()+2);
            return;
        }

        // =============== SHC Catalogs ================
        if (catButDetected && shcCatalog) { 

            disp.drawButton(CAT_X, CAT_Y+pre_shcIndex*(CAT_H+CAT_Y_SPACING), 
                    CAT_W, CAT_H, BUT_OUTLINE, BUT_BACKGROUND, CAT_TEXT_X_OFF, CAT_TEXT_Y_OFF, shcObjName[pre_shcIndex]);  
            
            disp.drawButton(CAT_X, CAT_Y+catButSelPos*(CAT_H+CAT_Y_SPACING), 
                    CAT_W, CAT_H, BUT_OUTLINE, BUT_ON_BGD, CAT_TEXT_X_OFF, CAT_TEXT_Y_OFF, shcObjName[catButSelPos]);
            
            // the following 5 lines are displayed on the Catalog/More page
            snprintf(catSelectionStr1, 26, "Name-:%-19s",  shcObjName[catButSelPos]);    //VF("shcObjName="); VL(shcObjName[catButSelPos]);
            snprintf(catSelectionStr2, 11, "Mag--:%-4.1f", shcMag[catButSelPos]);        //VF("shcMag=");     VL(shcMag[catButSelPos]);
            snprintf(catSelectionStr3, 11, "Const:%-4s",   shcCons[catButSelPos]);       //VF("shcCons=");    VL(shcCons[catButSelPos]);
            snprintf(catSelectionStr4, 16, "Type-:%-9s",   objTypeStr[catButDetected]);  //VF("objType=");    VL(objTypeStr[catButSelPos]);
            snprintf(catSelectionStr5, 15, "Id---:%-7s",   shcSubId[catButSelPos]);      //VF("shcSubId=");   VL(shcSubId[catButSelPos]);

            if (shcAlt[catButSelPos] > 10.0) {      // minimum 10 degrees altitude
                disp.canvPrint(STATUS_STR_X, STATUS_STR_Y, 0, STATUS_STR_W, STATUS_STR_H, TEXT_COLOR, BUT_BACKGROUND, "Above +10 deg");
            } else {
                disp.canvPrint(STATUS_STR_X, STATUS_STR_Y, 0, STATUS_STR_W, STATUS_STR_H, TEXT_COLOR, BUT_BACKGROUND, "Below +10 deg");
            }

            writeTarget(catButSelPos); // write RA and DEC as target for GoTo
            showTargetCoords(); // display the target coordinates that were just written            
            pre_shcIndex = catButSelPos;
            curSelSIndex = catButSelPos;
            catButDetected = false;
        }
        tft.setFont(&Inconsolata_Bold8pt7b);

        // ======  now check buttons not part of catalog listings  ========
        // ============== Save to Custom Catalog Button ===================
        if (saveTouched) {
            if (customCatalog) {
                disp.canvPrint(STATUS_STR_X, STATUS_STR_Y, 0, STATUS_STR_W, STATUS_STR_H, TEXT_COLOR, BUT_BACKGROUND, "Can't Save Custom");
                return;
            }
            disp.drawButton(SAVE_LIB_X, SAVE_LIB_Y, SAVE_LIB_W, SAVE_LIB_H, BUT_OUTLINE, BUT_ON_BGD, SAVE_LIB_T_X_OFF, SAVE_LIB_T_Y_OFF, " Saving  ");
        
            if (treasureCatalog) { 
                // Custom User Catalog Format: SubID or ObjName, Mag, Cons, ObjType, ObjName or SubID, RA, DEC
                snprintf(treaCustWrSD, 81, "%-19s;%-4s;%-4s;%-9s;%-18s;%8s;%9s\n", 
                                                                _tArray[curSelTIndex].tSubId,
                                                                _tArray[curSelTIndex].tMag, 
                                                                _tArray[curSelTIndex].tCons, 
                                                                _tArray[curSelTIndex].tObjType,
                                                                _tArray[curSelTIndex].tObjName, 
                                                                tRAhhmmss[curSelTIndex],
                                                                tDECsddmmss[curSelTIndex]);
                // write string to the SD card
                File wrFile = SD.open("custom.csv", FILE_WRITE);
                if (wrFile) {
                    wrFile.print(treaCustWrSD);
                    }
                    //VF("twrFileSize="); VL(wrFile.size());
                wrFile.close();

            } else if (shcCatalog) { // ======= handle any of the SHC catalogs button presses =======
                // "UNK",  "OC",  "GC",  "PN",  "DN",  "SG",  "EG",  "IG", "KNT", "SNR", "GAL",  "CN", "STR", "PLA", "CMT", "AST"
                if (strstr(objTypeStr[curSelSIndex],L_CAT_UNK)) strcpy(truncObjType,"UNK"); else
                if (strstr(objTypeStr[curSelSIndex],L_CAT_OC )) strcpy(truncObjType,"OC" ); else
                if (strstr(objTypeStr[curSelSIndex],L_CAT_GC )) strcpy(truncObjType,"GC" ); else
                if (strstr(objTypeStr[curSelSIndex],L_CAT_PN )) strcpy(truncObjType,"PN" ); else
                if (strstr(objTypeStr[curSelSIndex],L_CAT_SG )) strcpy(truncObjType,"SG" ); else
                if (strstr(objTypeStr[curSelSIndex],L_CAT_EG )) strcpy(truncObjType,"EG" ); else
                if (strstr(objTypeStr[curSelSIndex],L_CAT_IG )) strcpy(truncObjType,"IG" ); else
                if (strstr(objTypeStr[curSelSIndex],L_CAT_KNT)) strcpy(truncObjType,"KNT"); else
                if (strstr(objTypeStr[curSelSIndex],L_CAT_SNR)) strcpy(truncObjType,"SNR"); else
                if (strstr(objTypeStr[curSelSIndex],L_CAT_GAL)) strcpy(truncObjType,"GAL"); else
                if (strstr(objTypeStr[curSelSIndex],L_CAT_CN )) strcpy(truncObjType,"CN" ); else
                if (strstr(objTypeStr[curSelSIndex],L_CAT_STR)) strcpy(truncObjType,"STR"); else
                if (strstr(objTypeStr[curSelSIndex],L_CAT_PLA)) strcpy(truncObjType,"PLA"); else
                if (strstr(objTypeStr[curSelSIndex],L_CAT_CMT)) strcpy(truncObjType,"CMT"); else
                if (strstr(objTypeStr[curSelSIndex],L_CAT_AST)) strcpy(truncObjType,"AST");
        
                // ObjName, shcMag, Cons, ObjType, shcSubId, RA, DEC
                snprintf(shcCustWrSD, 81, "%-19s;%4.1f;%-4s;%-9s;%-18s;%8s;%9s\n", 
                                                                shcObjName[curSelSIndex], 
                                                                shcMag[curSelSIndex],
                                                                shcCons[curSelSIndex], 
                                                                truncObjType,
                                                                shcSubId[curSelSIndex],
                                                                shcRACustLine[curSelSIndex],
                                                                shcDECCustLine[curSelSIndex]);                                 
                // write string to the SD card
                File wrFile = SD.open("custom.csv", FILE_WRITE);
                if (wrFile) {
                    wrFile.print(shcCustWrSD);
                    }
                    //VF("size="); VL(wrFile.size());
                wrFile.close();  
            }
            saveTouched = false;
        } else { // save button not touched
            disp.drawButton(SAVE_LIB_X, SAVE_LIB_Y, SAVE_LIB_W, SAVE_LIB_H, BUT_OUTLINE, BUT_BACKGROUND, SAVE_LIB_T_X_OFF, SAVE_LIB_T_Y_OFF, "SaveToCat");
        }
        tft.setFont();
    }
    screenTouched = false; // passed back to the touchscreen handler
}

//=====================================================
// **** Handle any buttons that have been selected ****
//=====================================================
void touchCatalogUpdate() {
    // check the Catalog Buttons
    uint16_t i=0;
    if (shcCatalog || treasureCatalog) {
        for (i=0; i<(NUM_CAT_ROWS_PER_SCREEN); i++) {
            if (p.y > CAT_Y+(i*(CAT_H+CAT_Y_SPACING)) && p.y < (CAT_Y+(i*(CAT_H+CAT_Y_SPACING))) + CAT_H 
                    && p.x > CAT_X && p.x < (CAT_X+CAT_W)) {
                if (treasureCatalog && tAbsRow == MAX_TREASURE_ROWS+1) return; 
                if (shcCatalog && shcLastPage && i >= shcRow)  return; 
                soundClick();
                catButSelPos = i;
                catButDetected = true;
                return;
            }
        }
    }

    if (customCatalog) { 
        for (i=0; i<=cusRowEntries || i<NUM_CUS_ROWS_PER_SCREEN; i++) {
            if (cAbsRow == cusRowEntries+2) return;
            if (p.y > CUS_Y+(i*(CUS_H+CUS_Y_SPACING)) && p.y < (CUS_Y+(i*(CUS_H+CUS_Y_SPACING))) + CUS_H 
                    && p.x > CUS_X && p.x < (CUS_X+CUS_W)) {
                if (customCatalog && cLastPage==0 && i >= cRow) return; // take care of only one entry on the page
                soundClick();
                catButSelPos = i;
                catButDetected = true;
                return;
            }
        }
    }

    // BACK button
    if (p.y > BACK_Y && p.y < (BACK_Y + BACK_H) && p.x > BACK_X && p.x < (BACK_X + BACK_W)) {
        soundClick();
        if (treasureCatalog && tCurrentPage > 0) {
            tPrevPage = tCurrentPage;
            tEndOfList = false;
            tCurrentPage--;
            drawACatPage();
        }
        if (customCatalog && cCurrentPage > 0) {
            cPrevPage = cCurrentPage;
            cEndOfList = false;
            cCurrentPage--;
            drawACatPage();
        }
        if (shcCatalog && shcCurrentPage > 0) {
            shcEndOfList = false;
            shcCurrentPage--;
            drawACatPage();
        }
    }

    // NEXT page button - reuse BACK button box size
    if (p.y > NEXT_Y && p.y < (NEXT_Y + BACK_H) && p.x > NEXT_X && p.x < (NEXT_X + BACK_W)) {
        soundClick();
        if (treasureCatalog && !tEndOfList) {
            tPrevPage = tCurrentPage;
            tCurrentPage++;
            drawACatPage();
        }
        if (customCatalog && !cEndOfList) {
            cPrevPage = cCurrentPage;
            cCurrentPage++;
            drawACatPage();
        }
        if (shcCatalog && !shcEndOfList) { 
            shcCurrentPage++;
            drawACatPage();
        }
    }

    // RETURN page button - reuse BACK button box size
    if (p.y > RETURN_Y && p.y < (RETURN_Y + BACK_H) && p.x > RETURN_X && p.x < (RETURN_X + RETURN_W)) {
        soundClick();
        screenTouched = false;
        objectSelected = objSel; 
        if (returnToPage == ALIGN_PAGE) {
                drawAlignPage();
                return;
        } else if (returnToPage == MORE_PAGE) {
                drawMorePage();
                return;
        }
    }

    // SAVE page to custom library button
    if (p.y > SAVE_LIB_Y && p.y < (SAVE_LIB_Y + SAVE_LIB_H) && p.x > SAVE_LIB_X && p.x < (SAVE_LIB_X + SAVE_LIB_W)) {
        soundClick();
        saveTouched = true;
    }   

    // Delete custom library item that is selected 
    if (p.y > 3 && p.y < 42 && p.x > 282 && p.x < 317) {
        soundClick();
        delSelected = true;
    }   
}

// ======= write Target Coordinates to controller =========
void writeTarget(uint16_t index) {
    //:Sr[HH:MM.T]# or :Sr[HH:MM:SS]# 
    if (shcCatalog) {
        disp.setLocalCmd(shcRaSrCmd[index]);
    } else if (treasureCatalog) { 
        disp.setLocalCmd(tRaSrCmd[index]);
    } else if (customCatalog) {
        disp.setLocalCmd(cRaSrCmd[index]);
    }
        
    //:Sd[sDD*MM]# or :Sd[sDD*MM:SS]#
    if (shcCatalog) {
        disp.setLocalCmd(shcDecSrCmd[index]);
    } else if (treasureCatalog) {
        disp.setLocalCmd(tDecSrCmd[index]);
    } else if (customCatalog) {
        disp.setLocalCmd(cDecSrCmd[index]);
    }
    objSel = true;
}

// Show target coordinates RA/DEC and ALT/AZM
void showTargetCoords() {
    char raTargReply[15];
    char decTargReply[15];
    double _catAzm, _catAlt;
    uint16_t radec_x = 160;
    uint16_t ra_y = 420;
    uint16_t dec_y = 433;
    uint16_t altazm_x = 240;
 
    // Get Target RA: Returns: HH:MM.T# or HH:MM:SS (based on precision setting)
    disp.getLocalCmdTrim(":Gr#", raTargReply);  
    raTargReply[8] = 0; // clear # character
    
    tft.fillRect(radec_x, ra_y, 75, 11, PAGE_BACKGROUND);
    tft.setCursor(radec_x, ra_y); tft.print(" RA:");
    tft.setCursor(radec_x+24, ra_y); tft.print(raTargReply);    
    
    // Get Target DEC: sDD*MM# or sDD*MM:SS# (based on precision setting)
    disp.getLocalCmdTrim(":Gd#", decTargReply); 
    decTargReply[9] = 0; // clear # character

    tft.fillRect(radec_x, dec_y, 75, 11, PAGE_BACKGROUND);
    tft.setCursor(radec_x, dec_y); tft.print("DEC:");
    tft.setCursor(radec_x+24, dec_y); tft.print(decTargReply);

    // === Show ALT and AZM ===
    double DEC_d=0.0, RA_d=0.0;
    shc.hmsToDouble(&RA_d, raTargReply);
    shc.dmsToDouble(&DEC_d, decTargReply, true, true);
    cat_mgr.EquToHor(RA_d*15, DEC_d, &_catAlt, &_catAzm);
 
    tft.fillRect(altazm_x, ra_y, 70, 11, PAGE_BACKGROUND);
    tft.setCursor(altazm_x, ra_y); tft.print("| AZ:");
    tft.setCursor(altazm_x+29, ra_y); tft.print(_catAzm);   

    tft.fillRect(altazm_x, dec_y, 70, 11, PAGE_BACKGROUND);
    tft.setCursor(altazm_x, dec_y); tft.print("| AL:");
    tft.setCursor(altazm_x+29, dec_y); tft.print(_catAlt);
}
