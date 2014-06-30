#include <TSL2561.h>
#include <Wire.h>
#include <U8glib.h>

//Sensor polling rate in ms:

#define SENSORPOLLINGRATE 1200

// Luminosity sensor I2C address:

TSL2561 tsl(TSL2561_ADDR_FLOAT);

// U8GLIB device definition:

U8GLIB_SSD1309_128X64 u8g(U8G_I2C_OPT_NONE);

// Pins and global variables assignation.

const int led = 13;
const int magnetOut = 6;

const int buttonOne = 12;
const int buttonTwo = 11;
const int buttonThree = 10;
const int buttonFour = 9;
const int preShutterPin = 8;
const int shutterPin = 7;

int shutterCounter = 3;
String shutterSpeedStatus;

int buttonOneState = 0;
int buttonOneLastState = 0;
int buttonTwoState = 0;
int buttonTwoLastState = 0;
int buttonThreeState = 0;
int buttonThreeLastState = 0;
int buttonFourState = 0;
int buttonFourLastState = 0;

int modeSwitch = 0;
String modeSwitchStatus;
int modeSwitchCounter = 0;

int settingsSwitchStatusIndex = 0;
String settingsSwitchStatus;

uint32_t lux = 0;
unsigned long sensorPollingCounter = 0;

long openLenghtus = 1;
long openLenghtms = 0;

int isoSpeedStatus = 100;
int isoSpeedIndex = 1;

String exposureCompensation;
int exposureCompensationIndex = 4;

int apertureSpeedIndex = 0;
String apertureSpeedStatus = "f/8.8";

int preShutterState;
int preShutterLastState = 0;

int shutterState = 0;
int shutterLastState = 0;

int photoDiode = A0;
int sensorValue;
unsigned long t1,t2;
unsigned long openTime;


// Exposure Compensation icon.

const uint8_t exp_comp_logo[] PROGMEM = {
  0x07, 0xff, 0x04, 0x01, 0x07, 0x09, 0x07, 0x1d, 0x07, 0xc9, 0x07,
  0xc1, 0x07, 0xf1, 0x07, 0xf1, 0x06, 0x3d, 0x07, 0xfd, 0x07, 0xff
};


// This routine is used by u8glib to compose the framebuffer

void draw(void) {
  
  u8g.setFont(u8g_font_6x13);
  u8g.setPrintPos(120,64);
  u8g.print(modeSwitchStatus);
  u8g.setPrintPos(0, 48);
  u8g.print("SH.: ");
  u8g.setPrintPos(25,48);
  u8g.print(shutterSpeedStatus);
  
  // The following options are displayed in Auto mode.
  
  if (modeSwitch == 1){
    u8g.setPrintPos(0, 35);
    u8g.print("AP.: ");
    u8g.setPrintPos(25, 35);
    u8g.print(apertureSpeedStatus);
    u8g.setPrintPos(0, 23);
    u8g.print("ISO: ");
    u8g.setPrintPos(25, 23);
    u8g.print(isoSpeedStatus);
    u8g.setPrintPos(0,60);
    u8g.print(settingsSwitchStatus);
    u8g.setPrintPos(97,11);
    u8g.print(exposureCompensation);
    u8g.drawBitmapP( 79, 0, 2, 11, exp_comp_logo);
    u8g.setPrintPos(0,11);
    u8g.print("LUX: ");
    u8g.setPrintPos(25,11);
    u8g.print(lux);
  }
  
}


void setup() {
  
  Serial.begin(9600);
  
// Pin setup.

  pinMode(led, OUTPUT);
  pinMode(buttonOne, INPUT);
  pinMode(buttonTwo, INPUT);
  pinMode(buttonThree, INPUT);
  pinMode(shutterPin, INPUT);
  pinMode(magnetOut, OUTPUT);
  pinMode(preShutterPin, INPUT);
  
  pinMode(photoDiode, INPUT);
 
// Display settings.

  // flip screen, if required
  //u8g.setRot180();
  // assign default color value

  if ( u8g.getMode() == U8G_MODE_R3G3B2 ) {
    u8g.setColorIndex(255);     // white
  }
  else if ( u8g.getMode() == U8G_MODE_GRAY2BIT ) {
    u8g.setColorIndex(3);         // max intensity
  }
  else if ( u8g.getMode() == U8G_MODE_BW ) {
    u8g.setColorIndex(1);         // pixel on
  }
  else if ( u8g.getMode() == U8G_MODE_HICOLOR ) {
    u8g.setHiColorByRGB(255,255,255);
  }
  
  
// Luminosity sensor settings.

  // You can change the gain on the fly, to adapt to brighter/dimmer light situations
  //tsl.setGain(TSL2561_GAIN_0X);         // set no gain (for bright situtations)
  tsl.setGain(TSL2561_GAIN_16X);      // set 16x gain (for dim situations)
  // Changing the integration time gives you a longer time over which to sense light
  // longer timelines are slower, but are good in very low light situtations!
  tsl.setTiming(TSL2561_INTEGRATIONTIME_13MS);  // shortest integration time (bright light)
  //tsl.setTiming(TSL2561_INTEGRATIONTIME_101MS);  // medium integration time (medium light)
  //tsl.setTiming(TSL2561_INTEGRATIONTIME_402MS);  // longest integration time (dim light)

}


// Pre-shutter routine.

// This is needed because for mechanical reasons the electromagnet has to be on before the shutter release.
// The proper shutter routine only cares about timing and shutter closure.

void preShutter(){
  
  preShutterState = digitalRead(preShutterPin);
  
  if ((preShutterState != preShutterLastState) && (preShutterState == HIGH)){
      digitalWrite(magnetOut, HIGH);
      digitalWrite(led, HIGH);
  }
  
  preShutterLastState = preShutterState;  
  
}

// Shutter routine.

void openShutter(int duratams, unsigned int durataus) {

  shutterState = digitalRead(shutterPin);

  if ((shutterState != shutterLastState) && (shutterState == LOW)){

      
    // Time Mode.
    
    if (duratams == 0 && durataus == 1) {
      
      while (digitalRead(preShutterPin) == HIGH){
      }
      while (digitalRead(preShutterPin) == LOW){
      }
        
      digitalWrite(magnetOut, LOW);
      digitalWrite(led, LOW);
      
    }
    
    // Bulb Mode.
  
    else if (duratams == 0 && durataus == 0) {
      while (digitalRead(preShutterPin) == HIGH){
      }
      
      digitalWrite(magnetOut, LOW);
      digitalWrite(led, LOW);
    }
    
    // Normal Mode (1/1000" to 4").

    else {
      
      while (analogRead(photoDiode) > 50){}
      t1 = micros();
      
      delay(duratams);
      delayMicroseconds(durataus);
      
      digitalWrite(magnetOut, LOW);
      digitalWrite(led, LOW);
      while (analogRead(photoDiode) < 50){}
      t2 = micros();
      
      openTime = (t2 - t1);
  
      Serial.print("Durata: ");
      Serial.print(openTime);
      Serial.print("ms");
    } 
  }
 
  shutterLastState = shutterState;
}  


// Get shutter speed by luminosity.

int shutterSpeedByLux(int lux){

  if (lux > 0 && lux <= 100) {
    shutterCounter = 39;
  }
  else if (lux > 100 && lux <= 200) {
    shutterCounter = 36;
  }
  else if (lux > 200 && lux <= 300) {
    shutterCounter = 33;
  }
  else if (lux > 300 && lux <= 400) {
    shutterCounter = 30;
  }
  else if (lux > 400 && lux <= 500) {
    shutterCounter = 27;
  }
  else if (lux > 500 && lux <= 600) {
    shutterCounter = 24;
  }
  else if (lux > 600 && lux <= 700) {
    shutterCounter = 21;
  }
  else if (lux > 700 && lux <= 800) {
    shutterCounter = 18;
  }
  else if (lux > 800 && lux <= 900) {
    shutterCounter = 15;
  }
  else if (lux > 900 && lux <= 1000) {
    shutterCounter = 12;
  }
  else if (lux > 1000 && lux <= 1100) {
    shutterCounter = 9;
  }
  else if (lux > 1100 && lux <= 1200) {
    shutterCounter = 6;
  }
  else if (lux > 1200 && lux <= 1300) {
    shutterCounter = 3;
  }
  else if (lux > 1300 && lux <= 1400) {
    shutterCounter = 0;
  }
  else {
    shutterCounter = 999;
  }
  
  return shutterCounter;
}


// ISO Speed Compensation.

int isoCompensationByIndex(int isoSpeedIndex){
  
  int isoSpeedCompensationValue;

  switch (isoSpeedIndex){
  case 0:
    isoSpeedCompensationValue = -15;
    isoSpeedStatus = 3000;
    break;
  case 1:
    isoSpeedCompensationValue = 0;
    isoSpeedStatus = 100;
    break;
  }
  
  return isoSpeedCompensationValue;
}


// Aperture Compensation.

int apertureCompensationByIndex(int scelta){
  
  int apertureSpeedCompensationValue = 0;

  switch (scelta){
  
  case 0:
    apertureSpeedStatus = "f/8.8";
    apertureSpeedCompensationValue = 0;
    break;
  case 1:
    apertureSpeedStatus = "f/12.5";
    apertureSpeedCompensationValue = 3;
    break;
  case 2:
    apertureSpeedStatus = "f/17.5";
    apertureSpeedCompensationValue = 6;
    break;
  case 3:
    apertureSpeedStatus = "f/25";
    apertureSpeedCompensationValue = 9;
    break;
  case 4:
    apertureSpeedStatus = "f/35";
    apertureSpeedCompensationValue = 12;
    break;
  case 5:
    apertureSpeedStatus = "f/50";
    apertureSpeedCompensationValue = 15;
    break;
  } 
  
  return apertureSpeedCompensationValue;
}


// Exposure Compensation.

int exposureCompensationByIndex(int exposureCompensationIndex){
  
  int exposureCompensationValue = 0;

  switch(exposureCompensationIndex){
    case 1:
      exposureCompensation = "-1";
      exposureCompensationValue = -3;
      break;
    case 2:
      exposureCompensation = "-2/3";
      exposureCompensationValue = -2;
      break;
    case 3:
      exposureCompensation = "-1/3";
      exposureCompensationValue = -1;
      break;
    case 4:
      exposureCompensation = "0";
      exposureCompensationValue = 0;
      break;
    case 5:
      exposureCompensation = "+1/3";
      exposureCompensationValue = 1;
      break;
    case 6:
      exposureCompensation = "+2/3";
      exposureCompensationValue = 2;
      break;
    case 7:
      exposureCompensation = "+1";
      exposureCompensationValue = 3;
      break;
   }
   
   return exposureCompensationValue;
}


// Shutter speed setting routine. Invoking the delayMicroseconds function whit 0
// as a parameter cause an effective delay of about 16000us, so i never set openLenghtus to 0 but to 1.

void shutterSpeed(int scelta){

  switch (scelta) {

  default:
    shutterSpeedStatus = "ERR";
    break;
  case 0:
    shutterSpeedStatus = "1/125\"";
    openLenghtms = 8;
    openLenghtus = 1;
    break;
  case 1:
    shutterSpeedStatus = "1/100\"";
    openLenghtms = 10;
    openLenghtus = 1;
    break;
  case 2:
    shutterSpeedStatus = "1/80\"";
    openLenghtms = 12;
    openLenghtus = 500;
    break;
  case 3:
    shutterSpeedStatus = "1/60\"";
    openLenghtms = 16;
    openLenghtus = 666;
    break;
  case 4:
    shutterSpeedStatus = "1/50\"";
    openLenghtms = 20;
    openLenghtus = 1;
    break;
  case 5:
    shutterSpeedStatus = "1/40\"";
    openLenghtms = 25;
    openLenghtus = 1;
    break;
  case 6:
    shutterSpeedStatus = "1/30\"";
    openLenghtms = 33;
    openLenghtus = 333;
    break;
  case 7:
    shutterSpeedStatus = "1/25\"";
    openLenghtms = 40;
    openLenghtus = 1;
    break;
  case 8:
    shutterSpeedStatus = "1/20\"";
    openLenghtms = 50;
    openLenghtus = 1;
    break;
  case 9:
    shutterSpeedStatus = "1/15\"";
    openLenghtms = 66;
    openLenghtus = 666;
    break;
  case 10:
    shutterSpeedStatus = "1/13\"";
    openLenghtms = 76;
    openLenghtus = 923;
    break;
  case 11:
    shutterSpeedStatus = "1/10\"";
    openLenghtms = 100;
    openLenghtus = 1;
    break;
  case 12:
    shutterSpeedStatus = "1/8\"";
    openLenghtms = 125;
    openLenghtus = 1;
    break;
  case 13:
    shutterSpeedStatus = "1/6\"";
    openLenghtms = 166;
    openLenghtus = 666;
    break;
  case 14:
    shutterSpeedStatus = "1/5\"";
    openLenghtms = 200;
    openLenghtus = 1;
    break;
  case 15:
    shutterSpeedStatus = "1/4\"";
    openLenghtms = 250;
    openLenghtus = 1;
    break;
  case 16:
    shutterSpeedStatus = "1/3\"";
    openLenghtms = 333;
    openLenghtus = 333;
    break;
  case 17:
    shutterSpeedStatus = "1/2.5\"";
    openLenghtms = 400;
    openLenghtus = 1;
    break;
  case 18:
    shutterSpeedStatus = "1/2\"";
    openLenghtms = 500;
    openLenghtus = 1;
    break;
  case 19:
    shutterSpeedStatus = "1/1.6\"";
    openLenghtms = 625;
    openLenghtus = 1;
    break;
  case 20:
    shutterSpeedStatus = "1/1.3\"";
    openLenghtms = 769;
    openLenghtus = 230;
    break;
  case 21:
    shutterSpeedStatus = "1\"";
    openLenghtms = 1000;
    openLenghtus = 1;
    break;
  case 22:
    shutterSpeedStatus = "1.3\"";
    openLenghtms = 1300;
    openLenghtus = 1;
    break;
  case 23:
    shutterSpeedStatus = "1.6\"";
    openLenghtms = 1600;
    openLenghtus = 1;
    break;
  case 24:
    shutterSpeedStatus = "2\"";
    openLenghtms = 2000;
    openLenghtus = 1;
    break;
  case 25:
    shutterSpeedStatus = "2.5\"";
    openLenghtms = 2500;
    openLenghtus = 1;
    break;
  case 27:
    shutterSpeedStatus = "3\"";
    openLenghtms = 3000;
    openLenghtus = 1;
    break;
  case 30:
    shutterSpeedStatus = "4\"";
    openLenghtms = 4000;
    openLenghtus = 1;
    break;
  case 33:
    shutterSpeedStatus = "Bulb";
    openLenghtms = 0;
    openLenghtus = 0;
    break;
  case 36:
    shutterSpeedStatus = "Time";
    openLenghtms = 0;
    openLenghtus = 30;
    break;
  }
  
  // compensation of aperture time
  if (openLenghtms != 0){
    openLenghtms -= 7;
    openLenghtus += 400;
  } 

}


// Luminosity Analysis.

uint32_t getLuxLuminosity(){

  if ((millis() > 700) && ((millis() - sensorPollingCounter) > SENSORPOLLINGRATE)) {
    sensorPollingCounter = millis();
    uint32_t lum = tsl.getFullLuminosity();
    uint16_t ir, full;
    ir = lum >> 16;
    full = lum & 0xFFFF;
    lux = tsl.calculateLux(full, ir);
  }
  else {
    lux = lux;
  }
  
  return lux;
}


void loop() {
  

// Picture loop. This is how U8GLIB transfers to the display the framebuffer defined in the draw() function.

  if (digitalRead(led) == LOW){
  
    u8g.firstPage();  
    do {
      draw();
    } 
    while( u8g.nextPage() );
  }
  
// Working mode selection (Auto or Manual).

  buttonThreeState = digitalRead(buttonThree);

  if ((buttonThreeState != buttonThreeLastState) && (buttonThreeState == HIGH)){
    modeSwitch = !modeSwitch;
  }
  
  buttonThreeLastState = buttonThreeState;
  
  switch (modeSwitch){
    
    case 0:
    
      modeSwitchStatus = "M";
      modeSwitchCounter = 0;
      break;
      
    case 1:
    
      modeSwitchStatus = "A";
    
      if (modeSwitchCounter == 0){
        exposureCompensationIndex = 4;
        modeSwitchCounter++;
      }
      break;
  }
   
  
// Toggle Shutter, Aperture, ISO settings and Exposure Compensation to be modified.
  
  buttonFourState = digitalRead(buttonFour);

  if ((buttonFourState != buttonFourLastState) && (buttonFourState == HIGH)){
    settingsSwitchStatusIndex++;
    }
    
  if (settingsSwitchStatusIndex > 2){
    settingsSwitchStatusIndex = 0;
  }
  
  buttonFourLastState = buttonFourState;
  
  // In Manual mode, only Shutter speed is shown editable.
  
  if (modeSwitch == 0) {
 
        settingsSwitchStatusIndex = 3;
        settingsSwitchStatus = "Shutt.";
        
   }
  
  // In Auto Mode, Aperture, ISO and Exposure Compensation are shown and editable.
  
  else {
  
    switch (settingsSwitchStatusIndex){
    
      case 0:
        
        settingsSwitchStatus = "Aper.";
        break;
        
      case 1:
        
        settingsSwitchStatus = "ISO";
        break;
        
      case 2:
        
        settingsSwitchStatus = "ExpComp.";
        break;
        
    }
  }
  
  
// Configuration of buttonOne and buttonTwo for value modification.

  // ButtonOne.
  
  buttonOneState = digitalRead(buttonOne);

  if ((buttonOneState != buttonOneLastState) && (buttonOneState == HIGH)){
    
    switch (settingsSwitchStatusIndex){
      
      // Aperture.
      
      case 0:

        apertureSpeedIndex++;
        if (apertureSpeedIndex > 5) {
          apertureSpeedIndex = 5;
        }
        break;
        
      // ISO.
        
      case 1:
      
        isoSpeedIndex = !isoSpeedIndex;
        break;
      
      // Exposure Compensation.
      
      case 2:
        
        exposureCompensationIndex++;
        if (exposureCompensationIndex > 7) {
          exposureCompensationIndex = 7;
        }
        break;
        
      // Shutter.
        
      case 3:
      
        shutterCounter += 3;
        if (shutterCounter > 36) {
          shutterCounter = 36;
        }
        break;

    }
    
  }
  
    
  buttonOneLastState = buttonOneState;

  // ButtonTwo.
  
  buttonTwoState = digitalRead(buttonTwo);

  if ((buttonTwoState != buttonTwoLastState) && (buttonTwoState == HIGH)){
    
    switch (settingsSwitchStatusIndex){
      
      // Aperture.
      
      case 0:

        apertureSpeedIndex--;
        if (apertureSpeedIndex < 0) {
          apertureSpeedIndex = 0;
        }
        break;
        
      // ISO.
        
      case 1:
      
        isoSpeedIndex = !isoSpeedIndex;
        break;
        
      // Exposure Compensation.
        
      case 2:
        
        exposureCompensationIndex--;
        if (exposureCompensationIndex < 1) {
          exposureCompensationIndex = 1
          
          
          ;
        }
        break;
        
      // Shutter.
        
      case 3:
      
        shutterCounter -= 3;
        if (shutterCounter < 0) {
          shutterCounter = 0;
        }
        break;

    }
    
  }

  buttonTwoLastState = buttonTwoState;
  

// Main run.
  
  // If Mode is Auto:
  
  if (modeSwitch == 1){
    shutterSpeed(shutterSpeedByLux(getLuxLuminosity()) + isoCompensationByIndex(isoSpeedIndex) + exposureCompensationByIndex(exposureCompensationIndex) + apertureCompensationByIndex(apertureSpeedIndex));  
  }
  
  // If mode is Manual:
  
  else if (modeSwitch == 0){
    shutterSpeed(shutterCounter);
  }
  
  // Preshutter and Shutter.
  
  preShutter(); 
  openShutter(openLenghtms,openLenghtus);


} 
