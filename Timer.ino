//--------------------------------------------------------------------------------------------------
// Timer1 high-speed ISR for timing and issuing step pulses to the stepper motor drivers

ISR(TIMER1_COMPA_vect)
{
  // subroutine overhead, about 1uS + about 60 clocks = 5uS... this is a very rough guess, double it for some breathing room and this 
  // routine might run in say 10uS?
  // and we don't want to take more than half the clock cycles from the main loop, so this should never be pushed past about 15-20uS.
  // someday I might look at a disassembly and really see what we have here, but it's fast enough for most steppers as is.
  
  // drivers step on the rising edge, they need 1uS to settle so this is early in the routine to allow time after they're primed for a step
  CLR(HAStepPORT,  HAStepBit);
  CLR(DecStepPORT, DecStepBit);
  
  // keep sidereal clock ticking
  siderealCount++; if (siderealCount>=siderealInterval) { 
    siderealCount=0;
    lst++;                                           // one tick is 0.01 of a sidereal second
  }
  
  // move the stepper motors, max about 4uS
  skipHA++;                                          // Rate control RA
  if ((skipHA>=skipCountHA) || 
      (inBacklashHA && (skipHA>=skipCountBacklashHA))) {                        

    if (posHA<(targetHA+PEC_HA)) dirHA=1; else dirHA=0;                        // Direction control
#ifdef ReverseHA
    if (HADir==dirHA) CLR(HADirPORT, HADirBit); else SET(HADirPORT, HADirBit); // Set direction, HADir default LOW (=0, for my wiring)
#else
    if (HADir==dirHA) SET(HADirPORT, HADirBit); else CLR(HADirPORT, HADirBit); // Set direction, HADir default LOW (=0, for my wiring)
#endif
    skipHA=0;

    if (posHA!=(targetHA+PEC_HA)) {                  // Move the RA stepper to the target

      // telescope moves WEST with the sky, blHA is the amount of EAST backlash
      if (dirHA==1) {
       if (blHA<backlashHA) { blHA++; inBacklashHA=true; } else { inBacklashHA=false; posHA++; }
      } else {
        if (blHA>0)         { blHA--; inBacklashHA=true; } else { inBacklashHA=false; posHA--; }
      }

      SET(HAStepPORT, HAStepBit);
    }

  }
  
  skipDec++;                                         // Rate control Dec
  if ((skipDec>=skipCountDec) || 
      (inBacklashDec && (skipDec>=skipCountBacklashDec))) {                        

    // telescope normally starts on the EAST side of the pier looking at the WEST sky
    if (posDec<targetDec) dirDec=1; else dirDec=0;                                   // Direction control
#ifdef ReverseDec
    if (DecDir==dirDec) SET(DecDirPORT, DecDirBit); else CLR(DecDirPORT, DecDirBit); // Set direction, decDir default HIGH (=1, for my wiring)
#else
    if (DecDir==dirDec) CLR(DecDirPORT, DecDirBit); else SET(DecDirPORT, DecDirBit); // Set direction, decDir default HIGH (=1, for my wiring)
#endif
    skipDec=0;

    if (posDec!=targetDec) {                         // move the Dec stepper to the target

      // telescope moving NORTH in the sky, blDec is the amount of SOUTH backlash
      if (dirDec==1) {
        if (blDec<backlashDec) { blDec++; inBacklashDec=true; } else { inBacklashDec=false; posDec++; }
      } else {
        if (blDec>0)           { blDec--; inBacklashDec=true; } else { inBacklashDec=false; posDec--; }
      }

      SET(DecStepPORT, DecStepBit);
    }
  }
}
