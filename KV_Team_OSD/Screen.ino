
char *ItoaPadded(int val, char *str, uint8_t bytes, uint8_t decimalpos)  {
  uint8_t neg = 0;
  if(val < 0) {
    neg = 1;
    val = -val;
  }

  str[bytes] = 0;
  for(;;) {
    if(bytes == decimalpos) {
      str[--bytes] = DECIMAL;
      decimalpos = 0;
    }
    str[--bytes] = '0' + (val % 10);
    val = val / 10;
    if(bytes == 0 || (decimalpos == 0 && val == 0))
      break;
  }

  if(neg && bytes > 0)
    str[--bytes] = '-';

  while(bytes != 0)
    str[--bytes] = ' ';
  return str;
}

char *FormatGPSCoord(int32_t val, char *str, uint8_t p, char pos, char neg) {
  if(val < 0) {
    pos = neg;
    val = -val;
  }

  uint8_t bytes = p+8;

  str[bytes] = 0;
  str[--bytes] = pos;
  for(;;) {
    if(bytes == p) {
      str[--bytes] = DECIMAL;
      continue;
    }
    str[--bytes] = '0' + (val % 10);
    val = val / 10;
    if(bytes < 3 && val == 0)
       break;
   }

   while(bytes != 0)
     str[--bytes] = ' ';

   return str;
}

// Take time in Seconds and format it as 'MM:SS'
// Alternately Take time in Minutes and format it as 'HH:MM'
// If hhmmss is 1, display as HH:MM:SS
char *formatTime(uint16_t val, char *str, uint8_t hhmmss) {
  int8_t bytes = 5;
  if(hhmmss)
    bytes = 8;
  str[bytes] = 0;
  do {
    str[--bytes] = '0' + (val % 10);
    val = val / 10;
    str[--bytes] = '0' + (val % 6);
    val = val / 6;
    str[--bytes] = ':';
  } while(hhmmss-- != 0);
  do {
    str[--bytes] = '0' + (val % 10);
    val = val / 10;
  } while(val != 0 && bytes != 0);

  while(bytes != 0)
     str[--bytes] = ' ';

  return str;
}

uint8_t FindNull(void)
{
  uint8_t xx;
  for(xx=0;screenBuffer[xx]!=0;xx++)
    ;
  return xx;
}

void displayTemperature(void)        // WILL WORK ONLY WITH V1.2
{
  int xxx;
  if (Settings[S_UNITSYSTEM])
    xxx = temperature*1.8+32;       //Fahrenheit conversion for imperial system.
  else
    xxx = temperature;

  if(xxx > temperMAX)
    temperMAX = xxx;

  itoa(xxx,screenBuffer,10);
  uint8_t xx = FindNull();   // find the NULL
  screenBuffer[xx++]=temperatureUnitAdd[Settings[S_UNITSYSTEM]];
  screenBuffer[xx]=0;  // Restore the NULL
  MAX7456_WriteString(screenBuffer,getPosition(temperaturePosition));
}

void displayMode(void)
{
  if(Settings[S_MODEICON]){
    // Put sensor symbold (was displaySensors)
    screenBuffer[0] = (MwSensorPresent&ACCELEROMETER) ? SYM_ACC : ' ';
    screenBuffer[1] = (MwSensorPresent&BAROMETER) ? SYM_BAR : ' ';
    screenBuffer[2] = (MwSensorPresent&MAGNETOMETER) ? SYM_MAG : ' ';
    screenBuffer[3] = (MwSensorPresent&GPSSENSOR) ? SYM_GPS : ' ';
  
    if(MwSensorActive&mode_stable)
    {
      screenBuffer[4]=SYM_STABLE;
      screenBuffer[5]=SYM_STABLE1;
    }
    else
    {
      screenBuffer[4]=SYM_ACRO;
      screenBuffer[5]=SYM_ACRO1;
    }
    screenBuffer[6]=' ';
    if(MwSensorActive&mode_gpshome)
      screenBuffer[7]=SYM_G_HOME;
    else if(MwSensorActive&mode_gpshold)
      screenBuffer[7]=SYM_HOLD;
    else if(GPS_fix)
      screenBuffer[7]=SYM_3DFIX;
    else
      screenBuffer[7]=' ';
  
    screenBuffer[8]=0;
    MAX7456_WriteString(screenBuffer,getPosition(sensorPosition));
  
    // Put ON indicator under sensor symbol
    screenBuffer[0] = (MwSensorActive&mode_stable) ? SYM_CHECK : ' ';
    screenBuffer[1] = (MwSensorActive&mode_baro) ? SYM_CHECK : ' ';
    screenBuffer[2] = (MwSensorActive&mode_mag) ? SYM_CHECK : ' ';
    screenBuffer[3] = (MwSensorActive&(mode_gpshome|mode_gpshold)) ? SYM_CHECK : ' ';
    screenBuffer[4] = 0;
    MAX7456_WriteString(screenBuffer,getPosition(sensorPosition)+LINE);
    }
}

void displayArmed(void)
{
  if(!armed)
    MAX7456_WriteString_P(disarmed_text, getPosition(motorArmedPosition));
  else if(Blink10hz && flyTime < 8)
    MAX7456_WriteString_P(armed_text, getPosition(motorArmedPosition));
}

void displayCallsign(void)
{
  uint16_t position = getPosition(callSignPosition);
  if(Settings[S_DISPLAY_CS]){
      for(int X=0; X<10; X++) {
          screenBuffer[X] = char(Settings[S_CS0 + X]);
     }   
       screenBuffer[10] = 0;
       MAX7456_WriteString(screenBuffer, getPosition(callSignPosition)); 
  }
}
void displayHorizon(int rollAngle, int pitchAngle)
{
  if(!fieldIsVisible(horizonPosition))
    return;

  uint16_t position = getPosition(horizonPosition);

  if(pitchAngle>200) pitchAngle=200;
  if(pitchAngle<-250) pitchAngle=-250;
  if(rollAngle>400) rollAngle=400;
  if(rollAngle<-400) rollAngle=-400;

  for(int X=0; X<=8; X++) {
    int Y = (rollAngle * (4-X)) / 64;
    Y -= pitchAngle / 8;
    Y += 41;
    if(Y >= 0 && Y <= 81) {
      uint16_t pos = position + LINE*(Y/9) + 2 - 2*LINE + X;
      screen[pos] = SYM_AH_BAR9_0+(Y%9);
      if(Y>=9 && (Y%9) == 0)
        screen[pos-LINE] = SYM_AH_BAR9_9;
    }
  }

  if(Settings[S_DISPLAY_HORIZON_BR]){
    //Draw center screen
    screen[position+2*LINE+6-1] = SYM_AH_CENTER_LINE;
    screen[position+2*LINE+6+1] = SYM_AH_CENTER_LINE;
    screen[position+2*LINE+6] =   SYM_AH_CENTER;
  }
  if (Settings[S_WITHDECORATION]){
    // Draw AH sides
    screen[position+2*LINE+1] =   SYM_AH_LEFT;
    screen[position+2*LINE+11] =  SYM_AH_RIGHT;
    screen[position+0*LINE] =     SYM_AH_DECORATION_LEFT;
    screen[position+1*LINE] =     SYM_AH_DECORATION_LEFT;
    screen[position+2*LINE] =     SYM_AH_DECORATION_LEFT;
    screen[position+3*LINE] =     SYM_AH_DECORATION_LEFT;
    screen[position+4*LINE] =     SYM_AH_DECORATION_LEFT;
    screen[position+0*LINE+12] =  SYM_AH_DECORATION_RIGHT;
    screen[position+1*LINE+12] =  SYM_AH_DECORATION_RIGHT;
    screen[position+2*LINE+12] =  SYM_AH_DECORATION_RIGHT;
    screen[position+3*LINE+12] =  SYM_AH_DECORATION_RIGHT;
    screen[position+4*LINE+12] = SYM_AH_DECORATION_RIGHT;
  }
}

void displayVoltage(void)
{
  if (Settings[S_VIDVOLTAGE_VBAT]){
    vidvoltage=MwVBat;
  }
  if (Settings[S_MAINVOLTAGE_VBAT]){
    voltage=MwVBat;
  }
  ItoaPadded(voltage, screenBuffer, 4, 3);
  screenBuffer[4] = SYM_VOLT;
  screenBuffer[5] = 0;
  MAX7456_WriteString(screenBuffer,getPosition(voltagePosition));

  if (Settings[S_SHOWBATLEVELEVOLUTION]){
    // For battery evolution display
    int BATTEV1 =Settings[S_BATCELLS] * 35;
    int BATTEV2 =Settings[S_BATCELLS] * 36;
    int BATTEV3 =Settings[S_BATCELLS] * 37;
    int BATTEV4 =Settings[S_BATCELLS] * 38;
    int BATTEV5 =Settings[S_BATCELLS] * 40;
    int BATTEV6 = Settings[S_BATCELLS] * 41;

    if (voltage < BATTEV1) screenBuffer[0]=SYM_BATT_EMPTY;
    else if (voltage < BATTEV2) screenBuffer[0]=SYM_BATT_1;
    else if (voltage < BATTEV3) screenBuffer[0]=SYM_BATT_2;
    else if (voltage < BATTEV4) screenBuffer[0]=SYM_BATT_3;
    else if (voltage < BATTEV5) screenBuffer[0]=SYM_BATT_4;
    else if (voltage < BATTEV6) screenBuffer[0]=SYM_BATT_5;
    else screenBuffer[0]=SYM_BATT_FULL;                              // Max charge icon
  }
  else {
    screenBuffer[0]=SYM_MAIN_BATT;
  }
  screenBuffer[1]=0;
  MAX7456_WriteString(screenBuffer,getPosition(voltagePosition)-1);

  if (Settings[S_VIDVOLTAGE]){
    ItoaPadded(vidvoltage, screenBuffer, 4, 3);
    screenBuffer[4]=SYM_VOLT;
    screenBuffer[5]=0;
    MAX7456_WriteString(screenBuffer,getPosition(vidvoltagePosition));
    screenBuffer[0]=SYM_VID_BAT;
    screenBuffer[1]=0;
    MAX7456_WriteString(screenBuffer,getPosition(vidvoltagePosition)-1);
  }
}

void displayCurrentThrottle(void)
{

  if (MwRcData[THROTTLESTICK] > HighT) HighT = MwRcData[THROTTLESTICK] -5;
  if (MwRcData[THROTTLESTICK] < LowT) LowT = MwRcData[THROTTLESTICK];      // Calibrate high and low throttle settings  --defaults set in GlobalVariables.h 1100-1900
  screenBuffer[0]=SYM_THR;
  screenBuffer[1]=0;
  MAX7456_WriteString(screenBuffer,getPosition(CurrentThrottlePosition));
  if(!armed) {
    screenBuffer[0]=' ';
    screenBuffer[1]=' ';
    screenBuffer[2]='-';
    screenBuffer[3]='-';
    screenBuffer[4]=0;
    MAX7456_WriteString(screenBuffer,getPosition(CurrentThrottlePosition)+2);
  }
  else
  {
    int CurThrottle = map(MwRcData[THROTTLESTICK],LowT,HighT,0,100);
    ItoaPadded(CurThrottle,screenBuffer,3,0);
    screenBuffer[3]='%';
    screenBuffer[4]=0;
    MAX7456_WriteString(screenBuffer,getPosition(CurrentThrottlePosition)+2);
  }
}

void displayTime(void)
{ 
  if(flyTime < 3600) {
    screenBuffer[0] = SYM_FLY_M;
    formatTime(flyTime, screenBuffer+1, 0);
  }
  else {
    screenBuffer[0] = SYM_FLY_H;
    formatTime(flyTime/60, screenBuffer+1, 0);
  }
  MAX7456_WriteString(screenBuffer,getPosition(flyTimePosition));

  if (armed) return ;
  uint16_t position = getPosition(onTimePosition);
  if(onTime < 3600) {
    screenBuffer[0] = SYM_ON_M;
    formatTime(onTime, screenBuffer+1, 0);
  }
  else {
    screenBuffer[0] = SYM_ON_H;
    formatTime(onTime/60, screenBuffer+1, 0);
  }
  MAX7456_WriteString(screenBuffer,getPosition(onTimePosition));
}

void displayAmperage(void)
{
  // Real Ampere is ampere / 10
  ItoaPadded(amperage, screenBuffer, 4, 3);     // 99.9 ampere max!
  screenBuffer[4] = SYM_AMP;
  screenBuffer[5] = 0;
  MAX7456_WriteString(screenBuffer,getPosition(amperagePosition));
}

void displaypMeterSum(void)
{
  if (Settings[S_ENABLEADC]){
    pMeterSum = amperagesum;
  }
  screenBuffer[0]=SYM_MAH;
  int xx = pMeterSum / EST_PMSum;
  itoa(xx,screenBuffer+1,10);
  MAX7456_WriteString(screenBuffer,getPosition(pMeterSumPosition));
}

void displayRSSI(void)
{
  screenBuffer[0] = SYM_RSSI;
  // Calcul et affichage du Rssi
  itoa(rssi,screenBuffer+1,10);
  uint8_t xx = FindNull();
  screenBuffer[xx++] = '%';
  screenBuffer[xx] = 0;
  MAX7456_WriteString(screenBuffer,getPosition(rssiPosition));
}

void displayHeading(void)
{
 if (Settings[S_SHOWHEADING]) {  
      int16_t heading = MwHeading;
      if (Settings[S_HEADING360]) {
        if(heading < 0)
          heading += 360;
        ItoaPadded(heading,screenBuffer,3,0);
        screenBuffer[3]=SYM_DEGREES;
        screenBuffer[4]=0;
      }
      else {
        ItoaPadded(heading,screenBuffer,4,0);
        screenBuffer[4]=SYM_DEGREES;
        screenBuffer[5]=0;
      }
      MAX7456_WriteString(screenBuffer,getPosition(MwHeadingPosition));
  }  
}

void displayHeadingGraph(void)
{
  int xx;
  xx = MwHeading * 4;
  xx = xx + 720 + 45;
  xx = xx / 90;

  uint16_t pos = getPosition(MwHeadingGraphPosition);
  memcpy_P(screen+pos, headGraph+xx, 10);
}

void displayIntro(void)
{

  MAX7456_WriteString_P(message0, KVTeamVersionPosition);

  if (Settings[S_VIDEOSIGNALTYPE])
    MAX7456_WriteString_P(message2, KVTeamVersionPosition+30);
  else
    MAX7456_WriteString_P(message1, KVTeamVersionPosition+30);

    

  MAX7456_WriteString_P(MultiWiiLogoL1Add, KVTeamVersionPosition+120);
  MAX7456_WriteString_P(MultiWiiLogoL2Add, KVTeamVersionPosition+120+LINE);
  MAX7456_WriteString_P(MultiWiiLogoL3Add, KVTeamVersionPosition+120+LINE+LINE);

  MAX7456_WriteString_P(message5, KVTeamVersionPosition+120+LINE+LINE+LINE);
  MAX7456_WriteString(itoa(MwVersion,screenBuffer,10),KVTeamVersionPosition+131+LINE+LINE+LINE);

  MAX7456_WriteString_P(message6, KVTeamVersionPosition+120+LINE+LINE+LINE+LINE+LINE);
  MAX7456_WriteString_P(message7, KVTeamVersionPosition+125+LINE+LINE+LINE+LINE+LINE+LINE);
  MAX7456_WriteString_P(message8, KVTeamVersionPosition+125+LINE+LINE+LINE+LINE+LINE+LINE+LINE);
  
  MAX7456_WriteString_P(message9, KVTeamVersionPosition+120+LINE+LINE+LINE+LINE+LINE+LINE+LINE+LINE);
   if(Settings[S_DISPLAY_CS]){
      for(int X=0; X<10; X++) {
          screenBuffer[X] = char(Settings[S_CS0 + X]);
      }
   if (Blink2hz)
   MAX7456_WriteString(screenBuffer, KVTeamVersionPosition+130+LINE+LINE+LINE+LINE+LINE+LINE+LINE+LINE);;     // Call Sign on the beggining of the transmission (blink 2hz)  
   }
}

void displayFontScreen(void) {
  MAX7456_WriteString_P(PSTR("UPLOADING FONT"), 35);
  MAX7456_WriteString(itoa(nextCharToRequest, screenBuffer, 10), 51);

  for(uint16_t i = 0; i < 256; i++)
    screen[90+i] = i;
}

void displayGPSPosition(void)
{
  if(!GPS_fix)
    return;

  if(Settings[S_COORDINATES]){
    if(fieldIsVisible(MwGPSLatPosition)) {
      screenBuffer[0] = SYM_LAT;
      FormatGPSCoord(GPS_latitude,screenBuffer+1,3,'N','S');
      if(!Settings[S_GPSCOORDTOP])
        MAX7456_WriteString(screenBuffer,getPosition(MwGPSLatPosition));
      else
        MAX7456_WriteString(screenBuffer,getPosition(MwGPSLatPositionTop));  
    }

    if(fieldIsVisible(MwGPSLatPosition)) {
      screenBuffer[0] = SYM_LON;
      FormatGPSCoord(GPS_longitude,screenBuffer+1,3,'E','W');
      if(!Settings[S_GPSCOORDTOP])
        MAX7456_WriteString(screenBuffer,getPosition(MwGPSLonPosition));
      else
        MAX7456_WriteString(screenBuffer,getPosition(MwGPSLonPositionTop));          
    }
  }
  
  if(Settings[S_GPSALTITUDE]){
      screenBuffer[0] = MwGPSAltPositionAdd[Settings[S_UNITSYSTEM]];
      uint16_t xx;
      if(Settings[S_UNITSYSTEM])
        xx = GPS_altitude * 3.2808; // Mt to Feet
      else
        xx = GPS_altitude;          // Mt
      itoa(xx,screenBuffer+1,10);
      MAX7456_WriteString(screenBuffer,getPosition(MwGPSAltPosition));
      }
}

void displayNumberOfSat(void)
{
  screenBuffer[0] = SYM_SAT_L;
  screenBuffer[1] = SYM_SAT_R;
  itoa(GPS_numSat,screenBuffer+2,10);
  if(!Settings[S_GPSCOORDTOP])
    MAX7456_WriteString(screenBuffer,getPosition(GPS_numSatPosition));
  else 
    MAX7456_WriteString(screenBuffer,getPosition(GPS_numSatPositionTop)); 
}


void displayGPS_speed(void)
{

  if(!GPS_fix) return;
  if(!armed) GPS_speed=0;

  int xx;
  if(!Settings[S_UNITSYSTEM])
    xx = GPS_speed * 0.036;           // From MWii cm/sec to Km/h
  else
    xx = GPS_speed * 0.02236932;      // (0.036*0.62137)  From MWii cm/sec to mph

  if(xx > speedMAX)
    speedMAX = xx;
    
  screenBuffer[0]=speedUnitAdd[Settings[S_UNITSYSTEM]];
  itoa(xx,screenBuffer+1,10);
  MAX7456_WriteString(screenBuffer,getPosition(speedPosition));
}

void displayAltitude(void)
{
  int16_t altitude;
  if(Settings[S_UNITSYSTEM])
    altitude = MwAltitude*0.032808;    // cm to feet
  else
    altitude = MwAltitude/100;         // cm to mt

  if(armed && allSec>5 && altitude > altitudeMAX)
    altitudeMAX = altitude;

  screenBuffer[0]=MwAltitudeAdd[Settings[S_UNITSYSTEM]];
  itoa(altitude,screenBuffer+1,10);
  MAX7456_WriteString(screenBuffer,getPosition(MwAltitudePosition));
}

void displayClimbRate(void)
{

  if(MwVario > 70)       screenBuffer[0] = SYM_POS_CLIMB3;
  else if(MwVario > 50)  screenBuffer[0] = SYM_POS_CLIMB2;
  else if(MwVario > 30)  screenBuffer[0] = SYM_POS_CLIMB1;
  else if(MwVario > 20)  screenBuffer[0] = SYM_POS_CLIMB;
  else if(MwVario < -70) screenBuffer[0] = SYM_NEG_CLIMB3;
  else if(MwVario < -50) screenBuffer[0] = SYM_NEG_CLIMB2;
  else if(MwVario < -30) screenBuffer[0] = SYM_NEG_CLIMB1;
  else if(MwVario < -20) screenBuffer[0] = SYM_NEG_CLIMB;
  else                   screenBuffer[0] = SYM_ZERO_CLIMB;

  screenBuffer[1] = MwClimbRateAdd[Settings[S_UNITSYSTEM]];
  int16_t vario;
  if(Settings[S_UNITSYSTEM])
    vario = MwVario * 0.032808;       // cm/sec ----> ft/sec
  else
    vario = MwVario / 100;            // cm/sec ----> mt/sec
  itoa(vario, screenBuffer+2, 10);

  MAX7456_WriteString(screenBuffer,getPosition(MwClimbRatePosition));
}

void displayDistanceToHome(void)
{
  if(!GPS_fix)
    return;

  int16_t dist;
  if(Settings[S_UNITSYSTEM])
    dist = GPS_distanceToHome * 3.2808;           // mt to feet
  else
    dist = GPS_distanceToHome;                    // Mt

  if(dist > distanceMAX)
    distanceMAX = dist;

  screenBuffer[0] = GPS_distanceToHomeAdd[Settings[S_UNITSYSTEM]];
  itoa(dist, screenBuffer+1, 10);
  MAX7456_WriteString(screenBuffer,getPosition(GPS_distanceToHomePosition));
}

void displayAngleToHome(void)
{
  if(!GPS_fix)
      return;
      
  if(Settings[S_ANGLETOHOME]){
    if(GPS_distanceToHome <= 2 && Blink2hz)
      return;
  
    ItoaPadded(GPS_directionToHome,screenBuffer,3,0);
    screenBuffer[3] = SYM_DEGREES;
    screenBuffer[4] = 0;
    MAX7456_WriteString(screenBuffer,getPosition(GPS_angleToHomePosition));
    }
}

void displayDirectionToHome(void)
{
  if(!GPS_fix)
    return;
  if(GPS_distanceToHome <= 2 && Blink2hz)
    return;
  
  int16_t d = MwHeading + 180 + 360 - GPS_directionToHome;
  d *= 4;
  d += 45;
  d = (d/90)%16;

  screenBuffer[0] = SYM_ARROW_SOUTH + d;
  screenBuffer[1] = 0;
  MAX7456_WriteString(screenBuffer,getPosition(GPS_directionToHomePosition));
}
/*
void displayCursor(void)
{
  int cursorpos;

  if(ROW==10){
    if(COL==3) cursorpos=SAVEP+16-1;    // page
    if(COL==1) cursorpos=SAVEP-1;       // exit
    if(COL==2) cursorpos=SAVEP+6-1;     // save/exit
  }
  if(ROW<10){
    if(configPage==1){
      if (ROW==9) ROW=7;
      if (ROW==8) ROW=10;
      if(COL==1) cursorpos=(ROW+2)*30+10;
      if(COL==2) cursorpos=(ROW+2)*30+10+6;
      if(COL==3) cursorpos=(ROW+2)*30+10+6+6;
      }
    if(configPage==2){
      COL=3;
      if (ROW==7) ROW=5;
      if (ROW==6) ROW=10;
      if (ROW==9) ROW=5;
      cursorpos=(ROW+2)*30+10+6+6;
      }
    if(configPage==3){
      COL=3;
      if (ROW==9) ROW=7;
      if (ROW==8) ROW=10;
      cursorpos=(ROW+2)*30+10+6+6;
     
      }
    if(configPage==4){
      COL=3;
      if (ROW==2) ROW=3;
      if (ROW==9) ROW=5;
      if (ROW==6) ROW=10;
      cursorpos=(ROW+2)*30+10+6+6;
      }
      
    if(configPage==5 || configPage==6)
      {  
      COL=3;
      if (ROW==9) ROW=7;
      if (ROW==8) ROW=10;
      cursorpos=(ROW+2)*30+10+6+6;
      }

    if(configPage==7)
      {  
      COL=3;
      if (ROW==9) ROW=5;
      if (ROW==6) ROW=10;
      if (ROW==4) cursorpos=(ROW+2)*30+10+6+6-2;  // Metric/Imperial string longer            
      else cursorpos=(ROW+2)*30+10+6+6;
      }

    if(configPage==8){
      ROW=10;
      }
    
    if(configPage == 9){
      COL = 3;
      if (ROW == 9)
        ROW = 1;
      if (ROW == 2)
        ROW = 10;
      cursorpos = (ROW+2)*30+10+6+6;
    }
  }
  if(Blink10hz)
    screen[cursorpos] = SYM_CURSOR;
}


void displayConfigScreen(void)
{
  MAX7456_WriteString_P(configMsgEXT, SAVEP);    //EXIT
  if(!previousarmedstatus) {
    MAX7456_WriteString_P(configMsgSAVE, SAVEP+6);  //SaveExit
    MAX7456_WriteString_P(configMsgPGS, SAVEP+16); //<Page>
  }

  if(configPage==1)
  {
    MAX7456_WriteString_P(configMsg10, 38);
    MAX7456_WriteString_P(configMsg11, ROLLT);
    MAX7456_WriteString(itoa(P8[0],screenBuffer,10),ROLLP);
    MAX7456_WriteString(itoa(I8[0],screenBuffer,10),ROLLI);
    MAX7456_WriteString(itoa(D8[0],screenBuffer,10),ROLLD);

    MAX7456_WriteString_P(configMsg12, PITCHT);
    MAX7456_WriteString(itoa(P8[1],screenBuffer,10), PITCHP);
    MAX7456_WriteString(itoa(I8[1],screenBuffer,10), PITCHI);
    MAX7456_WriteString(itoa(D8[1],screenBuffer,10), PITCHD);

    MAX7456_WriteString_P(configMsg13, YAWT);
    MAX7456_WriteString(itoa(P8[2],screenBuffer,10),YAWP);
    MAX7456_WriteString(itoa(I8[2],screenBuffer,10),YAWI);
    MAX7456_WriteString(itoa(D8[2],screenBuffer,10),YAWD);

    MAX7456_WriteString_P(configMsg14, ALTT);
    MAX7456_WriteString(itoa(P8[3],screenBuffer,10),ALTP);
    MAX7456_WriteString(itoa(I8[3],screenBuffer,10),ALTI);
    MAX7456_WriteString(itoa(D8[3],screenBuffer,10),ALTD);

    MAX7456_WriteString_P(configMsg15, VELT);
    MAX7456_WriteString(itoa(P8[4],screenBuffer,10),VELP);
    MAX7456_WriteString(itoa(I8[4],screenBuffer,10),VELI);
    MAX7456_WriteString(itoa(D8[4],screenBuffer,10),VELD);

    MAX7456_WriteString_P(configMsg16, LEVT);
    MAX7456_WriteString(itoa(P8[7],screenBuffer,10),LEVP);
    MAX7456_WriteString(itoa(I8[7],screenBuffer,10),LEVI);
    MAX7456_WriteString(itoa(D8[7],screenBuffer,10),LEVD);

    MAX7456_WriteString_P(configMsg17, MAGT);
    MAX7456_WriteString(itoa(P8[8],screenBuffer,10),MAGP);

    MAX7456_WriteString("P",71);
    MAX7456_WriteString("I",77);
    MAX7456_WriteString("D",83);
  }

  if(configPage==2)
  {
    MAX7456_WriteString_P(configMsg20, 38);
    MAX7456_WriteString_P(configMsg21, ROLLT);
    MAX7456_WriteString(itoa(rcRate8,screenBuffer,10),ROLLD);
    MAX7456_WriteString_P(configMsg22, PITCHT);
    MAX7456_WriteString(itoa(rcExpo8,screenBuffer,10),PITCHD);
    MAX7456_WriteString_P(configMsg23, YAWT);
    MAX7456_WriteString(itoa(rollPitchRate,screenBuffer,10),YAWD);
    MAX7456_WriteString_P(configMsg24, ALTT);
    MAX7456_WriteString(itoa(yawRate,screenBuffer,10),ALTD);
    MAX7456_WriteString_P(configMsg25, VELT);
    MAX7456_WriteString(itoa(dynThrPID,screenBuffer,10),VELD);
    MAX7456_WriteString_P(configMsg26, LEVT);
    MAX7456_WriteString(itoa(cycleTime,screenBuffer,10),LEVD);
    MAX7456_WriteString_P(configMsg27, MAGT);
    MAX7456_WriteString(itoa(I2CError,screenBuffer,10),MAGD);
  }

  if(configPage==3)
  {
    MAX7456_WriteString_P(configMsg30, 35);
    
    MAX7456_WriteString_P(configMsg31, ROLLT);
    if(Settings[S_DISPLAYVOLTAGE]){
      MAX7456_WriteString_P(configMsgON, ROLLD);
    }
    else {
      MAX7456_WriteString_P(configMsgOFF, ROLLD);
    }
    
    MAX7456_WriteString_P(configMsg32, PITCHT);
    MAX7456_WriteString(itoa(Settings[S_VOLTAGEMIN],screenBuffer,10),PITCHD);
    
    MAX7456_WriteString_P(configMsg33, YAWT);
    if(Settings[S_VIDVOLTAGE]){
      MAX7456_WriteString_P(configMsgON, YAWD);
    }
    else{
      MAX7456_WriteString_P(configMsgOFF, YAWD);
    }

    MAX7456_WriteString_P(configMsg34, ALTT);
    if(Settings[S_DISPLAYTEMPERATURE] ){
      MAX7456_WriteString_P(configMsgON, ALTD);
    }
    else {
      MAX7456_WriteString_P(configMsgOFF, ALTD);
    }
    
    MAX7456_WriteString_P(configMsg35, VELT);
    MAX7456_WriteString(itoa(Settings[S_TEMPERATUREMAX],screenBuffer,10),VELD);
    
    MAX7456_WriteString_P(configMsg36, LEVT);
    if(Settings[S_AMPER_HOUR]){
      MAX7456_WriteString_P(configMsgON, LEVD);
    }
    else{
      MAX7456_WriteString_P(configMsgOFF, LEVD);
    }

    MAX7456_WriteString_P(configMsg37, MAGT);
    if(Settings[S_AMPERAGE]){
      MAX7456_WriteString_P(configMsgON, MAGD);
    }
    else{
      MAX7456_WriteString_P(configMsgOFF, MAGD);
    }
    
  }

  if(configPage==4)
  {
    MAX7456_WriteString_P(configMsg40, 39);

    MAX7456_WriteString_P(configMsg41, ROLLT);
    MAX7456_WriteString(itoa(rssiADC,screenBuffer,10),ROLLD);

    MAX7456_WriteString_P(configMsg42, PITCHT);
    MAX7456_WriteString(itoa(rssi,screenBuffer,10),PITCHD);

    MAX7456_WriteString_P(configMsg43, YAWT);
    if(rssiTimer>0) MAX7456_WriteString(itoa(rssiTimer,screenBuffer,10),YAWD-5);
    MAX7456_WriteString(itoa(Settings[S_RSSIMIN],screenBuffer,10),YAWD);

    MAX7456_WriteString_P(configMsg44, ALTT);
    MAX7456_WriteString(itoa(Settings[S_RSSIMAX],screenBuffer,10),ALTD);

    MAX7456_WriteString_P(configMsg45, VELT);
    if(Settings[S_DISPLAYRSSI]){
      MAX7456_WriteString_P(configMsgON, VELD);
    }
    else{
      MAX7456_WriteString_P(configMsgOFF, VELD);
    }
  }

  if(configPage==5)
  {
    MAX7456_WriteString_P(configMsg50, 37);

    MAX7456_WriteString_P(configMsg51, ROLLT);
    if(accCalibrationTimer>0)
      MAX7456_WriteString(itoa(accCalibrationTimer,screenBuffer,10),ROLLD);
    else
      MAX7456_WriteString("-",ROLLD);

    MAX7456_WriteString_P(configMsg52, PITCHT);
    MAX7456_WriteString(itoa(MwAccSmooth[0],screenBuffer,10),PITCHD);

    MAX7456_WriteString_P(configMsg53, YAWT);
    MAX7456_WriteString(itoa(MwAccSmooth[1],screenBuffer,10),YAWD);

    MAX7456_WriteString_P(configMsg54, ALTT);
    MAX7456_WriteString(itoa(MwAccSmooth[2],screenBuffer,10),ALTD);

    MAX7456_WriteString_P(configMsg55, VELT);
    if(magCalibrationTimer>0)
      MAX7456_WriteString(itoa(magCalibrationTimer,screenBuffer,10),VELD);
    else
      MAX7456_WriteString("-",VELD);

    MAX7456_WriteString_P(configMsg56, LEVT);
    MAX7456_WriteString(itoa(MwHeading,screenBuffer,10),LEVD);

    MAX7456_WriteString_P(configMsg57, MAGT);
    if(eepromWriteTimer>0)
      MAX7456_WriteString(itoa(eepromWriteTimer,screenBuffer,10),MAGD);
    else
      MAX7456_WriteString("-",MAGD);
  }

  if(configPage==6)
  {
    MAX7456_WriteString_P(configMsg60, 39);

    MAX7456_WriteString_P(configMsg61, ROLLT);
    if(Settings[S_DISPLAYGPS]){
      MAX7456_WriteString_P(configMsgON, ROLLD);
    }
    else{
      MAX7456_WriteString_P(configMsgOFF, ROLLD);
    }

    MAX7456_WriteString_P(configMsg62, PITCHT);
    if(Settings[S_COORDINATES]){
      MAX7456_WriteString_P(configMsgON, PITCHD);
    }
    else{
      MAX7456_WriteString_P(configMsgOFF, PITCHD);
    }

    MAX7456_WriteString_P(configMsg63, YAWT);
    if(Settings[S_GPSCOORDTOP]){
      MAX7456_WriteString_P(configMsgON, YAWD);
    }
    else{
      MAX7456_WriteString_P(configMsgOFF, YAWD);
    }

        MAX7456_WriteString_P(configMsg64, ALTT);
    if(Settings[S_GPSALTITUDE]){
      MAX7456_WriteString_P(configMsgON, ALTD);
    }
    else{
      MAX7456_WriteString_P(configMsgOFF, ALTD);
    }

    MAX7456_WriteString_P(configMsg65, VELT);
    if(Settings[S_ANGLETOHOME]){
      MAX7456_WriteString_P(configMsgON, VELD);
    }
    else{
      MAX7456_WriteString_P(configMsgOFF, VELD);
    }
    
    MAX7456_WriteString_P(configMsg66, LEVT);
    if(Settings[S_SHOWHEADING]){
      MAX7456_WriteString_P(configMsgON, LEVD);
    }
    else{
      MAX7456_WriteString_P(configMsgOFF, LEVD);
    }
    
    MAX7456_WriteString_P(configMsg67, MAGT);
    if(Settings[S_MODEICON]){
      MAX7456_WriteString_P(configMsgON, MAGD);
    }
    else{
      MAX7456_WriteString_P(configMsgOFF, MAGD);
    }    
 }

  if(configPage==7)
  {
    MAX7456_WriteString_P(configMsg70, 39);

    MAX7456_WriteString_P(configMsg71, ROLLT);
    if(Settings[S_DISPLAY_CS]){
      MAX7456_WriteString_P(configMsgON, ROLLD);
    }
    else{
      MAX7456_WriteString_P(configMsgOFF, ROLLD);
    }

    MAX7456_WriteString_P(configMsg72, PITCHT);
    if(Settings[S_THROTTLEPOSITION]){
      MAX7456_WriteString_P(configMsgON, PITCHD);
    }
    else{
      MAX7456_WriteString_P(configMsgOFF, PITCHD);
    }
    
    MAX7456_WriteString_P(configMsg73, YAWT);
    if(Settings[S_WITHDECORATION]){
      MAX7456_WriteString_P(configMsgON, YAWD);
    }
    else{
      MAX7456_WriteString_P(configMsgOFF, YAWD);      
    }
    
    MAX7456_WriteString_P(configMsg74, ALTT);
    if(Settings[S_UNITSYSTEM]==METRIC){
      MAX7456_WriteString_P(configMsg75, ALTD-2);
    }
    else {
      MAX7456_WriteString_P(configMsg76, ALTD-2);
    }

    MAX7456_WriteString_P(configMsg77, VELT);
    if(Settings[S_VIDEOSIGNALTYPE]){
      MAX7456_WriteString_P(configMsg79, VELD);
    }
    else {
      MAX7456_WriteString_P(configMsg78, VELD);
      }
   }

  if(configPage==8)
  {
    int xx;
    MAX7456_WriteString_P(configMsg80, 38);

    MAX7456_WriteString_P(configMsg81, ROLLT);
    MAX7456_WriteString(itoa(trip,screenBuffer,10),ROLLD-3);

    MAX7456_WriteString_P(configMsg82, PITCHT);
    MAX7456_WriteString(itoa(distanceMAX,screenBuffer,10),PITCHD-3);

    MAX7456_WriteString_P(configMsg83, YAWT);
    MAX7456_WriteString(itoa(altitudeMAX,screenBuffer,10),YAWD-3);

    MAX7456_WriteString_P(configMsg84, ALTT);
    MAX7456_WriteString(itoa(speedMAX,screenBuffer,10),ALTD-3);

    MAX7456_WriteString_P(configMsg85, VELT);

    formatTime(flyingTime, screenBuffer, 1);
    MAX7456_WriteString(screenBuffer,VELD-4);

    MAX7456_WriteString_P(configMsg86, LEVT);
    xx= pMeterSum / EST_PMSum;
    MAX7456_WriteString(itoa(xx,screenBuffer,10),LEVD-3);

    MAX7456_WriteString_P(configMsg87, MAGT);
    MAX7456_WriteString(itoa(temperMAX,screenBuffer,10),MAGD-3);
  }
   
  if(configPage==9)
  {
    MAX7456_WriteString_P(configMsg90, 38);
    MAX7456_WriteString_P(configMsg91, ROLLT);
    MAX7456_WriteString(itoa(ConfigCurrentSet,screenBuffer,10),ROLLD);
  }
    
  displayCursor();
}
*/


/* Config screen data */

/* handler function type */

typedef char *(*callback_ptr)(void *v, char *b, int8_t delta);

struct configLabel {
  uint16_t pos;
  const char *txt;
};

struct configInput {
  uint16_t pos;
  void *v;
  callback_ptr callback;
};

struct configScreen {
  uint8_t labelsNum;
  uint8_t inputsNum;
  configLabel *labels;
  configInput *inputs;
};


configLabel labelsPage1[] PROGMEM = {
  { 8 + 30*1, configMsg10 },
  { 11 + 30*2, configMsgPID },
  { 3 + 30*3, configMsg11 },
  { 3 + 30*4, configMsg12 },
  { 3 + 30*5, configMsg13 },
  { 3 + 30*6, configMsg14 },
  { 3 + 30*7, configMsg15 },
  { 3 + 30*8, configMsg16 },
  { 3 + 30*9, configMsg17 }
};

configLabel labelsPage2[] PROGMEM = {
  { 8 + 30*1, configMsg20 },
  { 3 + 30*3, configMsg21 },
  { 3 + 30*4, configMsg22 },
  { 3 + 30*5, configMsg23 },
  { 3 + 30*6, configMsg24 },
  { 3 + 30*7, configMsg25 },
  { 3 + 30*8, configMsg26 },
  { 3 + 30*9, configMsg27 }
};

configLabel labelsPage3[] PROGMEM = {
  { 5 + 30*1, configMsg30 },
  { 3 + 30*3, configMsg31 },
  { 3 + 30*4, configMsg32 },
  { 3 + 30*5, configMsg33 },
  { 3 + 30*6, configMsg34 },
  { 3 + 30*7, configMsg35 },
  { 3 + 30*8, configMsg36 },
  { 3 + 30*9, configMsg37 }
};

configLabel labelsPage4[] PROGMEM = {
  { 9 + 30*1, configMsg40 },
  { 3 + 30*3, configMsg41 },
  { 3 + 30*4, configMsg42 },
  { 3 + 30*5, configMsg43 },
  { 3 + 30*6, configMsg44 },
  { 3 + 30*7, configMsg45 }
};

configLabel labelsPage5[] PROGMEM = {
  { 7 + 30*1, configMsg50 },
  { 3 + 30*3, configMsg51 },
  { 3 + 30*4, configMsg52 },
  { 3 + 30*5, configMsg53 },
  { 3 + 30*6, configMsg54 },
  { 3 + 30*7, configMsg55 },
  { 3 + 30*8, configMsg56 },
  { 3 + 30*9, configMsg57 }
};

configLabel labelsPage6[] PROGMEM = {
  { 9 + 30*1, configMsg60 },
  { 3 + 30*3, configMsg61 },
  { 3 + 30*4, configMsg62 },
  { 3 + 30*5, configMsg63 },
  { 3 + 30*6, configMsg64 },
  { 3 + 30*7, configMsg65 },
  { 3 + 30*8, configMsg66 },
  { 3 + 30*9, configMsg67 }
};

configLabel labelsPage7[] PROGMEM = {
  { 9 + 30*1, configMsg70 },
  { 3 + 30*3, configMsg71 },
  { 3 + 30*4, configMsg72 },
  { 3 + 30*5, configMsg73 },
  { 3 + 30*6, configMsg74 },
  { 3 + 30*7, configMsg77 }
};

configLabel labelsPage8[] PROGMEM = {
  { 8 + 30*1, configMsg80 },
  { 3 + 30*3, configMsg81 },
  { 3 + 30*4, configMsg82 },
  { 3 + 30*5, configMsg83 },
  { 3 + 30*6, configMsg84 },
  { 3 + 30*7, configMsg85 },
  { 3 + 30*8, configMsg86 },
  { 3 + 30*9, configMsg87 }
};

configLabel labelsPage9[] PROGMEM = {
  { 8 + 30*1, configMsg90 },
  { 3 + 30*3, configMsg91 }
};

// input fields data for the config screens

configInput inputsPage1[] PROGMEM = {
  { 11 + 30*3, &P8[0] },
  { 17 + 30*3, &I8[0] },
  { 23 + 30*3, &D8[0] },
  { 11 + 30*4, &P8[1] },
  { 17 + 30*4, &I8[1] },
  { 23 + 30*4, &D8[1] },
  { 11 + 30*5, &P8[2] },
  { 17 + 30*5, &I8[2] },
  { 23 + 30*5, &D8[2] },
  { 11 + 30*6, &P8[3] },
  { 17 + 30*6, &I8[3] },
  { 23 + 30*6, &D8[3] },
  { 11 + 30*7, &P8[4] },
  { 17 + 30*7, &I8[4] },
  { 23 + 30*7, &D8[4] },
  { 11 + 30*8, &P8[7] },
  { 17 + 30*8, &I8[7] },
  { 23 + 30*8, &D8[7] },
  { 11 + 30*9, &P8[8] },
  
  { 3 + 30*11, NULL, &callbackExit },
  { 17 + 30*11, &configPage, &callbackPage }
};

configInput inputsPage2[] PROGMEM = {
  { 23 + 30*3, &rcRate8 },
  { 23 + 30*4, &rcExpo8 },
  { 23 + 30*5, &rollPitchRate },
  { 23 + 30*6, &yawRate },
  { 23 + 30*7, &dynThrPID },
  { 23 + 30*8, &cycleTime, &callbackWord },
  { 23 + 30*9, &I2CError, &callbackWord },
  
  { 3 + 30*11, NULL, &callbackExit },
  { 17 + 30*11, &configPage, &callbackPage }
};

configInput inputsPage3[] PROGMEM = {
  { 23 + 30*3, &Settings[S_DISPLAYVOLTAGE], &callbackBoolean },
  { 23 + 30*4, &Settings[S_VOLTAGEMIN] },
  { 23 + 30*5, &Settings[S_VIDVOLTAGE], &callbackBoolean },
  { 23 + 30*6, &Settings[S_DISPLAYTEMPERATURE], &callbackBoolean },
  { 23 + 30*7, &Settings[S_TEMPERATUREMAX] },
  { 23 + 30*8, &Settings[S_AMPER_HOUR], &callbackBoolean },
  { 23 + 30*9, &Settings[S_AMPERAGE], &callbackBoolean },
  
  { 3 + 30*11, NULL, &callbackExit },
  { 17 + 30*11, &configPage, &callbackPage }
};

configInput inputsPage4[] PROGMEM = {
  { 23 + 30*3, &rssiADC, &callbackWordRO },
  { 23 + 30*4, &rssi, &callbackWordRO },  // should be a byte?
  { 18 + 30*5, &rssiTimer, &callbackByteRO }, // special input - not selectable, maybe make it a label?
  { 23 + 30*5, &Settings[S_RSSIMIN], &callbackRSSIMIN }, // start countdown, set RSSIMIN, ignore delta
  { 23 + 30*6, &Settings[S_RSSIMAX], &callbackRSSIMAX }, // set RSSIMAX, ignore delta
  { 23 + 30*7, &Settings[S_DISPLAYRSSI], &callbackBoolean },
  
  { 3 + 30*11, NULL, &callbackExit },
  { 17 + 30*11, &configPage, &callbackPage }
};

configInput inputsPage5[] PROGMEM = {
  { 23 + 30*3, &accCalibrationTimer }, // print byte or "-" if zero
  { 23 + 30*4, &MwAccSmooth[0], &callbackWord },
  { 23 + 30*5, &MwAccSmooth[1], &callbackWord },
  { 23 + 30*6, &MwAccSmooth[2], &callbackWord },
  { 23 + 30*7, &magCalibrationTimer }, // print byte or "-" if zero
  { 23 + 30*8, &MwHeading },
  { 23 + 30*9, &eepromWriteTimer },     // print byte or "-" if zero
  
  { 3 + 30*11, NULL, &callbackExit },
  { 17 + 30*11, &configPage, &callbackPage }
};

configInput inputsPage6[] PROGMEM = {
  { 23 + 30*3, &Settings[S_DISPLAYGPS], &callbackBoolean },
  { 23 + 30*4, &Settings[S_COORDINATES], &callbackBoolean },
  { 23 + 30*5, &Settings[S_GPSCOORDTOP], &callbackBoolean },
  { 23 + 30*6, &Settings[S_GPSALTITUDE], &callbackBoolean },
  { 23 + 30*7, &Settings[S_ANGLETOHOME], &callbackBoolean },
  { 23 + 30*8, &Settings[S_SHOWHEADING], &callbackBoolean },
  { 23 + 30*9, &Settings[S_MODEICON], &callbackBoolean },
  
  { 3 + 30*11, NULL, &callbackExit },
  { 17 + 30*11, &configPage, &callbackPage }
};

configInput inputsPage7[] PROGMEM = {
  { 23 + 30*3, &Settings[S_DISPLAY_CS] },
  { 23 + 30*4, &Settings[S_THROTTLEPOSITION] },
  { 23 + 30*5, &Settings[S_WITHDECORATION], &callbackBoolean },
  { 21 + 30*6, &Settings[S_UNITSYSTEM], &callbackUnitType },
  { 23 + 30*7, &Settings[S_VIDEOSIGNALTYPE], &callbackVidType },
  
  { 3 + 30*11, NULL, &callbackExit },
  { 17 + 30*11, &configPage, &callbackPage }
};

configInput inputsPage8[] PROGMEM = { // read-only page, non-selectable inputs..  convert to labels maybe? the problem is the labels are all in PROGMEM
  { 20 + 30*3, &trip, &callbackWordRO }, // trip is float, original code is printing it with itoa TODO: check how it works.. 
  { 20 + 30*4, &distanceMAX, &callbackWordRO },
  { 20 + 30*5, &altitudeMAX, &callbackWordRO },
  { 20 + 30*6, &speedMAX, &callbackWordRO },
  { 19 + 30*7, &flyingTime, &callbackWordRO }, // formatTime(flyingTime, buf, 1)
  { 20 + 30*8, &pMeterSum, &callbackPMeterRO }, // calc pMeterSum / EST_PMSum
  { 20 + 30*9, &temperMAX, &callbackByteRO },
  
  { 3 + 30*11, NULL, &callbackExit },
  { 17 + 30*11, &configPage, &callbackPage }
};

configInput inputsPage9[] PROGMEM = {
  { 23 + 30*3, &ConfigCurrentSet },
  
  { 3 + 30*11, NULL, &callbackExit },
  { 17 + 30*11, &configPage, &callbackPage }
};

configScreen configScreens[] = {
  { 9, 21, labelsPage1, inputsPage1 },
  { 8, 9, labelsPage2, inputsPage2 },
  { 8, 9, labelsPage3, inputsPage3 },
  { 6, 8, labelsPage4, inputsPage4 },
  { 8, 9, labelsPage5, inputsPage5 },
  { 8, 9, labelsPage6, inputsPage6 },
  { 6, 7, labelsPage7, inputsPage7 },
  { 8, 9, labelsPage8, inputsPage8 },
  { 2, 3, labelsPage9, inputsPage9 }
};

// input update/display callbacks

char *callbackByte( void *v, char *b, int8_t delta ) {
  *(uint8_t *)v += delta;
  return itoa( *(uint8_t *)v, b, 10 );
}

char *callbackWord( void *v, char *b, int8_t delta ) {
  *(int16_t *)v += delta;
  return itoa( *(int16_t *)v, b, 10 );
}

char *callbackByteRO( void *v, char *b, int8_t delta ) { // read only input - ignore delta
  return itoa( *(uint8_t *)v, b, 10 );
}

char *callbackWordRO( void *v, char *b, int8_t delta ) { // read only input - ignore delta
  return itoa( *(int16_t *)v, b, 10 );
}

char *callbackBoolean( void *v, char *b, int8_t delta ) {
  char *msg;
  boolean x = *(boolean *)v;
  
  if( delta != 0 ) {
    *(boolean *)v = !x;
  }
  
  if( *(boolean *)v ) {
    msg = (char *)configMsgON;
  }
  else {
    msg = (char *)configMsgOFF;
  }
  
  return strcpy_P( b, msg );
}

char *callbackVidType( void *v, char *b, int8_t delta ) {
  char *msg;
  boolean x = *(boolean *)v;
  
  if( delta != 0 ) {
    *(boolean *)v = !x;
  }
  
  if( *(boolean *)v ) {
    msg = (char *)configMsg79; // 1 - PAL
  }
  else {
    msg = (char *)configMsg78; // 0 - NTSC
  }
  
  return strcpy_P( b, msg );
}

char *callbackUnitType( void *v, char *b, int8_t delta ) {
  char *msg;
  boolean x = *(boolean *)v;
  
  if( delta != 0 ) {
    *(boolean *)v = !x;
  }
  
  if( *(boolean *)v ) {
    msg = (char *)configMsg76; // 1 - Imperial
  }
  else {
    msg = (char *)configMsg75; // 0 - Metric
  }
  
  return strcpy_P( b, msg );
}

char *callbackPMeterRO( void *v, char *b, int8_t delta ) {
  uint16_t p = 12345; // test value
  
  // calc p (pMeterSum / EST_PMSum) ;
  
  return itoa( p, b, 10 );
}

char *callbackRSSIMIN( void *v, char *b, int8_t delta ) {
  // if countdown == 0 start countdown for RSSIMIN
  
  // print RSSIMIN
  return itoa( *(int8_t *)v , b, 10 );
}


char *callbackRSSIMAX( void *v, char *b, int8_t delta ) {
  // set RSSIMAX
  *(int8_t *)v = 123; // calc current RSSI
  // print RSSIMAX
  return itoa( *(int8_t *)v , b, 10 );
}

char *callbackExit( void *v, char *b, int8_t delta ) {
  if( delta != 0 ) {
    configExit();
  }
  
  return strcpy_P( b, configMsgEXT ); // EXIT msg
}

char *callbackPage( void *v, char *b, int8_t delta ) {
  uint8_t page = *(uint8_t *)v; // either this or configPage
  
  // limit delta to -1..1
  
  // TODO: clean up this mess
  
  if ( delta <= -1 ) {
    if ( page > MINPAGE ) {
      page--;
    }
    else {
      page = MAXPAGE;
    }
    currentInput = configScreens[page].inputsNum - 1; // set cursor to the last input (PAGE)
    *(uint8_t *)v = page; // update page var
  }
  else if ( delta >= 1 ) {
    if ( page < MAXPAGE ) {
      page++;
    }
    else {
      page = MINPAGE;
    }
    currentInput = configScreens[page].inputsNum - 1; // set cursor to the last input (PAGE)
    *(uint8_t *)v = page; // update page var
  }

  //return strcpy_P( b, configMsgPGS ); // PAGE msg
  return itoa( page, b, 10 ); // PAGE num
}

// Display cursor for current input

void displayCursor( void ) {
  uint16_t pos;
  
  /*
  // debug - loop through the config screens
  if( currentInput >= configScreens[configPage].inputsNum ) {
    configPage++;
    currentInput = 0;
    if( configPage > MAXPAGE ) {
      configPage = MINPAGE;
    }
  }
  */
  
  
  if( currentInput >= configScreens[configPage].inputsNum ) {
    return;  // no inputs available on this page
  }
  
  pos = pgm_read_word( &(configScreens[configPage].inputs[currentInput].pos) ) - 1; // input pos - 1
  
  if(Blink10hz) {
    screen[pos] = SYM_CURSOR;
  }
  
}

void cursorMoveUp() {
  uint8_t i = currentInput;
  
  boolean editable = true; // need to store this in array for each input  .inputs[i].editable smth like that
  
  while ( i > 0 ) {
    i--;
    if ( editable ) {
      currentInput = i;
      break;
    }
  }
}

void cursorMoveDown() {
  uint8_t i = currentInput;
  uint8_t lastInput = configScreens[configPage].inputsNum - 1;
  
  boolean editable = true; // need to store this in array for each input  .inputs[i].editable smth like that
  
  while ( i < lastInput ) {
    i++;
    if ( editable ) {
      currentInput = i;
      break;
    }
  }
}

void displayConfigScreen( void ) {
  uint8_t i;
  
  int8_t delta;
  
  uint16_t pos;
  callback_ptr callback;
  void *v;
  
  
  // display labels for current config page
  
  for( i = 0; i < configScreens[configPage].labelsNum; i++ ) {
    strcpy_P( screen + pgm_read_word( &(configScreens[configPage].labels[i].pos) ), (char *)pgm_read_word( &(configScreens[configPage].labels[i].txt) ) );
  }
  
  // display inputs for current config page
  for( i = 0; i < configScreens[configPage].inputsNum; i++ ) {
    
    callback = ( callback_ptr )pgm_read_word( &(configScreens[configPage].inputs[i].callback) );
    v = (void *)pgm_read_word( &(configScreens[configPage].inputs[i].v) );
    pos = (uint16_t)pgm_read_word( &(configScreens[configPage].inputs[i].pos) );
    
    // call callback with delta
    if( currentInput == i ) {
      delta = currentDelta;
    }
    else {
      delta = 0;
    }
    
    if( callback != NULL ) {
      callback( v, screen + pos, delta );
    }
    else {
      // by default, update/display the input as unsigned byte
      callbackByte( v, screen + pos, delta ); 
    }
  }
  
  // tmp - reset delta
  currentDelta = 0;
  
  displayCursor();
}

