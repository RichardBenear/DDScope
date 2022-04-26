// ==============================================
// =============== Planets Page =================
// ==============================================
// Author: Richard Benear - March 2022
//
// This page uses the Ephemeris by:
// Copyright (c) 2017 by Sebastien MARCHAND (Web:www.marscaper.com, Email:sebastien@marscaper.com)
//
// Also uses some routines from Smart Hand Controller (SHC) 
// Copyright (C) 2018 to 2021 Charles Lemaire, Howard Dutton, and Others
// Author: Charles Lemaire, https://pixelstelescopes.wordpress.com/teenastro/
// Author: Howard Dutton, http://www.stellarjourney.com, hjd1964@gmail.com


// Catalog Selection buttons
#define PLANET_ROWS         8
#define PLANET_X            7
#define PLANET_Y           43
#define PLANET_W           80
#define PLANET_H           31
#define PLANET_Y_SPACING    6
#define PLANET_TEXT_X_OFF  15
#define PLANET_TEXT_Y_OFF  19

// SolarSystemObjectIndex from Ephemeris.hpp
    //Sun        = 0,
    //Mercury    = 1,
    //Venus      = 2,
    //Earth      = 3,
    //Mars       = 4,
    //Jupiter    = 5,
    //Saturn     = 6,
    //Uranus     = 7,
    //Neptune    = 8,
    //EarthsMoon = 9

// Planet name index used here eliminates Sun and Earth that are used in the Ephemeris Solar System Index
char PlanetNames[8][8] = {"Mercury", "Venus", "Mars", "Jupiter", "Saturn", "Uranus", "Neptune", "Moon"};
char planetSelectionStr[9];
bool planetButDetected = true; // highlight the Moon as the first selection
int planetButSelPos = 0;
int planetPrevSel = 0;
int utc = 0;
//SolarSystemObjectIndex ndx=EarthsMoon;

// Initialize the PLANETS page
void drawPlanetsPage() {
    currentPage = PLANETS_PAGE;
    disp.updateColors();
    tft.setTextColor(TEXT_COLOR);
    tft.fillScreen(PAGE_BACKGROUND);
    disp.drawTitle(110, 30, "Planets");
    planetPrevSel = 0;
    planetButSelPos = 2; // Mars on page entry

    // Get the UTC offset from Onstep 
    char utcOffset[4];
    disp.getLocalCmdTrim(":GG#", utcOffset); 
    utcOffset[3] = 0; // clear # character

    // UTC adjustment for Ephemeris code...e.g. :GG# returns 7 for my location..Ephemeris wants -6 
    // probably a result of the difference in how OnStep defines UTC and how Ephermeris defines it
    utc = (atoi(utcOffset) -1) * -1; // adjust Onstep UTC to Ephemeris expected UT
    
    getPlanet(planetButSelPos); // init screen

    for(int row=0; row<PLANET_ROWS; row++) {
        disp.drawButton(PLANET_X, PLANET_Y+row*(PLANET_H+PLANET_Y_SPACING), PLANET_W, PLANET_H, BUT_OUTLINE, BUT_BACKGROUND, PLANET_TEXT_X_OFF, PLANET_TEXT_Y_OFF, PlanetNames[row]);
    }
    disp.drawButton(RETURN_X, RETURN_Y, RETURN_W, BACK_H, BUT_OUTLINE, BUT_BACKGROUND, BACK_T_X_OFF, BACK_T_Y_OFF, "RETURN");
}

// Need to map the index used here for selected planets to the ones used for solar system index in the Ephermeris
uint8_t mapPlanetIndex(uint8_t planetIndex) {
  uint8_t pIndex;
  switch (planetIndex) {
    case 0: return pIndex = 1;
    case 1: return pIndex = 2;
    case 2: return pIndex = 4;
    case 3: return pIndex = 5;
    case 4: return pIndex = 6;
    case 5: return pIndex = 7;
    case 6: return pIndex = 8;
    case 7: return pIndex = 9;
    default: return pIndex = 1;
  }
}

// Copied this function from SmartHandController LX200.cpp
void char2RA(char* txt, unsigned int& hour, unsigned int& minute, unsigned int& second)
{
  char* pEnd;
  hour = (int)strtol(&txt[0], &pEnd, 10);
  minute = (int)strtol(&txt[3], &pEnd, 10);
  second = (int)strtol(&txt[6], &pEnd, 10);
}

// Copied and Modified from SmartHandController LX200.cpp
void GetTime(unsigned int &hour, unsigned int &minute, unsigned int &second, boolean ut)
{
  char out[20];
  if (ut) {
    disp.getLocalCmdTrim(":GX80#", out);
  } else {
    disp.getLocalCmdTrim(":GL#", out);
  }
    char2RA(out, hour, minute, second);
}

// Copied and Modified from SmartHandController LX200.cpp
void GetDate(unsigned int &day, unsigned int &month, unsigned int &year, boolean ut)
{
  char out[20];
  if (ut) {
    disp.getLocalCmdTrim(":GX81#", out);
  } else {
    disp.getLocalCmdTrim(":GC#", out);
  }
  char* pEnd;
  month = strtol(&out[0], &pEnd, 10);
  day = strtol(&out[3], &pEnd, 10);
  year = strtol(&out[6], &pEnd, 10) + 2000L;
}

// Copied and Modified from SmartHandController LX200.cpp
void GetLatitude(int &degree, int &minute, int &second)
{
  char out[20];
  disp.getLocalCmdTrim(":Gt#", out);
  char* pEnd;
  degree = strtol(&out[0], &pEnd, 10);
  minute = strtol(&out[4], &pEnd, 10);
  second = 0;
}

// Copied and modified from SmartHandController LX200.cpp
void GetLongitude(int &degree, int &minute, int &second)
{
  char out[20];
  disp.getLocalCmdTrim(":Gg#", out);
  char* pEnd;
  degree = strtol(&out[0], &pEnd, 10);
  minute = strtol(&out[5], &pEnd, 10);
  second = 0;
}

// copied this function from ephemeris_full.ino Example
void equatorialCoordinatesToString(EquatorialCoordinates coord, char raCoord[14] , char decCoord[14])
{
  int raHour,raMinute;
  float raSecond;
  Ephemeris::floatingHoursToHoursMinutesSeconds(coord.ra, &raHour, &raMinute, &raSecond);
    
  sprintf(raCoord," %02dh%02dm%02ds.%02d",raHour,raMinute,(int)raSecond,(int)round(((float)(raSecond-(int)raSecond)*pow(10,2))));
    
  int decDegree,decMinute;
  float decSecond;
  Ephemeris::floatingDegreesToDegreesMinutesSeconds(coord.dec, &decDegree, &decMinute, &decSecond);
    
  if(decDegree<0)
  {
    sprintf(decCoord,"%02dd%02d'%02d\".%02d",(int)decDegree,decMinute,(int)decSecond,(int)round(((float)(decSecond-(int)decSecond)*pow(10,2))));
  }
  else
  {
    sprintf(decCoord," %02dd%02d'%02d\".%02d",(int)decDegree,decMinute,(int)decSecond,(int)round(((float)(decSecond-(int)decSecond)*pow(10,2))));
  }
}

// Get Ephemeris Information for selected Planet
// Leveraged/copied from SmartHandController LX200.cpp and ephemeris_full.ino Example
void getPlanet(unsigned short planetNum) { 
    Ephemeris Eph;

    unsigned int dayP, monthP, yearP; 
    unsigned int hourP, minuteP, secondP;
    int longD, longM, longS;
    int latD, latM, latS;
    GetDate(dayP, monthP, yearP, true); // Required: UT = true
    GetTime(hourP, minuteP, secondP, true); // Required: UT = true
    GetLongitude(longD, longM, longS);
    GetLatitude(latD, latM, latS);
    
    // Set Latitude and Longitude
    Eph.setLocationOnEarth(latD,latM,latS,longD,longM,longS);
    Eph.flipLongitude(true);   // true = positive = West; East is negative
    Eph.setAltitude(altitudeFt*0.3048);                 

    uint8_t planetIndex = mapPlanetIndex(planetNum); // planet indexes don't match so map them
    SolarSystemObjectIndex objI = static_cast<SolarSystemObjectIndex>(planetIndex);
    SolarSystemObject obj = Eph.solarSystemObjectAtDateAndTime(objI, dayP, monthP, yearP, hourP, minuteP, secondP);
  
    float Ra = obj.equaCoordinates.ra; //float 
    float Dec = obj.equaCoordinates.dec;  //float
    int ivr1, ivr2, ivd1, ivd2;
    float fvr3, fvd3;
    char sign='+';
    char raCoord[14];
    char decCoord[14];

  /*// equToHor is from Onstep code...used here to check and debug the Ephemeris horz conversion
    // A key learning from this exercise is that the "Hour" passed to Ephemeris must have UTC offset for your location
    double eAlt, eAzm;
    equToHor(haRange(LST()*15.0-Ra*15.0), Dec, &eAlt, &eAzm);
    VF("obj.azm="); VL(obj.horiCoordinates.azi);
    VF("obj.alt="); VL(obj.horiCoordinates.alt);
  */

    Eph.floatingHoursToHoursMinutesSeconds(Ra, &ivr1, &ivr2, &fvr3); 
    Eph.floatingDegreesToDegreesMinutesSeconds(Dec, &ivd1, &ivd2, &fvd3);
    equatorialCoordinatesToString(obj.equaCoordinates, raCoord, decCoord);

    // Print date, time, latitude, longitude
    int x = 5; int y=365; int y_off=0; int y_spc=12; int w = 180; int h=17;
    char d[14], t[14], la[14], lg[14];

    tft.fillRect(x, y-y_spc, w, h, BUT_BACKGROUND);
    tft.setCursor(x, y);
    sprintf(d, "Date-----: %02d/%02d/%4d", monthP, dayP, yearP);
    tft.print(d);

    tft.fillRect(x, y+=y_spc-y_off, w, h, BUT_BACKGROUND);
    tft.setCursor(x, y+=y_spc);
    sprintf(t, "UTC Time-: %02d:%02d:%02d", hourP, minuteP, secondP);
    tft.print(t);

    tft.fillRect(x, y+=y_spc-y_off, w, h, BUT_BACKGROUND);
    tft.setCursor(x, y+=y_spc);
    sprintf(la, "Latitude-:  %02d:%02d:%02d", latD, latM, latS);
    tft.print(la);

    tft.fillRect(x, y+=y_spc-y_off, w, h, BUT_BACKGROUND);
    tft.setCursor(x, y+=y_spc);
    sprintf(lg, "Longitude: %+3d:%2d:%2d", longD, longM, longS);
    tft.print(lg);

    // Print the Selected Planet's coordinates and other data
    int x1 = 95; int y1=70; int y1_off=0; int y1_spc=13; int w1=220; int h1=18;
    tft.fillRect(x1,  y1-y_spc, w1, h1, BUT_BACKGROUND);
    tft.setCursor(x1, y1);
    tft.print("Name : ");
    tft.println(PlanetNames[planetButSelPos]);

    // Print the RA/DEC and AZ/ALT
    tft.fillRect(x1,  y1+=y1_spc-y1_off, w1, h1, BUT_BACKGROUND);
    tft.setCursor(x1, y1+=y1_spc);
    tft.print("R.A. : "); tft.print(Ra); tft.print("|");
    tft.println(raCoord);
    
    tft.fillRect(x1,  y1+=y1_spc-y1_off, w1, h1, BUT_BACKGROUND);
    tft.setCursor(x1, y1+=y1_spc);
    tft.print("Dec  : "); tft.print(Dec); tft.print("|");
    tft.println(decCoord);

    tft.fillRect(x1,  y1+=y1_spc-y1_off, w1, h1, BUT_BACKGROUND);
    tft.setCursor(x1, y1+=y1_spc);
    tft.print("Azm  : ");
    tft.print(obj.horiCoordinates.azi,2);
    tft.println(" deg");

    tft.fillRect(x1,  y1+=y1_spc-y1_off, w1, h1, BUT_BACKGROUND);
    tft.setCursor(x1, y1+=y1_spc);
    tft.print("Alt  : ");
    tft.print(obj.horiCoordinates.alt,2);
    tft.println(" deg");

    tft.fillRect(x1,  y1+=y1_spc-y1_off, w1, h1, BUT_BACKGROUND);
    tft.setCursor(x1, y1+=y1_spc);
    tft.print("Dist : ");
    tft.print(obj.distance);
    tft.println(" AU");

    int hr,mi;
    float sec;
    char strg[14];
    Eph.floatingHoursToHoursMinutesSeconds(Eph.floatingHoursWithUTCOffset(obj.rise, utc), &hr, &mi, &sec); 
    tft.fillRect(x1,  y1+=y1_spc-y1_off, w1, h1, BUT_BACKGROUND);
    tft.setCursor(x1, y1+=y1_spc);
    sprintf(strg, "Rise : %02dh %02dm %2.1fs", hr, mi, sec);
    tft.print(strg);
    
    Eph.floatingHoursToHoursMinutesSeconds(Eph.floatingHoursWithUTCOffset(obj.set, utc), &hr, &mi, &sec);
    tft.fillRect(x1,  y1+=y1_spc-y1_off, w1, h1, BUT_BACKGROUND);
    tft.setCursor(x1, y1+=y1_spc);
    sprintf(strg, "Set  : %02dh %02dm %2.1fs", hr, mi, sec);
    tft.print(strg);

    // Write the coordinates as a target to Onstep
    // Make sure that degrees does not have a sign, if we are south of the celestial equator
    ivd1 = abs(ivd1);
    if (Dec<0.0) sign='-';
    char cmd[20];
    
    char raPrint[9], decPrint[10];
    sprintf(raPrint, "%02d:%02d:%02d", ivr1, ivr2, (int)fvr3);
    sprintf(cmd, ":Sr%02d:%02d:%02d#", ivr1, ivr2, (int)fvr3);
    disp.setLocalCmd(cmd);

    sprintf(decPrint, "%c%02d:%02d:%02d", sign, ivd1, ivd2, (int)fvd3);  
    sprintf(cmd, ":Sd%c%02d:%02d:%02d#", sign, ivd1, ivd2, (int)fvd3);
    disp.setLocalCmd(cmd);
    
    // the following 5 lines are displayed on the Catalog/More page
    snprintf(catSelectionStr1, 16, "Name-:%-16s", PlanetNames[planetButSelPos]);
    snprintf(catSelectionStr2, 12, "AZM--:%-12f", obj.horiCoordinates.azi);
    snprintf(catSelectionStr3, 12, "ALT--:%-12f", obj.horiCoordinates.alt);
    snprintf(catSelectionStr4, 16, "RA---:%-16s", raPrint);
    snprintf(catSelectionStr5, 16, "DEC--:%-16s", decPrint);

    objectSelected = true;
}

// ******************************************************
// Update for buttons 
// ******************************************************
void updatePlanetsStatus() {
    if (screenTouched || firstDraw || refreshScreen) { 
      refreshScreen = false;
      if (screenTouched) refreshScreen = true;
        //VF("planetButSelPos="); VL(planetButSelPos);
        //VF("planetName=");  VL(PlanetNames[planetButSelPos]);
        // Detect which Planet selected
        if (planetButDetected) {
           
            // ERASE old: set background back to unselected and replace the previous name field
            disp.drawButton(PLANET_X, PLANET_Y+planetPrevSel*(PLANET_H+PLANET_Y_SPACING), 
                    PLANET_W, PLANET_H, BUT_OUTLINE, BUT_BACKGROUND, PLANET_TEXT_X_OFF, PLANET_TEXT_Y_OFF, PlanetNames[planetPrevSel]);  
            
            // DRAW new: highlight by settting background ON color for button selected
            disp.drawButton(PLANET_X, PLANET_Y+planetButSelPos*(PLANET_H+PLANET_Y_SPACING), 
                    PLANET_W, PLANET_H, BUT_OUTLINE, BUT_ON_BGD, PLANET_TEXT_X_OFF, PLANET_TEXT_Y_OFF, PlanetNames[planetButSelPos]);
            
            getPlanet(planetButSelPos);
        
            planetButDetected = false;
            planetPrevSel = planetButSelPos;
        }
    screenTouched = false; // passed back to the touchscreen handler
    }
}

// *****************************************************
// **** Handle any buttons that have been pressed ****
// *****************************************************
void touchPlanetsUpdate() {
    // check the Planet Buttons
    for (int row=0; row<PLANET_ROWS; row++) {
        if (p.y > PLANET_Y+(row*(PLANET_H+PLANET_Y_SPACING)) && p.y < (PLANET_Y+(row*(PLANET_H+PLANET_Y_SPACING))) + PLANET_H 
                && p.x > PLANET_X && p.x < (PLANET_X+PLANET_W)) {
            soundClick();
            planetButSelPos = row;
            mapPlanetIndex(row);
            planetButDetected = true;
            return;
        }
    }

    // RETURN page button - reuse BACK button box size
    if (p.y > RETURN_Y && p.y < (RETURN_Y + BACK_H) && p.x > RETURN_X && p.x < (RETURN_X + RETURN_W)) {
        soundClick();
        screenTouched = false;
        drawMorePage();
        return;
    }
}