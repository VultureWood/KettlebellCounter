#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#define CE_PIN  27
#define CSN_PIN 26  
#define CLK_SPI 18 // 18
#define MISO 19 // 19
#define MOSI 23 //23
#define SS 15 // HSPI 15 // Yes same pin as CSN, but it doesn't seem to matter.

// Corrected 29012019 for my use on ESP32. HSPI //EHB
#define _cs  17  // goes to TFT CS (17)
#define _dc  16  // goes to TFT DC (16)
#define _mosi 23  // goes to TFT MOSI (23)
#define _sclk 18  // goes to TFT SCK/CLK (18)
#define _rst  5   // goes to TFT RESET (5)
#define _miso 19   // Not connected
//       3.3V     // Goes to TFT LED  
//       5v       // Goes to TFT Vcc
//       Gnd      // Goes to TFT Gnd     

Adafruit_ILI9341 tft = Adafruit_ILI9341(_cs, _dc, _rst);

// Butten and buzzer pins
#define repButtonUP 34
#define buttonPin 35    // select/menu
#define repButtonDOWN 32
#define channelSwitch 33

// Variables for general functions.
int8_t countState = LOW;         // the current state of the output pin
int8_t buttonState;             // the current reading from the input pin
int8_t lastButtonState = LOW;   // the previous reading from the input pin
int8_t buttonStateRepUP;             // the current reading from the input pin
int8_t lastButtonStateRepUP = LOW; 
int8_t buttonStateRepDOWN;             // the current reading from the input pin
int8_t lastButtonStateRepDOWN = LOW; 
int8_t readingRepUP;
int8_t readingRepDOWN;
int8_t reading;
int8_t lastButtonStateRepDOWNRESET; 
int8_t buttonStateRepDOWNRESET;
int8_t readingRepDOWNRESET;
uint8_t TRANSMITMenu =0;
uint8_t InitializationMENU = 0;
uint8_t Menu =1; 
uint8_t choiceMade =0;
uint8_t Select =0;
uint8_t CountUP = 0;
uint8_t CountDOWN = 0;
uint8_t Pentathlon = 0;
uint8_t doOnceCount = 1;
uint8_t setCountdownTimer =1 ;
uint8_t doOnceTRANSMITMENU =1;
uint8_t SetTimerMenu =0;
uint8_t runningDiscipline = 0;
uint8_t doOncerunningDiscipline =1;
uint8_t countDowntoStart = 1;
int8_t CDT_hh_1 = 0;
int8_t CDT_mm_10 = 0;
int8_t CDT_mm_1 = 0;
int8_t CDT_ss_10 = 0;
int8_t CDT_ss_1 = 0;
int8_t CDT_menu = 1;
int8_t CDT_menu_Last =0;
int8_t CDT_var_val =0;
int8_t hh_1_old = 10;
int8_t mm_10_old = 10;
int8_t mm_1_old = 10;
int8_t ss_10_old = 10;
int8_t ss_1_old = 10;
int i ;
int j;
int lineSetCDT = 140; // line definition for bars
int radioTXitterations = 10;
uint8_t fourDigits = 0;
uint8_t doOnceTimer = 1;
unsigned long oldMasterMillis = 0;
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 20;    // the debounce time; increase if the output flickers
unsigned long timeHolder2;
unsigned long timeHolder1;
long startMillis;
long currentMillis;
long difference_millis;
unsigned long oldTimerForTX = 0;
unsigned long mm_10 = 0;
unsigned long mm_1 = 0;
unsigned long hh_10 = 0; 
unsigned long hh_1 = 0;
unsigned long ss_10 = 0;
unsigned long ss_1 = 0;
long hours_10 = 36000000;
long hours_1 = 3600000;
long minutes_10 = 600000;
long minutes_1 = 60000;
long seconds_10 = 10000;
long seconds_1 = 1000;

//////////////////////////////////////////////////////////
//
//  RADIO SPECIFIC VARIABLES ETC
//
//////////////////////////////////////////////////////////

bool doOnceRadio = true;
RF24 radio(CE_PIN,CSN_PIN);

const uint64_t pipe1 = 0xE8E8EEF0A1LL;
const uint64_t pipe2 = 0xE8E8EEF0A2LL;  
const uint64_t pipe3 = 0xE8E8EEF0A3LL;
const uint64_t pipe4 = 0xE8E8EEF0A4LL;
const uint64_t pipe5 = 0xE8E8EEF0A5LL;
const uint64_t pipe6 = 0xE8E8EEF0A6LL;
  


typedef struct {
  char filterChar[1];
  bool startBit;
  bool reset1;
  uint8_t setDiscipline;
  unsigned long setDuration;
  unsigned long masterMillis;
  bool returnResult;
  uint16_t result;
  bool switchChannel;
  bool reset2;
  } TXstruct;

  TXstruct message;


////////////////////////////////////////////////
//
//  SETUP
//
////////////////////////////////////////////////

void setup() {
 
  //Serial.begin(115200);

   pinMode(repButtonUP, INPUT); // Counter increment
  pinMode(repButtonDOWN, INPUT); // Counter decrement
  pinMode(buttonPin, INPUT); // Start/Stop
  pinMode(channelSwitch, INPUT );

 
  tft.begin();
  tft.setRotation(3);
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextSize(3);
  tft.println("Starting...");
  delay(1000);
  MainText();
  delay(1000);
 PrintMainMenu();
} 

///////////////////////////////////
//
// main LOOP
//
/////////////////////////////////

void loop() {


buttonPressedLoopTOP(); // TOP section for buttons

  if(Menu>4 ){  Menu = 1;  } // KEEPS MENU WITHIN BOUNDS
  if(Menu<1){  Menu = 4;  }


 
if(Menu==1 && !choiceMade){
  tft.setTextSize(4);
  tft.fillRect(10, 125, 20 ,30, ILI9341_WHITE) ; // overwrite the text with white
  tft.fillRect(10, 165, 20 ,30, ILI9341_WHITE) ; // overwrite the text with white
  tft.fillRect(10, 205, 20 ,30, ILI9341_WHITE) ; // overwrite the text with white
  tft.setTextColor(ILI9341_RED);
  tft.setCursor(10, 85); 
  tft.println("*");
 if(Select){
    message.setDiscipline = 1;
    CountDOWN = 1;
    choiceMade = 1;
    SetTimerMenu =1;
   }
   }

   if(Menu==2 && !choiceMade){
    tft.setTextSize(4);
  tft.fillRect(10, 85, 20 ,30, ILI9341_WHITE) ; // overwrite the text with white
  tft.fillRect(10, 165, 20 ,30, ILI9341_WHITE) ; // overwrite the text with white
  tft.fillRect(10, 205, 20 ,30, ILI9341_WHITE) ; // overwrite the text with white
  tft.setTextColor(ILI9341_RED);
  tft.setCursor(10, 125); 
  tft.println("*");
  if(Select){
    message.setDiscipline = 2;
    CountUP = 1;
    choiceMade = 1;
    SetTimerMenu =1;
    }
   }


   if(Menu==3 && !choiceMade){
    tft.setTextSize(4);
  tft.fillRect(10, 85, 20 ,30, ILI9341_WHITE) ; // overwrite the text with white
  tft.fillRect(10, 125, 20 ,30, ILI9341_WHITE) ; // overwrite the text with white
  tft.fillRect(10, 205, 20 ,30, ILI9341_WHITE) ; // overwrite the text with white
  tft.setTextColor(ILI9341_RED);
  tft.setCursor(10, 165); 
  tft.println("*");
  if(Select){
    Pentathlon = 1;
    choiceMade = 1;
    InitializationMENU = 1;
    timeHolder1 = 3000000; // set time to 50 min
    message.setDiscipline = 3;   // set discipline to pentathlon
    }
   }

      if(Menu==4 && !choiceMade){
    tft.setTextSize(4);
  tft.fillRect(10, 85, 20 ,30, ILI9341_WHITE) ; // overwrite the text with white
  tft.fillRect(10, 125, 20 ,30, ILI9341_WHITE) ; // overwrite the text with white
  tft.fillRect(10, 165, 20 ,30, ILI9341_WHITE) ; // overwrite the text with white
  tft.setTextColor(ILI9341_RED);
  tft.setCursor(10, 205); 
  tft.println("*");
  if(Select){
    choiceMade = 1;
  resettingFunction();
    }
   }

//////////////////////////////////////
//
// Set timer menu
//
/////////////////////////////////////
while(SetTimerMenu){

if(doOnceCount){
  
 
  tft.fillScreen(ILI9341_WHITE);
  tft.setTextSize(3);
  tft.setCursor(10, 10);
  tft.setTextColor(ILI9341_BLACK);
  if( message.setDiscipline==1){
     tft.setCursor(10, 10);
     tft.println("COUNT DOWN TIMER");
     }
  if( message.setDiscipline==2){
     tft.setCursor(30, 10);
     tft.println("COUNT UP TIMER"); 
     }
  
  tft.setTextSize(2);
  tft.setCursor(30, 50);
  tft.println("SELECT TIMER DURATION");
   tft.setCursor(40, 180); 
  tft.println("PRESS and HOLD 'DOWN'");
  tft.setCursor(120, 210); 
  tft.println("TO RESET");
   tft.fillRect(50, lineSetCDT, 25 ,4 ,ILI9341_RED); // SET RED BAR under first digit

   
  doOnceCount = 0;
  }

 // while(setCountdownTimer){
  
  // Select button
   reading = digitalRead(buttonPin);
  if (reading != lastButtonState) {
    // reset the debouncing timer
    lastDebounceTime = millis();
      }
  if ((millis() - lastDebounceTime) > debounceDelay) {
     if (reading != buttonState) {
      buttonState = reading;
     if (buttonState == HIGH) {
        CDT_menu++;
        if(CDT_menu>6){
          CDT_menu =1;}
          
          if(CDT_menu != CDT_menu_Last){
            if(CDT_menu == 1){
              tft.fillRect(30, 150, 300 ,90, ILI9341_WHITE); // overwrite text
              CDT_var_val = CDT_hh_1 ;
              tft.fillRect(50, lineSetCDT, 25 ,4 ,ILI9341_RED); // SET RED BAR
              tft.setTextSize(2);
              tft.setTextColor(ILI9341_BLACK);
                  tft.setCursor(40, 180); 
                  tft.println("PRESS and HOLD 'DOWN'");
                  tft.setCursor(120, 210); 
                  tft.println("TO RESET");
                           }
           if(CDT_menu == 2){
             CDT_var_val = CDT_mm_10;
             tft.fillRect(50, lineSetCDT, 25 ,4, ILI9341_WHITE); // SET WHITE BAR
             tft.fillRect(100, lineSetCDT,25 ,4, ILI9341_RED); // SET RED BAR
            }
           if(CDT_menu == 3){
            CDT_var_val = CDT_mm_1 ;
             tft.fillRect(100, lineSetCDT, 25 ,4, ILI9341_WHITE); // SET WHITE BAR
             tft.fillRect(140, lineSetCDT, 25 ,4, ILI9341_RED); // SET RED BAR
            }
            if(CDT_menu == 4){
              CDT_var_val = CDT_ss_10;
             tft.fillRect(140, lineSetCDT, 25 ,4, ILI9341_WHITE); // SET WHITE BAR
             tft.fillRect(190, lineSetCDT, 25 ,4, ILI9341_RED); // SET RED BAR
             }
           if(CDT_menu == 5){
              CDT_var_val = CDT_ss_1;
             tft.fillRect(190, lineSetCDT, 25 ,4, ILI9341_WHITE); // SET WHITE BAR
             tft.fillRect(230, lineSetCDT, 25 ,4, ILI9341_RED); // SET RED BAR
              }
              if(CDT_menu == 6){
              tft.fillRect(230, lineSetCDT, 25 ,4, ILI9341_WHITE); // SET WHITE BAR
              tft.fillRect(40, 180, 300 ,60, ILI9341_WHITE); // SET WHITE BAR
              
               tft.setTextColor(ILI9341_RED);
               tft.setTextSize(3);
              tft.setCursor(80, 170);
              tft.println("PRESS UP");
              tft.setCursor(30, 200);
              tft.println("to confirm TIME");

              // if (button pressed) { setCountdownTimer = 0;}
                }
            
                          
             
            }
            CDT_menu_Last =  CDT_menu;
                    }
                }
  }

 // Button debounce CD-timer UP
   readingRepUP = digitalRead(repButtonUP);
  if (readingRepUP != lastButtonStateRepUP) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }
  if ((millis() - lastDebounceTime) > debounceDelay) {
     if (readingRepUP != buttonStateRepUP) {
      buttonStateRepUP = readingRepUP;
     if (buttonStateRepUP == HIGH) {
       CDT_var_val++;
       if(CDT_menu == 1 || CDT_menu == 3 || CDT_menu == 5 ){
        if(CDT_var_val>9){
          CDT_var_val = 0;}
          }
           if(CDT_menu == 2 || CDT_menu == 4 ){
        if(CDT_var_val>5){
          CDT_var_val = 0;}
          }
          if(CDT_menu == 1){
   CDT_hh_1 = CDT_var_val;
    }
    if(CDT_menu == 2){
   CDT_mm_10 = CDT_var_val;
    }
    if(CDT_menu == 3){
   CDT_mm_1 = CDT_var_val;
    }
    if(CDT_menu == 4){
   CDT_ss_10 = CDT_var_val;
    }
    if(CDT_menu == 5){
   CDT_ss_1 = CDT_var_val;
    }

    if(CDT_menu ==6 ){ // Starts CDT
      SetTimerMenu = 0;
      InitializationMENU =1;
      doOnceCount = 1;
      }
     }
   }
}
// Button debounce CD-timer DOWN
 readingRepDOWN = digitalRead(repButtonDOWN);
 readingRepDOWNRESET = digitalRead(repButtonDOWN);
  if (readingRepDOWN != lastButtonStateRepDOWN) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }

  if (readingRepDOWNRESET != lastButtonStateRepDOWNRESET) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }

  // allowing for reset via the down button
   if ((millis() - lastDebounceTime) > 3000) {
     if (readingRepDOWNRESET != buttonStateRepDOWNRESET) {
      buttonStateRepDOWNRESET = readingRepDOWNRESET;
     if (buttonStateRepDOWNRESET == HIGH) {
      ESP.restart();
      //resettingFunction();
    }
     }
   }else{
  if ((millis() - lastDebounceTime) > debounceDelay) {
     if (readingRepDOWN != buttonStateRepDOWN) {
      buttonStateRepDOWN = readingRepDOWN;
     if (buttonStateRepDOWN == HIGH) {
       CDT_var_val--;
       if(CDT_menu == 1 || CDT_menu == 3 || CDT_menu == 5 ){
       if(CDT_var_val<0){
          CDT_var_val =9;}
          }
           if(CDT_menu == 2 || CDT_menu == 4){
       if(CDT_var_val<0){
          CDT_var_val =5;}
          }
    if(CDT_menu == 1){
   CDT_hh_1 = CDT_var_val;
    }
    if(CDT_menu == 2){
   CDT_mm_10 = CDT_var_val;
    }
    if(CDT_menu == 3){
   CDT_mm_1 = CDT_var_val;
    }
    if(CDT_menu == 4){
   CDT_ss_10 = CDT_var_val;
    }
    if(CDT_menu == 5){
   CDT_ss_1 = CDT_var_val;
    }
          }
   }
}
}



 
timeHolder1 = GatherCDT(CDT_hh_1, CDT_mm_10, CDT_mm_1,CDT_ss_10,CDT_ss_1);
time_fractionCountdown(timeHolder1,100); // 100 is the line it's written on



  buttonPressedLoopBOTTOM(); //Bottom section for buttons

  } // end of while(SetTimerMenu)


/////////////////////////////////////////////
//
//  INITITALIZATION MENU
//
////////////////////////////////////////////

  while(InitializationMENU){

if(doOnceCount){
  doOnceTimer =1 ;
   if(timeHolder1 >= 3600000){ // IF timer is larger than 1 hour, set 5 digits
           doOnceTimer=1;
           }
            if(timeHolder1 < 3600000){ // IF timer is less than 1 hour, set 4 digits
           fourDigits=1;
           doOnceTimer=1;
           }
    tft.fillScreen(ILI9341_WHITE);
  tft.setTextSize(3);
  tft.setCursor(25, 10);
  tft.setTextColor(ILI9341_BLACK);
  tft.println("CONFIRM CHOICE");
  
  tft.setCursor(20, 50);
  tft.setTextColor(ILI9341_BLACK);
  tft.println("TIMER:");
  
  tft.setCursor(135, 50);
  tft.setTextColor(ILI9341_RED);
  
  if(message.setDiscipline == 1){
    tft.println("COUNT DOWN");
    }
if(message.setDiscipline == 2){
    tft.println("COUNT UP");
    }
if(message.setDiscipline == 3){
    tft.println("PENTATHLON");
    }
    
    tft.setTextColor(ILI9341_BLACK);
   tft.setCursor(90, 90);
   tft.println("DURATION" );
   
  time_fractionCountdown(timeHolder1,130); // 130 is the line it's written on
  
  tft.setTextSize(3);
  tft.setTextColor(ILI9341_GREEN);
  tft.setCursor(90, 190);
  tft.println("YES" );
  tft.setTextColor(ILI9341_BLACK);
  tft.setCursor(150, 190);
  tft.println("/" );
  tft.setTextColor(ILI9341_RED);
  tft.setCursor(180, 190);
  tft.println("NO" );
  CDT_menu = 1; // setting the menu select to default Yes
  doOnceCount = 0;
  }


if(CDT_menu ==1){
    tft.fillRect(180, 215, 35 ,4, ILI9341_WHITE); // SET WHITE BAR
    tft.fillRect(90, 215, 50 ,4, ILI9341_RED); // SET RED BAR
    }

if(CDT_menu ==2){
    tft.fillRect(90, 215, 50 ,4, ILI9341_WHITE); // SET WHITE BAR
    tft.fillRect(180, 215, 35 ,4, ILI9341_RED); // SET RED BAR
    }
    
  // Select button
   reading = digitalRead(buttonPin);
  if (reading != lastButtonState) {
    // reset the debouncing timer
    lastDebounceTime = millis();
      }
  if ((millis() - lastDebounceTime) > debounceDelay) {
     if (reading != buttonState) {
      buttonState = reading;
     if (buttonState == HIGH) {
        
        if(CDT_menu==1){
          InitializationMENU = 0;
          CountDOWN =0;
          TRANSMITMenu =1;  // go to the TX loop
          message.setDuration = timeHolder1;
          }
          
    if(CDT_menu==2){
          ESP.restart();  // restart the esp and reprogram
          }
            
                    }
                }
  }

 // Button debounce CD-timer UP
   readingRepUP = digitalRead(repButtonUP);
  if (readingRepUP != lastButtonStateRepUP) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }
  if ((millis() - lastDebounceTime) > debounceDelay) {
     if (readingRepUP != buttonStateRepUP) {
      buttonStateRepUP = readingRepUP;
     if (buttonStateRepUP == HIGH) {
      CDT_menu++;
      if(CDT_menu > 2){
   CDT_menu = 1;
    }
   
     }
   }
}
// Button debounce CD-timer DOWN
 readingRepDOWN = digitalRead(repButtonDOWN);
  if (readingRepDOWN != lastButtonStateRepDOWN) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }
  if ((millis() - lastDebounceTime) > debounceDelay) {
     if (readingRepDOWN != buttonStateRepDOWN) {
      buttonStateRepDOWN = readingRepDOWN;
     if (buttonStateRepDOWN == HIGH) {
       CDT_menu--;
          if(CDT_menu < 1){
   CDT_menu = 2;
    }
    }
     }
   }


      buttonPressedLoopBOTTOM(); //Bottom section for buttons
    } // end of while(InitializationMENU)

//////////////////////////////////////
//
// Transmitting menu and loop
//
/////////////////////////////////////



while(TRANSMITMenu){

   if(doOnceTRANSMITMENU){
    tft.fillScreen(ILI9341_WHITE);
  tft.setTextSize(3);
 
  tft.setTextColor(ILI9341_BLACK);

    if(message.setDiscipline == 1){ tft.setCursor(15, 10); tft.println("COUNT DOWN TIMER"); }
    if(message.setDiscipline == 2){ tft.setCursor(30, 10); tft.println("COUNT UP TIMER");   }
    if(message.setDiscipline == 3){ tft.setCursor(15, 10); tft.println("PENTATHLON TIMER"); }

     if(doOnceRadio){  // starting up the radio (yes there is redundance in doOnce functions)
     RadioStartUp(); // calling the start up function for the radio
    //radio.printDetails();
    // sets discipline and duration, but doesn't start yet
    fillMessage( false, false, message.setDiscipline, message.setDuration, millis(),true, 135, false, false); // start, reset1, discipline, duration, master millis, return result, result, switch channel, reset2
       for(i=0; i<radioTXitterations;i++){
         radio.write(&message, sizeof(message));
         }
     doOnceRadio = false;
        }


  tft.setTextSize(3);
  tft.setTextColor(ILI9341_RED);
  tft.setCursor(50, 80); 
  tft.println("PRESS 'START'");
  tft.setCursor(90, 110); 
  tft.println("TO BEGIN");
  tft.setTextSize(2);
  tft.setTextColor(ILI9341_BLACK);
  tft.setCursor(40, 180); 
  tft.println("PRESS and HOLD 'DOWN'");
  tft.setCursor(120, 210); 
  tft.println("TO RESET");
  delay(4000); // ensures that start command isn't given to early
     doOnceTRANSMITMENU = 0;
  }
  // this loop only waits for the start command
  
  // Select button
   reading = digitalRead(buttonPin);
  if (reading != lastButtonState) {
    // reset the debouncing timer
    lastDebounceTime = millis();
      }
  if ((millis() - lastDebounceTime) > debounceDelay) {
     if (reading != buttonState) {
      buttonState = reading;
     if (buttonState == HIGH) {
        runningDiscipline = 1; // start the timing loop
        TRANSMITMenu = 0; // end the transmit menu
        
        // when the start command is given, the transmitter sends 10 times
        
        int theStartVariable = true;
        fillMessage( theStartVariable, false, message.setDiscipline, message.setDuration, millis(),true, 135, false, false); // start, reset1, discipline, duration, master millis, return result, result, switch channel, reset2
         for(i=0; i<radioTXitterations;i++){
         radio.write(&message, sizeof(message));
         }
       }
       }
  }
// Hold for 3 seconds and ESP will restart
// Button debounce for DOWN button
 readingRepDOWN = digitalRead(repButtonDOWN);
  if (readingRepDOWN != lastButtonStateRepDOWN) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }
  if ((millis() - lastDebounceTime) > 3000) {
     if (readingRepDOWN != buttonStateRepDOWN) {
      buttonStateRepDOWN = readingRepDOWN;
     if (buttonStateRepDOWN == HIGH) {
      ESP.restart();
    }
     }
   }

 buttonPressedLoopBOTTOM(); //Bottom section for buttons
} // end of transmit menu


  while(runningDiscipline){

       if(doOncerunningDiscipline){
    // important that this is prior to the tft functions, bacause of the shift from hardware to software spi, which makes the tft much slower
    fillMessage( false, false, message.setDiscipline, 0, millis(),true, 135, false, false); // start, reset1, discipline, duration, master millis, return result, result, switch channel, reset2
    timeHolder2 = 6100; // compensates for the initial count down to start function 6500 millis, but adjust to make timers seem syncronous
    
    tft.fillRect(50, 80, 250 ,60, ILI9341_WHITE ); // overwrite the text with white
    tft.setTextColor(ILI9341_RED);
    tft.setTextSize(2);
        tft.setCursor(40, 60); 
        tft.println("COUNT DOWN TO START");
    

     if(timeHolder1 >= 3600000){ // IF timer is larger than 1 hour, set 5 digits
           doOnceTimer=1;
           }
            if(timeHolder1 < 3600000){ // IF timer is less than 1 hour, set 4 digits
           fourDigits=1;
           doOnceTimer=1;
           }
           startMillis = millis(); // ensures that the start time is saved
           
           // a small count down loop
           while(countDowntoStart){
          currentMillis = (millis()- startMillis)*1;  //To accelerate time while testing, set multiplyer here

        if(timeHolder2 >= currentMillis){
         difference_millis = timeHolder2-currentMillis;
       if(difference_millis >0){                                  /*  The actual count down loop!*/

        time_fractionCountdown(difference_millis, 80);      // 80 is the line the TIME written on
             }
       }
       if(timeHolder2 < currentMillis){
        countDowntoStart = 0;
        }
            } // end of while(countDowntoStart){
       
      
       startMillis = millis(); // ensures that the start time is saved
       tft.fillRect(40, 60, 250 ,60, ILI9341_WHITE ); // overwrite the text with white
         tft.setTextColor(ILI9341_RED);
         tft.setTextSize(2);
         if(message.setDiscipline == 1 || message.setDiscipline == 3 ){
        tft.setCursor(105, 60); 
        tft.println("TIME LEFT");
        }

        if(message.setDiscipline == 2){
          tft.setCursor(105, 60); 
        tft.println("TIME ELAPSED");
          }
       doOnceTimer=1;
    doOncerunningDiscipline = 0;
       }

////////////////////////////////
//
// the actual countdwon loops
//
/////////////////////////////////

//////////////
// COUNTDOWN
//////////////
if(message.setDiscipline == 1){

       currentMillis = (millis()- startMillis)*1;  //To accelerate time while testing, set multiplyer here

        if(timeHolder1 >= currentMillis){
         difference_millis = timeHolder1-currentMillis;
       if(difference_millis >0){                                  /*  The actual count down loop!*/

        time_fractionCountdown(difference_millis, 80);      // 80 is the line the TIME written on
             }
       }
   // transmitting
       if(oldTimerForTX <= millis()-1000){ //                               millis()
    fillMessage( false, false, message.setDiscipline, message.setDuration, currentMillis,true, 135, false, false); // start, reset1, discipline, duration, master millis, return result, result, switch channel, reset2
    radio.write(&message, sizeof(message));
    oldTimerForTX = millis();
      }

// when the clock runs out
       if(timeHolder1 < currentMillis){
        tft.fillRect(40, 60, 250 ,60, ILI9341_WHITE ); // overwrite the text with white
        doOnceTimer = 1; //ensures the right numbers being written
        time_fractionCountdown(timeHolder1, 80);
        tft.setTextSize(2);
        tft.setCursor(85, 120); 
        tft.println("END OF TIMER");
        message.setDiscipline=0;
        }
  } // end of if(message.setDiscipline == 1)


//////////////
// COUNT UP
//////////////

  
if(message.setDiscipline == 2){
      currentMillis = (millis()- startMillis)*1;  //To accelerate time while testing, set multiplyer here

        if(timeHolder1 >= currentMillis){
         difference_millis = currentMillis+900; // plus ~1 sec due to spectator friendlyness
                                                
        time_fractionCountdown(difference_millis, 80);      // 80 is the line the TIME written on
         }

         // transmitting
       if(oldTimerForTX <= millis()-1000){ //                               millis()
    fillMessage( false, false, message.setDiscipline, message.setDuration, currentMillis,true, 135, false, false); // start, reset1, discipline, duration, master millis, return result, result, switch channel, reset2
    radio.write(&message, sizeof(message));
    oldTimerForTX = millis();
      }

// when the clock runs out
       if(timeHolder1 < currentMillis){
        tft.fillRect(40, 60, 250 ,60, ILI9341_WHITE ); // overwrite the text with white
        doOnceTimer = 1; //ensures the right numbers being written
        time_fractionCountdown(timeHolder1, 80);
        tft.setTextSize(2);
        tft.setCursor(85, 120); 
        tft.println("END OF TIMER");
        message.setDiscipline=0;
        }
}// end of if(message.setDiscipline == 2)

//////////////
// PENTATHLON
//////////////

    if(message.setDiscipline == 3){
         currentMillis = (millis()- startMillis)*1;  //To accelerate time while testing, set multiplyer here

        if(timeHolder1 >= currentMillis){
         difference_millis = timeHolder1-currentMillis;
         
       if(difference_millis >0){                                  /*  The actual count down loop!*/

           time_fractionCountdown(difference_millis, 80);      // 80 is the line the TIME written on
 
        
             } // end of if(difference_millis >0)
       }
   // transmitting
       if(oldTimerForTX <= millis()-1000){ //                               millis()
    fillMessage( false, false, message.setDiscipline, message.setDuration, currentMillis,true, 135, false, false); // start, reset1, discipline, duration, master millis, return result, result, switch channel, reset2
    radio.write(&message, sizeof(message));
    oldTimerForTX = millis();
      }

// when the clock runs out
       if(timeHolder1 < currentMillis){
        tft.fillRect(40, 60, 250 ,60, ILI9341_WHITE ); // overwrite the text with white
        doOnceTimer = 1; //ensures the right numbers being written
        time_fractionCountdown(timeHolder1, 80);
        tft.setTextSize(2);
        tft.setCursor(85, 120); 
        tft.println("END OF TIMER");
        message.setDiscipline=0;
        }
  } // end of if(message.setDiscipline == 3)
  
  
// allowing for reset via the down button

// Button debounce for DOWN button
 readingRepDOWN = digitalRead(repButtonDOWN);
  if (readingRepDOWN != lastButtonStateRepDOWN) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }
  if ((millis() - lastDebounceTime) > 3000) {
     if (readingRepDOWN != buttonStateRepDOWN) {
      buttonStateRepDOWN = readingRepDOWN;
     if (buttonStateRepDOWN == HIGH) {
      resettingFunction();
    }
     }
   }



 buttonPressedLoopBOTTOM(); //Bottom section for buttons
  } // end of while(runningDiscipline)

  

  buttonPressedLoopBOTTOM(); //Bottom section for buttons
  
} // end of main loop 
/////////////////////////////////
//
// END OF MAIN LOOP
//
//////////////////////////////////


////////////////////////////////////
//
// RADIO FUNCTIONS
//
////////////////////////////////////

void RadioStartUp(){
  radio.begin();
  radio.openWritingPipe(pipe1);
  radio.setPALevel(RF24_PA_MAX); // RF24_PA_MIN, RF24_PA_LOW, RF24_PA_HIGH and RF24_PA_MAX
  radio.setDataRate(RF24_2MBPS); // 1MBPS   2MBPS
  //radio.enableAckPayload();
  //radio.setRetries(15,15);
  radio.setChannel(108); // 0-124
  radio.stopListening();
 
      }

void fillMessage(  bool start, bool resetOne, uint8_t discipline, unsigned long duration, unsigned long Mmillis, bool retRes, uint16_t result, bool swCHN, bool resetTWO){
 strcpy( message.filterChar,"H" );
  message.startBit = start;
  message.reset1 = resetOne;
  message.setDiscipline = discipline;
  message.setDuration = duration;
  message.masterMillis = Mmillis;
  message.returnResult = retRes;
  message.result = result;
  message.switchChannel = swCHN;
  message.reset2 = resetTWO;
 }

////////////////////////////////////////////////
//
// DISPLAY FUNCTIONS
//
/////////////////////////////////////////////////

 void PrintMainMenu(){
   tft.fillScreen(ILI9341_WHITE);
  tft.setTextSize(3);
  tft.setCursor(70, 10);
  tft.setTextColor(ILI9341_BLACK);
  tft.println("MAIN MENU");
  tft.setTextSize(2);
   tft.setCursor(20, 50);
  tft.setTextColor(ILI9341_BLACK);
  tft.println("SELECT TIMER FUNCTION");
  tft.setTextSize(3);
  tft.setCursor(40, 90);
  tft.setTextColor(ILI9341_RED);
  tft.println("COUNT DOWN ");
  tft.setCursor(40, 130);
  tft.setTextColor(ILI9341_RED);
  tft.println("COUNT UP");
  tft.setCursor(40, 170);
  tft.setTextColor(ILI9341_RED);
  tft.println("PENTATHLON");
  tft.setCursor(40, 210);
  tft.println("RESET COUNTERS");
  }

  
void MainText(){
  
  tft.setRotation(3);
  tft.fillScreen(ILI9341_WHITE);
  tft.setTextSize(4);
  tft.setCursor(20, 50);
  tft.setTextColor(ILI9341_OLIVE);
  tft.println("VULTURE WOOD");
  tft.setTextSize(4);
  tft.setCursor(70, 100);
  tft.setTextColor(ILI9341_RED);
  tft.println("ELECTRIC");
  tft.setCursor(100, 150);
  tft.setTextColor(ILI9341_ORANGE);
  tft.setTextSize(3);
  tft.println("REMOTE");
  tft.setCursor(100, 180);
  tft.setTextColor(ILI9341_NAVY);
  tft.println("MASTER");
  }


///////////////////////////////////////////
//
// TIMING FUNCTIONS
//
/////////////////////////////////////////////




    unsigned long GatherCDT(int CDThh_1, int CDTmm_10, int CDTmm_1,int CDTss_10, int CDTss_1){
 timeHolder2 = (unsigned long)CDThh_1*3600000; //60*60*1000
 timeHolder2 = timeHolder2 + (unsigned long)CDTmm_10*600000; //10*60*1000
 timeHolder2 = timeHolder2 + (unsigned long)CDTmm_1*60000; //1*60*1000
 timeHolder2 = timeHolder2 + (unsigned long)CDTss_10*10000; //10*1000
 timeHolder2 = timeHolder2 + (unsigned long)CDTss_1*1000; //1*1000
 return timeHolder2;
  }

 void time_fractionCountdown(unsigned long brutto, int lineSet){

      int TextSize =5;
      int lineOffset =50;
  tft.setTextColor(ILI9341_RED);
 tft.setTextSize(TextSize);
    if(doOnceTimer){
    hh_1_old = 10;  
    mm_10_old = 10;
    mm_1_old = 10;
    ss_10_old = 10;
    ss_1_old = 10;

    if(fourDigits){
     tft.setCursor(145,lineSet);
     tft.print(":");
    }else{
      tft.setCursor(75,lineSet);
     tft.print(":");
      tft.setCursor(165,lineSet);
     tft.print(":");
    }
     doOnceTimer=0;
      }
      
if(brutto >hours_10*10){
  brutto = 0;
  }
  
hh_10 = brutto/hours_10; // yields 10 hours
brutto = brutto-hours_10*hh_10;  // subtracts the difference
hh_1 = brutto/hours_1; // yields 1 hours
brutto = brutto-hours_1*hh_1;  // subtracts the difference
mm_10 = brutto/minutes_10; // yields 10 minutes
brutto = brutto-minutes_10*mm_10; 
mm_1 = brutto/minutes_1; // yields 1 minutes
brutto = brutto-minutes_1*mm_1;
ss_10 = brutto/seconds_10; // yields 10 seconds
brutto = brutto-seconds_10*ss_10;
ss_1 = brutto/seconds_1; // yields 1 seconds
brutto = brutto-seconds_1*ss_1;


  tft.setTextColor(ILI9341_RED);
  tft.setTextSize(TextSize);
 
if(!fourDigits){
if(hh_1_old != hh_1){
 tft.fillRect(lineOffset, lineSet, 25 ,35, ILI9341_WHITE ); // overwrite the text with black
 tft.setCursor(lineOffset,lineSet);
 tft.print(hh_1);
 hh_1_old = hh_1;
}

if(mm_10_old != mm_10){
 tft.fillRect(lineOffset+50, lineSet, 25 ,35, ILI9341_WHITE ); // overwrite the text with black
 tft.setCursor(lineOffset+50,lineSet);
 tft.print(mm_10);
 mm_10_old = mm_10;
}

if(mm_1_old != mm_1){
 tft.fillRect(lineOffset+90, lineSet, 25 ,35, ILI9341_WHITE ); // overwrite the text with black
 tft.setCursor(lineOffset+90,lineSet);
 tft.print(mm_1);
 mm_1_old = mm_1;
}

if(ss_10_old != ss_10){
 tft.fillRect(lineOffset+140, lineSet, 25 ,35, ILI9341_WHITE ); // overwrite the text with black
 tft.setCursor(lineOffset+140,lineSet);
 tft.print(ss_10);
 ss_10_old = ss_10;
}
if(ss_1_old != ss_1){
tft.fillRect(lineOffset+180, lineSet, 25 ,35, ILI9341_WHITE ); // overwrite the text with black
 tft.setCursor(lineOffset+180,lineSet);
 tft.print(ss_1);
 ss_1_old = ss_1;
}
}

if(fourDigits){

if(mm_10_old != mm_10){
 tft.fillRect(lineOffset+30, lineSet, 25 ,35, ILI9341_WHITE ); // overwrite the text with black
 tft.setCursor(lineOffset+30,lineSet);
 tft.print(mm_10);
 mm_10_old = mm_10;
}

if(mm_1_old != mm_1){
 tft.fillRect(lineOffset+70, lineSet, 25 ,35, ILI9341_WHITE ); // overwrite the text with black
 tft.setCursor(lineOffset+70,lineSet);
 tft.print(mm_1);
 mm_1_old = mm_1;
}

if(ss_10_old != ss_10){
 tft.fillRect(lineOffset+120, lineSet, 25 ,35, ILI9341_WHITE ); // overwrite the text with black
 tft.setCursor(lineOffset+120,lineSet);
 tft.print(ss_10);
 ss_10_old = ss_10;
}
if(ss_1_old != ss_1){
 tft.fillRect(lineOffset+160, lineSet, 25 ,35, ILI9341_WHITE ); // overwrite the text with black
 tft.setCursor(lineOffset+160,lineSet);
 tft.print(ss_1);
 ss_1_old = ss_1;
}
}
  }
  
  ///////////////////////////////
  //
  // BUTTON FUNCTIONS
  //
  ///////////////////////////////

     void buttonPressedLoopTOP(){
   // Button debounce MENU SELECT UP
  readingRepUP = digitalRead(repButtonUP);
  if (readingRepUP != lastButtonStateRepUP) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }
  if ((millis() - lastDebounceTime) > debounceDelay) {
     if (readingRepUP != buttonStateRepUP) {
      buttonStateRepUP = readingRepUP;
     if (buttonStateRepUP == HIGH) {
       Menu--;
     }
   }
}
// Button debounce MENU SELECT DOWN
 readingRepDOWN = digitalRead(repButtonDOWN);
  if (readingRepDOWN != lastButtonStateRepDOWN) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }
  if ((millis() - lastDebounceTime) > debounceDelay) {
     if (readingRepDOWN != buttonStateRepDOWN) {
      buttonStateRepDOWN = readingRepDOWN;
     if (buttonStateRepDOWN == HIGH) {
       Menu++;
     
     }
   }
}
// Menu Select
   reading = digitalRead(buttonPin);
  if (reading != lastButtonState) {
    // reset the debouncing timer
    lastDebounceTime = millis();
      }
  if ((millis() - lastDebounceTime) > debounceDelay) {
     if (reading != buttonState) {
      buttonState = reading;
     if (buttonState == HIGH) {
        Select = !Select;
                    }
                }
  }
  }

    void buttonPressedLoopBOTTOM(){
    lastButtonState = reading;
lastButtonStateRepUP = readingRepUP;
lastButtonStateRepDOWN = readingRepDOWN;
lastButtonStateRepDOWNRESET = readingRepDOWNRESET;

   }

void resettingFunction(){

  RadioStartUp(); // calling the start up function for the radio
  // filling the message so the counter restarts
fillMessage( false, true, message.setDiscipline, message.setDuration, millis(),true, 135, false, true);
    startMillis = millis();
    timeHolder1=10000;
    
  tft.fillScreen(ILI9341_WHITE);
  tft.setTextSize(4);
  tft.setCursor(40, 20);
  tft.setTextColor(ILI9341_RED);
  tft.println("RESETTING!");
  doOnceTimer =1;
  fourDigits = 1;
// 6 times send reset command 5 times 
 currentMillis = (millis()- startMillis)*1;  //To accelerate time while testing, set multiplyer here
while(timeHolder1> currentMillis){
  
 currentMillis = (millis()- startMillis)*1;  //To accelerate time while testing, set multiplyer here

        if(timeHolder1 >= currentMillis){
         difference_millis = timeHolder1-currentMillis;
       if(difference_millis >0){                                  /*  The actual count down loop!*/

        time_fractionCountdown(difference_millis, 80);      // 80 is the line the TIME written on
             }
             
  radio.write(&message, sizeof(message));
  }
  }

  ESP.restart();
  }  
