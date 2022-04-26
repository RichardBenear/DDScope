// -----------------------------------------------------------------------------------------------------------------------------
// Sound related functions

// sound/buzzer
void soundAlert() {
  if (soundEnabled) {
    #if BUZZER == ON
      digitalWrite(TonePin,HIGH); buzzerDuration=100;
    #endif
    #if BUZZER >= 0
      tone(TonePin,BUZZER,100);
    #endif
  }
}

// sound/beep
void soundBeep() {
  if (soundEnabled) {
    #if BUZZER == ON
      digitalWrite(TonePin,HIGH); buzzerDuration=30;
    #endif
    #if BUZZER >= 0
      tone(TonePin,BUZZER,30);
    #endif
  }
}

// sound/click
void soundClick() {
  if (soundEnabled) {
    #if BUZZER == ON
      digitalWrite(TonePin,HIGH); buzzerDuration=19;
    #endif
    #if BUZZER >= 0
      tone(TonePin,BUZZER,8);
    #endif
  }
}

// sound frequency
void soundFreq(int freq) {
  #if BUZZER >= 0
    tone(TonePin,freq,100);
  #endif
}
