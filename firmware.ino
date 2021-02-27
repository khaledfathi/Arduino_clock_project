//for LCD
#include <LiquidCrystal.h>
//for Reader and Audio stream
#include <SD.h>
#include <SPI.h>
#include <pcmRF.h>
#include <pcmConfig.h>
#include <TMRpcm.h>

//pins for lcd
#define pin1 2 //RS 
#define pin2 3 //en
#define pin4 4 //D4
#define pin5 5 //D5
#define pin6 6 //D6
#define pin7 7 //D7
//button
#define mode_button A5 // clock modes 
#define change_button A4 // change values 

//SD reader pins [SPI]
#define SD_ChipSelectPin 10
// mosi 11 // MOSI pin in SD reader
// miso 12 // MISO pin in SD reader
// sck 13 // SCK pin in SD reader

//speaker
#define speaker 9 // audio signal output [should be in PWM port]

//NOTE: Registers are output by default - no need to set them ; 

LiquidCrystal lcd(pin1 , pin2 , pin4 , pin5 , pin6, pin7);
TMRpcm tmrpcm;

uint32_t interrupt_time =0 ; 
short sec = 0;
short min_ = 0; 
short hr = 1 ; 
short day_zone = 0; 
short mode = 0 ; 


//close setting mode after while 
short setting_time_out = 0 ; 



void display_clock(int hr , int min_ , int sec){
  //hr
  if (hr == 0 ){
    lcd.print("00");
  }else if (hr < 10 ){
    lcd.print("0");
    lcd.print(hr);
  }else{
    lcd.print(hr);
  }
  lcd.print(" : ");
  
  //min
  if (min_ == 0 ){
    lcd.print("00");
  }else if (min_ < 10 ){
    lcd.print("0");
    lcd.print(min_);
  }else{
    lcd.print(min_);
  }
  lcd.print (" : "); 
  //sec
  if (sec == 0 ){
    lcd.print("00");
  }else if (sec < 10 ){
    lcd.print("0");
    lcd.print(sec);
  }else{
    lcd.print(sec);
  }
  if(day_zone == 1){
    lcd.print(" AM");
  }else if (day_zone == 0){
    lcd.print(" PM");
   }
}

void cough(unsigned int time_){ // stream 1st audio 
    tmrpcm.play((char*)"a.wav",30);
    delay(time_);
    //tmrpcm.stopPlayback();
    tmrpcm.disable();
    
}

void setup() { ///////////////////////
  Serial.begin(9600);
  
  //Reader and audio stream setup 
  tmrpcm.speakerPin = speaker;
  tmrpcm.setVolume(3);
  SD.begin();
  
  //lcd setup 
  lcd.begin(16,2);
  
  //TIMER 0 
  TCNT2 = 0 ;
  //prescaler 
  TCCR2B|= 1<<CS20;
  
  TIMSK2 |= 1<< TOIE2 ; //timer mask OVF interupt
  
  
  //enable global interrupt 
  sei();

  //set pins for push button 
  DDRB |= 1; // mode_button
  DDRB |= 1<<1; // change_button
  
  
}

void loop() {
  //check memory card
  if (!SD.begin(SD_ChipSelectPin)) {
    Serial.println("SD fail"); 
  }else{
    Serial.println("SD OK"); 
  }
    
   //display time
    lcd.setCursor(1,0);  
    display_clock(hr , min_ , sec);
    
   //timeout for mode setting
   if (setting_time_out == 10){
    lcd.clear();
    setting_time_out=0 ; 
    mode=0;
   }

  //clock user setting  
   if (digitalRead(mode_button)==1){
      setting_time_out=0 ; 
      mode++; 
      if (mode >3){
        mode =0 ;  
        lcd.clear();
      }
      if (mode == 1) {
        lcd.clear();
        lcd.setCursor(0,1);
        lcd.print("Set hours +");
      }else if (mode ==2){
        lcd.clear();
        lcd.setCursor(0,1);
        lcd.print("Set Minutes +");
      }else if (mode ==3){
        lcd.clear();
        lcd.setCursor(0,1);
        lcd.print("Set AM / PM +");
      }
      delay(300);
    }

   if (digitalRead(change_button)==1 && mode==1 ){
      setting_time_out=0 ; 
      hr++;
      delay(200);
    }else if (digitalRead(change_button)==1 && mode==2 ){
      min_++;
      delay(200);
    }else if (digitalRead(change_button)==1 && mode==3 ){
      day_zone ^=1;
      delay(200);
    }
    
    //Run Audio 
    if (hr%2 ==0 && min_ == 0 && sec == 0 ){
        lcd.setCursor(0,1);
        lcd.print("   Ventocough   ");
        cough(1000);
     }else if(hr%2 !=0 && min_ == 0 && sec == 0 ){
        lcd.setCursor(0,1);
        lcd.print("   Levoctivan   ");
        cough(4000);
     }
}
ISR(TIMER2_OVF_vect){
  interrupt_time++;
  if (interrupt_time == 256){
    sec++;
    setting_time_out++;
    interrupt_time=0 ;
   }
  
  //clock_cycle
  if(sec >= 60){
    sec=0; 
    min_++; 
   }else if (min_ >= 60){
    min_ =0; 
    hr ++;
   }else if (hr > 12){
    day_zone ^= 1;
    hr = 1; 
   }  
}
