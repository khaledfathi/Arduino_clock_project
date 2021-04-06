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

//for timer
short ovf = 0;
short ocr_value = 250;
short ovf_count = 250;

short sec = 59;
short min_ = 59; 
short hr = 12; 
short day_zone = 0; 
short mode = 0 ; 

char text[][17] = {"   Ventocough   ", "   Levoctivan   "} ; 
char current_text = 0; 

//close setting mode after while 
short setting_time_out = 0 ; 
short memory_error=0 ;  


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
    tmrpcm.loop(1); 
    tmrpcm.play((char*)"co.wav");
    
    delay(time_);
    
    tmrpcm.stopPlayback();
    
}
void sneeze(unsigned int time_){ // stream 1st audio 
    tmrpcm.loop(1); 
    tmrpcm.play((char*)"sn.wav");
    
    delay(time_);
    
    tmrpcm.stopPlayback();
    
}

void setup() { ///////////////////////
  
   pinMode(mode_button , INPUT_PULLUP);
   pinMode(change_button , INPUT_PULLUP);

   //pin control audio_amp circut 
   DDRB|=1;
   
  //Serial.begin(9600);
  //Reader and audio stream setup 
  tmrpcm.speakerPin = speaker;
  tmrpcm.setVolume(6);
  
  
  //lcd setup 
  lcd.begin(16,2);

  //Check Memory Card
  lcd.print("Starting . . .");
  SD.begin();
  if (!SD.begin(SD_ChipSelectPin)) {
    memory_error = 1; 
    lcd.clear();
    lcd.print("Memory ERROR");
   }else {
    memory_error = 0; 
   }
   delay(2000);
   lcd.clear();

  TCCR1A = 0x00 ; //cuz arduino use it for another function 
  
  //TIMER 0 
  TCNT2 = 0 ;//reset timer 0

  //prescaler 
  TCCR2B &= ~(1<<CS20);
  TCCR2B |= 1<<CS21;
  TCCR2B |= 1<<CS22;
  OCR2B = ocr_value;
  TIMSK2 |= 1<< OCIE1B ;//mask for compar with OCR2B

  
  //enable global interrupt 
  sei();

  //set pins for push button 
  
  
}

void loop() { 
     
   //display time
    lcd.setCursor(1,0);  
    display_clock(hr , min_ , sec);
    
   //timeout for mode setting
   if (setting_time_out == 10 && mode != 0){
    lcd.clear();
    lcd.setCursor(0,1);
    lcd.print(text[current_text]);
    setting_time_out=0 ; 
    mode=0;
   }

  //clock user setting  
   if (digitalRead(mode_button)==0){
      setting_time_out=0 ; 
      mode++; 
      if (mode >3){
        mode =0 ;  
        lcd.clear();
        lcd.setCursor(0,1);
        lcd.print(text[current_text]);
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

   if (digitalRead(change_button)==0 && mode==1 ){
      setting_time_out=0 ; 
      hr++;
      if(hr>12)hr=1;
      delay(200);
    }else if (digitalRead(change_button)==0 && mode==2 ){
      setting_time_out=0 ; 
      min_++;
      if(min_==60)min_=0;
      delay(200);
    }else if (digitalRead(change_button)==0 && mode==3 ){
      setting_time_out=0 ; 
      day_zone ^=1;
      delay(200);
    }
    //Run Audio 
    if (hr%2 ==0 && min_ == 0 && sec == 0){
        current_text^=1; //toggle names
        lcd.setCursor(0,1);
        lcd.print(text[current_text]);
        if (memory_error == 0){
          PORTB |=1;//enable audio circut first
          sneeze(600);
          cough(1000);
          PORTB &=~(1);//disable audio circut 
         }
        
     }else if(hr%2 !=0 && min_ == 0 && sec == 0 ){
        current_text^=1;
        lcd.setCursor(0,1);
        lcd.print(text[current_text]);
        if (memory_error == 0){
          PORTB |=1;//enable audio circut first
          sneeze(600);
          cough(1000);
          PORTB &=~(1);//disable audio circut 
        }
     }
}

ISR(TIMER2_COMPB_vect){
  ovf++;
   if (ovf == ovf_count){
    sec++;
    setting_time_out++;  
    ovf=0 ; 
    //clock_cycle
    if(sec == 60){
      sec=0; 
      min_++; 
     }
     if (min_ == 60){
      min_ =0; 
      hr ++;
     }
     if (hr > 12){
      day_zone ^= 1;
      hr = 1; 
     }
    }  
}
