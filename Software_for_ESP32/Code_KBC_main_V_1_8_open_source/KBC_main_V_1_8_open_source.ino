/*
This code is the work of Erik Hofman-Bang, Denmark (the author)
For the project known as Vulture Wood Electric
These next ~4400 lines make up the main Kettlebell Counter including:
Base functions and Remote operation via radio/SPI link
Bluetooth functions have been erased, but as of 30/11-22 the menu point still exists 
This code is designed for an ESP32 and takes up 23% of storage and 4 % o dynamic memory

This code can be used and/or altered under the creative Commons license for all non-commercial purposes if the author is credited
*/

#include <Adafruit_GFX.h>   // Core graphics library
#include <P3RGB64x32MatrixPanel.h> // RGB display driver from NEOCAT github
// That is the library which contains the matrix.begin(); command which makes the watchdog timer panic
#include <SPI.h> // spi for radio use
#include <nRF24L01.h> // radio libraries
#include <RF24.h> // radio libraries  // 

// Radio pins
#define CE_PIN  27
#define CSN_PIN 26  
#define CLK_SPI 14 // 
#define MISO 12 // 
#define MOSI 13 //
#define SS 15 // 

//Matrix pins
#define R1 23
#define G1 22
#define BL1 1
#define R2 3
#define G2 21
#define BL2 19
#define pin_A 18
#define pin_B 5
#define pin_C 17
#define pin_D 16
#define CLK_RGB 25
#define LAT_RGB 32
#define OE_RGB 33

//////////////////////////////////////////////////////////
//
//  RADIO SPECIFIC VARIABLES ETC
//
//////////////////////////////////////////////////////////

bool doOnceRadio = true;
bool rpdPrint;
RF24 radio(CE_PIN,CSN_PIN);

unsigned long timeDifference;

const uint64_t pipe1 = 0xE8E8EEF0A1LL;;
const uint64_t pipe2 = 0xA2LL;  
const uint64_t pipe3 = 0xA3LL;
const uint64_t pipe4 = 0xA4LL;
const uint64_t pipe5 = 0xA5LL;
const uint64_t pipe6 = 0xA6LL;

// RADIO memory string

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
  TXstruct validMessage;

  // Matrix pin constructor
  //P3RGB64x32MatrixPanel matrix; // default constructor
P3RGB64x32MatrixPanel matrix(R1,G1,BL1,R2,G2,BL2,CLK_RGB,LAT_RGB,OE_RGB,pin_A,pin_B,pin_C,pin_D);

//////////////////////////////////////////
//
// Butten and buzzer pins
//
/////////////////////////////////////////

#define buttonPin 2   // select/menu
#define repButtonUP 36
#define repButtonDOWN 34
#define remote_on 35
#define BuzzerPin 4
uint16_t Buzzerfreq = 2000;
uint8_t BuzzerChannel = 0;
uint8_t BuzzerResolution = 8;
uint16_t BuzzerDuty = 128; // Volumen intensity in 8 bits, 125 (50%) will be loudest I think)

/////////////////////////////////////
//
// Variables for general functions.
//
////////////////////////////////////

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
int8_t readingONEmenuUP;
long lastDebounceTimeONEmenuUP;
int8_t buttonStateONEmenuUP = 0;
int8_t lastbuttonStateONEmenuUP = 0;
uint8_t line; // for time_fractionPause
int8_t Menu = 1;
int8_t Select =0;
int8_t choiceMade =0;
uint16_t sum_CLEAN = 0;
uint16_t sum_CandP = 0;
uint16_t sum_JERK = 0;
uint16_t sum_HS = 0;
uint16_t sum_PP = 0;
uint16_t divider = 0 ;
int8_t run_pause=0;
int8_t doOnceTimer=1;
int8_t doOnceCounter=1;
int8_t doOnceCountdownTimer =1;
int8_t doOnceRepCounter = 0;  // for testing rep numbers
int8_t runStopRoutine =1;
int8_t CDT_hh_1 = 0;
int8_t CDT_mm_10 = 0;
int8_t CDT_mm_1 = 0;
int8_t CDT_ss_10 = 0;
int8_t CDT_ss_1 = 0;
int8_t CDT_menu = 1;
int8_t CDT_menu_Last =1;
int8_t CDT_var_val =0;
int8_t fourDigits = 0;
int8_t Pentathlon = 0;
int8_t doOncePentathlon =1;
int8_t Countdown = 0;
int8_t UPdownMenu =0;
uint8_t ONEmenuUP  =0;
int8_t doOnceUPdownMenu =1;
int8_t UpDown_var_val = 2;
int8_t doOnceCountdown =1;
int8_t setCountdownTimer =1;
int8_t CountdownTimer = 0;
int8_t CountUpTimer = 0 ;
int8_t Interval = 0;
int8_t doOnceInterval =1;
uint8_t XonYoff = 0;
int XOnctr = 1;
int XOntime = 60;
int XOntimeOLD = 0;
int YOffctr = 1;
int YOfftime = 60;
int YOfftimeOLD = 0;
int XOnYOffctr = 0;
uint8_t doOnceYOfftimeRUN = 1;
int16_t XonYoffRounds =1; 
int16_t XonYoffRoundsOLD =0; 
uint8_t doOnceXonYoff = 1;
uint8_t XOnYOfftimer =0;
uint8_t doOnceXOnYOfftimer = 1;
uint8_t XOntimeRUN = 1;
uint8_t YOfftimeRUN = 1;
uint8_t doOnceEndXOnYOff =1;
uint8_t MERKULIN = 0;
uint8_t EMOTM = 0;
uint8_t EMOTMtimer = 0;
uint8_t endOfEMOTM = 0;
uint8_t disciplineDisplay = 1;
uint8_t durationDisplay =0;
uint8_t doOnceremote =1;
uint8_t waitingForStartCommand = 1;
uint8_t DisciplineNOTset =1;

uint16_t sum_MERKULIN[]= {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
uint8_t doOnceMERKULIN =1;
uint8_t MiddleMERKULINstep;
uint8_t UNEVEN = 0;
uint8_t UNEVENctr =0;
int8_t UNEVENrounds = 1;
int8_t UNEVENitteration = 1;
int16_t UNEVENtime =1;
int16_t UNEVENtimeTEN = 10;
uint8_t UNEVENtime_mm_10;
uint8_t UNEVENtime_mm_1;
uint8_t UNEVENtime_ss_10 = 1;
int16_t UNEVENdecrement = 0;
uint8_t UNEVENdecrement_mm_1;
uint8_t UNEVENdecrement_ss_10;
uint8_t UNEVENdecrement_ss_1;
uint8_t doOnceUNEVENmenu5 = 1;
uint16_t lastUNEVENtime;
uint16_t lastUNEVENdecrement;
uint8_t MERKULINtimer = 0;
uint8_t UNEVENtimer =0;
uint16_t PauseSteps;
uint16_t PauseStepsOLD;
uint8_t doOnceMERKULINtimer = 1;
uint8_t LADDERS = 0;
uint8_t doOnceLadders =1;
uint8_t UNEVENup =0;
uint8_t UNEVENdown =0;
uint8_t doOnceUneven =1;
uint8_t TABATA = 0;
uint8_t TABATAtimer = 0;
uint8_t doOnceTABATAendResult =1;
int TabataOntime;
int TabataOntimeOLD;
int TabataOfftime;
int TabataOfftimeOLD;
int TabataRounds;
int TabataRoundsOLD;
int TabataItterations;
int TabataItterationsOLD;
int bigTABATAbreak;
uint8_t choiceMadeInterval =0 ;
int8_t BLUETOOTH = 0;
int8_t remote = 0;
int8_t horn_offset = 50; // ændre tilbage til 50
int8_t hh_1_old = 10;
int8_t mm_10_old = 10;
int8_t mm_1_old = 10;
int8_t ss_10_old = 10;
int8_t ss_1_old = 10;
int i;
int k;
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 20;    // the debounce time; increase if the output flickers
unsigned long ss_10 = 0;
unsigned long ss_1 = 0;
long sum_CDT =0;
long repCounter = 0;
long lastRepWas = 0;
long timer_total;
long XOntimeTimeHolder;
long YOfftimeTimeHolder;
unsigned long mm_10 = 0;
unsigned long mm_1 = 0;
unsigned long hh_10 = 0; 
unsigned long hh_1 = 0;
long hours_10 = 36000000;
long hours_1 = 3600000;
long minutes_10 = 600000;
long minutes_1 = 60000;
long seconds_10 = 10000;
long seconds_1 = 1000;
long startMillis = 0;
long currentMillis = 0 ;
long timerSet = 3000000; //50*60*1000
long difference_millis;
long intermediate_millis;
unsigned long timeHolder1 = 0;
unsigned long timeHolder2 = 0;    


void RadioStartUp(){
  SPI.begin(CLK_SPI,MISO,MOSI,SS);
  
  radio.begin();
  radio.openReadingPipe(1,pipe1); // (1, const uint8_t *address) Pipe number (usually 1), pipe address (which is usually 5 bytes in an array structure).
  radio.setPALevel(RF24_PA_MIN); // RF24_PA_MIN, RF24_PA_LOW, RF24_PA_HIGH and RF24_PA_MAX
  radio.setDataRate(RF24_2MBPS); // 500kBPS 1MBPS   2MBPS
  //radio.enableAckPayload();
  //radio.setRetries(15,15);
  radio.setChannel(108); // 0-124
  //radio.powerUp();
  radio.startListening();
  delay(5);
      }

void RadioRestart(){
        radio.startListening();
        delay(5);    
        }

void RadioListenandSave(){
    if (radio.available()) {    
      //Serial.println("radio is available: ");
    radio.read(&message, sizeof(message));
    timeDifference = message.masterMillis-millis();
    if(message.filterChar[0]=='H'){
    validMessage = message;
    }
    } 
  }


////////////////////////////////////////////////////////
//
//             SETUP
//
/////////////////////////////////////////////////////////
  
void setup(){  
   //Serial.begin(115200);
   RadioStartUp(); // Setup of radio functions and parameters
   //Serial.println("Serial startup");

   
 ///////////////////////////////
// Setting up buzzer and buttons
//////////////////////////////

  ledcSetup(BuzzerChannel, Buzzerfreq, BuzzerResolution);

  
  ledcAttachPin(BuzzerPin, BuzzerChannel);   // remember to UNCOMMENT after test

  
  pinMode(repButtonUP, INPUT); // Counter increment
  pinMode(repButtonDOWN, INPUT); // Counter decrement
  pinMode(buttonPin, INPUT); // Start/Stop
  pinMode(remote_on, INPUT); // remote on/off

 remote = digitalRead(remote_on);
 
 
  
///////////////////////////////////
// Initialize Matrix
//////////////////////////////////
  matrix.begin();
  matrix.setTextWrap(false); // Don't wrap at end of line - will do ourselves
 if(!remote){
  startUpText();
  printMainMenu();
 } // End if(!remote)

  if(remote){
  matrix.fillScreen(matrix.color444(0, 0, 0)); //Clear screen
  matrix.setTextSize(1);  
  matrix.setTextColor(matrix.color555(31,0,0));
  matrix.setCursor(14, 12); 
  matrix.println("REMOTE");
  delay(5000); // 5000
  } // end if(remote)

  } // end of void setup

////////////////////////////////////////////////////////////
//
//    MAIN LOOP
//
/////////////////////////////////////////////////////////////

 
void loop() {

////////////////////////////////
//
// REMOTE sequence
//
///////////////////////////////
if(remote){

  if(doOnceremote){
  matrix.stop(); // matrix has to stop before SPI for the NRF24L01 Radio
  RadioRestart(); // Restart the radio, ready to listen
  doOnceremote = 0;
  }
  
  while(DisciplineNOTset){
  RadioListenandSave(); // Listen and save if something is incoming

  if(validMessage.filterChar[0]=='H' && disciplineDisplay){
     matrix.fillScreen(matrix.color444(0, 0, 0)); //Clear screen
    matrix.setTextColor(matrix.color555(1,31,1));
    if(validMessage.setDiscipline==1){
      matrix.begin(); // Restart the matrix
        matrix.setCursor(1, 16); 
        matrix.println("COUNT DOWN");
        disciplineDisplay=0;
        durationDisplay = 1;
         delay(2000);
      }

       if(validMessage.setDiscipline==2){
        matrix.begin(); // Restart the matrix
        matrix.setCursor(8, 16); 
        matrix.println("COUNT UP");
        disciplineDisplay=0;
        durationDisplay = 1;
        delay(2000);
      }

       if(validMessage.setDiscipline==3){
        matrix.begin(); // Restart the matrix
        matrix.setCursor(2, 16); 
        matrix.println("PENTATHLON");
        disciplineDisplay=0;
        durationDisplay = 1;
        delay(2000);
      }
  }
   
   matrix.fillScreen(matrix.color444(0, 0, 0)); //Clear screen
  
   if(validMessage.filterChar[0]=='H' && durationDisplay){

      if(validMessage.setDuration >= 3600000){ // IF timer is larger than 1 hour, set 5 digits
           doOnceTimer=1;
           }
            if(validMessage.setDuration < 3600000){ // IF timer is less than 1 hour, set 4 digits
           fourDigits=1;
           doOnceTimer=1;
           }

           // PRINT THE DURATION ON SCREEN
           
    time_fractionCountdown(validMessage.setDuration,14); // 14 is the line it's written on 
     delay(2000);
      DisciplineNOTset = 0;
      doOnceremote=1;
    }
    
     } // end of while(DisciplineNOTset)
     

   while(waitingForStartCommand){  
   if(doOnceremote){
  matrix.stop();
  RadioRestart(); // Restart the radio, ready to listen
  doOnceremote = 0;
  }
    RadioListenandSave(); // Listen and save if something is incoming
  //Serial.println("waitingForStartCommand");
   if(validMessage.filterChar[0]=='H'){
    //Serial.print("validMessage.startBit ");
    //Serial.println(validMessage.startBit);
   
    if(validMessage.startBit == 1){
   
    // count down instructions
      if(validMessage.setDiscipline==1){
   Countdown = 1;  // go to countdown
   CountdownTimer =1 ; // GO TO countdown timer
   setCountdownTimer = 0;  // skip SET COUNT DOWN TIMER
   UPdownMenu = 0; // skip UP DOWN MENU
   waitingForStartCommand = 0; // leave this loop
   matrix.begin();
   timeHolder1=validMessage.setDuration;
      }
// Count up instructions
      if(validMessage.setDiscipline==2){
   Countdown = 1;  // go to countdown
   CountUpTimer =1 ; // GO TO countdown timer
   setCountdownTimer = 0;  // skip SET COUNT DOWN TIMER
   UPdownMenu = 0; // skip UP DOWN MENU
   waitingForStartCommand = 0; // leave this loop
   matrix.begin();
   timeHolder1=validMessage.setDuration;
      }

// pentathlon instructions
      if(validMessage.setDiscipline==3){
   Pentathlon = 1;  // go to pentathlon
   countState = 1;
   waitingForStartCommand = 0; // leave this loop
   matrix.begin();
   timeHolder1=validMessage.setDuration;
      }
      
    } // end of if(validMessage.startBit == 1)
   } // end of if(validMessage.filterChar[0]=='H')
   }
} // end if (remote)

/////////////////////////////////////////////
//
//   NORMAL MENU FUNCTION
//
/////////////////////////////////////////////

if(!remote){  // basically the old Void loop
buttonPressedLoopTOP();
  

if(Menu>4 ){
  Menu = 1;
  }

  if(Menu<1){
  Menu = 4;
  }
  
if(Menu==1 && !choiceMade){
  matrix.fillRect(0, 8, 5 ,8, matrix.color444(0, 0, 0)); // overwrite the text with black
  matrix.fillRect(0, 16, 5 ,8, matrix.color444(0, 0, 0)); // overwrite the text with black
  matrix.fillRect(0, 24, 5 ,8, matrix.color444(0, 0, 0)); // overwrite the text with black
  matrix.setTextColor(matrix.color555(31,31,31));
  matrix.setCursor(0, 0); 
  matrix.println("*");
  if(Select){
    Countdown = 1;
    choiceMade = 1;
    setCountdownTimer = 1;
    CDT_menu = 1;
    CDT_var_val=0;
    doOnceCountdown =1;
    ONEmenuUP = 0;
    }
   }

   if(Menu==2 && !choiceMade){
  matrix.fillRect(0, 0, 5 ,8, matrix.color444(0, 0, 0)); // overwrite the text with black
  matrix.fillRect(0, 16, 5 ,8, matrix.color444(0, 0, 0)); // overwrite the text with black
  matrix.fillRect(0, 24, 5 ,8, matrix.color444(0, 0, 0)); // overwrite the text with black
  matrix.setTextColor(matrix.color555(31,31,31));
  matrix.setCursor(0, 8); 
  matrix.println("*");
   if(Select){
    Pentathlon = 1;
    choiceMade = 1;
    }
   }

   
   if(Menu==3 && !choiceMade){
  matrix.fillRect(0, 0, 5 ,8, matrix.color444(0, 0, 0)); // overwrite the text with black
  matrix.fillRect(0, 8, 5 ,8, matrix.color444(0, 0, 0)); // overwrite the text with black
  matrix.fillRect(0, 24, 5 ,8, matrix.color444(0, 0, 0)); // overwrite the text with black
  matrix.setTextColor(matrix.color555(31,31,31));
  matrix.setCursor(0, 16); 
  matrix.println("*");
   if(Select){
    Interval = 1;
    ONEmenuUP = 0;
    choiceMade = 1;
    }
   }

     if(Menu==4 && !choiceMade){
  matrix.fillRect(0, 0, 5 ,8, matrix.color444(0, 0, 0)); // overwrite the text with black
  matrix.fillRect(0, 8, 5 ,8, matrix.color444(0, 0, 0)); // overwrite the text with black
  matrix.fillRect(0, 16, 5 ,8, matrix.color444(0, 0, 0)); // overwrite the text with black
  matrix.setTextColor(matrix.color555(31,31,31));
  matrix.setCursor(0, 24); 
  matrix.println("*");
   if(Select){
    BLUETOOTH= 1;  // This is the old bluetooth menu point
    choiceMade = 1;
    }
   }

    } // end of if(!remote)

    
while(Countdown){
  
  if(doOnceCountdown && !remote){
  
  matrix.fillScreen(matrix.color444(0, 0, 0)); //Clear screen
  matrix.setTextColor(matrix.color555(1,15,31));
  matrix.setTextSize(1); 
  matrix.setCursor(8, 2); 
  matrix.println("Set Time");
  matrix.setTextColor(matrix.color555(0,15,0));
  doOnceCountdown = 0;
  CDT_hh_1 =0;
  CDT_mm_10 =0;
  CDT_mm_1 = 0;
  CDT_ss_10 = 0;
  CDT_ss_1 = 0;
  CDT_menu = 1;
  CDT_menu_Last = 1;
  time_fractionCountdown(0 , 14);
  matrix.fillRect(1, 29, 10 ,2, matrix.color444(15, 0, 0)); // SET RED BAR
  }
  //Serial.println("before While setCountdownTimer");
while(setCountdownTimer){
  // Serial.println("While setCountdownTimer");
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
              CDT_var_val = CDT_hh_1 ;
              matrix.fillRect(1, 29, 10 ,2, matrix.color444(15, 0, 0)); // SET RED BAR
                           }
           if(CDT_menu == 2){
             CDT_var_val = CDT_mm_10;
             matrix.fillRect(1, 29, 10 ,2, matrix.color444(0, 0, 0)); // SET BLACK BAR
             matrix.fillRect(15, 29, 10 ,2, matrix.color444(15, 0, 0)); // SET RED BAR
            }
           if(CDT_menu == 3){
            CDT_var_val = CDT_mm_1 ;
             matrix.fillRect(15, 29, 10 ,2, matrix.color444(0, 0, 0)); // SET BLACK BAR
             matrix.fillRect(27, 29, 10 ,2, matrix.color444(15, 0, 0)); // SET RED BAR
            }
            if(CDT_menu == 4){
              CDT_var_val = CDT_ss_10;
              matrix.fillRect(27, 29, 10 ,2, matrix.color444(0, 0, 0)); // SET BLACK BAR
              matrix.fillRect(41, 29, 10 ,2, matrix.color444(15, 0, 0)); // SET RED BAR
             }
           if(CDT_menu == 5){
              CDT_var_val = CDT_ss_1;
              matrix.fillRect(41, 29, 10 ,2, matrix.color444(0, 0, 0)); // SET BLACK BAR
              matrix.fillRect(53, 29, 10 ,2, matrix.color444(15, 0, 0)); // SET RED BAR
              }
              if(CDT_menu == 6){
              matrix.fillRect(53, 29, 10 ,2, matrix.color444(0, 0, 0)); // SET BLACK BAR
              matrix.fillRect(0, 0, 64 ,11, matrix.color444(0, 0, 0)); // overwrite the text with black  
              matrix.setTextColor(matrix.color555(1,15,31));
              matrix.setTextSize(1); 
              matrix.setCursor(2, 2); 
              matrix.println("UP");
              matrix.setCursor(14, 2); 
              matrix.println("=");
              matrix.setCursor(21, 2); 
              matrix.println("CONFIRM");
                }
                if(CDT_menu != 6){
                  matrix.fillRect(0, 0, 64 ,11, matrix.color444(0, 0, 0)); // overwrite the text with black
                  matrix.setTextColor(matrix.color555(1,15,31));
                  matrix.setTextSize(1); 
                  matrix.setCursor(8, 2); 
                  matrix.println("Set Time");
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
      setCountdownTimer = 0;
      UPdownMenu =1;
      }
     }
   }
}
// Button debounce CD-timer DOWN

 readingRepDOWN = digitalRead(repButtonDOWN);
readingONEmenuUP = digitalRead(repButtonDOWN);
 
  if (readingRepDOWN != lastButtonStateRepDOWN) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }
  
      if ((millis() - lastDebounceTime) > 3000) {
     if (readingONEmenuUP != buttonStateONEmenuUP) {
      buttonStateONEmenuUP = readingONEmenuUP;
     if (buttonStateONEmenuUP == HIGH) {
      ESP.restart();
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
}} // end else{}

timeHolder1 = GatherCDT(CDT_hh_1, CDT_mm_10, CDT_mm_1,CDT_ss_10,CDT_ss_1);

time_fractionCountdown(timeHolder1,14); // 14 is the line it's written on

lastButtonState = reading;
lastButtonStateRepUP = readingRepUP;
lastButtonStateRepDOWN = readingRepDOWN;
lastbuttonStateONEmenuUP = buttonStateONEmenuUP;
  } // end of while(setCountdownTimer)

/////////////////////////
//
// UP or DOWN menu
//
/////////////////////////
while(UPdownMenu){

if(doOnceUPdownMenu){
  matrix.fillScreen(matrix.color444(0, 0, 0)); //Clear screen
  matrix.setTextSize(1); 
  matrix.setTextColor(matrix.color555(1,15,31));
  matrix.setTextSize(1); 
  matrix.setCursor(7, 16); 
  matrix.println("Count");
  matrix.setCursor(39, 16); 
  matrix.println("UP");
  matrix.setCursor(7, 24); 
  matrix.println("Count");
  matrix.setCursor(39, 24); 
  matrix.println("DOWN");
  
  if(timeHolder1 >= 3600000){ // IF timer is larger than 1 hour, set 5 digits
           doOnceTimer=1;
           }
            if(timeHolder1 < 3600000){ // IF timer is less than 1 hour, set 4 digits
           fourDigits=1;
           doOnceTimer=1;
           }
  time_fractionCountdown(timeHolder1, 0); // write time duration

  doOnceUPdownMenu = 0;
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
       UpDown_var_val++;
     if(UpDown_var_val >2){
      UpDown_var_val = 1;
      }
      }
     }
   }

// Button debounce CD-timer DOWN
 readingRepDOWN = digitalRead(repButtonDOWN);
   readingONEmenuUP = digitalRead(repButtonDOWN);
  if (readingRepDOWN != lastButtonStateRepDOWN) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }
  
    if ((millis() - lastDebounceTime) > 3000) {
     if (readingONEmenuUP != buttonStateONEmenuUP) {
      buttonStateONEmenuUP = readingONEmenuUP;
     if (buttonStateONEmenuUP == HIGH) {
      ESP.restart(); // restart the entire counter
    }
   }
}else{
  if ((millis() - lastDebounceTime) > debounceDelay) {
     if (readingRepDOWN != buttonStateRepDOWN) {
      buttonStateRepDOWN = readingRepDOWN;
     if (buttonStateRepDOWN == HIGH) {
       UpDown_var_val--;
      if(UpDown_var_val <1){
      UpDown_var_val = 2;
      }
     }
   }
}} // end of else{}
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
  if(UpDown_var_val == 1 ){
  CountUpTimer = 1; // select the count up timer
  UPdownMenu=0;
  }

 if(UpDown_var_val == 2 ){
  
  CountdownTimer = 1;  // Select the countdowntimer
  UPdownMenu=0;
  }
   }
    }
  } //end of if(millis....)

if(UpDown_var_val == 1){ 
   matrix.fillRect(0, 24, 5 ,8, matrix.color444(0, 0, 0)); // overwrite the text with black
  matrix.setTextColor(matrix.color555(31,31,31));
  matrix.setTextSize(1); 
  matrix.setCursor(0, 16); 
  matrix.println("*");
}
if(UpDown_var_val == 2){ 
   matrix.fillRect(0, 16, 5 ,8, matrix.color444(0, 0, 0)); // overwrite the text with black
  matrix.setTextColor(matrix.color555(31,31,31));
  matrix.setTextSize(1); 
  matrix.setCursor(0, 24); 
  matrix.println("*");
}

lastButtonState = reading;
lastButtonStateRepUP = readingRepUP;
lastButtonStateRepDOWN = readingRepDOWN;

} // end of while(UPdownMenu)

  while(CountUpTimer){
    if(doOnceCountdownTimer){
          countDownToStart(); // CALLING THE COUNTDOWN FUNCTION ( BUZZER AND DISPLAY ) !!!!!!
          //timeHolder1 = timeHolder1; 
          matrix.fillScreen(matrix.color444(0, 0, 0)); //Clear screen
          startMillis = millis();
          doOnceCountdownTimer = 0;
    }
          currentMillis = (millis()- startMillis)*1;  //To accelerate time while testing, set multiplyer here

        if(timeHolder1 >= currentMillis){
         difference_millis = currentMillis+1000; // plus 1 sec due to spectator friendlyness
                                                     //  The actual count down loop!
                                                      //  The actual count down loop!
        time_fractionCountdown(difference_millis, 1);       // Displaying time left on line 1
        sum_CDT = CountingREPS();
       }
       
        if(timeHolder1 < currentMillis && runStopRoutine){
    matrix.fillScreen(matrix.color444(0, 0, 0)); //Clear the screen
    matrix.setTextColor(matrix.color555(31,0,0));
    matrix.setTextSize(2);
    matrix.setCursor(10,8);
    matrix.print("STOP");
    horn(); // end of set horn and counter reset
     matrix.fillScreen(matrix.color444(0, 0, 0)); //Clear the screen
     doOnceTimer = 1; // Ensures that all digits are written out
     timeHolder1 = timeHolder1; //Compensates for the 500 millis deducted to begin with, which ensures the horn doesn't offset the time by 500 millis.
    time_fractionCountdown(timeHolder1, 1);  // END OF SET DISPLAY last number is the Line on which the time is written

   if(sum_CDT >= 0 && sum_CDT <10 ){
   matrix.setTextSize(2); 
   matrix.fillRect(21, 16, 24,14, matrix.color444(0, 0, 0)); // overwrite the text with black
   matrix.setTextColor(matrix.color444(1,8,15));
   matrix.setCursor(27,16);
   matrix.print(sum_CDT);
   }
   
  if(sum_CDT >= 10 && sum_CDT <100){
   matrix.setTextSize(2); 
   matrix.fillRect(15, 16, 34,14, matrix.color444(0, 0, 0)); // overwrite the text with black
   matrix.setTextColor(matrix.color444(1,8,15));
   matrix.setCursor(21,16);
   matrix.print(sum_CDT);
   }

     if(sum_CDT >= 100 && sum_CDT<1000){
   matrix.setTextSize(2); 
   matrix.fillRect(9, 16, 34,14, matrix.color444(0, 0, 0)); // overwrite the text with black
   matrix.setTextColor(matrix.color444(1,8,15));
   matrix.setCursor(15,16);
   matrix.print(sum_CDT);
   }

    if(sum_CDT >= 1000 && sum_CDT <10000){
   matrix.setTextSize(2); 
   matrix.fillRect(3, 16, 45,14, matrix.color444(0, 0, 0)); // overwrite the text with black
   matrix.setTextColor(matrix.color444(1,8,15));
   matrix.setCursor(9,16);
   matrix.print(sum_CDT);
   }

   if(sum_CDT >= 10000 && sum_CDT <100000){
   matrix.setTextSize(2); 
   matrix.fillRect(3, 16, 45,14, matrix.color444(0, 0, 0)); // overwrite the text with black
   matrix.setTextColor(matrix.color444(1,8,15));
   matrix.setCursor(3,16);
   matrix.print(sum_CDT);
   }

   runStopRoutine = 0;
   if(remote){
    waitForReset();}
//   break;
   }
    } // end of while(countuptimer)

     while(CountdownTimer){

if(doOnceCountdownTimer){
          countDownToStart(); // CALLING THE COUNTDOWN FUNCTION ( BUZZER AND DISPLAY ) !!!!!!
          //timeHolder1 = timeHolder1; 
          matrix.fillScreen(matrix.color444(0, 0, 0)); //Clear screen
          startMillis = millis();
          doOnceCountdownTimer = 0;
          if(timeHolder1 >= 3600000){ // IF timer is larger than 1 hour, set 5 digits
           doOnceTimer=1;
           }
            if(timeHolder1 < 3600000){ // IF timer is less than 1 hour, set 4 digits
           fourDigits=1;
           }
} // end of if(doOnceCountdownTimer)
          currentMillis = (millis()- startMillis)*1;  //To accelerate time while testing, set multiplyer here

        if(timeHolder1 >= currentMillis){
         difference_millis = timeHolder1-currentMillis;
       if(difference_millis >0){                                  /*  The actual count down loop!*/

        time_fractionCountdown(difference_millis, 1);       // Displaying time left on line 1
        sum_CDT = CountingREPS(); // displaying reps on screen
        }
       }
       
  if(timeHolder1 < currentMillis && runStopRoutine){
    matrix.fillScreen(matrix.color444(0, 0, 0)); //Clear the screen
    matrix.setTextColor(matrix.color555(31,0,0));
    matrix.setTextSize(2);
    matrix.setCursor(10,8);
    matrix.print("STOP");
    horn(); // end of set horn and counter reset
    matrix.fillScreen(matrix.color444(0, 0, 0)); //Clear the screen
    doOnceTimer = 1; // Ensures that all digits are written out
    timeHolder1 = timeHolder1; //Compensates for the 500 millis deducted to begin with, which ensures the horn doesn't offset the time by 500 millis.
    time_fractionCountdown(timeHolder1, 1);  // END OF SET DISPLAY last number is the Line on which the time is written
    
   if(sum_CDT >= 0 && sum_CDT <10 ){
   matrix.setTextSize(2); 
   matrix.fillRect(21, 16, 24,14, matrix.color444(0, 0, 0)); // overwrite the text with black
   matrix.setTextColor(matrix.color444(1,8,15));
   matrix.setCursor(27,16);
   matrix.print(sum_CDT);
   }
   
  if(sum_CDT >= 10 && sum_CDT <100){
   matrix.setTextSize(2); 
   matrix.fillRect(15, 16, 34,14, matrix.color444(0, 0, 0)); // overwrite the text with black
   matrix.setTextColor(matrix.color444(1,8,15));
   matrix.setCursor(21,16);
   matrix.print(sum_CDT);
   }

     if(sum_CDT >= 100 && sum_CDT<1000){
   matrix.setTextSize(2); 
   matrix.fillRect(9, 16, 34,14, matrix.color444(0, 0, 0)); // overwrite the text with black
   matrix.setTextColor(matrix.color444(1,8,15));
   matrix.setCursor(15,16);
   matrix.print(sum_CDT);
   }

    if(sum_CDT >= 1000 && sum_CDT <10000){
   matrix.setTextSize(2); 
   matrix.fillRect(3, 16, 45,14, matrix.color444(0, 0, 0)); // overwrite the text with black
   matrix.setTextColor(matrix.color444(1,8,15));
   matrix.setCursor(9,16);
   matrix.print(sum_CDT);
   }

   if(sum_CDT >= 10000 && sum_CDT <100000){
   matrix.setTextSize(2); 
   matrix.fillRect(3, 16, 45,14, matrix.color444(0, 0, 0)); // overwrite the text with black
   matrix.setTextColor(matrix.color444(1,8,15));
   matrix.setCursor(3,16);
   matrix.print(sum_CDT);
   }
  
   runStopRoutine = 0;
   if(remote){
    waitForReset();}
//   break;
   }
  } // end of while(countdowntimer)
} // end of while(countdown) line 559

while(Pentathlon){

 if(doOncePentathlon && remote){
matrix.fillScreen(matrix.color444(0, 0, 0)); //Clear screen
  countDownToStart(); // CALLING THE COUNTDOWN FUNCTION ( BUZZER AND DISPLAY ) !!!!!!
   matrix.fillScreen(matrix.color444(0, 0, 0)); //Clear screen
    startMillis = millis();
  doOncePentathlon = 0;
  }

  if(doOncePentathlon && !remote){
  
matrix.fillScreen(matrix.color444(0, 0, 0)); //Clear screen
  matrix.setTextColor(matrix.color444(15,0,0));
  matrix.setTextSize(1); 
  matrix.setCursor(2, 4); 
  matrix.println("PENTATHLON");
  matrix.setTextColor(matrix.color444(0,15,0));
  matrix.setCursor(2, 19); 
  matrix.println("PUSH START");
  doOncePentathlon = 0;
  }
int reading = digitalRead(buttonPin);
  if (reading != lastButtonState) {
    // reset the debouncing timer
    lastDebounceTime = millis();
      }
  if ((millis() - lastDebounceTime) > debounceDelay) {
     if (reading != buttonState) {
      buttonState = reading;
     if (buttonState == HIGH) {
        countState = !countState;
        if(countState == 1){
          
          countDownToStart(); // CALLING THE COUNTDOWN FUNCTION ( BUZZER AND DISPLAY ) !!!!!!
           matrix.fillScreen(matrix.color444(0, 0, 0)); //Clear screen
          startMillis = millis();
                          }
                        }
                    }
  }

 readingRepDOWN = digitalRead(repButtonDOWN);
 
  if (readingRepDOWN != lastButtonStateRepDOWN) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }

      if ((millis() - lastDebounceTime) > 3000) {
     if (readingRepDOWN != buttonStateONEmenuUP) {
      buttonStateONEmenuUP = readingRepDOWN;
     if (buttonStateONEmenuUP == HIGH) {
      ESP.restart();
    }
   }
}
// If the start button is pressed, do:
int doOnceRep = 1;
// update button states
 lastButtonState = reading;
 lastButtonStateRepDOWN = readingRepDOWN;
lastbuttonStateONEmenuUP = readingRepDOWN;

while(countState){

   
currentMillis = (millis()- startMillis)*1;  //To accelerate time while testing, set multiplyer here
    
    
     
   if(timerSet>=currentMillis){
   difference_millis = timerSet-currentMillis;
   if(difference_millis >0){

      if(difference_millis >2640000){ //larger than 44 min CLEAN
      intermediate_millis = difference_millis-2640000;
    time_fraction(intermediate_millis ); 
   
    sum_CLEAN = CountingREPS();
    if(sum_CLEAN>120){ // limits counter to max number
       sum_CLEAN = 120;
      }

   if(intermediate_millis<=horn_offset){ 
    matrix.fillScreen(matrix.color444(0, 0, 0));
    matrix.setTextColor(matrix.color555(31,0,0));
    matrix.setTextSize(2);
    matrix.setCursor(10,8);
    matrix.print("STOP");

    horn(); // end of set horn and counter reset
         }
    } //larger than 44 min CLEAN
    
if(difference_millis < 2640000 && difference_millis>2346500){ //pause between 44 and 39 min
    intermediate_millis = difference_millis-2340000;
    time_fractionPause(intermediate_millis);
    if(run_pause==1){
    pause();
  
    matrix.setTextColor(matrix.color555(1,15,31));
    matrix.setCursor(12, 22);
    matrix.print("REPS");
    matrix.setCursor(34, 22);
    matrix.print(":");
    matrix.setCursor(40, 22);
    matrix.print(sum_CLEAN);
    
  run_pause=0;
  }
 } //pause between 44 and 39 min

    if(difference_millis < 2346500 && difference_millis>2340000){ // COUNTDOWN TO CLEAN AND PRESS
    countDownToStart();
    matrix.fillScreen(matrix.color444(0, 0, 0));
     }
     
if(difference_millis < 2340000 && difference_millis>1980000){ //  CLEAN AND PRESS
      intermediate_millis = difference_millis-1980000;
    time_fraction(intermediate_millis ); 
    
    sum_CandP = CountingREPS();
    
    if(sum_CandP > 60){ // limits counter to max number
       sum_CandP = 60;
      }
    
   if(intermediate_millis<=horn_offset){
    matrix.fillScreen(matrix.color444(0, 0, 0));
    matrix.setTextColor(matrix.color555(31,0,0));
    matrix.setTextSize(2);
    matrix.setCursor(10,8);
    matrix.print("STOP"); 
   
   horn(); // end of set horn and counter reset
         }
    } // end clean and jerk
    
if(difference_millis < 1980000 && difference_millis>1686500){ //pause between 33 and 28 min
    intermediate_millis = difference_millis-1680000; // differrence on 6500 millis for countdown
    time_fractionPause(intermediate_millis);
    if(run_pause==1){
    pause();
  
    matrix.setTextColor(matrix.color555(1,15,31));
    matrix.setCursor(12, 22);
    matrix.print("REPS");
    matrix.setCursor(34, 22);
    matrix.print(":");
    matrix.setCursor(40, 22);
    matrix.print(sum_CandP);
    
  run_pause=0;
  }
 } //pause between 33 and 28 min
 
if(difference_millis < 1686500 && difference_millis>1680000){ // COUNTDOWN TO JERK
    countDownToStart();
     matrix.fillScreen(matrix.color444(0, 0, 0));
     }
     
if(difference_millis < 1680000 && difference_millis>1320000){ //  JERK
      intermediate_millis = difference_millis-1320000;
    time_fraction(intermediate_millis ); 
    sum_JERK = CountingREPS();
    if(sum_JERK >120){ // limits counter to max number
       sum_JERK = 120;
      }
   
   if(intermediate_millis<=horn_offset){ 
    matrix.fillScreen(matrix.color444(0, 0, 0));
    matrix.setTextColor(matrix.color555(31,0,0));
    matrix.setTextSize(2);
    matrix.setCursor(10,8);
    matrix.print("STOP");
    horn(); // end of set horn and counter reset
         }
    } // end Jerk
    
if(difference_millis < 1320000 && difference_millis>1026500){ //pause between 22 and 17 min
    intermediate_millis = difference_millis-1020000;; // differrence on 6500 millis for countdown
    time_fractionPause(intermediate_millis);
    if(run_pause==1){
    pause();

    matrix.setTextColor(matrix.color555(1,15,31));
    matrix.setCursor(12, 22);
    matrix.print("REPS");
    matrix.setCursor(34, 22);
    matrix.print(":");
    matrix.setCursor(40, 22);
    matrix.print(sum_JERK);
    

  run_pause=0;
  }
 } // end pause between 22 and 17 min
 
if(difference_millis < 1026500 && difference_millis>1020000){ // COUNTDOWN TO HALF-SNATCH
    countDownToStart();
     matrix.fillScreen(matrix.color444(0, 0, 0));
     }
     
if(difference_millis < 1020000 && difference_millis>660000){ //  HALF-SNATCH
      intermediate_millis = difference_millis-660000;
    time_fraction(intermediate_millis); 

    sum_HS = CountingREPS();
    if(sum_HS >108){ // limits counter to max number
       sum_HS = 108;
      }

   if(intermediate_millis<=horn_offset){ 
    matrix.fillScreen(matrix.color444(0, 0, 0));
    matrix.setTextColor(matrix.color555(31,0,0));
    matrix.setTextSize(2);
    matrix.setCursor(10,8);
    matrix.print("STOP");
    horn(); // end of set horn and counter reset
         }
} // end  HALF-SNATCH

if(difference_millis < 660000 && difference_millis>366500){ //pause between 11 and 6 min
    intermediate_millis = difference_millis-360000;; // differrence on 6500 millis for countdown
    time_fractionPause(intermediate_millis);
    if(run_pause==1){
    pause();

    matrix.setTextColor(matrix.color555(1,15,31));
    matrix.setCursor(12, 22);
    matrix.print("REPS");
    matrix.setCursor(34, 22);
    matrix.print(":");
    matrix.setCursor(40, 22);
    matrix.print(sum_HS);

  run_pause=0;
  }
 } // end pause between 11 and 6 min
 
if(difference_millis < 366500&& difference_millis>360000){ // COUNTDOWN TO PUSH-PRESS
    countDownToStart();
     matrix.fillScreen(matrix.color444(0, 0, 0));
     }
     
if(difference_millis < 360000 && difference_millis>0){ //  PUSH-PRESS
      intermediate_millis = difference_millis;
    time_fraction(intermediate_millis ); 
 
    sum_PP = CountingREPS();
    if(sum_PP >120){ // limits counter to max number
       sum_PP = 120;
      }
       if(sum_PP >120){ // limits counter to max number
       sum_PP = 120;
      }
     } // end PUSH-PRESS
    } // end if(difference_millis >0}
    
   } // end  if(timerSet>=currentMillis)--- stops time and inhibits counting
   
if(timerSet-currentMillis<0){
    countState=0;
    matrix.fillScreen(matrix.color444(0, 0, 0));
    matrix.setTextColor(matrix.color555(31,0,0));
    matrix.setTextSize(2);
    matrix.setCursor(10,8);
    matrix.print("STOP");
    horn(); // end of set horn and counter reset
    endResult(); // dsplay end result on screen
 
    if(remote){
    waitForReset();}
   break;
   } // end if(timerSet-currentMillis<0
   } // end of while(countstate)
  } // end of while(pentathlon)


while(Interval){
//Menu text written in previous MENU select.
  if(doOnceInterval){
    printIntervalMenu(); // Prints the menu
    Menu=1; // Sets the cursor to the top choice
    doOnceInterval = 0; // Ensures this statement isn't repeated
    Select = 0; // resets the Select variable which is reused.
    }
   
  buttonPressedLoopTOP(); // TOP half of the button functions
  
// keeps the menu point within bounds
if(Menu>4){
  Menu = 1;
  }

  if(Menu<1){
  Menu = 4;
  }

 if(Menu==1 && !choiceMadeInterval){
  matrix.fillRect(0, 8, 5 ,8, matrix.color444(0, 0, 0)); // overwrite the text with black
  matrix.fillRect(0, 16, 5 ,8, matrix.color444(0, 0, 0)); // overwrite the text with black
  matrix.fillRect(0, 24, 5 ,8, matrix.color444(0, 0, 0)); // overwrite the text with black
  matrix.setTextColor(matrix.color555(31,31,31));
  matrix.setCursor(0, 0); 
  matrix.println("*");
   if(Select){
    LADDERS = 1;
    choiceMadeInterval = 1;
    }
   }

 if(Menu==2 && !choiceMadeInterval){
  matrix.fillRect(0, 0, 5 ,8, matrix.color444(0, 0, 0)); // overwrite the text with black
  matrix.fillRect(0, 16, 5 ,8, matrix.color444(0, 0, 0)); // overwrite the text with black
  matrix.fillRect(0, 24, 5 ,8, matrix.color444(0, 0, 0)); // overwrite the text with black
  matrix.setTextColor(matrix.color555(31,31,31));
  matrix.setCursor(0, 8); 
  matrix.println("*");
   if(Select){
    XonYoff= 1;
    choiceMadeInterval = 1;
    }
   }
   
   if(Menu==3 && !choiceMadeInterval){
    matrix.fillRect(0, 0, 5 ,8, matrix.color444(0, 0, 0)); // overwrite the text with black
    matrix.fillRect(0, 8, 5 ,8, matrix.color444(0, 0, 0)); // overwrite the text with black
    matrix.fillRect(0, 24, 5 ,8, matrix.color444(0, 0, 0)); // overwrite the text with black
    matrix.setTextColor(matrix.color555(31,31,31));
    matrix.setCursor(0, 16); 
    matrix.println("*");
   if(Select){
    EMOTM= 1;
    choiceMadeInterval = 1;
    }
   }

     if(Menu==4 && !choiceMadeInterval){
      matrix.fillRect(0, 0, 5 ,8, matrix.color444(0, 0, 0)); // overwrite the text with black
      matrix.fillRect(0, 8, 5 ,8, matrix.color444(0, 0, 0)); // overwrite the text with black
      matrix.fillRect(0, 16, 5 ,8, matrix.color444(0, 0, 0)); // overwrite the text with black
      matrix.setTextColor(matrix.color555(31,31,31));
      matrix.setCursor(0, 24); 
  matrix.println("*");
   if(Select){
    TABATA= 1;
   choiceMadeInterval = 1;
    }
   }
   
if(ONEmenuUP){
  doOnceInterval = 1;
  Interval = 0;
  Menu =1;
  choiceMade = 0;
  Select = 0;
  ONEmenuUP = 0;
  choiceMadeInterval = 0;
  buttonStateONEmenuUP = LOW;
  matrix.fillScreen(matrix.color444(0, 0, 0)); //Clear screen
  printMainMenu();
  }
//////////////////////////////////////////
//
// X on Y OFF menu and setup
//
//////////////////////////////////////////

while(XonYoff){
buttonPressedTopXOnYOff(); // TOP half of the button functions


if(doOnceXonYoff){
  matrix.fillScreen(matrix.color444(0, 0, 0)); //Clear screen
  matrix.setTextColor(matrix.color555(1,15,31));
  matrix.setTextSize(1); 
  matrix.setCursor(0,0); 
  matrix.println(" On :");
  matrix.setTextColor(matrix.color555(15,31,1));
  matrix.setCursor(0, 8); 
  matrix.println(" Off:");
  matrix.setTextColor(matrix.color555(31,15,1));
  matrix.setCursor(0, 16); 
  matrix.println(" Rds:");
  matrix.setTextColor(matrix.color555(1,31,15));
  matrix.setCursor(0, 24); 
  matrix.println("Start");
  
  Menu=1; // Sets the cursor to the top choice
  Select = 0; // resets the Select variable which is reused.
  printXOnYOfftime(XOntime, 0);
  printXOnYOfftime(YOfftime, 8);
  printXOnYOffRDS(XonYoffRounds,16);
  choiceMade =0;
  doOnceXonYoff = 0; // Ensures this statement isn't repeated
  } // end of if(doOnceXonYoff)
  
    
if(Menu==1 && !choiceMade){
  matrix.fillRect(58, 8, 5 ,8, matrix.color444(0, 0, 0)); // overwrite the text with black
  matrix.fillRect(58, 16, 5 ,8, matrix.color444(0, 0, 0)); // overwrite the text with black
  matrix.fillRect(58, 24, 5 ,8, matrix.color444(0, 0, 0)); // overwrite the text with black
  matrix.setTextColor(matrix.color555(31,0,0));
  matrix.setCursor(58, 0); 
  matrix.println("*");   // Draw menu star
 
if(XOntime != XOnYOffctr){  // Setting the time via the up down button
   if(XOnYOffctr==0){    // After menu switch, XOnYOffctr becomes 0, and then the old value is reloaded
    XOnYOffctr = XOntime;
    }

     if(XOnYOffctr > XOntime){ // if increment, different increment depending on time size
      if(XOnYOffctr>=600 && XOnYOffctr<=3600){
        XOnYOffctr= XOnYOffctr +59; //60 sec
        }
       if(XOnYOffctr>=300 && XOnYOffctr<600){
      XOnYOffctr= XOnYOffctr +29; //30 sec
      }
      if(XOnYOffctr>=120 && XOnYOffctr <300){
      XOnYOffctr= XOnYOffctr +14; //15 sec
      }
       if(XOnYOffctr>=60 && XOnYOffctr <120){
         XOnYOffctr= XOnYOffctr +9; // 10 sec
      }
       if(XOnYOffctr<60){
        XOnYOffctr= XOnYOffctr +4; // 5 sec
        }
       }
          if(XOnYOffctr < XOntime){ // if decrement, different increment depending on time size

             if(XOnYOffctr>=600 && XOnYOffctr<=3600){
        XOnYOffctr= XOnYOffctr -59; //60 sec
        }
         if(XOnYOffctr>=300 && XOnYOffctr<600){
      XOnYOffctr= XOnYOffctr -29; //30 sec
      }
       if(XOnYOffctr>=120 && XOnYOffctr <300){
      XOnYOffctr= XOnYOffctr -14; //15 sec
      }
       if(XOnYOffctr>=60 && XOnYOffctr <120){
      XOnYOffctr= XOnYOffctr -9; // 10 sec
      }
            if(XOnYOffctr<60){
        XOnYOffctr= XOnYOffctr -4; // 5 sec
        }
       }
   }    
       if(XOnYOffctr>3600){ // limits to one hours
      XOnYOffctr=3600;
      }
      if(XOnYOffctr<10){  // limits to above 5 sec
      XOnYOffctr=10;
      }
      XOntime  = XOnYOffctr;
 
   if(XOntime != XOntimeOLD){
    printXOnYOfftime(XOntime, 0);  // print time on line 0
      XOntimeOLD = XOntime;
      }
     } // end of if(Menu==1 && !choiceMade)

  if(Menu==2 && !choiceMade){
  matrix.fillRect(58, 0, 5 ,8, matrix.color444(0, 0, 0)); // overwrite the text with black
  matrix.fillRect(58, 16, 5 ,8, matrix.color444(0, 0, 0)); // overwrite the text with black
  matrix.fillRect(58, 24, 5 ,8, matrix.color444(0, 0, 0)); // overwrite the text with black
  matrix.setTextColor(matrix.color555(31,0,0));
  matrix.setCursor(58, 8); 
  matrix.println("*");

   if(YOfftime != XOnYOffctr){  // Setting the time via the up down button
   if(XOnYOffctr==0){    // After menu switch, XOnYOffctr becomes 0, and then the old value is reloaded
    XOnYOffctr = YOfftime;
    }

      
    if(XOnYOffctr > YOfftime){ // if increment, different increment depending on time size

      if(XOnYOffctr>=600 && XOnYOffctr<=3600){
        XOnYOffctr= XOnYOffctr +59; //60 sec
        }
       if(XOnYOffctr>=300 && XOnYOffctr<600){
      XOnYOffctr= XOnYOffctr +29; //30 sec
      }
      if(XOnYOffctr>=120 && XOnYOffctr <300){
      XOnYOffctr= XOnYOffctr +14; //15 sec
      }
       if(XOnYOffctr>=60 && XOnYOffctr <120){
         XOnYOffctr= XOnYOffctr +9; // 10 sec
      }
       if(XOnYOffctr<60){
        XOnYOffctr= XOnYOffctr +4; // 5 sec
        }
       }

          if(XOnYOffctr < YOfftime){ // if decrement, different increment depending on time size

             if(XOnYOffctr>=600 && XOnYOffctr<=3600){
        XOnYOffctr= XOnYOffctr -59; //60 sec
        }
         if(XOnYOffctr>=300 && XOnYOffctr<600){
      XOnYOffctr= XOnYOffctr -29; //30 sec
      }
       if(XOnYOffctr>=120 && XOnYOffctr <300){
      XOnYOffctr= XOnYOffctr -14; //15 sec
      }
       if(XOnYOffctr>=60 && XOnYOffctr <120){
      XOnYOffctr= XOnYOffctr -9; // 10 sec
      }
            if(XOnYOffctr<60){
        XOnYOffctr= XOnYOffctr -4; // 5 sec
        }
       }
   }    
       if(XOnYOffctr>3600){
      XOnYOffctr=3600;
      }
        if(XOnYOffctr<10){
      XOnYOffctr=10;
      }
      YOfftime  = XOnYOffctr;
 


      if(YOfftime != YOfftimeOLD){
    printXOnYOfftime(YOfftime, 8);// print time on line 8
      YOfftimeOLD = YOfftime;
      }
     } // end of if(Menu==2 && !choiceMade)

      if(Menu==3 && !choiceMade){
  matrix.fillRect(58, 0, 5 ,8, matrix.color444(0, 0, 0)); // overwrite the text with black
  matrix.fillRect(58, 8, 5 ,8, matrix.color444(0, 0, 0)); // overwrite the text with black
  matrix.fillRect(58, 24, 5 ,8, matrix.color444(0, 0, 0)); // overwrite the text with black
  matrix.setTextColor(matrix.color555(31,0,0));
  matrix.setCursor(58, 16); 
  matrix.println("*");

  if(XonYoffRounds != XOnYOffctr){  // Setting the time via the up down button
   if(XOnYOffctr==0){    // After menu switch, XOnYOffctr becomes 0, and then the old value is reloaded
    XOnYOffctr = XonYoffRounds;
    }
    XonYoffRounds =XOnYOffctr ; // If ctr is different from rds value, and not 0, then increment or decrease as instructed.
  }
        if(XonYoffRounds<1){
        XonYoffRounds=1;    // KEEP the number of rounds greater than one
        }
        if(XonYoffRounds>99){
          XonYoffRounds =99; // Keep number of rds less than 1000
            XOnYOffctr =XonYoffRounds;
          }
          
    if(XonYoffRounds !=XonYoffRoundsOLD){
      printXOnYOffRDS(XonYoffRounds,16); // print the rounds on line 16
      XonYoffRoundsOLD = XonYoffRounds;
     }

   } // end of if(Menu==3 && !choiceMade)

    if(Menu==4 && !choiceMade){
  if(doOnceUNEVENmenu5){ 
  matrix.fillRect(30, 0, 34 ,32, matrix.color444(0, 0, 0)); // overwrite the text with black
  matrix.setCursor(30, 2); 
  matrix.println("PRESS");
  matrix.setCursor(30, 12);
  matrix.println("UP TO");
  matrix.setCursor(30, 22);
  matrix.println("START");
  doOnceUNEVENmenu5 =0;
  UNEVENctr=0;
  }
if (XOnYOffctr){
  XOnYOfftimer=1;
  choiceMade =1;
  XonYoff=0;
  }

   } // end of if(Menu==4 && !choiceMade)
   
if(Menu==5 && !choiceMade){
  // Re write menu with values.
   matrix.fillRect(30, 0, 34 ,32, matrix.color444(0, 0, 0)); // overwrite the text with black
 matrix.setTextColor(matrix.color555(31,0,0)); // Red
 printXOnYOfftime(XOntime, 0);
  printXOnYOfftime(YOfftime, 8);
  printXOnYOffRDS(XonYoffRounds,16);
   Menu =1;
  doOnceUNEVENmenu5 =1;
    } // end of if(Menu==5 && !choiceMade)

buttonPressedLoopBOTTOM(); // bottom half of the button functions
  }


//////////////////////////////////////
//
// The actual X on Y off interval timer
//
//////////////////////////////////////


while(XOnYOfftimer){
 
   
 if(doOnceXOnYOfftimer){   // do once run count down.

    doOnceXOnYOfftimer = 0;

for(i=XonYoffRounds; i>0; i-- ){
     countDownToStart();
    startMillis = millis();
    matrix.fillScreen(matrix.color444(0, 0, 0)); //Clear screen
    fourDigits=1;
    //roundXofY(XonYoffRounds, i);

 // possible if loop to limit the other X Y loops
  XOntimeRUN =1;
  YOfftimeRUN = 1;
  
  XOntimeTimeHolder = (long)XOntime*1000;
  YOfftimeTimeHolder = (long)YOfftime*1000;
  
  while(XOntimeRUN){
    
      currentMillis = (millis()- startMillis)*1;  //To accelerate time while testing, set multiplyer here

        if(XOntimeTimeHolder>= currentMillis){
         difference_millis = XOntimeTimeHolder-currentMillis;
       if(difference_millis >0){                                  /*  The actual count down loop!*/
        time_fractionCountdown(difference_millis, 1);  // displaying the time remaining
   
   sum_MERKULIN[i-1] = CountingREPS();  // Displaying reps
         }
        }
       if(XOntimeTimeHolder < currentMillis) {
    matrix.fillScreen(matrix.color444(0, 0, 0)); //Clear the screen
    matrix.setTextColor(matrix.color555(31,0,0));
    matrix.setTextSize(2);
    matrix.setCursor(10,8);
    matrix.print("STOP");
    startMillis = millis(); // Begins the Y time
    horn(); // end of set horn and counter reset
    XOntimeRUN = 0;
    YOfftimeRUN=1;
    doOnceYOfftimeRUN = 1;
    doOnceTimer = 1;
     matrix.fillScreen(matrix.color444(0, 0, 0)); //Clear the screen
  }
 } // end of while(XOntimeRUN
 
if(i==1){   // If  X has run he approproate amount of times, stop the loop, because Y has to run one less round than X
 
   XOntimeRUN =0;
  YOfftimeRUN =0;
// break;  
}

while(YOfftimeRUN){
    if(doOnceYOfftimeRUN){
    matrix.setTextColor(matrix.color555(1,15,31));
    matrix.setTextSize(1);
    matrix.setCursor(18,0);
    matrix.print("PAUSE"); // write Pause on screen
    roundXofY(XonYoffRounds,i-1); // write the round X of Y on screen
    doOnceYOfftimeRUN = 0;
    }
    
    currentMillis = (millis()- startMillis)*1;  //To accelerate time while testing, set multiplyer here
    if(YOfftimeTimeHolder>= currentMillis){
        difference_millis = YOfftimeTimeHolder-currentMillis;
    if(difference_millis >6500){ 
          time_fractionPause(difference_millis); // write remaining pause time on screen

         matrix.setTextColor(matrix.color555(1,15,31)); // VW blue
         matrix.setTextSize(1);
         matrix.setCursor(6,16);
         matrix.print("Reps"); // write Pause on screen
         
         matrix.setTextColor(matrix.color555(31,15,1));
         matrix.setCursor(35,16);
         matrix.print(sum_MERKULIN[i-1]);
         }
        if(difference_millis <=6500){
    YOfftimeRUN=0;
    XOntimeRUN = 1;
         }
    }
  } //end of while(YOfftimeRUN)
  
 } // end of for(i=XonYoffRounds; i>0; i-- )
 } // End of if(doOnceXOnYOfftimer)
if(doOnceEndXOnYOff){
 matrix.fillScreen(matrix.color444(0, 0, 0)); //Clear the screen
// Printing end text
    matrix.setTextSize(1);
    matrix.setTextColor(matrix.color555(1,15,31));
    matrix.setCursor(8,0);
    matrix.print("Rds:");
    matrix.setCursor(8,8);
    matrix.print(" ON:");
    matrix.setCursor(8,16);
    matrix.print("Off:");
    matrix.setTextColor(matrix.color555(31,0,0));
    printXOnYOfftime(XOntime, 8);
    printXOnYOfftime(YOfftime, 16);
 // subdividing the rounds and printing them nicely
     int TenRDS = XonYoffRounds/10;
     XonYoffRounds = XonYoffRounds- 10*TenRDS;
    matrix.setTextColor(matrix.color555(31,0,0));
        if(TenRDS>0){
      matrix.setCursor(45, 0); 
       matrix.println(TenRDS);
       }
       matrix.setCursor(51, 0); 
       matrix.println(XonYoffRounds);

      int s = 0; // s is the total sum of reps performed
      for (i=0; i< XonYoffRounds; i++)
    {
    s = s+ sum_MERKULIN[i];
      }
      matrix.setTextColor(matrix.color555(1,15,31));
   matrix.setCursor(2,24);
    matrix.print("REPS:");
    matrix.setTextColor(matrix.color555(31,15,1));
     matrix.setCursor(37,24);
    matrix.print(s);

// Display end result
doOnceEndXOnYOff = 0;
}
} // end of while(XOnYOfftimer)


////////////////////////////////////////////////
//
//  EMOTM
//
///////////////////////////////////////////////


while(EMOTM){
  buttonPressedTopEMOTM(); // TOP half of the button functions

if(doOnceXonYoff){ // reused from X on Y off
  matrix.fillScreen(matrix.color444(0, 0, 0)); //Clear screen
  matrix.setTextColor(matrix.color555(1,15,31));
  matrix.setTextSize(1); 
  matrix.setCursor(0,0); 
  matrix.println(" Base");
  matrix.setTextColor(matrix.color555(15,31,1));
  matrix.setCursor(0, 8); 
  matrix.println("Iter");
  matrix.setCursor(22, 8); 
  matrix.println(".");
  matrix.setTextColor(matrix.color555(1,31,15));
  matrix.setCursor(0, 16); 
  matrix.println("Start");

  XonYoffRounds = 10;
  Menu=1; // Sets the cursor to the top choice
  Select = 0; // resets the Select variable which is reused.
  printXOnYOfftime(XOntime, 0);
  printXOnYOffRDS(XonYoffRounds,8);
  choiceMade =0;
  doOnceXonYoff = 0; // Ensures this statement isn't repeated
  }

if(Menu==1 && !choiceMade){
  matrix.fillRect(58, 8, 5 ,8, matrix.color444(0, 0, 0)); // overwrite the text with black
  matrix.setTextColor(matrix.color555(31,0,0));
  matrix.setCursor(58, 0); 
  matrix.println("*");   // Draw menu star
 
if(XOntime != XOnYOffctr){  // Setting the time via the up down button
   if(XOnYOffctr==0){    // After menu switch, XOnYOffctr becomes 0, and then the old value is reloaded
    XOnYOffctr = XOntime;
    }

      
    if(XOnYOffctr > XOntime){ // if increment, different increment depending on time size

    
       if(XOnYOffctr>=300 && XOnYOffctr<600){
      XOnYOffctr= XOnYOffctr +29; //30 sec
      }
      if(XOnYOffctr>=120 && XOnYOffctr <300){
      XOnYOffctr= XOnYOffctr +14; //15 sec
      }
       if(XOnYOffctr>=60 && XOnYOffctr <120){
         XOnYOffctr= XOnYOffctr +9; // 10 sec
      }
       if(XOnYOffctr<60){
        XOnYOffctr= XOnYOffctr +4; // 5 sec
        }
       }

          if(XOnYOffctr < XOntime){ // if decrement, different increment depending on time size

   
         if(XOnYOffctr>=300 && XOnYOffctr<600){
      XOnYOffctr= XOnYOffctr -29; //30 sec
      }
       if(XOnYOffctr>=120 && XOnYOffctr <300){
      XOnYOffctr= XOnYOffctr -14; //15 sec
      }
       if(XOnYOffctr>=60 && XOnYOffctr <120){
      XOnYOffctr= XOnYOffctr -9; // 10 sec
      }
            if(XOnYOffctr<60){
        XOnYOffctr= XOnYOffctr -4; // 5 sec
        }
       }
   }    
       if(XOnYOffctr>600){ // limits base to 10 min max
      XOnYOffctr=600;
      }
      if(XOnYOffctr<10){  // limits base to above 10 sec
      XOnYOffctr=10;
      }
      XOntime  = XOnYOffctr;
 


      if(XOntime != XOntimeOLD){
    printXOnYOfftime(XOntime, 0);  // print time on line 0
      XOntimeOLD = XOntime;
      }
     }

     
   if(Menu==2 && !choiceMade){
  matrix.fillRect(58, 0, 5 ,8, matrix.color444(0, 0, 0)); // overwrite the text with black
  matrix.setTextColor(matrix.color555(31,0,0));
  matrix.setCursor(58, 8); 
  matrix.println("*");

  if(XonYoffRounds != XOnYOffctr){  // Setting the time via the up down button
   if(XOnYOffctr==0){    // After menu switch, XOnYOffctr becomes 0, and then the old value is reloaded
    XOnYOffctr = XonYoffRounds;
    }
        
    XonYoffRounds =XOnYOffctr ; // If ctr is different from rds value, and not 0, then increment or decrease as instructed.
  }
        if(XonYoffRounds<1){
        XonYoffRounds=1;    // KEEP the number of rounds greater than one
        }
        if(XonYoffRounds>99){
          XonYoffRounds =99; // Keep number of rds to 21
            XOnYOffctr =XonYoffRounds;
          }
          
    if(XonYoffRounds !=XonYoffRoundsOLD){
      printXOnYOffRDS(XonYoffRounds,8); // print the rounds on line 16
      XonYoffRoundsOLD = XonYoffRounds;
     }

   }
       if(Menu==3 && !choiceMade){
  if(doOnceUNEVENmenu5){ 
  matrix.fillRect(30, 0, 34 ,32, matrix.color444(0, 0, 0)); // overwrite the text with black
  matrix.setCursor(30, 2); 
  matrix.println("PRESS");
  matrix.setCursor(30, 12);
  matrix.println("UP TO");
  matrix.setCursor(30, 22);
  matrix.println("START");
  doOnceUNEVENmenu5 =0;
  UNEVENctr=0;
  }
if (XOnYOffctr){
  EMOTMtimer=1;
  choiceMade =1;
  EMOTM=0;
  }

   }
if(Menu==4 && !choiceMade){
  // Re write menu with values.
   matrix.fillRect(30, 0, 34 ,32, matrix.color444(0, 0, 0)); // overwrite the text with black
 matrix.setTextColor(matrix.color555(31,0,0)); // Red
 printXOnYOfftime(XOntime, 0);
 printXOnYOffRDS(XonYoffRounds,8);
   Menu =1;
  doOnceUNEVENmenu5 =1;
    }


      buttonPressedLoopBOTTOM(); // bottom half of the button functions
  
  }// end of while(EMOTM)


////////////////////////////////////////
//
// EMOTM timer loop
//
////////////////////////////////////////

while(EMOTMtimer){
  
   if(doOnceXOnYOfftimer){   // do once run count down.
    countDownToStart();
    doOnceXOnYOfftimer = 0;
 } // End of if(doOnceXOnYOfftimer)
 
for(i=XonYoffRounds; i>0; i-- ){
     
    startMillis = millis();
    matrix.fillScreen(matrix.color444(0, 0, 0)); //Clear screen
    fourDigits=1;
    roundXofY(XonYoffRounds, i);

 // possible if loop to limit the other X Y loops
  XOntimeRUN =1;
  
  
  XOntimeTimeHolder = (long)XOntime*1000;

  
  while(XOntimeRUN){
    
      currentMillis = (millis()- startMillis)*1;  //To accelerate time while testing, set multiplyer here

        if(XOntimeTimeHolder>= currentMillis ){
         difference_millis = XOntimeTimeHolder-currentMillis;
       if(difference_millis >0){                                  /*  The actual count down loop!*/
        time_fractionCountdown(difference_millis, 1);  // displaying the time remaining
         }
        }
       if(XOntimeTimeHolder < currentMillis && i>1) {
    matrix.fillScreen(matrix.color444(0, 0, 0)); //Clear the screen
    matrix.setTextColor(matrix.color555(0,31,0));
    matrix.setTextSize(2);
    matrix.setCursor(22,8);
    matrix.print("GO");
    startMillis = millis(); // Begins the Y time
    shortHorn(); // end of set horn and counter reset
    XOntimeRUN = 0;
    
    doOnceYOfftimeRUN = 1;
    doOnceTimer = 1;
     matrix.fillScreen(matrix.color444(0, 0, 0)); //Clear the screen
  }

     if(XOntimeTimeHolder < currentMillis && i ==1) {
    matrix.fillScreen(matrix.color444(0, 0, 0)); //Clear the screen
    matrix.setTextColor(matrix.color555(31,0,0));
    matrix.setTextSize(2);
    matrix.setCursor(10,8);
    matrix.print("STOP");
    startMillis = millis(); // Begins the Y time
    horn(); // end of set horn and counter reset
     
    XOntimeRUN =0;

    doOnceEndXOnYOff = 1;
   EMOTMtimer = 0;
   endOfEMOTM = 1;
     matrix.fillScreen(matrix.color444(0, 0, 0)); //Clear the screen
  }
  
  } // end of while(XOntimeRUN)

}// end of while(EMOTMtimer)

  
 } // end of for(i=XonYoffRounds; i>0; i-- )

while(endOfEMOTM){
if(doOnceEndXOnYOff){
 matrix.fillScreen(matrix.color444(0, 0, 0)); //Clear the screen
matrix.setTextColor(matrix.color555(1,31,1));
    matrix.setTextSize(1);
    matrix.setCursor(5,0);
    matrix.print("END EMOTM");
    matrix.setTextColor(matrix.color555(1,15,31));
   
    matrix.setCursor(8,16);
    matrix.print("Rds:");
    matrix.setCursor(0,24);
    matrix.print(" Time:");
    printXOnYOfftime(XOntime, 24);
    
// printing end rounds
    int TenRDS = XonYoffRounds/10;
     XonYoffRounds = XonYoffRounds- 10*TenRDS;
    matrix.setTextColor(matrix.color555(31,0,0));
        if(TenRDS>0){
      matrix.setCursor(45, 16); 
       matrix.println(TenRDS);
       }
       matrix.setCursor(51, 16); 
       matrix.println(XonYoffRounds);
       
   
// Display end result
doOnceEndXOnYOff = 0;
}

 } // end of while(endOfEMOTM)
  
  
  
////////////////////////////////////////////
//
// LADDERS MENU
//
////////////////////////////////////////////

while(LADDERS){

    //Menu text written in previous MENU select.
 if(doOnceLadders){
    printLADDERSMenu(); // Prints the menu
    Menu=1; // Sets the cursor to the top choice
    doOnceLadders = 0; // Ensures this statement isn't repeated
    choiceMadeInterval =0;
    Select = 0; // resets the Select variable which is reused.
    }
   
  buttonPressedLoopTOP(); // TOP half of the button functions
  

if(Menu>3){
  Menu = 1;
  }

  if(Menu<1){
  Menu = 3;
  }

 if(Menu==1 && !choiceMadeInterval){
  matrix.fillRect(0, 8, 5 ,8, matrix.color444(0, 0, 0)); // overwrite the text with black
  matrix.fillRect(0, 16, 5 ,8, matrix.color444(0, 0, 0)); // overwrite the text with black
  matrix.fillRect(0, 24, 5 ,8, matrix.color444(0, 0, 0)); // overwrite the text with black
  matrix.setTextColor(matrix.color555(31,31,31));
  matrix.setCursor(0, 0); 
  matrix.println("*");
   if(Select){
    MERKULIN= 1;
    choiceMadeInterval = 1;
    }
   }// end of Menu==1 && !choiceMadeInterval


 if(Menu==2 && !choiceMadeInterval){
  matrix.fillRect(0, 0, 5 ,8, matrix.color444(0, 0, 0)); // overwrite the text with black
  matrix.fillRect(0, 8, 5 ,8, matrix.color444(0, 0, 0)); // overwrite the text with black
  matrix.fillRect(0, 24, 5 ,8, matrix.color444(0, 0, 0)); // overwrite the text with black
  matrix.setTextColor(matrix.color555(31,31,31));
  matrix.setCursor(0, 16); 
  matrix.println("*");
   if(Select){
    UNEVENup= 1;
    UNEVEN = 1;
    choiceMadeInterval = 1;
    }
   }// end of Menu==2 && !choiceMadeInterval
   
   if(Menu==3 && !choiceMadeInterval){
    matrix.fillRect(0, 0, 5 ,8, matrix.color444(0, 0, 0)); // overwrite the text with black
    matrix.fillRect(0, 8, 5 ,8, matrix.color444(0, 0, 0)); // overwrite the text with black
    matrix.fillRect(0, 16, 5 ,8, matrix.color444(0, 0, 0)); // overwrite the text with black
    matrix.setTextColor(matrix.color555(31,31,31));
    matrix.setCursor(0, 24); 
    matrix.println("*");
   if(Select){
    UNEVENdown = 1;
    UNEVEN =1;
    choiceMadeInterval = 1;
    }
   } // end of Menu==3 && !choiceMadeInterval


////////////////////////////////////////////
//
// UNEVEN UP and DOWN
//
///////////////////////////////////////////

while(UNEVEN){
buttonPressedTopTABATA(); // TOP half of the button functions
  if(doOnceUneven){
matrix.fillScreen(matrix.color444(0, 0, 0)); //Clear screen
  matrix.setTextColor(matrix.color555(1,15,31));
  matrix.setTextSize(1); 
  matrix.setCursor(0,0); 
  matrix.println(" BASE:");
  matrix.setTextColor(matrix.color555(15,31,1));
  matrix.setCursor(0, 8); 
  matrix.println("  RDS:");
  matrix.setTextColor(matrix.color555(31,15,1));
  matrix.setCursor(0, 16); 
  matrix.println("PAUSE");
  matrix.setTextColor(matrix.color555(1,31,15));
  matrix.setCursor(0, 24); 
  matrix.println(" Step:");
  TabataOntime = 120;
  TabataOfftime = 60;
  PauseSteps = 15;
  TabataRounds = 4;
  Menu=1; // Sets the cursor to the third choice
  Select = 0; // resets the Select variable which is reused.
  printXOnYOfftime(TabataOntime, 0);  // BASE TIME
  printXOnYOffRDS(TabataRounds,8); // number of rounds
  printXOnYOfftime(TabataOfftime, 16); // PAUSE time
  printXOnYOfftime(PauseSteps, 24); // PAUSE time
  choiceMade =0;

    doOnceUneven = 0;
    } // end of doOnceUneven
  
    
if(Menu==1 && !choiceMade){
  matrix.fillRect(58, 8, 5 ,8, matrix.color444(0, 0, 0)); // overwrite the text with black
  matrix.fillRect(58, 16, 5 ,8, matrix.color444(0, 0, 0)); // overwrite the text with black
  matrix.fillRect(58, 24, 5 ,8, matrix.color444(0, 0, 0)); // overwrite the text with black
  matrix.setTextColor(matrix.color555(31,0,0));
  matrix.setCursor(58, 0); 
  matrix.println("*");   // Draw menu star
 
if(TabataOntime != XOnYOffctr){  // Setting the time via the up down button
   if(XOnYOffctr==0){    // After menu switch, XOnYOffctr becomes 0, and then the old value is reloaded
    XOnYOffctr = TabataOntime;
    }   
        if(XOnYOffctr > TabataOntime){ // if increment, different increment depending on time size

       if(XOnYOffctr>=600 && XOnYOffctr<1201){
      XOnYOffctr= XOnYOffctr +59; //30 sec
      }
       if(XOnYOffctr>=300 && XOnYOffctr<600){
      XOnYOffctr= XOnYOffctr +29; //30 sec
      }
      if(XOnYOffctr>=120 && XOnYOffctr <300){
      XOnYOffctr= XOnYOffctr +14; //15 sec
      }
       if(XOnYOffctr>=60 && XOnYOffctr <120){
         XOnYOffctr= XOnYOffctr +9; // 10 sec
      }
       if(XOnYOffctr<60){
        XOnYOffctr= XOnYOffctr +4; // 5 sec
        }
       }

          if(XOnYOffctr < TabataOntime){ // if decrement, different increment depending on time size
       
        if(XOnYOffctr>=600 && XOnYOffctr<1201){
      XOnYOffctr= XOnYOffctr -59; //60 sec
      }
   
         if(XOnYOffctr>=300 && XOnYOffctr<600){
      XOnYOffctr= XOnYOffctr -29; //30 sec
      }
       if(XOnYOffctr>=120 && XOnYOffctr <300){
      XOnYOffctr= XOnYOffctr -14; //15 sec
      }
       if(XOnYOffctr>=60 && XOnYOffctr <120){
      XOnYOffctr= XOnYOffctr -9; // 10 sec
      }
            if(XOnYOffctr<60){
        XOnYOffctr= XOnYOffctr -4; // 5 sec
        }
       }
   }    
       if(XOnYOffctr>1200){ // limits to one hours
      XOnYOffctr=1200;
      }
      if(XOnYOffctr<10){  // limits to above 5 sec
      XOnYOffctr=10;
      }
      TabataOntime  = XOnYOffctr;
 
      if(TabataOntime != TabataOntimeOLD){
    printXOnYOfftime(TabataOntime, 0);  // print time on line 0
      TabataOntimeOLD = TabataOntime;
      }
     } // end of Menu==1 && !choiceMade
     
     if(Menu==2 && !choiceMade){
  matrix.fillRect(58, 0, 5 ,8, matrix.color444(0, 0, 0)); // overwrite the text with black
  matrix.fillRect(58, 16, 5 ,8, matrix.color444(0, 0, 0)); // overwrite the text with black
  matrix.fillRect(58, 24, 5 ,8, matrix.color444(0, 0, 0)); // overwrite the text with black
  matrix.setTextColor(matrix.color555(31,0,0));
  matrix.setCursor(58, 8); 
  matrix.println("*");

  if(TabataRounds != XOnYOffctr){  // Setting the time via the up down button
   if(XOnYOffctr==0){    // After menu switch, XOnYOffctr becomes 0, and then the old value is reloaded
    XOnYOffctr = TabataRounds;
    }
    TabataRounds =XOnYOffctr ; // If ctr is different from rds value, and not 0, then increment or decrease as instructed.
  }
        if(TabataRounds<1){
        TabataRounds=1;    // KEEP the number of rounds greater than one
        }
        if(TabataRounds>20){
          TabataRounds =20;  // Keep number of rds less than 100
          XOnYOffctr = TabataRounds;
          }
          
    if(TabataRounds !=TabataRoundsOLD){
      printXOnYOffRDS(TabataRounds,8); // print the rounds on line 8
      TabataRoundsOLD = TabataRounds;
     }

   } // end of Menu==2 && !choiceMade

   if(Menu==3 && !choiceMade){
  matrix.fillRect(58, 0, 5 ,8, matrix.color444(0, 0, 0)); // overwrite the text with black
  matrix.fillRect(58, 8, 5 ,8, matrix.color444(0, 0, 0)); // overwrite the text with black
  matrix.fillRect(58, 24, 5 ,8, matrix.color444(0, 0, 0)); // overwrite the text with black
  matrix.setTextColor(matrix.color555(31,0,0));
  matrix.setCursor(58, 16); 
  matrix.println("*");

   if(TabataOfftime != XOnYOffctr){  // Setting the time via the up down button
   if(XOnYOffctr==0){    // After menu switch, XOnYOffctr becomes 0, and then the old value is reloaded
    XOnYOffctr = TabataOfftime;
    }      
        if(XOnYOffctr > TabataOfftime){ // if increment, different increment depending on time size

    
       if(XOnYOffctr>=300 && XOnYOffctr<600){
      XOnYOffctr= XOnYOffctr +29; //30 sec
      }
      if(XOnYOffctr>=120 && XOnYOffctr <300){
      XOnYOffctr= XOnYOffctr +14; //15 sec
      }
       if(XOnYOffctr>=60 && XOnYOffctr <120){
         XOnYOffctr= XOnYOffctr +9; // 10 sec
      }
       if(XOnYOffctr<60){
        XOnYOffctr= XOnYOffctr +4; // 5 sec
        }
       }

          if(XOnYOffctr < TabataOfftime){ // if decrement, different increment depending on time size

   
         if(XOnYOffctr>=300 && XOnYOffctr<600){
      XOnYOffctr= XOnYOffctr -29; //30 sec
      }
       if(XOnYOffctr>=120 && XOnYOffctr <300){
      XOnYOffctr= XOnYOffctr -14; //15 sec
      }
       if(XOnYOffctr>=60 && XOnYOffctr <120){
      XOnYOffctr= XOnYOffctr -9; // 10 sec
      }
            if(XOnYOffctr<60){
        XOnYOffctr= XOnYOffctr -4; // 5 sec
        }
       }
   }    

//   if(XOnYOffctr >TabataOntime){
//    XOnYOffctr =TabataOntime;
//    }
       if(XOnYOffctr>600){ // limits to 60 sec
      XOnYOffctr=600;
      }
        if(XOnYOffctr<5){ // limits to 5 sec
      XOnYOffctr=5;
      }
      TabataOfftime  = XOnYOffctr;
 
      if(TabataOfftime != TabataOfftimeOLD){
    printXOnYOfftime(TabataOfftime, 16);// print time on line 8
      TabataOfftimeOLD = TabataOfftime;
      }
     } //Menu==3 && !choiceMade

      if(Menu==4 && !choiceMade){
  matrix.fillRect(58, 0, 5 ,8, matrix.color444(0, 0, 0)); // overwrite the text with black
  matrix.fillRect(58, 8, 5 ,8, matrix.color444(0, 0, 0)); // overwrite the text with black
  matrix.fillRect(58, 16, 5 ,8, matrix.color444(0, 0, 0)); // overwrite the text with black
  matrix.setTextColor(matrix.color555(31,0,0));
  matrix.setCursor(58, 24); 
  matrix.println("*");

   if(PauseSteps != XOnYOffctr){  // Setting the time via the up down button
   if(XOnYOffctr==0){    // After menu switch, XOnYOffctr becomes 0, and then the old value is reloaded
    XOnYOffctr = PauseSteps;
    }      
        if(XOnYOffctr > PauseSteps){ // if increment, different increment depending on time size

    
       if(XOnYOffctr>=300 && XOnYOffctr<600){
      XOnYOffctr= XOnYOffctr +29; //30 sec
      }
      if(XOnYOffctr>=120 && XOnYOffctr <300){
      XOnYOffctr= XOnYOffctr +14; //15 sec
      }
       if(XOnYOffctr>=60 && XOnYOffctr <120){
         XOnYOffctr= XOnYOffctr +9; // 10 sec
      }
       if(XOnYOffctr<60){
        XOnYOffctr= XOnYOffctr +4; // 5 sec
        }
       }

          if(XOnYOffctr < PauseSteps){ // if decrement, different increment depending on time size

   
         if(XOnYOffctr>=300 && XOnYOffctr<600){
      XOnYOffctr= XOnYOffctr -29; //30 sec
      }
       if(XOnYOffctr>=120 && XOnYOffctr <300){
      XOnYOffctr= XOnYOffctr -14; //15 sec
      }
       if(XOnYOffctr>=60 && XOnYOffctr <120){
      XOnYOffctr= XOnYOffctr -9; // 10 sec
      }
            if(XOnYOffctr<60){
        XOnYOffctr= XOnYOffctr -4; // 5 sec
        }
       }
   }    
      if(XOnYOffctr>600 ){ // limits to 600 sec
      XOnYOffctr=600 ;
      }
        if(XOnYOffctr<0){ // limits to 0 sec
      XOnYOffctr=0;
      }
      PauseSteps  = XOnYOffctr;
 
      if(PauseSteps != PauseStepsOLD){
    printXOnYOfftime(PauseSteps, 24);// print time on line 8
      PauseStepsOLD = PauseSteps;
      }
     } // end of Menu==4 && !choiceMade

    if(Menu==5 && !choiceMade){
  if(doOnceUNEVENmenu5){ 
  matrix.fillRect(30, 0, 34 ,32, matrix.color444(0, 0, 0)); // overwrite the text with black
  matrix.setCursor(30, 2); 
  matrix.println("PRESS");
  matrix.setCursor(30, 12);
  matrix.println("UP TO");
  matrix.setCursor(30, 22);
  matrix.println("START");
  doOnceUNEVENmenu5 =0;
  UNEVENctr=0;
  }
if (XOnYOffctr){
  UNEVENtimer=1;
  choiceMade =1;
  UNEVEN=0;
  }
   }// end of Menu==5 && !choiceMade
   
if(Menu==6 && !choiceMade){
  // Re write menu with values.
   matrix.fillRect(30, 0, 34 ,32, matrix.color444(0, 0, 0)); // overwrite the text with black
 matrix.setTextColor(matrix.color555(31,0,0)); // Red
 printXOnYOfftime(TabataOntime, 0);  // BASE TIME
  printXOnYOffRDS(TabataRounds,8); // number of rounds
  printXOnYOfftime(TabataOfftime, 16); // PAUSE time
  printXOnYOfftime(PauseSteps, 24); // PAUSE time
   Menu =1;
  doOnceUNEVENmenu5 =1;
    } // end of Menu==6 && !choiceMade

buttonPressedLoopBOTTOM(); // bottom half of the button functions
  
  
  } // end of while(UNEVEN)

//////////////////////////////////////////////
//
// UNEVEN INTERVAL FUNCTION UP/DOWN
//
//////////////////////////////////////////////

while(UNEVENtimer){ // a bit of reuse
  
     if(doOnceMERKULINtimer){
         matrix.fillScreen(matrix.color444(0, 0, 0)); //Clear screen
         doOnceMERKULINtimer = 0;
         }

for(i=TabataRounds; i>0; i--){
  sum_CDT = 0;
   //work section
 
    if(UNEVENdown){ // one of two functions
    XOntimeTimeHolder = (long)TabataOntime*1000*(i);  // ensures the decrement of time
    YOfftimeTimeHolder = (long)(TabataOfftime+PauseSteps*(i-2))*1000;
    }
        if(UNEVENup){ // one of two functions
    XOntimeTimeHolder = (long)(TabataOntime*(TabataRounds-i+1))*1000;  // ensures the decrement of time
    YOfftimeTimeHolder = (long)(TabataOfftime+PauseSteps*(TabataRounds-i))*1000;
    }
    
    countDownToStart();
    matrix.fillScreen(matrix.color444(0, 0, 0)); //Clear screen
    startMillis = millis();
    doOnceTimer =1;
    if(XOntimeTimeHolder >=3600000){
      fourDigits=0;
    }else{
    fourDigits=1;
    }
    currentMillis = (millis()- startMillis);  
    //roundXofY(XonYoffRounds, i); // shows Round X of Y
 
  while(XOntimeTimeHolder>= currentMillis){ // time ON loop 
    currentMillis = (millis()- startMillis)*1;  //To accelerate time while testing, set multiplyer here

   sum_MERKULIN[i-1] = CountingREPS();  // Displaying reps
     
    difference_millis = XOntimeTimeHolder-currentMillis;
     if(difference_millis >0){                                  
        time_fractionCountdown(difference_millis, 1);  // displaying the time remaining
        }
    }
    
  ///////////////////////////////////////////////////
  // pause section
  ///////////////////////////////////////////////////
  if(i>1){
    startMillis = millis();
    currentMillis = (millis()- startMillis);  
    fourDigits=1;
    matrix.fillScreen(matrix.color444(0, 0, 0)); //Clear screen
    matrix.setTextColor(matrix.color555(31,0,0));
    matrix.setTextSize(2);
    matrix.setCursor(10,8);
    matrix.print("STOP");
    horn();
    
    matrix.fillScreen(matrix.color444(0, 0, 0)); //Clear screen
    int j = i-1;
    roundXofY(TabataRounds, j); // shows Round X of Y
    doOnceTimer = 1; // ensures right amount of digits on screen
    matrix.setTextColor(matrix.color555(0,31,0));
    matrix.setTextSize(1);
    matrix.setCursor(18,0);
    matrix.print("PAUSE");
    

    matrix.setTextColor(matrix.color555(1,15,31));
    matrix.setCursor(8,16);
    matrix.print("REPS");
     matrix.setTextColor(matrix.color555(31,15,1));
    matrix.setCursor(36,16);
    matrix.print(sum_MERKULIN[i-1]);

   while( YOfftimeTimeHolder >= currentMillis+6500){ // time OFF loop 
    currentMillis = (millis()- startMillis)*1;  //To accelerate time while testing, set multiplyer here
    difference_millis =  YOfftimeTimeHolder -currentMillis;
     if(difference_millis >6500){ // takes the countdown into account                                  
       time_fractionPause(difference_millis); // displaying the time remaining
      }
    }
   }
  } // end of for(i=XonYoffRounds; i>0; i--)


      while(1){  // end result loop
      if(doOnceTABATAendResult){
    matrix.fillScreen(matrix.color444(0, 0, 0)); //Clear screen
    matrix.setTextColor(matrix.color555(31,0,0));
    matrix.setTextSize(2);
    matrix.setCursor(10,8);
    matrix.print("STOP");
        horn();
        matrix.fillScreen(matrix.color444(0, 0, 0)); //Clear screen
        matrix.setTextColor(matrix.color555(1,31,1));
         matrix.setTextSize(1);
         
         if(UNEVENdown){
    matrix.setCursor(2,0);
    matrix.print("UNEVEN");
     matrix.setTextColor(matrix.color555(0,1,31));
    matrix.setCursor(40,0);
    matrix.print("DOWN");
         }
            if(UNEVENup){
    matrix.setCursor(8,0);
    matrix.print("UNEVEN");
    matrix.setTextColor(matrix.color555(0,1,31));
    matrix.setCursor(47,0);
    matrix.print("UP");
   
         }
    matrix.setTextColor(matrix.color555(31,1,1));
    matrix.setCursor(9,8);
    matrix.print("COMPLETE");
         
    int s = 0;
      for (k=0; k< TabataRounds; k++)
    {
    s = s+ sum_MERKULIN[k];
      }
    matrix.setTextColor(matrix.color555(1,15,31));
    matrix.setCursor(0,16);
    matrix.print("REPS");
    matrix.setTextColor(matrix.color555(31,15,1));
    matrix.setCursor(40,16);
    matrix.print(s);

    i = i+1; // compensates so the end result displays correctly 
    roundXofY(TabataRounds,i);
    doOnceTABATAendResult = 0;
        }
        // Do nothing
      } // end of while(1)

  
  
  } // end of while(UNEVENtimer)




////////////////////////////////////////////
//
// MERKULIN INTERVAL TIMER
//
////////////////////////////////////////////
while(MERKULIN){
buttonPressedTopXOnYOff(); // TOP half of the button functions


if(doOnceXonYoff){ // reused from X on Y off
  matrix.fillScreen(matrix.color444(0, 0, 0)); //Clear screen
  matrix.setTextColor(matrix.color555(1,15,31));
  matrix.setTextSize(1); 
  matrix.setCursor(0,0); 
  matrix.println(" Base");
  matrix.setTextColor(matrix.color555(15,31,1));
  matrix.setCursor(0, 8); 
  matrix.println("Pause");
  matrix.setTextColor(matrix.color555(31,15,1));
  matrix.setCursor(0, 16); 
  matrix.println("Steps");
  matrix.setTextColor(matrix.color555(1,31,15));
  matrix.setCursor(0, 24); 
  matrix.println("Start");

  XonYoffRounds = 7;
  Menu=1; // Sets the cursor to the top choice
  Select = 0; // resets the Select variable which is reused.
  printXOnYOfftime(XOntime, 0);
  printXOnYOfftime(YOfftime, 8);
  printXOnYOffRDS(XonYoffRounds,16);
  choiceMade =0;
  doOnceXonYoff = 0; // Ensures this statement isn't repeated
  }
  
    
if(Menu==1 && !choiceMade){
  matrix.fillRect(58, 8, 5 ,8, matrix.color444(0, 0, 0)); // overwrite the text with black
  matrix.fillRect(58, 16, 5 ,8, matrix.color444(0, 0, 0)); // overwrite the text with black
  matrix.fillRect(58, 24, 5 ,8, matrix.color444(0, 0, 0)); // overwrite the text with black
  matrix.setTextColor(matrix.color555(31,0,0));
  matrix.setCursor(58, 0); 
  matrix.println("*");   // Draw menu star
 
if(XOntime != XOnYOffctr){  // Setting the time via the up down button
   if(XOnYOffctr==0){    // After menu switch, XOnYOffctr becomes 0, and then the old value is reloaded
    XOnYOffctr = XOntime;
    }

      
    if(XOnYOffctr > XOntime){ // if increment, different increment depending on time size

    
       if(XOnYOffctr>=300 && XOnYOffctr<600){
      XOnYOffctr= XOnYOffctr +29; //30 sec
      }
      if(XOnYOffctr>=120 && XOnYOffctr <300){
      XOnYOffctr= XOnYOffctr +14; //15 sec
      }
       if(XOnYOffctr>=60 && XOnYOffctr <120){
         XOnYOffctr= XOnYOffctr +9; // 10 sec
      }
       if(XOnYOffctr<60){
        XOnYOffctr= XOnYOffctr +4; // 5 sec
        }
       }

          if(XOnYOffctr < XOntime){ // if decrement, different increment depending on time size

   
         if(XOnYOffctr>=300 && XOnYOffctr<600){
      XOnYOffctr= XOnYOffctr -29; //30 sec
      }
       if(XOnYOffctr>=120 && XOnYOffctr <300){
      XOnYOffctr= XOnYOffctr -14; //15 sec
      }
       if(XOnYOffctr>=60 && XOnYOffctr <120){
      XOnYOffctr= XOnYOffctr -9; // 10 sec
      }
            if(XOnYOffctr<60){
        XOnYOffctr= XOnYOffctr -4; // 5 sec
        }
       }
   }    
       if(XOnYOffctr>600){ // limits base to 10 min max
      XOnYOffctr=600;
      }
      if(XOnYOffctr<10){  // limits base to above 10 sec
      XOnYOffctr=10;
      }
      XOntime  = XOnYOffctr;
 
      if(XOntime != XOntimeOLD){
    printXOnYOfftime(XOntime, 0);  // print time on line 0
      XOntimeOLD = XOntime;
      }
     } // end of manu==1 && !choicemade

 if(Menu==2 && !choiceMade){
  matrix.fillRect(58, 0, 5 ,8, matrix.color444(0, 0, 0)); // overwrite the text with black
  matrix.fillRect(58, 16, 5 ,8, matrix.color444(0, 0, 0)); // overwrite the text with black
  matrix.fillRect(58, 24, 5 ,8, matrix.color444(0, 0, 0)); // overwrite the text with black
  matrix.setTextColor(matrix.color555(31,0,0));
  matrix.setCursor(58, 8); 
  matrix.println("*");

   if(YOfftime != XOnYOffctr){  // Setting the time via the up down button
   if(XOnYOffctr==0){    // After menu switch, XOnYOffctr becomes 0, and then the old value is reloaded
    XOnYOffctr = YOfftime;
    }
      
    if(XOnYOffctr > YOfftime){ // if increment, different increment depending on time size

     if(XOnYOffctr>=300 && XOnYOffctr<600){
      XOnYOffctr= XOnYOffctr +29; //30 sec
      }
      if(XOnYOffctr>=120 && XOnYOffctr <300){
      XOnYOffctr= XOnYOffctr +14; //15 sec
      }
       if(XOnYOffctr>=60 && XOnYOffctr <120){
         XOnYOffctr= XOnYOffctr +9; // 10 sec
      }
       if(XOnYOffctr<60){
        XOnYOffctr= XOnYOffctr +4; // 5 sec
        }
       }

          if(XOnYOffctr < YOfftime){ // if decrement, different increment depending on time size

      if(XOnYOffctr>=300 && XOnYOffctr<600){
      XOnYOffctr= XOnYOffctr -29; //30 sec
      }
       if(XOnYOffctr>=120 && XOnYOffctr <300){
      XOnYOffctr= XOnYOffctr -14; //15 sec
      }
       if(XOnYOffctr>=60 && XOnYOffctr <120){
      XOnYOffctr= XOnYOffctr -9; // 10 sec
      }
            if(XOnYOffctr<60){
        XOnYOffctr= XOnYOffctr -4; // 5 sec
        }
       }
   }    
       if(XOnYOffctr>600){  // limits pause to 10 min max
      XOnYOffctr=600;
      }
        if(XOnYOffctr<10){  // limits pause to 10 sec min
      XOnYOffctr=10;
      }
      YOfftime  = XOnYOffctr;
 


      if(YOfftime != YOfftimeOLD){
    printXOnYOfftime(YOfftime, 8);// print time on line 8
      YOfftimeOLD = YOfftime;
      }
     } // end of menu==2 && !choicemade
   
   if(Menu==3 && !choiceMade){
  matrix.fillRect(58, 0, 5 ,8, matrix.color444(0, 0, 0)); // overwrite the text with black
  matrix.fillRect(58, 8, 5 ,8, matrix.color444(0, 0, 0)); // overwrite the text with black
  matrix.fillRect(58, 24, 5 ,8, matrix.color444(0, 0, 0)); // overwrite the text with black
  matrix.setTextColor(matrix.color555(31,0,0));
  matrix.setCursor(58, 16); 
  matrix.println("*");

  if(XonYoffRounds != XOnYOffctr){  // Setting the time via the up down button
   if(XOnYOffctr==0){    // After menu switch, XOnYOffctr becomes 0, and then the old value is reloaded
    XOnYOffctr = XonYoffRounds;
    }
    if(XOnYOffctr > XonYoffRounds){
      XOnYOffctr= XOnYOffctr+1;
      }

        if(XOnYOffctr < XonYoffRounds){
       XOnYOffctr= XOnYOffctr-1;
      }
    XonYoffRounds =XOnYOffctr ; // If ctr is different from rds value, and not 0, then increment or decrease as instructed.
  }
        if(XonYoffRounds<1){
        XonYoffRounds=1;    // KEEP the number of rounds greater than one
        }
        if(XonYoffRounds>21){
          XonYoffRounds =21; // Keep number of rds to 21
            XOnYOffctr =XonYoffRounds;
          }
          
    if(XonYoffRounds !=XonYoffRoundsOLD){
      printXOnYOffRDS(XonYoffRounds,16); // print the rounds on line 16
      XonYoffRoundsOLD = XonYoffRounds;
     }

   }// end of manu==3 && !choicemade

    if(Menu==4 && !choiceMade){
  if(doOnceUNEVENmenu5){ 
  matrix.fillRect(30, 0, 34 ,32, matrix.color444(0, 0, 0)); // overwrite the text with black
  matrix.setCursor(30, 2); 
  matrix.println("PRESS");
  matrix.setCursor(30, 12);
  matrix.println("UP TO");
  matrix.setCursor(30, 22);
  matrix.println("START");
  doOnceUNEVENmenu5 =0;
  UNEVENctr=0;
  }
if (XOnYOffctr){
  MERKULINtimer=1;
  choiceMade =1;
  MERKULIN=0;
  }
   }// end of manu==4 && !choicemade
   
if(Menu==5 && !choiceMade){
  // Re write menu with values.
   matrix.fillRect(30, 0, 34 ,32, matrix.color444(0, 0, 0)); // overwrite the text with black
 matrix.setTextColor(matrix.color555(31,0,0)); // Red
 printXOnYOfftime(XOntime, 0);
  printXOnYOfftime(YOfftime, 8);
  printXOnYOffRDS(XonYoffRounds,16);
   Menu =1;
  doOnceUNEVENmenu5 =1;
    }// end of manu==5 && !choicemade
      buttonPressedLoopBOTTOM(); // bottom half of the button functions
  } // END OF MERKULIN LOOP

  
///////////////////////////////////////////
//
//THE actual MERKULIN count down timer
//
///////////////////////////////////////////


 while(MERKULINtimer){
        if(doOnceMERKULINtimer){
          doOnceMERKULINtimer = 0;
          MiddleMERKULINstep = (XonYoffRounds+1)/2;
          matrix.fillScreen(matrix.color444(0, 0, 0)); //Clear screen
         }


for(i=XonYoffRounds; i>0; i--){
  sum_CDT = 0;
   //work section
    if(i>=MiddleMERKULINstep){
    XOntimeTimeHolder = (long)XOntime*1000*(XonYoffRounds-i+1);  // ensures the increment of time until the middle step
   }
    if(i<MiddleMERKULINstep){
    XOntimeTimeHolder = (long)XOntime*1000*(i);  // ensures the increment of time
    }
     YOfftimeTimeHolder = (long)YOfftime*1000;
     
    countDownToStart();
    matrix.fillScreen(matrix.color444(0, 0, 0)); //Clear screen
    startMillis = millis();
    doOnceTimer =1;
    fourDigits=1;
    currentMillis = (millis()- startMillis);  
    //roundXofY(XonYoffRounds, i); // shows Round X of Y
 
  while(XOntimeTimeHolder>= currentMillis){ // time ON loop 
    currentMillis = (millis()- startMillis)*1;  //To accelerate time while testing, set multiplyer here
   sum_MERKULIN[i-1] = CountingREPS();  // Displaying reps
         
    difference_millis = XOntimeTimeHolder-currentMillis;
     if(difference_millis >0){                                  
        time_fractionCountdown(difference_millis, 1);  // displaying the time remaining
       
         }
    }

    
  // pause section

  if(i>1){
    startMillis = millis();
    currentMillis = (millis()- startMillis);  
    fourDigits=1;
    matrix.fillScreen(matrix.color444(0, 0, 0)); //Clear screen
    matrix.setTextColor(matrix.color555(31,0,0));
    matrix.setTextSize(2);
    matrix.setCursor(10,8);
    matrix.print("STOP");
    horn();
    
    matrix.fillScreen(matrix.color444(0, 0, 0)); //Clear screen
    int j = i-1;
    roundXofY(XonYoffRounds, j); // shows Round X of Y
    doOnceTimer = 1; // ensures right amount of digits on screen
    matrix.setTextColor(matrix.color555(0,31,0));
    matrix.setTextSize(1);
    matrix.setCursor(18,0);
    matrix.print("PAUSE");
    
    matrix.setTextColor(matrix.color555(1,15,31));
    matrix.setCursor(8,16);
    matrix.print("REPS");
     matrix.setTextColor(matrix.color555(31,15,1));
    matrix.setCursor(36,16);
    matrix.print(sum_MERKULIN[i-1]);
 

   while( YOfftimeTimeHolder >= currentMillis+6500){ // time OFF loop 
    currentMillis = (millis()- startMillis)*1;  //To accelerate time while testing, set multiplyer here
    difference_millis =  YOfftimeTimeHolder -currentMillis;
     if(difference_millis >6500){ // takes the countdown into account                                  
       time_fractionPause(difference_millis); // displaying the time remaining
      }
    }
   }
  } // end of for(i=XonYoffRounds; i>0; i--)

              while(1){  // end result loop
      if(doOnceTABATAendResult){
    matrix.fillScreen(matrix.color444(0, 0, 0)); //Clear screen
    matrix.setTextColor(matrix.color555(31,0,0));
    matrix.setTextSize(2);
    matrix.setCursor(10,8);
    matrix.print("STOP");
        horn();
        matrix.fillScreen(matrix.color444(0, 0, 0)); //Clear screen
        matrix.setTextColor(matrix.color555(0,31,0));
         matrix.setTextSize(1);
    matrix.setCursor(9,0);
    matrix.print("MERKULIN");
    matrix.setCursor(9,8);
    matrix.print("COMPLETE");

    int s = 0;
      for (k=0; k< XonYoffRounds; k++)
    {
    s = s+ sum_MERKULIN[k];
      }
      matrix.setTextColor(matrix.color555(1,15,31));
   matrix.setCursor(0,16);
    matrix.print("REPS");
    matrix.setTextColor(matrix.color555(31,15,1));
     matrix.setCursor(40,16);
    matrix.print(s);

    i = i+1; // compensates so the end result displays correctly 
    roundXofY(XonYoffRounds,i);
    doOnceTABATAendResult = 0;
        }
        // Do nothing
      } // end of while(1)
      } // end of  while(MERKULINtimer)



  buttonPressedLoopBOTTOM(); // bottom half of the button functions
  }// end of while(LADDERS)


////////////////////////////////////////////
// TABATA INTERVAL TIMER
//
////////////////////////////////////////////

  while(TABATA){
buttonPressedTopTABATA(); // TOP half of the button functions


if(doOnceXonYoff){ // reused from X on Y off menu
  matrix.fillScreen(matrix.color444(0, 0, 0)); //Clear screen
  matrix.setTextColor(matrix.color555(1,15,31));
  matrix.setTextSize(1); 
  matrix.setCursor(0,0); 
  matrix.println(" On :");
  matrix.setTextColor(matrix.color555(15,31,1));
  matrix.setCursor(0, 8); 
  matrix.println(" Off:");
  matrix.setTextColor(matrix.color555(31,15,1));
  matrix.setCursor(0, 16); 
  matrix.println(" Rds:");
  matrix.setTextColor(matrix.color555(1,31,15));
  matrix.setCursor(0, 24); 
  matrix.println("Iter:");
  TabataOntime = 20;
  TabataOfftime = 10;
  TabataRounds = 6;
  TabataItterations = 6;
  Menu=3; // Sets the cursor to the third choice
  Select = 0; // resets the Select variable which is reused.
  printXOnYOfftime(TabataOntime, 0);
  printXOnYOfftime(TabataOfftime, 8);
  printXOnYOffRDS(TabataRounds,16);
  printXOnYOffRDS(TabataItterations, 24);
  choiceMade =0;
  doOnceXonYoff = 0; // Ensures this statement isn't repeated
  }

    
if(Menu==1 && !choiceMade){
  matrix.fillRect(58, 8, 5 ,8, matrix.color444(0, 0, 0)); // overwrite the text with black
  matrix.fillRect(58, 16, 5 ,8, matrix.color444(0, 0, 0)); // overwrite the text with black
  matrix.fillRect(58, 24, 5 ,8, matrix.color444(0, 0, 0)); // overwrite the text with black
  matrix.setTextColor(matrix.color555(31,0,0));
  matrix.setCursor(58, 0); 
  matrix.println("*");   // Draw menu star
 
if(TabataOntime != XOnYOffctr){  // Setting the time via the up down button
   if(XOnYOffctr==0){    // After menu switch, XOnYOffctr becomes 0, and then the old value is reloaded
    XOnYOffctr = TabataOntime;
    }   
    
   }    
       if(XOnYOffctr>60){ // limits to one hours
      XOnYOffctr=60;
      }
      if(XOnYOffctr<5){  // limits to above 5 sec
      XOnYOffctr=5;
      }
      TabataOntime  = XOnYOffctr;
 
      if(TabataOntime != TabataOntimeOLD){
    printXOnYOfftime(TabataOntime, 0);  // print time on line 0
      TabataOntimeOLD = TabataOntime;
      }
     }// end of menu==1 && !choicemade
  

   if(Menu==2 && !choiceMade){
  matrix.fillRect(58, 0, 5 ,8, matrix.color444(0, 0, 0)); // overwrite the text with black
  matrix.fillRect(58, 16, 5 ,8, matrix.color444(0, 0, 0)); // overwrite the text with black
  matrix.fillRect(58, 24, 5 ,8, matrix.color444(0, 0, 0)); // overwrite the text with black
  matrix.setTextColor(matrix.color555(31,0,0));
  matrix.setCursor(58, 8); 
  matrix.println("*");

   if(TabataOfftime != XOnYOffctr){  // Setting the time via the up down button
   if(XOnYOffctr==0){    // After menu switch, XOnYOffctr becomes 0, and then the old value is reloaded
    XOnYOffctr = TabataOfftime;
    }      
    
   }    
       if(XOnYOffctr>60){ // limits to 60 sec
      XOnYOffctr=60;
      }
        if(XOnYOffctr<5){ // limits to 5 sec
      XOnYOffctr=5;
      }
      TabataOfftime  = XOnYOffctr;
 
      if(TabataOfftime != TabataOfftimeOLD){
    printXOnYOfftime(TabataOfftime, 8);// print time on line 8
      TabataOfftimeOLD = TabataOfftime;
      }
     } // end of menu==2 && !choicemade

   
   if(Menu==3 && !choiceMade){
  matrix.fillRect(58, 0, 5 ,8, matrix.color444(0, 0, 0)); // overwrite the text with black
  matrix.fillRect(58, 8, 5 ,8, matrix.color444(0, 0, 0)); // overwrite the text with black
  matrix.fillRect(58, 24, 5 ,8, matrix.color444(0, 0, 0)); // overwrite the text with black
  matrix.setTextColor(matrix.color555(31,0,0));
  matrix.setCursor(58, 16); 
  matrix.println("*");

  if(TabataRounds != XOnYOffctr){  // Setting the time via the up down button
   if(XOnYOffctr==0){    // After menu switch, XOnYOffctr becomes 0, and then the old value is reloaded
    XOnYOffctr = TabataRounds;
    }
    TabataRounds =XOnYOffctr ; // If ctr is different from rds value, and not 0, then increment or decrease as instructed.
  }
        if(TabataRounds<1){
        TabataRounds=1;    // KEEP the number of rounds greater than one
        }
        if(TabataRounds>99){
          TabataRounds =99;  // Keep number of rds less than 100
          XOnYOffctr = TabataRounds;
          }
          
    if(TabataRounds !=TabataRoundsOLD){
      printXOnYOffRDS(TabataRounds,16); // print the rounds on line 16
      TabataRoundsOLD = TabataRounds;
     }

   } //end of menu== 3  && choicemade
   
      if(Menu==4 && !choiceMade){
  matrix.fillRect(58, 0, 5 ,8, matrix.color444(0, 0, 0)); // overwrite the text with black
  matrix.fillRect(58, 8, 5 ,8, matrix.color444(0, 0, 0)); // overwrite the text with black
  matrix.fillRect(58, 16, 5 ,8, matrix.color444(0, 0, 0)); // overwrite the text with black
  matrix.setTextColor(matrix.color555(31,0,0));
  matrix.setCursor(58, 24); 
  matrix.println("*");

  if(TabataItterations != XOnYOffctr){  // Setting the time via the up down button
   if(XOnYOffctr==0){    // After menu switch, XOnYOffctr becomes 0, and then the old value is reloaded
    XOnYOffctr = TabataItterations;
    }
    TabataItterations =XOnYOffctr ; // If ctr is different from rds value, and not 0, then increment or decrease as instructed.
  }
        if(TabataItterations<1){
        TabataItterations=1;    // KEEP the number of rounds greater than one
        }
        if(TabataItterations>99){
          TabataItterations =99;  // Keep number of rds less than 100
          XOnYOffctr = TabataItterations;
          }
          
    if(TabataItterations !=TabataItterationsOLD){
      printXOnYOffRDS(TabataItterations,24); // print the rounds on line 16
      TabataItterationsOLD = TabataItterations;
     }

   } // end of menu==4 && !choicemade

    if(Menu==5 && !choiceMade){
  if(doOnceUNEVENmenu5){ 
  matrix.fillRect(30, 0, 34 ,32, matrix.color444(0, 0, 0)); // overwrite the text with black
  matrix.setCursor(30, 2); 
  matrix.println("PRESS");
  matrix.setCursor(30, 12);
  matrix.println("UP TO");
  matrix.setCursor(30, 22);
  matrix.println("START");
  doOnceUNEVENmenu5 =0;
  UNEVENctr=0;
  }
if (XOnYOffctr){
  TABATAtimer=1;
  choiceMade =1;
  TABATA=0;
  }
   }
   
if(Menu==6 && !choiceMade){
  // Re write menu with values.
   matrix.fillRect(30, 0, 34 ,32, matrix.color444(0, 0, 0)); // overwrite the text with black
 matrix.setTextColor(matrix.color555(31,0,0)); // Red
 printXOnYOfftime(TabataOntime, 0);
  printXOnYOfftime(TabataOfftime, 8);
  printXOnYOffRDS(TabataRounds,16);
  printXOnYOffRDS(TabataItterations,24);
   Menu =1;
  doOnceUNEVENmenu5 =1;
    }

buttonPressedLoopBOTTOM(); // bottom half of the button functions
   
    } // end of TABATA

/////////////////////////////////////////////////////
//
//   THE ACTUAL TABATA TIMER
//
/////////////////////////////////////////////////////

while(TABATAtimer){
 
   if(doOnceXOnYOfftimer){

    bigTABATAbreak = (long)(TabataOntime+ 2*TabataOfftime)*1000 ; // sets the break between each round
    XOntimeTimeHolder = (long)TabataOntime*1000;
    YOfftimeTimeHolder = (long)TabataOfftime*1000;
    doOnceXOnYOfftimer = 0; // Reuse of variable for Xon Yoff timer
   }
   
for(i=TabataRounds; i>0 ; i-- ){  //  ROUNDS LOOP
  countDownToStart();
  
for(k=TabataItterations; k>0 ; k-- ){  // ITTERATIONS LOOP
  
  matrix.fillScreen(matrix.color444(0, 0, 0)); //Clear screen
  startMillis = millis();
  doOnceTimer =1;
  fourDigits=1;
  currentMillis = (millis()- startMillis);  
  roundXofY(TabataItterations, k);
  matrix.fillRect(0, 24, 29,7, matrix.color444(0, 0, 0)); // overwrite the text with black
  matrix.setTextColor(matrix.color555(1,15,31));
  matrix.setCursor(0, 24);
  matrix.println("Iter."); // Maskes ROUND
  matrix.fillRect(0, 16, 64,7, matrix.color444(0, 0, 0)); // overwrite the text with black
  
    matrix.setTextColor(matrix.color555(0,31,0));
    matrix.setTextSize(1);
    matrix.setCursor(21,16);
    matrix.print("WORK");
    
  while(XOntimeTimeHolder>= currentMillis){ // time ON loop 
    currentMillis = (millis()- startMillis)*1;  //To accelerate time while testing, set multiplyer here
   
    difference_millis = XOntimeTimeHolder-currentMillis;
     if(difference_millis >0){                                  
        time_fractionCountdown(difference_millis, 1);  // displaying the time remaining
         }
    }
    
 if(k>1){    // keeps the loop from running on the last itteration.
    startMillis = millis();
    currentMillis = (millis()- startMillis);  
    fourDigits=1;
    doOnceTimer =1;
     shortHorn();
     matrix.fillRect(0, 16, 64,7, matrix.color444(0, 0, 0)); // overwrite the text with black

    matrix.setTextColor(matrix.color555(0,31,0));
    matrix.setTextSize(1);
    matrix.setCursor(18,16);
    matrix.print("BREAK");

 while(YOfftimeTimeHolder>= currentMillis){ // time OFF loop 
    currentMillis = (millis()- startMillis)*1;  //To accelerate time while testing, set multiplyer here
    difference_millis = YOfftimeTimeHolder-currentMillis;
     if(difference_millis >0){   
      time_fractionCountdown(difference_millis, 1);  // displaying the time remaining
      }
    }
  shortHorn();
 }
  } // End of for(k=1; k<= TabataItterations; k++ )
  
     if(i>1){ //keeps the break from running the last round
    startMillis = millis();
    currentMillis = (millis()- startMillis);  
    fourDigits=1;
    matrix.fillScreen(matrix.color444(0, 0, 0)); //Clear screen
    matrix.setTextColor(matrix.color555(31,0,0));
    matrix.setTextSize(2);
    matrix.setCursor(10,8);
    matrix.print("STOP");
    horn();
    
    matrix.fillScreen(matrix.color444(0, 0, 0)); //Clear screen
    
    roundXofY(TabataRounds,i);
    doOnceTimer = 1;
    matrix.setTextColor(matrix.color555(0,31,0));
    matrix.setTextSize(1);
    matrix.setCursor(18,0);
    matrix.print("PAUSE");
  
   while( bigTABATAbreak >= currentMillis+6500){ // time OFF loop 
    currentMillis = (millis()- startMillis)*1;  //To accelerate time while testing, set multiplyer here
    difference_millis =  bigTABATAbreak-currentMillis;
     if(difference_millis >6500){ // takes the countdown into account                                  
       time_fractionPause(difference_millis); // displaying the time remaining
         }
    }
     } // end if(i>1)
}// for(i=1; i<= TabataRounds; i++ )

     while(1){  // end result loop
      if(doOnceTABATAendResult){
    matrix.fillScreen(matrix.color444(0, 0, 0)); //Clear screen
    matrix.setTextColor(matrix.color555(31,0,0));
    matrix.setTextSize(2);
    matrix.setCursor(10,8);
    matrix.print("STOP");
        horn();
        matrix.fillScreen(matrix.color444(0, 0, 0)); //Clear screen
        matrix.setTextColor(matrix.color555(0,31,0));
         matrix.setTextSize(1);

    matrix.setCursor(15,2);
    matrix.print("TABATA");
    matrix.setCursor(9,12);
    matrix.print("COMPLETE");
              
    i = i+1; // compensates so the end result displays correctly 
    roundXofY(TabataRounds,i);
    doOnceTABATAendResult = 0;
        }
        // Do nothing
      }
  } // end of while(TABATAtimer)

buttonPressedLoopBOTTOM(); // bottom half of the button functions

  } //end of INTERVAL

while(BLUETOOTH){ 
 /* this menu point has no longer any function. My suggestions is to make a start/stop timer (stop watch) for when an athlete is doing a challenge and need to time it
  e.g. if the athlede wants to time a 1000 rep challenge.
 something like that the start (S/S) is the start top buttom, where the up/down is still for counting reps
 but if the in the "menu" where the timer is stopped, if the let's say down buttom is pressed, then the round is finished at the counter switch to a "End result / finish screen"
 */

  matrix.fillScreen(matrix.color444(0, 0, 0)); //Clear screen
  matrix.setTextColor(matrix.color555(1,15,31)); //VW blue
  matrix.setTextSize(1); 
  matrix.setCursor(4, 0); 
  matrix.println("BLE");
  matrix.setCursor(4, 8); 
  matrix.println("NOT Working");

}
  
  ledcWrite(BuzzerChannel, 0); // ensures the buzzer is turned off
  buttonPressedLoopBOTTOM();
 
  

} // end of void loop()


///////////////////////////////////////////////////////
//
// Functions 
//
///////////////////////////////////////////////////////


void endResult(){
  int R = 1;
  int G = 15;
  int B = 31;
  matrix.fillScreen(matrix.color444(0, 0, 0));
    matrix.setTextColor(matrix.color555(R,G,B)); 
    matrix.setTextSize(1);
    matrix.setCursor(1,1);
    matrix.print("C");

    matrix.setTextColor(matrix.color555(0,16,0)); 
    matrix.setCursor(9,1);
    matrix.print(sum_CLEAN);
    matrix.setTextColor(matrix.color555(R,G,B)); 
    matrix.setCursor(1,13);
    matrix.print("CP");

    matrix.setTextColor(matrix.color555(0,16,0)); 
    matrix.setCursor(15,13);
    matrix.print(sum_CandP);
    matrix.setTextColor(matrix.color555(R,G,B)); 
    matrix.setCursor(1,24);
    matrix.print("J");

    matrix.setTextColor(matrix.color555(0,16,0)); 
    matrix.setCursor(9,24);
    matrix.print(sum_JERK);
    matrix.setTextColor(matrix.color555(R,G,B)); 
    matrix.setCursor(33,7);
    matrix.print("HS");

    matrix.setTextColor(matrix.color555(0,16,0)); 
    matrix.setCursor(46,7);
    matrix.print(sum_HS);
    matrix.setTextColor(matrix.color555(R,G,B)); 
    matrix.setCursor(33,19);
    matrix.print("PP");

    matrix.setTextColor(matrix.color555(0,16,0)); 
    matrix.setCursor(46,19);
    matrix.print(sum_PP);
  }

  
int pause(){
  doOnceTimer = 1;
   matrix.fillScreen(matrix.color444(0, 0, 0)); //Clear screen
   matrix.setTextSize(1); 
   matrix.setTextColor(matrix.color555(1,31,1));
   matrix.setCursor(18, 0);
   matrix.print("PAUSE");
  // Serial.println("Pause funtion called");
  }

// countdown clock and buzzer
    void countDownToStart(){
      matrix.setTextSize(1); 
   long  MystartMillis = millis();
   matrix.fillScreen(matrix.color444(0, 0, 0));  // Clear Screen
   matrix.setTextColor(matrix.color555(0,31,0));
   matrix.setCursor(8, 2);
   matrix.println("Countdown");
   matrix.setCursor(8, 14);
   matrix.println("to Start");
  int doOnce5 =1;
  int doOnce4 =1;
  int doOnce3 =1;
  int doOnce2 =1; 
  int doOnce1 =1;
  int doOnce0 =1;
  doOnceTimer = 1;
  doOnceCounter = 1;   
   long time_left = 6500; 
   
   delay(1000);
  while(time_left >=-500 ){
    
  time_left = 6500-millis()+MystartMillis;
  
  if(time_left >5000 && doOnce5 ==1){
    matrix.fillScreen(matrix.color444(0, 0, 0));
    matrix.setTextSize(3); 
    matrix.setTextColor(matrix.color444(0,15,0));
     matrix.setCursor(24,6);
    matrix.print("5");
   doOnce5 =0;
  }
 
    if(time_left <4500 && time_left >4000 && doOnce4 ==1){
      matrix.fillScreen(matrix.color444(0, 0, 0));
      matrix.setCursor(24,6);
      matrix.print("4");
    ledcWriteTone(BuzzerChannel, 1000);
    ledcWrite(BuzzerChannel, 255);
      doOnce4 = 0;
    }
    
  if(time_left <4000 && time_left >3500 ){
      ledcWrite(BuzzerChannel, 0);
  }
      if(time_left <3500 && time_left >3000 && doOnce3 ==1){
        matrix.fillScreen(matrix.color444(0, 0, 0));
        matrix.setCursor(24,6);
         matrix.print("3");
       doOnce3 =0;
    ledcWrite(BuzzerChannel, 255);
     }
     
       if(time_left <3000 && time_left >2500 ){
       ledcWrite(BuzzerChannel, 0);
  }
        if(time_left <2500 && time_left >2000 && doOnce2 ==1){
          matrix.fillScreen(matrix.color444(0, 0, 0));
          matrix.setCursor(24,6);
          matrix.print("2");
          ledcWrite(BuzzerChannel, 255);
          doOnce2 =0;
     }

     if(time_left <2000 && time_left >1500 ){
       ledcWrite(BuzzerChannel, 0);
  }
     if(time_left <1500 && time_left >1000 && doOnce1 ==1){
      matrix.fillScreen(matrix.color444(0, 0, 0));
         matrix.setCursor(24,6);
     matrix.print("1");
     doOnce1 =0;
      ledcWrite(BuzzerChannel, 255);
        }
    
      if(time_left <1000 && time_left >500){
      ledcWrite(BuzzerChannel, 0);
  }
 
   if(time_left <500&& time_left>0 && doOnce0 ==1){
    matrix.fillScreen(matrix.color444(0, 0, 0));
    matrix.setTextSize(2);
        matrix.setCursor(10,8);
    matrix.print("LIFT");
    doOnce0 =0;
   ledcWriteTone(BuzzerChannel, 2000);
  ledcWrite(BuzzerChannel, 200);
    delay(500);
    ledcWrite(BuzzerChannel, 0);
     }
  }
ledcWrite(BuzzerChannel, 0);
  repCounter=0;
  return;
  }
  
// Input a value 0 to 24 to get a color value.
// The colours are a transition r - g - b - back to r.
void  RainbowText(int Y , int X , const char *text){
   matrix.setCursor(Y,X);    // start at line 23, with 3 pixel of spacing
  uint8_t w = 0;
int strLen = strlen(text);
  for (w=0; w<strLen; w++) {
    matrix.setTextColor(Wheel(w));
    matrix.print(text[w]);
  }
  }
  
uint16_t Wheel(byte WheelPos) {
  if(WheelPos < 8) {
   return matrix.color444(15 - WheelPos*2, WheelPos*2, 0);
  } else if(WheelPos < 16) {
   WheelPos -= 8;
   return matrix.color444(0, 15-WheelPos*2, WheelPos*2);
  } else {
   WheelPos -= 16;
   return matrix.color444(0, WheelPos*2, 7 - WheelPos*2);
  }
} // end RainbowText

int CountingREPS(){
 
int repLine = 17;
//For testing purpose
 if(doOnceRepCounter){
  repCounter = 999;
  doOnceRepCounter =0;
  }
  // Button debounce Rep counter UP
   readingRepUP = digitalRead(repButtonUP);
  if (readingRepUP != lastButtonStateRepUP) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }
  if ((millis() - lastDebounceTime) > debounceDelay) {
     if (readingRepUP != buttonStateRepUP) {
      buttonStateRepUP = readingRepUP;
     if (buttonStateRepUP == HIGH) {
       repCounter++;
     }
   }
}
// Button debounce Rep counter down
 readingRepDOWN = digitalRead(repButtonDOWN);
  if (readingRepDOWN != lastButtonStateRepDOWN) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }
  if ((millis() - lastDebounceTime) > debounceDelay) {
     if (readingRepDOWN != buttonStateRepDOWN) {
      buttonStateRepDOWN = readingRepDOWN;
     if (buttonStateRepDOWN == HIGH) {
      if(repCounter > 0){
       repCounter--;
      }
     }
   }
}

// prints the reps 
if(repCounter == 0 && doOnceCounter ==1 ){ // So it prints to begin with
   matrix.setTextSize(2); 
   matrix.fillRect(27, repLine, 32,16, matrix.color444(0, 0, 0)); // overwrite the text with black
   matrix.setTextColor(matrix.color444(1,8,15));
   matrix.setCursor(27,repLine);
   matrix.print(repCounter);
   lastRepWas = repCounter;
   doOnceCounter = 0;
   }

if(repCounter != lastRepWas  ){
  
  if(repCounter >= 0 && repCounter <10 && doOnceCounter == 0){
   matrix.setTextSize(2); 
   matrix.fillRect(21, repLine,34,14, matrix.color444(0, 0, 0)); // overwrite the text with black
   matrix.setTextColor(matrix.color444(1,8,15));
   matrix.setCursor(27,repLine);
   matrix.print(repCounter);
   lastRepWas = repCounter;
   }
  if(repCounter >= 10 && repCounter <100){
   matrix.setTextSize(2); 
   matrix.fillRect(15, repLine, 40,14, matrix.color444(0, 0, 0)); // overwrite the text with black
   matrix.setTextColor(matrix.color444(1,8,15));
   matrix.setCursor(21,repLine);
   matrix.print(repCounter);
   lastRepWas = repCounter;
   }

     if(repCounter >= 100 && repCounter <1000){
   matrix.setTextSize(2); 
   matrix.fillRect(9, repLine, 48,14, matrix.color444(0, 0, 0)); // overwrite the text with black
   matrix.setTextColor(matrix.color444(1,8,15));
   matrix.setCursor(15,repLine);
   matrix.print(repCounter);
   lastRepWas = repCounter;
   }

        if(repCounter >= 1000 && repCounter <10000){
   matrix.setTextSize(2); 
   matrix.fillRect(3, repLine, 58,14, matrix.color444(0, 0, 0)); // overwrite the text with black
   matrix.setTextColor(matrix.color444(1,8,15));
   matrix.setCursor(9,repLine);
   matrix.print(repCounter);
   lastRepWas = repCounter;
   }

   if(repCounter >= 10000 && repCounter <100000){
   matrix.setTextSize(2); 
   matrix.fillRect(3, repLine, 58,14, matrix.color444(0, 0, 0)); // overwrite the text with black
   matrix.setTextColor(matrix.color444(1,8,15));
   matrix.setCursor(3,repLine);
   matrix.print(repCounter);
   lastRepWas = repCounter;
   }
   
   }
 // storing old reading values for later use
  lastButtonStateRepUP = readingRepUP;
  lastButtonStateRepDOWN = readingRepDOWN;
 
  return repCounter;
  }


    void horn(){
    ledcWriteTone(BuzzerChannel, 2000);
    ledcWrite(BuzzerChannel, 200);
    delay(2000);
    ledcWrite(BuzzerChannel, 0);
    run_pause=1;
  }

    void shortHorn(){
    ledcWriteTone(BuzzerChannel, 2000);
    ledcWrite(BuzzerChannel, 255);
    delay(750);
    ledcWrite(BuzzerChannel, 0);
    run_pause=1;
  }
  
  void time_fraction(unsigned long brutto){
 matrix.setTextColor(matrix.color444(15,0,0));
 matrix.setTextSize(2);
    if(doOnceTimer){
    mm_10_old = 10;
    mm_1_old = 10;
    ss_10_old = 10;
    ss_1_old = 10;
     matrix.setCursor(27,1);
     matrix.print(":");
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

 matrix.setTextColor(matrix.color444(15,0,0));
 matrix.setTextSize(2); 

if(mm_10_old != mm_10){
 matrix.fillRect(7, 1, 10 ,14, matrix.color444(0, 0, 0)); // overwrite the text with black
 matrix.setCursor(7,1);
 matrix.print(mm_10);
 mm_10_old = mm_10;
}

if(mm_1_old != mm_1){
 matrix.fillRect(19, 1, 10 ,14, matrix.color444(0, 0, 0)); // overwrite the text with black
 matrix.setCursor(19,1);
 matrix.print(mm_1);
 mm_1_old = mm_1;
}

if(ss_10_old != ss_10){
 matrix.fillRect(35, 1, 10 ,14, matrix.color444(0, 0, 0)); // overwrite the text with black
 matrix.setCursor(35,1);
 matrix.print(ss_10);
 ss_10_old = ss_10;
}
if(ss_1_old != ss_1){
 matrix.fillRect(47, 1, 10 ,14, matrix.color444(0, 0, 0)); // overwrite the text with black
 matrix.setCursor(47,1);
 matrix.print(ss_1);
 ss_1_old = ss_1;
}


  }

   void time_fractionPause(unsigned long brutto){
 matrix.setTextColor(matrix.color444(15,0,0));
 matrix.setTextSize(1);
    if(doOnceTimer){
      if(MERKULINtimer || YOfftimeRUN){
        line = 8;
        }
        else{
          line = 12;
          }
    mm_10_old = 10;
    mm_1_old = 10;
    ss_10_old = 10;
    ss_1_old = 10;
     matrix.setCursor(30,line);
     matrix.print(":");
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

 matrix.setTextColor(matrix.color444(15,0,0));
 matrix.setTextSize(1); 

if(mm_10_old != mm_10){
 matrix.fillRect(19, line, 6 ,7, matrix.color444(0, 0, 0)); // overwrite the text with black
 matrix.setCursor(19,line);
 matrix.print(mm_10);
 mm_10_old = mm_10;
}

if(mm_1_old != mm_1){
 matrix.fillRect(25, line, 6 ,7, matrix.color444(0, 0, 0)); // overwrite the text with black
 matrix.setCursor(25,line);
 matrix.print(mm_1);
 mm_1_old = mm_1;
}

if(ss_10_old != ss_10){
 matrix.fillRect(35, line, 6 ,7, matrix.color444(0, 0, 0)); // overwrite the text with black
 matrix.setCursor(35,line);
 matrix.print(ss_10);
 ss_10_old = ss_10;
}
if(ss_1_old != ss_1){
 matrix.fillRect(41, line, 6 ,7, matrix.color444(0, 0, 0)); // overwrite the text with black
 matrix.setCursor(41,line);
 matrix.print(ss_1);
 ss_1_old = ss_1;
}


  }


unsigned long GatherCDT(int CDThh_1, int CDTmm_10, int CDTmm_1,int CDTss_10, int CDTss_1){
 timeHolder2 = (unsigned long)CDThh_1*3600000; //60*60*1000
 timeHolder2 = timeHolder2 + (unsigned long)CDTmm_10*600000; //10*60*1000
 timeHolder2 = timeHolder2 + (unsigned long)CDTmm_1*60000; //1*60*1000
 timeHolder2 = timeHolder2 + (unsigned long)CDTss_10*10000; //10*1000
 timeHolder2 = timeHolder2 + (unsigned long)CDTss_1*1000; //1*1000
 return timeHolder2;
  }



 void time_fractionCountdown(unsigned long brutto, int lineSet){

      
      int TextSize =2;
      int lineOffset = 1;
 matrix.setTextColor(matrix.color444(15,0,0));
 matrix.setTextSize(TextSize);
    if(doOnceTimer){
    hh_1_old = 10;  
    mm_10_old = 10;
    mm_1_old = 10;
    ss_10_old = 10;
    ss_1_old = 10;

    if(fourDigits){
     matrix.setCursor(27,lineSet);
     matrix.print(":");
    }else{
      matrix.setCursor(lineOffset+33,lineSet);
     matrix.print(":");
     matrix.setCursor(lineOffset+7,lineSet);
     matrix.print(":");
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


 matrix.setTextColor(matrix.color444(15,0,0));
 matrix.setTextSize(TextSize); 
 
if(!fourDigits){
if(hh_1_old != hh_1){
 matrix.fillRect(lineOffset, lineSet, 10 ,14, matrix.color444(0, 0, 0)); // overwrite the text with black
 matrix.setCursor(lineOffset,lineSet);
 matrix.print(hh_1);
 hh_1_old = hh_1;
}

if(mm_10_old != mm_10){
 matrix.fillRect(lineOffset+14, lineSet, 10 ,14, matrix.color444(0, 0, 0)); // overwrite the text with black
 matrix.setCursor(lineOffset+14,lineSet);
 matrix.print(mm_10);
 mm_10_old = mm_10;
}

if(mm_1_old != mm_1){
 matrix.fillRect(lineOffset+26, lineSet, 10 ,14, matrix.color444(0, 0, 0)); // overwrite the text with black
 matrix.setCursor(lineOffset+26,lineSet);
 matrix.print(mm_1);
 mm_1_old = mm_1;
}

if(ss_10_old != ss_10){
 matrix.fillRect(lineOffset+40, lineSet, 10 ,14, matrix.color444(0, 0, 0)); // overwrite the text with black
 matrix.setCursor(lineOffset+40,lineSet);
 matrix.print(ss_10);
 ss_10_old = ss_10;
}
if(ss_1_old != ss_1){
 matrix.fillRect(lineOffset+52, lineSet, 10 ,14, matrix.color444(0, 0, 0)); // overwrite the text with black
 matrix.setCursor(lineOffset+52,lineSet);
 matrix.print(ss_1);
 ss_1_old = ss_1;
}
}

if(fourDigits){

if(mm_10_old != mm_10){
 matrix.fillRect(8, lineSet, 10 ,14, matrix.color444(0, 0, 0)); // overwrite the text with black
 matrix.setCursor(8,lineSet);
 matrix.print(mm_10);
 mm_10_old = mm_10;
}

if(mm_1_old != mm_1){
 matrix.fillRect(20, lineSet, 10 ,14, matrix.color444(0, 0, 0)); // overwrite the text with black
 matrix.setCursor(20,lineSet);
 matrix.print(mm_1);
 mm_1_old = mm_1;
}

if(ss_10_old != ss_10){
 matrix.fillRect(34, lineSet, 10 ,14, matrix.color444(0, 0, 0)); // overwrite the text with black
 matrix.setCursor(34,lineSet);
 matrix.print(ss_10);
 ss_10_old = ss_10;
}
if(ss_1_old != ss_1){
 matrix.fillRect(46, lineSet, 10 ,14, matrix.color444(0, 0, 0)); // overwrite the text with black
 matrix.setCursor(46,lineSet);
 matrix.print(ss_1);
 ss_1_old = ss_1;
}
}
  }

    void printMainMenu(){
      matrix.fillScreen(matrix.color444(0, 0, 0)); //Clear screen
  matrix.setTextColor(matrix.color555(1,15,31));
  matrix.setTextSize(1); 
  matrix.setCursor(5, 0); 
  matrix.println("COUNT");
  matrix.setTextColor(matrix.color555(1,15,31));
  matrix.setCursor(37, 0); 
  matrix.println("U");
  matrix.setCursor(49, 0); 
  matrix.println("D");
   matrix.setTextColor(matrix.color555(1,15,31));
  matrix.setCursor(43, 0); 
  matrix.println("/");
  matrix.setTextColor(matrix.color555(0,31,0));
  matrix.setCursor(5, 8); 
  matrix.println("PENTATHLON");
  matrix.setTextColor(matrix.color555(31,5,1));
  matrix.setCursor(5, 16); 
  matrix.println("INTERVAL");
  matrix.setTextColor(matrix.color555(1,15,31));
  matrix.setCursor(5, 24); 
  matrix.println("BLE");
  matrix.setTextColor(matrix.color555(1,15,31));
  matrix.setCursor(26, 24); 
  matrix.println("HRM");
    }

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
 readingONEmenuUP = digitalRead(repButtonDOWN);
  if (readingRepDOWN != lastButtonStateRepDOWN) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }
      if ((millis() - lastDebounceTime) > 3000) {
     if (readingONEmenuUP != buttonStateONEmenuUP) {
      buttonStateONEmenuUP = readingONEmenuUP;
     if (buttonStateONEmenuUP == HIGH) {
      ESP.restart();
      }
   }
}else{
  if ((millis() - lastDebounceTime) > debounceDelay) {
     if (readingRepDOWN != buttonStateRepDOWN) {
      buttonStateRepDOWN = readingRepDOWN;
     if (buttonStateRepDOWN == HIGH) {
       Menu++;
      }
   }
}} // end of else{}


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
lastbuttonStateONEmenuUP = readingRepDOWN;
    }

void  startUpText(){
 
  matrix.setTextSize(1);     // size 1 == 8 pixels high
  matrix.setTextColor(matrix.color555(1,15,31));
     matrix.setCursor(11, 2); 
  matrix.println("VULTURE");
   matrix.setCursor(20, 12); 
  matrix.println("WOOD");
  RainbowText(9,22,"ELECTRIC"); //X,Y, text
  delay(4000); // keep at 4000
}

    void printLADDERSMenu(){
  matrix.fillScreen(matrix.color444(0, 0, 0)); //Clear screen
  matrix.setTextSize(1); 
  matrix.setTextColor(matrix.color555(31,1,1));
  matrix.setCursor(9, 0); 
  matrix.println("MERKULIN"); 
  matrix.setTextColor(matrix.color555(1,15,31)); // VW blue
  matrix.setCursor(15, 8); 
  matrix.println("UNEVEN");
  matrix.setTextColor(matrix.color555(31,0,0));
  matrix.setCursor(27, 16); 
  matrix.println("UP");
  matrix.setTextColor(matrix.color555(1,31,1));
  matrix.setCursor(21, 24); 
  matrix.println("DOWN");
  }

    void printIntervalMenu(){
  matrix.fillScreen(matrix.color444(0, 0, 0)); //Clear screen
  matrix.setTextSize(1); 
  matrix.setTextColor(matrix.color555(31,0,0));
  matrix.setCursor(5, 8); 
  matrix.println("X"); 
  matrix.setTextColor(matrix.color555(0,31,0));
  matrix.setCursor(12, 8); 
  matrix.println("on");
  matrix.setTextColor(matrix.color555(31,0,0));
  matrix.setCursor(26, 8); 
  matrix.println("Y");
  matrix.setTextColor(matrix.color555(0,31,0));
  matrix.setCursor(33, 8); 
  matrix.println("off");
  matrix.setTextColor(matrix.color555(1,15,31));
  matrix.setCursor(5, 16); 
  matrix.println("E");
  matrix.setCursor(9, 16); 
  matrix.println(".");
  matrix.setCursor(14, 16); 
  matrix.println("M");
  matrix.setCursor(18, 16); 
  matrix.println(".");
  matrix.setCursor(23, 16); 
  matrix.println("O");
  matrix.setCursor(27, 16); 
  matrix.println(".");
  matrix.setCursor(31, 16); 
  matrix.println("T");
  matrix.setCursor(34, 16); 
  matrix.println(".");
  matrix.setCursor(39, 16); 
  matrix.println("M");
  matrix.setCursor(43, 16); 
  matrix.println(".");
  matrix.setTextColor(matrix.color555(28,3,0));
  matrix.setCursor(5, 0); 
  matrix.println("LADDERS");
    matrix.setTextColor(matrix.color555(31,16,0));
  matrix.setCursor(5, 24); 
  matrix.println("TABATA");
}

void printUNEVENtime(uint16_t inputTime){

      UNEVENtime_mm_10 = inputTime/600;
      inputTime = inputTime-600*UNEVENtime_mm_10 ;
      
      UNEVENtime_mm_1 = inputTime/60;
      inputTime = inputTime-60*UNEVENtime_mm_1;
     
      UNEVENtime_ss_10 = inputTime;
     
   matrix.fillRect(30, 16, 34 ,8, matrix.color444(0, 0, 0)); // overwrite the text with black
   matrix.setTextColor(matrix.color555(31,0,0));
 if(UNEVENtime_mm_10>=1){
  matrix.setCursor(31, 16); 
  matrix.println(UNEVENtime_mm_10);}
  matrix.setCursor(37, 16); 
  matrix.println(UNEVENtime_mm_1);
  matrix.setCursor(41, 16); 
  matrix.println(":");
  matrix.setCursor(45, 16); 
  matrix.println(UNEVENtime_ss_10);
   matrix.setCursor(51, 16); 
  matrix.println("0");
      }


       void printUNEVENdecrement(uint16_t inputTime){

      UNEVENdecrement_mm_1 = inputTime/60;
      inputTime = inputTime-60*UNEVENdecrement_mm_1;
      UNEVENdecrement_ss_10 =inputTime/10;
      inputTime = inputTime-10*UNEVENdecrement_ss_10;
      UNEVENdecrement_ss_1 =inputTime;
  matrix.fillRect(30, 24, 34 ,8, matrix.color444(0, 0, 0)); // overwrite the text with black
    matrix.setTextColor(matrix.color555(31,0,0));
  matrix.setCursor(37, 24); 
  matrix.println(UNEVENdecrement_mm_1);
  matrix.setCursor(41, 24); 
  matrix.println(":");
  matrix.setCursor(45, 24); 
  matrix.println(UNEVENdecrement_ss_10);
   matrix.setCursor(51, 24); 
 matrix.println(UNEVENdecrement_ss_1);
  
  }
void printXOnYOfftime(int16_t inputTime, int line){

      UNEVENtime_mm_10 = inputTime/600;
      inputTime = inputTime-600*UNEVENtime_mm_10 ;
      
      UNEVENtime_mm_1 = inputTime/60;
      inputTime = inputTime-60*UNEVENtime_mm_1;
     
      UNEVENtime_ss_10 = inputTime/10;
      inputTime = inputTime-10*UNEVENtime_ss_10;

      
     
   matrix.fillRect(30, line, 34 ,8, matrix.color444(0, 0, 0)); // overwrite the text with black
   matrix.setTextColor(matrix.color555(31,0,0));
 if(UNEVENtime_mm_10>=1){
  matrix.setCursor(31, line); 
  matrix.println(UNEVENtime_mm_10);}
  matrix.setCursor(37, line); 
  matrix.println(UNEVENtime_mm_1);
  matrix.setCursor(41, line); 
  matrix.println(":");
  matrix.setCursor(45, line); 
  matrix.println(UNEVENtime_ss_10);
   matrix.setCursor(51, line); 
  matrix.println(inputTime);
      }

  void printXOnYOffRDS(int numberOfRounds, int line){
     matrix.fillRect(30, line, 34 ,8, matrix.color444(0, 0, 0)); // overwrite the text with black
       int TenRDS = numberOfRounds/10;
     numberOfRounds = numberOfRounds- 10*TenRDS;
    matrix.setTextColor(matrix.color555(31,0,0));
        if(TenRDS>0){
      matrix.setCursor(44, line); 
       matrix.println(TenRDS);
       }
       matrix.setCursor(51, line); 
       matrix.println(numberOfRounds);
       }
           
  void buttonPressedTopMERKULIN(){
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
       UNEVENctr++;
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
       UNEVENctr--;
     if(UNEVENctr<0){
      UNEVENctr=0;
      }
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
           if(Select){
            Menu++;
            UNEVENctr=0;
            if(Menu>6){
              Menu=1;
              }
            Select=0;
            }
                }
  }
  }

  void buttonPressedTopEMOTM(){
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
       XOnYOffctr++;
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
       XOnYOffctr--;
     if(XOnYOffctr<0){
      XOnYOffctr=0;
      }
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
           if(Select){
            Menu++;
            XOnYOffctr=0;
            if(Menu>5){
              Menu=1;
              }
            Select=0;
            }
                }
  }
  }


  void buttonPressedTopXOnYOff(){
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
       XOnYOffctr++;
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
       XOnYOffctr--;
     if(XOnYOffctr<0){
      XOnYOffctr=0;
      }
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
           if(Select){
            Menu++;
            XOnYOffctr=0;
            if(Menu>5){
              Menu=1;
              }
            Select=0;
            }
                }
  }
  }

 void roundXofY(int totalRounds, int ik){
    matrix.fillRect(0, 24, 64 ,8, matrix.color444(0, 0, 0)); // overwrite the buttom quarter with black
  matrix.setTextSize(1);
  matrix.setTextColor(matrix.color555(1,15,31)); //VW blue     
  matrix.setCursor(0, 24);
  matrix.println("Round");
  matrix.setCursor(42, 24);
  matrix.println("o");
  matrix.setCursor(47, 24);
  matrix.println("f");
  matrix.setTextColor(matrix.color555(31,0,0)); //RED   
  int roundXofY = totalRounds-ik+1;
  int roundXofY_10 = roundXofY/10;
  int roundXofY_1 = roundXofY-10*roundXofY_10;
  int XonYoffRounds_10 = totalRounds/10;
  int XonYoffRounds_1 = totalRounds-10*XonYoffRounds_10;
  if(roundXofY_10>0){
  matrix.setCursor(30, 24);
  matrix.println(roundXofY_10);
  }
  matrix.setCursor(36, 24);
  matrix.println(roundXofY_1);
  
  if(XonYoffRounds_10 >0){
  matrix.setCursor(52, 24);
  matrix.println(XonYoffRounds_10);
  }
  matrix.setCursor(58, 24);
  matrix.println(XonYoffRounds_1);
    }

     void buttonPressedTopTABATA(){
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
       XOnYOffctr++;
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
       XOnYOffctr--;
     if(XOnYOffctr<0){
      XOnYOffctr=0;
      }
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
           if(Select){
            Menu++;
            XOnYOffctr=0;
            if(Menu>6){
              Menu=1;
              }
            Select=0;
            }
                }
  }
  }

  void waitForReset(){

while(1){
  matrix.stop();
  
  for(i=0; i<10;i++){
  radio.startListening();
  delay(1); // Restart the radio, ready to listen
  RadioListenandSave(); 
  
  if(validMessage.filterChar[0]=='H' && validMessage.reset1 && validMessage.reset2){
    ESP.restart();
    }
  }
  matrix.begin(); // restarts the display and displays the last text visible prior to it being turned off
  delay(4000);
  }    
    }
