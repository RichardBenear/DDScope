// ---------------------------------------------------------------------------------------------------
// Guide, commands to move the mount in any direction at a series of fixed rates

#if ST4_INTERFACE == ON || ST4_INTERFACE == ON_PULLUP
#include "OnStep/PushButton.h"

#if ST4_HAND_CONTROL == ON
  #define debounceMs 100
#else
  #define debounceMs 5
#endif

button st4n;
button st4s;
button st4e;
button st4w;

// Single byte guide commands
#define ccMe 14
#define ccMw 15
#define ccMn 16
#define ccMs 17
#define ccQe 18
#define ccQw 19
#define ccQn 20
#define ccQs 21

#endif

unsigned long guideStartTime             = 0;
long          guideTimeRemainingAxis1    = -1;
unsigned long guideTimeThisIntervalAxis1 = -1;
long          guideTimeRemainingAxis2    = -1;
unsigned long guideTimeThisIntervalAxis2 = -1;
double        guideTimerCustomRateAxis1  = 0.0;
double        guideTimerCustomRateAxis2  = 0.0;

// initialize guiding
void initGuide() {
  guideDirAxis1              =  0;
  guideTimeRemainingAxis1    = -1;
  guideTimeThisIntervalAxis1 = -1;
  guideDirAxis2              =  0;
  guideTimeRemainingAxis2    = -1;
  guideTimeThisIntervalAxis2 = -1;

#if ST4_INTERFACE == ON || ST4_INTERFACE == ON_PULLUP && PINMAP != DDT
  #if ST4_INTERFACE == ON
    bool pullup=false;
  #else
    bool pullup=true;
  #endif
  st4n.init(ST4DEn,debounceMs,pullup); // active low is default (true)
  st4s.init(ST4DEs,debounceMs,pullup);
  st4e.init(ST4RAe,debounceMs,pullup);
  st4w.init(ST4RAw,debounceMs,pullup);
#endif
}

void guide() {
  // 1/100 second sidereal timer, controls issue of steps at the selected RA and/or Dec rate(s) 
  guideAxis1.fixed=0;
  cli(); long guideLst=lst; sei();
  if (guideLst != guideSiderealTimer) {
    guideSiderealTimer=guideLst;  
    if (guideDirAxis1) {
      if (!inbacklashAxis1) {
        // guideAxis1 keeps track of how many steps we've moved for PEC recording
        if (guideDirAxis1 == 'e') guideAxis1.fixed=-amountGuideAxis1.fixed; else if (guideDirAxis1 == 'w') guideAxis1.fixed=amountGuideAxis1.fixed;

        // for pulse guiding, count down the mS and stop when timed out
        if (guideTimeRemainingAxis1 > 0) {
          guideTimeRemainingAxis1-=(long)(micros()-guideTimeThisIntervalAxis1);
          guideTimeThisIntervalAxis1=micros();
          if (guideTimeRemainingAxis1 <= 0) { guideDirAxis1='b'; } // break
        }
      } else {
        // don't count time if in backlash
        guideTimeThisIntervalAxis1=micros();
      }
    }
    
    if (guideDirAxis2) {
      if (!inbacklashAxis2) {
        // for pulse guiding, count down the mS and stop when timed out
        if (guideTimeRemainingAxis2 > 0) {
          guideTimeRemainingAxis2-=(long)(micros()-guideTimeThisIntervalAxis2);
          guideTimeThisIntervalAxis2=micros();
          if (guideTimeRemainingAxis2 <= 0) { guideDirAxis2='b'; }  // break 
        }
      } else {
        // don't count time if in backlash
        guideTimeThisIntervalAxis2=micros();
      }
    }

    // set rates for spiral guide
    if (spiralGuide) {
      if ((guideDirAxis1 == 'e' || guideDirAxis1 == 'w') && (guideDirAxis2 == 'n' || guideDirAxis2 == 's')) guideSpiralPoll(); else stopGuideSpiral();
    }
  }
}

// returns true if a spiral guide is happening
bool lastGuideSpiralGuide = false;
bool isSpiralGuiding() {
  if ((guideDirAxis1 || guideDirAxis2) && lastGuideSpiralGuide) return true; else { 
    if (lastGuideSpiralGuide) { lastGuideSpiralGuide=false; guideTimerCustomRateAxis1=0.0; guideTimerCustomRateAxis2=0.0; }
    return false;
  }
}

// returns true if a pulse guide is happening
bool lastGuidePulseGuideAxis1 = false;
bool lastGuidePulseGuideAxis2 = false;
bool isPulseGuiding() {
  if ((guideDirAxis1 && lastGuidePulseGuideAxis1) || (guideDirAxis2 && lastGuidePulseGuideAxis2)) return true; else return false;
}

// returns true if rapid movement is happening
bool isSlewing() {
  return guideDirAxis1 || guideDirAxis2 || trackingState == TrackingMoveTo || trackingSyncInProgress();
}

// reactivate or deactivate backlash comp. if necessary
int backlashAxis1PriorToGuide=0;
int backlashAxis2PriorToGuide=0;

void reactivateBacklashComp() {
#if GUIDE_DISABLE_BACKLASH == ON
  if (backlashAxis1PriorToGuide > 0) { cli(); backlashAxis1=backlashAxis1PriorToGuide; sei(); backlashAxis1PriorToGuide=0; }
  if (backlashAxis2PriorToGuide > 0) { cli(); backlashAxis2=backlashAxis2PriorToGuide; sei(); backlashAxis2PriorToGuide=0; }
#endif
}

void deactivateBacklashComp() {
#if GUIDE_DISABLE_BACKLASH == ON
  if (backlashAxis1PriorToGuide == 0) { backlashAxis1PriorToGuide=backlashAxis1; cli(); backlashAxis1=0; sei(); }
  if (backlashAxis2PriorToGuide == 0) { backlashAxis2PriorToGuide=backlashAxis2; cli(); backlashAxis2=0; sei(); }
#endif
}

// start a guide in RA or Azm, direction must be 'e', 'w', or 'b', guideRate is the rate selection (0 to 9), guideDuration is in ms (0 to ignore) 
CommandErrors startGuideAxis1(char direction, int guideRate, long guideDuration, bool pulseGuide) {
  // Check state
  if (faultAxis1)                         return CE_SLEW_ERR_HARDWARE_FAULT;
  if (!axis1Enabled)                      return CE_SLEW_ERR_IN_STANDBY;
  if (parkStatus == Parked)               return CE_SLEW_ERR_IN_PARK;
  if (trackingSyncInProgress())           return CE_MOUNT_IN_MOTION;
  if (trackingState == TrackingMoveTo)    return CE_MOUNT_IN_MOTION;
  if (isSpiralGuiding())                  return CE_MOUNT_IN_MOTION;
  if (direction == guideDirAxis1)         return CE_NONE;
  if (direction == 'e' && !guideEastOk()) return CE_SLEW_ERR_OUTSIDE_LIMITS;
  if (direction == 'w' && !guideWestOk()) return CE_SLEW_ERR_OUTSIDE_LIMITS;
  if (guideRate < 3 && (generalError == ERR_ALT_MIN ||
                        generalError == ERR_LIMIT_SENSE ||
                        generalError == ERR_DEC ||
                        generalError == ERR_AZM ||
                        generalError == ERR_UNDER_POLE ||
                        generalError == ERR_MERIDIAN ||
                        generalError == ERR_ALT_MAX)) return CE_SLEW_ERR_OUTSIDE_LIMITS;
  
  if (guideRate < 3) deactivateBacklashComp(); else reactivateBacklashComp();
  enableGuideRate(guideRate);
  guideDirAxis1=direction;
  guideTimeThisIntervalAxis1=micros();
  guideTimeRemainingAxis1=guideDuration*1000L;
  cli();
  if (guideDirAxis1 == 'e') guideTimerRateAxis1=-guideTimerBaseRateAxis1; else guideTimerRateAxis1=guideTimerBaseRateAxis1; 
  sei();
  lastGuidePulseGuideAxis1 = pulseGuide;
  
  return CE_NONE;
}

// stops guide in RA or Azm
void stopGuideAxis1() {
  cli(); if (guideDirAxis1 && guideDirAxis1 != 'b') { guideDirAxis1='b'; } sei();
}

// start a guide in Dec or Alt, direction must be 'n', 's', or 'b', guideRate is the rate selection (0 to 9), guideDuration is in ms (0 to ignore) 
CommandErrors startGuideAxis2(char direction, int guideRate, long guideDuration, bool pulseGuide, bool absolute) {
  if (faultAxis2)                          return CE_SLEW_ERR_HARDWARE_FAULT;
  if (!axis1Enabled)                       return CE_SLEW_ERR_IN_STANDBY;
  if (parkStatus == Parked)                return CE_SLEW_ERR_IN_PARK;
  if (trackingSyncInProgress())            return CE_MOUNT_IN_MOTION;
  if (trackingState == TrackingMoveTo)     return CE_MOUNT_IN_MOTION;
  if (isSpiralGuiding())                   return CE_MOUNT_IN_MOTION;
  if (direction == guideDirAxis2)          return CE_NONE;
  if (direction == 'n' && !guideNorthOk()) return CE_SLEW_ERR_OUTSIDE_LIMITS;
  if (direction == 's' && !guideSouthOk()) return CE_SLEW_ERR_OUTSIDE_LIMITS;
  if (guideRate < 3 && (generalError == ERR_ALT_MIN ||
                        generalError == ERR_LIMIT_SENSE ||
                        generalError == ERR_DEC ||
                        generalError == ERR_AZM ||
                        generalError == ERR_UNDER_POLE ||
                        generalError == ERR_MERIDIAN ||
                        generalError == ERR_ALT_MAX)) return CE_SLEW_ERR_OUTSIDE_LIMITS;

  enableGuideRate(guideRate);
  if (guideRate < 3) deactivateBacklashComp(); else reactivateBacklashComp();
  guideDirAxis2=direction;
  guideTimeThisIntervalAxis2=micros();
  guideTimeRemainingAxis2=guideDuration*1000L;
  if (guideDirAxis2 == 's') { cli(); guideTimerRateAxis2=-guideTimerBaseRateAxis2; sei(); } 
  if (guideDirAxis2 == 'n') { cli(); guideTimerRateAxis2= guideTimerBaseRateAxis2; sei(); }
  if (!absolute && (getInstrPierSide() == PierSideWest)) { cli(); guideTimerRateAxis2=-guideTimerRateAxis2; sei(); }
  lastGuidePulseGuideAxis2 = pulseGuide;
  
  return CE_NONE;
}

bool guideNorthOk() {
  if (!safetyLimitsOn) return true;
  double a2; if (AXIS2_TANGENT_ARM == ON) { cli(); a2=posAxis2/axis2Settings.stepsPerMeasure; sei(); } else a2=getInstrAxis2();
  if (a2 < axis2Settings.min && getInstrPierSide() == PierSideWest) return false;
  if (a2 > axis2Settings.max && getInstrPierSide() == PierSideEast) return false;
  if (MOUNT_TYPE == ALTAZM && currentAlt > maxAlt) return false;
  return true;
}
bool guideSouthOk() {
  if (!safetyLimitsOn) return true;
  double a2; if (AXIS2_TANGENT_ARM == ON) { cli(); a2=posAxis2/axis2Settings.stepsPerMeasure; sei(); } else a2=getInstrAxis2();
  if (a2 < axis2Settings.min && getInstrPierSide() == PierSideEast) return false;
  if (a2 > axis2Settings.max && getInstrPierSide() == PierSideWest) return false;
  if (MOUNT_TYPE == ALTAZM && currentAlt < minAlt) return false;
  return true;
}
bool guideEastOk() {
  if (!safetyLimitsOn) return true;
  if (meridianFlip != MeridianFlipNever && getInstrPierSide() == PierSideEast) { if (getInstrAxis1() < -degreesPastMeridianE) return false; }
  if (getInstrAxis1() < axis1Settings.min) return false;
  return true;
}
bool guideWestOk() {
  if (!safetyLimitsOn) return true;
  if (meridianFlip != MeridianFlipNever && getInstrPierSide() == PierSideWest) { if (getInstrAxis1() > degreesPastMeridianW) return false; }
  if (getInstrAxis1() > axis1Settings.max) return false;
  return true;
}

CommandErrors startGuideAxis2(char direction, int guideRate, long guideDuration, bool pulseGuide) {
  return startGuideAxis2(direction, guideRate, guideDuration, pulseGuide, false);
}

// stops guide in Dec or Alt
void stopGuideAxis2() {
  cli(); if (guideDirAxis2 && guideDirAxis2 != 'b') { guideDirAxis2='b'; } sei();
}

// start a guide spiral, guideRate is the rate selection (0 to 9), guideDuration is in ms (0 to ignore) 
double spiralScaleAxis1=0;
CommandErrors startGuideSpiral(int guideRate, long guideDuration) {
  if (faultAxis1 || faultAxis2)            return CE_SLEW_ERR_HARDWARE_FAULT;
  if (!axis1Enabled)                       return CE_SLEW_ERR_IN_STANDBY;
  if (parkStatus == Parked)                return CE_SLEW_ERR_IN_PARK;
  if (trackingSyncInProgress())            return CE_MOUNT_IN_MOTION;
  if (trackingState == TrackingMoveTo)     return CE_MOUNT_IN_MOTION;
  if (guideDirAxis1 || guideDirAxis2)      { if (spiralGuide) stopGuideSpiral(); return CE_NONE; }
  if (isSpiralGuiding())                   return CE_MOUNT_IN_MOTION;
  if (abs(getInstrAxis2() > 75.0))         return CE_SLEW_ERR_OUTSIDE_LIMITS;
  if (!guideNorthOk() || !guideSouthOk())  return CE_SLEW_ERR_OUTSIDE_LIMITS;
  if ((generalError == ERR_ALT_MIN ||
       generalError == ERR_LIMIT_SENSE ||
       generalError == ERR_DEC ||
       generalError == ERR_AZM ||
       generalError == ERR_UNDER_POLE ||
       generalError == ERR_MERIDIAN ||
       generalError == ERR_ALT_MAX))       return CE_SLEW_ERR_OUTSIDE_LIMITS;

  spiralGuide = guideRate;
  if (spiralGuide < 3) spiralGuide=3;
  if (spiralGuide > 8) spiralGuide=8;

  guideStartTime=millis();
  guideTimeThisIntervalAxis1=micros();
  guideTimeRemainingAxis1=guideDuration*1000L;
  guideTimeThisIntervalAxis2=micros();
  guideTimeRemainingAxis2=guideDuration*1000L;

  spiralScaleAxis1=cos(getInstrAxis2()/Rad);

  lastGuideSpiralGuide=true;
  guideSpiralPoll();

  return CE_NONE;
}

// stop guide spiral
void stopGuideSpiral() {
  spiralGuide=0;
  cli();
  if (guideDirAxis1) guideDirAxis1='b';
  if (guideDirAxis2) guideDirAxis2='b';
  sei();
}

// set guide spiral rates in RA/Azm and Dec/Alt, rate is in x-sidereal, guideElapsed time is in ms 
void guideSpiralPoll() {
  // current elapsed time in seconds
  double T=((long)(millis()-guideStartTime))/1000.0;

  // actual rate we'll be using (in arc-seconds per second)
  double rate=guideRates[spiralGuide];
  if (rate > 1800.0) rate=1800.0;

  // apparaent FOV (in arc-seconds)
  // double fov=rate*2.0;

  // current radius assuming movement at 2 seconds per fov
  double radius=pow(T/6.28318,1.0/1.74);

  // current angle in radians
  double angle =(radius-trunc(radius))*6.28318;

  // calculate the Axis rates for this moment
  guideTimerCustomRateAxis1=(rate/15.0)*cos(angle);
  guideTimerCustomRateAxis2=(rate/15.0)*sin(angle);

  // set direction
  if (guideTimerCustomRateAxis1 < 0) { guideTimerCustomRateAxis1=fabs(guideTimerCustomRateAxis1); guideDirAxis1='e'; } else guideDirAxis1='w';
  if (guideTimerCustomRateAxis2 < 0) { guideTimerCustomRateAxis2=fabs(guideTimerCustomRateAxis2); guideDirAxis2='s'; } else guideDirAxis2='n';

  // adjust Axis1 due to spherical coordinates
  guideTimerCustomRateAxis1/=spiralScaleAxis1;

  // limit Axis1 to speeds we can reach
  double rateXPerSec = RateToXPerSec/(maxRate/16.0);
  if (guideTimerCustomRateAxis1 > rateXPerSec) guideTimerCustomRateAxis1=rateXPerSec;

  // activate the new guide rates
  enableGuideRate(-1);
  if (guideDirAxis1 == 'e') { cli(); guideTimerRateAxis1=-guideTimerBaseRateAxis1; sei(); }
  if (guideDirAxis1 == 'w') { cli(); guideTimerRateAxis1= guideTimerBaseRateAxis1; sei(); }
  if (guideDirAxis2 == 's') { cli(); guideTimerRateAxis2=-guideTimerBaseRateAxis2; sei(); } 
  if (guideDirAxis2 == 'n') { cli(); guideTimerRateAxis2= guideTimerBaseRateAxis2; sei(); }
}

// custom guide rate in RA or Azm, rate is in x-sidereal, guideDuration is in ms (0 to ignore) 
bool customGuideRateAxis1(double rate, long guideDuration) {
  guideTimerCustomRateAxis1=rate;
  enableGuideRate(-1);
  if ((parkStatus == NotParked) && (trackingState != TrackingMoveTo) && (axis1Enabled) && (guideDirAxis1)) {
    guideTimeThisIntervalAxis1=micros();
    guideTimeRemainingAxis1=guideDuration*1000L;
    if (guideDirAxis1 == 'e') { cli(); guideTimerRateAxis1=-guideTimerBaseRateAxis1; sei(); }
    if (guideDirAxis1 == 'w') { cli(); guideTimerRateAxis1= guideTimerBaseRateAxis1; sei(); }
  } else return false;
  return true;
}

// custom guide rate in Dec or Alt, rate is in x-sidereal, guideDuration is in ms (0 to ignore)
bool customGuideRateAxis2(double rate, long guideDuration) {
  guideTimerCustomRateAxis2=rate;
  enableGuideRate(-1);
  if ((parkStatus == NotParked) && (trackingState != TrackingMoveTo) && (axis2Enabled) && (guideDirAxis2)) {
    guideTimeThisIntervalAxis2=micros();
    guideTimeRemainingAxis2=guideDuration*1000L;
    if (guideDirAxis2 == 's') { cli(); guideTimerRateAxis2=-guideTimerBaseRateAxis2; sei(); } 
    if (guideDirAxis2 == 'n') { cli(); guideTimerRateAxis2= guideTimerBaseRateAxis2; sei(); }
    if (getInstrPierSide() == PierSideWest) { cli(); guideTimerRateAxis2=-guideTimerRateAxis2; sei(); }
  } else return false;
  return true;
}

// sets the rates for guide commands
void setGuideRate(int g) {
  currentGuideRate=g;
  if ((g <= GuideRate1x) && (currentPulseGuideRate != g)) { currentPulseGuideRate=g; nv.update(EE_pulseGuideRate,g); }
  guideTimerCustomRateAxis1=0.0;
  guideTimerCustomRateAxis2=0.0;
}

// gets the rate for guide commands
int getGuideRate() {
  return currentGuideRate;
}

// gets the rate for pulse-guide commands
int getPulseGuideRate() {
#if SEPARATE_PULSE_GUIDE_RATE == ON
  return currentPulseGuideRate; 
#else
  return currentGuideRate;
#endif
}

// enables the guide rate
// -1 to use guideTimerCustomRateAxis1/2, otherwise rates are:
// 0=.25X 1=.5x 2=1x 3=2x 4=4x 5=8x 6=24x 7=48x 8=half-MaxRate 9=MaxRate
void enableGuideRate(int g) {
  if (g == activeGuideRate) return;
  
  if (g >= 0) activeGuideRate=g;

  // this enables the guide rates
  if (guideTimerCustomRateAxis1 != 0.0) {
    guideTimerBaseRateAxis1=guideTimerCustomRateAxis1;
  } else {
    guideTimerBaseRateAxis1=(double)(guideRates[g]/15.0);
  }
  if (guideTimerCustomRateAxis2 != 0.0) {
    guideTimerBaseRateAxis2=guideTimerCustomRateAxis2;
  } else {
    guideTimerBaseRateAxis2=(double)(guideRates[g]/15.0);
  }
  amountGuideAxis1.fixed=doubleToFixed((guideTimerBaseRateAxis1*stepsPerSecondAxis1)/100.0);
  amountGuideAxis2.fixed=doubleToFixed((guideTimerBaseRateAxis2*stepsPerSecondAxis2)/100.0);
}

// handle the ST4 interface and hand controller features
void ST4() {
#if ST4_INTERFACE == ON || ST4_INTERFACE == ON_PULLUP
  // get ST4 button presses
  st4e.poll();
  static bool shcActive=false;
  if (!shcActive) {
    st4w.poll();
    st4n.poll();
    st4s.poll();
  }

#if ST4_HAND_CONTROL == ON

  // check for smart hand control
  if (st4e.hasTone()) {
    if (!shcActive) {
      if (st4w.hasTone()) {
        pinMode(ST4DEs,OUTPUT);    // clock
        pinMode(ST4DEn,OUTPUT);    // send data
        digitalWrite(ST4DEs,HIGH); // idle
        shcActive=true;
        SerialST4.begin();
        VLF("MSG: SerialST4 mode activated");
      }
      return;
    } else { 
      char c=SerialST4.poll();

      // process any single byte guide commands
      if (c == ccMe) startGuideAxis1('e',currentGuideRate,GUIDE_TIME_LIMIT*1000,false);
      if (c == ccMw) startGuideAxis1('w',currentGuideRate,GUIDE_TIME_LIMIT*1000,false);
      if (c == ccMn) startGuideAxis2('n',currentGuideRate,GUIDE_TIME_LIMIT*1000,false);
      if (c == ccMs) startGuideAxis2('s',currentGuideRate,GUIDE_TIME_LIMIT*1000,false);
      if ((c == ccQe) || (c == ccQw)) stopGuideAxis1();
      if ((c == ccQn) || (c == ccQs)) stopGuideAxis2();
      
      return;
    }
  } else {
    if (shcActive) {
      #if ST4_INTERFACE == ON
        pinMode(ST4DEs,INPUT);
        pinMode(ST4DEn,INPUT);
      #elif ST4_INTERFACE == ON_PULLUP
        pinMode(ST4DEs,INPUT_PULLUP);
        pinMode(ST4DEn,INPUT_PULLUP);
      #endif
      shcActive=false;
      SerialST4.end();
      VLF("MSG: SerialST4 mode deactivated");
      return;
    }
  }

  // standard hand control
  const long Shed_ms=4000;
  const long AltMode_ms=2000;

  // stop any guide that might be triggered by combination button presses
  if (st4e.isDown() && st4w.isDown()) stopGuideAxis1(); 
  if (st4n.isDown() && st4s.isDown()) stopGuideAxis2();
  
  // see if a combination was down for long enough for an alternate mode
  static bool altModeA=false;
  static bool altModeB=false;
  if ((trackingState != TrackingMoveTo) && (!waitingHome)) {
    if ((st4e.timeDown() > AltMode_ms) && (st4w.timeDown() > AltMode_ms) && (!altModeB)) { if (!altModeA) { altModeA=true; soundBeep(); } }
    if ((st4n.timeDown() > AltMode_ms) && (st4s.timeDown() > AltMode_ms) && (!altModeA)) { if (!altModeB) { altModeB=true; soundBeep(); } }
  }
  
  // if the alternate mode is allowed & selected & hasn't timed out, handle it
  if ( (altModeA || altModeB) && ((st4n.timeUp() < Shed_ms) || (st4s.timeUp() < Shed_ms) || (st4e.timeUp() < Shed_ms) || (st4w.timeUp() < Shed_ms)) ) {

    // make sure no cmdSend() is being processed
    if (!cmdWaiting()) {
      if (altModeA) {
        int c=currentGuideRate;
        if (st4w.wasPressed() && !st4e.wasPressed()) {
          if (trackingState == TrackingNone) cmdSend(":B+#",true); else { if (c >= 7) c=8; else if (c >= 5) c=7; else if (c >= 2) c=5; else if (c < 2) c=2; }
          soundClick();
        }
        if (st4e.wasPressed() && !st4w.wasPressed()) {
          if (trackingState == TrackingNone) cmdSend(":B-#",true); else { if (c <= 5) c=2; else if (c <= 7) c=5; else if (c <= 8) c=7; else if (c > 8) c=8; }
          soundClick();
        }
        if (st4s.wasPressed() && !st4n.wasPressed()) {
          if (alignThisStar > alignNumStars) cmdSend(":CS#",true); else cmdSend(":A+#",true);
          soundClick(); 
        }
        if (st4n.wasPressed() && !st4s.wasPressed()) {
          if (trackingState == TrackingSidereal) { trackingState=TrackingNone; disableStepperDrivers(); soundClick(); } else
          if (trackingState == TrackingNone) { trackingState=TrackingSidereal; enableStepperDrivers(); soundClick(); }
        }
        if (c != currentGuideRate) { setGuideRate(c); enableGuideRate(c); }
      }
      if (altModeB) {
#if ST4_HAND_CONTROL_FOCUSER == ON
        static int fs=0;
        static int fn=0;
        if (!fn && !fs) {
          if (st4w.wasPressed() && !st4e.wasPressed()) { cmdSend(":F2#",true); soundClick(); }
          if (st4e.wasPressed() && !st4w.wasPressed()) { cmdSend(":F1#",true); soundClick(); }
        }
        if (!fn) {
          if (st4s.isDown() && st4n.isUp()) { if (fs == 0) { cmdSend(":FS#",true); fs++; } else if (fs == 1) { cmdSend(":F-#",true); fs++; } else if ((st4s.timeDown() > 4000) && (fs == 2)) { fs++; cmdSend(":FF#",true); } else if (fs == 3) { cmdSend(":F-#",true); fs++; } }
          if (st4s.isUp()) { if (fs > 0) { cmdSend(":FQ#",true); fs=0; } }
        }
        if (!fs) {
          if (st4n.isDown() && st4s.isUp()) { if (fn == 0) { cmdSend(":FS#",true); fn++; } else if (fn == 1) { cmdSend(":F+#",true); fn++; } else if ((st4n.timeDown() > 4000) && (fn == 2)) { fn++; cmdSend(":FF#",true); } else if (fn == 3) { cmdSend(":F+#",true); fn++; }  }
          if (st4n.isUp()) { if (fn > 0) { cmdSend(":FQ#",true); fn=0; } }
        }
#else
        if (st4w.wasPressed() && !st4e.wasPressed()) { cmdSend(":LN#",true); soundClick(); }
        if (st4e.wasPressed() && !st4w.wasPressed()) { cmdSend(":LB#",true); soundClick(); }
        if (st4s.wasPressed() && !st4n.wasPressed()) { soundEnabled=!soundEnabled; soundClick(); }
        if (st4n.wasPressed() && !st4s.wasPressed()) { cmdSend(":LIG#",true); soundClick(); }
#endif
      }
    }
  } else {
    if ((altModeA || altModeB)) { 
#if ST4_HAND_CONTROL_FOCUSER == ON
      cmdSend(":FQ#",true);
#endif
      altModeA=false; altModeB=false; soundBeep();
    }
#endif
    if (axis1Enabled) {

      // guide E/W
      char newDirAxis1='b';
      if (st4w.isDown() && st4e.isUp()) newDirAxis1='w';
      if (st4e.isDown() && st4w.isUp()) newDirAxis1='e';
      
      if (newDirAxis1 != ST4DirAxis1) {
        ST4DirAxis1=newDirAxis1;
        if (newDirAxis1 != 'b') {
#if ST4_HAND_CONTROL == ON
          if (waitingHome) waitingHomeContinue=true; else
          if (trackingState == TrackingMoveTo) { if (!abortGoto) abortGoto=StartAbortGoto; } else
#endif
            {
#if SEPARATE_PULSE_GUIDE_RATE == ON && ST4_HAND_CONTROL != ON
            startGuideAxis1(newDirAxis1,currentPulseGuideRate,GUIDE_TIME_LIMIT*1000,false);
#else
            startGuideAxis1(newDirAxis1,currentGuideRate,GUIDE_TIME_LIMIT*1000,false);
#endif
          }
        } else stopGuideAxis1();
      }

      // guide N/S
      char newDirAxis2='b';
      if (st4n.isDown() && st4s.isUp()) newDirAxis2='n';
      if (st4s.isDown() && st4n.isUp()) newDirAxis2='s';

      if (newDirAxis2 != ST4DirAxis2) {
       
        ST4DirAxis2=newDirAxis2;
        if (newDirAxis2 != 'b') {
#if ST4_HAND_CONTROL == ON
          if (waitingHome) waitingHomeContinue=true; else
          if (trackingState == TrackingMoveTo) { if (!abortGoto) abortGoto=StartAbortGoto; } else
#endif
          {
#if SEPARATE_PULSE_GUIDE_RATE == ON && ST4_HAND_CONTROL != ON
            startGuideAxis2(newDirAxis2,currentPulseGuideRate,GUIDE_TIME_LIMIT*1000,false);
#else
            startGuideAxis2(newDirAxis2,currentGuideRate,GUIDE_TIME_LIMIT*1000,false);
#endif
          }
        } else stopGuideAxis2();
      }
    }
#if ST4_HAND_CONTROL == ON
  }
#endif
#endif
}
