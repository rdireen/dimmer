/**
*             Dimmer for Atmega328P
* Randy Direen
* 5/2/2018
*
* Supports external interrupt for phase lock on mains. 
* 
* Two timers used for two independent lights
*
*/

const unsigned char dtable[] = {121,121,121,121,120,120,120,120,120,119,119,119,119,
                          118,118,118,118,117,117,117,117,116,116,116,116,115,
                          115,115,114,114,114,113,113,112,112,112,111,111,111,
                          110,110,109,109,108,108,107,107,106,106,105,105,104,
                          104,103,102,102,101,101,100,99,98,98,97,96,96,95,94,
                          93,92,91,90,89,88,87,86,85,84,83,82,81,79,78,77,75,
                          74,72,70,68,67,65,63,60,58,55,52,49,45,41,35,28,5};


/****************************** Interrupts **********************************/

//**************************************************************
//Fire TRIAC1 for dimming Light1
//**************************************************************
// Setup interrupt and code for triggering the first TRIAC for 
// the first light to be dimmed.

#define PinTRIAC1 6

/* @brief ISR for triggering the first TRIAC
 * 
 * Turn on the TRIAC after a set period of time. COMPB will be 
 * used to turn this off.
 */
ISR(TIMER1_COMPA_vect) {
  digitalWrite(PinTRIAC1, HIGH);           
}

/* @brief ISR for triggering the first TRIAC
 * 
 * Turn off the TRIAC so that between this interrupt and the 
 * COMPA interrupt we have a pulse sent to the TRIAC
 */
ISR(TIMER1_COMPB_vect){
  digitalWrite(PinTRIAC1, LOW);
}

/* @brief Called after external interrupt by zero crossing
 * 
 */
void restartTimer1(){
  TCNT1 = 0x00;
  TCCR1B |=  ( 1 << CS12 ) | ( 1 << CS10 ) ;
}

/* @brief This delay controls the brightness of the light
 * 
 */
void setDelay1(int16_t pulses){
  OCR1A = pulses;
  OCR1B = pulses + 4;
}

/* @brief Initialize Timer1 for CTC mode
 * 
 * 
 */
void initTimer1ForTRIAC1Fire(){

  pinMode(PinTRIAC1, OUTPUT);
  
  TCCR1A = 0;
  TCCR1B = 0;
  TCCR1C = 0;
  TCNT1  = 0;
  TIMSK1 = 0;

  TIMSK1 |= (1 << OCIE1A);  // Start Compare register A interrupt
  TIMSK1 |= (1 << OCIE1B);  // Start Compare register B interrupt
  
  TCCR1A |= B11110000;              
  TCCR1B |=  ( 1 << CS12 ) | ( 1 << CS10 ) ; // START Timer, prescaler = 8

  setDelay1(dtable[99]);
}

//**************************************************************
//Fire TRIAC2 for dimming Light2
//**************************************************************
// Setup interrupt and code for triggering the second TRIAC for 
// the second light to be dimmed.

#define PinTRIAC2 5

/* @brief ISR for triggering the first TRIAC
 * 
 * Turn on the TRIAC after a set period of time. COMPB will be 
 * used to turn this off.
 */
ISR(TIMER2_COMPA_vect) {
  digitalWrite(PinTRIAC2, HIGH);           
}

/* @brief ISR for triggering the first TRIAC
 * 
 * Turn off the TRIAC so that between this interrupt and the 
 * COMPA interrupt we have a pulse sent to the TRIAC
 */
ISR(TIMER2_COMPB_vect){
  digitalWrite(PinTRIAC2, LOW);
}

/* @brief Called after external interrupt by zero crossing
 * 
 */
void restartTimer2(){
  TCNT2 = 0x00;
  TCCR2B |=  ( 1 << CS22 ) | ( 1 << CS21 ) | ( 1 << CS20 );
}

/* @brief This delay controls the brightness of the light
 * 
 */
void setDelay2(unsigned char pulses){
  OCR2A = pulses;
  OCR2B = pulses + 4;
}

/* @brief Initialize Timer1 for CTC mode
 * 
 * 
 */
void initTimer2ForTRIAC2Fire(){

  pinMode(PinTRIAC2, OUTPUT);
  
  TCCR2A = 0;
  TCCR2B = 0;
  TCNT2  = 0;
  TIMSK2 = 0;

  TIMSK2 |= (1 << OCIE2A);  // Start Compare register A interrupt
  TIMSK2 |= (1 << OCIE2B);  // Start Compare register B interrupt
  
  TCCR2A |= B11110000;              
  TCCR2B |=  ( 1 << CS22 ) | ( 1 << CS21 ) | ( 1 << CS20 ); // START Timer, prescaler = 1024

  setDelay2(dtable[99]);
}



//**************************************************************
//Zero Crossing Detector
//**************************************************************
const byte zeroCrossInterruptPin = 2;
volatile unsigned long count = 0;

void handleZeroCrossingInterrupt(){
  count++;
  noInterrupts();
  restartTimer1();
  restartTimer2();
  interrupts();
}

void initZeroCrossingInterrupt(){
  pinMode(zeroCrossInterruptPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(zeroCrossInterruptPin), handleZeroCrossingInterrupt, RISING);
}

//**************************************************************
//Encoders
//**************************************************************

class Encoder{
public:

  /**
   * @brief Initialize encoder for particular set of pins
   * 
   * TODO: update this to work with pushbutton on/off
   * 
   * @param pin1 the first pin getting tied low from encoder
   * @param pin2 the second pin getting tied low from encoder
   * @param pinb the button pin corresponding to the push button
   */
  void init(byte pin1, byte pin2, byte pinb){
    

    dimval_ = 0;
    pin1_ = pin1;
    pin2_ = pin2;
    pinb_ = pinb;
    
    *digitalPinToPCMSK(pin1) |= bit (digitalPinToPCMSKbit(pin1));  // enable pin
    PCIFR  |= bit (digitalPinToPCICRbit(pin1)); // clear any outstanding interrupt
    PCICR  |= bit (digitalPinToPCICRbit(pin1)); // enable interrupt for the group
  
    *digitalPinToPCMSK(pin2) |= bit (digitalPinToPCMSKbit(pin2));  // enable pin
    PCIFR  |= bit (digitalPinToPCICRbit(pin2)); // clear any outstanding interrupt
    PCICR  |= bit (digitalPinToPCICRbit(pin2)); // enable interrupt for the group

    *digitalPinToPCMSK(pinb) |= bit (digitalPinToPCMSKbit(pinb));  // enable pin
    PCIFR  |= bit (digitalPinToPCICRbit(pinb)); // clear any outstanding interrupt
    PCICR  |= bit (digitalPinToPCICRbit(pinb)); // enable interrupt for the group
    
    pinMode(pin1, INPUT_PULLUP);
    pinMode(pin2, INPUT_PULLUP);
    pinMode(pinb, INPUT_PULLUP);
  }

  /**
   * @brief Get the current dimmer value [0, 100]
   */
  uint8_t dimval(){return dimval_;}

  /** 
   * @brief Set the dimmer value
   */
  void set_dimval(uint8_t dimval){
    dimval = dimval < 0 ? 0 : dimval;
    dimval_ = dimval > 100 ? 100 : dimval; 
  }

  /**
   * @brief needs to be called every time there is a pin change
   */
  void ISR_pins_changed(){
    static int encLastState = 0;
    int encState = 0;
    
    static int encLastVal1 = 0;
    int encVal1 = 0;
    
    static int encLastVal2 = 0;
    int encVal2 = 0;
    
    const int dimStep = 4;
  
    static int lastDir = 0; // 1 for right, -1 left
    static int dir = 0; // 1 for right, -1 left
  
    encVal1 = digitalRead(pin1_);
    encVal2 = digitalRead(pin2_);
    encState = encVal2 << 1 | encVal1;
  
    if(enc1State == 0){
      dir = encLastState == 1 ? -1 : 1;
    }else if(encState == 1){
      dir = encLastState == 3 ? -1 : 1;
    }else if(encState == 2){
      dir = encLastState == 0 ? -1 : 1;
    }else if(encState == 3){
      dir = encLastState == 2 ? -1 : 1;
    }
  
    if(dir == lastDir){
      if((encLastState == 2) && (encState == 0)){
        dimval_ += dimStep;
      }else if((enc1LastState == 1) && (enc1State == 3)){
        dimval_ += dimStep;
      }
      dimval_ = dimval_ > 99? 99 : dimval_;
  
      if((encLastState == 1) && (encState == 0)){
        dimval_ -= dimStep;
      }else if((encLastState == 2) && (encState == 3)){
        dimval_ -= dimStep;
      }
        
      dimval_ = dimval_ < 0? 0 : dimval_;
    }
    //setDelay2(dtable[dimval]);
    lastDir = dir;

    encLastState = encState; 
  }

private:
  uint8_t dimval_;
  byte pin1_;
  byte pin2_;
  byte pinb_;

  
  
};


#define Pin1Enc1 8
#define Pin2Enc1 9

//Pin change interrupt for D8 to D13
ISR(PCINT0_vect){

  Serial.println("PinChange");

  static int enc1LastState = 0;
  int enc1State = 0;
  
  static int enc1LastVal1 = 0;
  int enc1Val1 = 0;
  
  static int enc1LastVal2 = 0;
  int enc1Val2 = 0;
  
  static int dimval = 99;
  const int dimStep = 4;

  static int lastDir = 0; // 1 for right, -1 left
  static int dir = 0; // 1 for right, -1 left

  enc1Val1 = digitalRead(Pin1Enc1);
  enc1Val2 = digitalRead(Pin2Enc1);
  enc1State = enc1Val2 << 1 | enc1Val1;

  if(enc1State == 0){
    dir = enc1LastState == 1 ? -1 : 1;
  }else if(enc1State == 1){
    dir = enc1LastState == 3 ? -1 : 1;
  }else if(enc1State == 2){
    dir = enc1LastState == 0 ? -1 : 1;
  }else if(enc1State == 3){
    dir = enc1LastState == 2 ? -1 : 1;
  }

  if(dir == lastDir){
    if((enc1LastState == 2) && (enc1State == 0)){
      Serial.println("UP");
      dimval += dimStep;
    }else if((enc1LastState == 1) && (enc1State == 3)){
      Serial.println("UP");
      dimval += dimStep;
    }
    dimval = dimval > 99? 99 : dimval;

    if((enc1LastState == 1) && (enc1State == 0)){
      Serial.println("DOWN");
      dimval -= dimStep;
    }else if((enc1LastState == 2) && (enc1State == 3)){
      Serial.println("DOWN");
      dimval -= dimStep;
    }
      
    dimval = dimval < 0? 0 : dimval;
  }
  Serial.println(dimval);
  setDelay2(dtable[dimval]);
  lastDir = dir;

  Serial.print("this: ");
  Serial.print(enc1State);
  Serial.print(" last: ");
  Serial.print(enc1LastState);
  Serial.print("\n");

  enc1LastState = enc1State;
  Serial.print("dir: ");
  Serial.print(dir);
  Serial.print("\n");
}

void initEncoder1(){


  *digitalPinToPCMSK(Pin1Enc1) |= bit (digitalPinToPCMSKbit(Pin1Enc1));  // enable pin
  PCIFR  |= bit (digitalPinToPCICRbit(Pin1Enc1)); // clear any outstanding interrupt
  PCICR  |= bit (digitalPinToPCICRbit(Pin1Enc1)); // enable interrupt for the group

  *digitalPinToPCMSK(Pin2Enc1) |= bit (digitalPinToPCMSKbit(Pin2Enc1));  // enable pin
  PCIFR  |= bit (digitalPinToPCICRbit(Pin2Enc1)); // clear any outstanding interrupt
  PCICR  |= bit (digitalPinToPCICRbit(Pin2Enc1)); // enable interrupt for the group
  
  pinMode(Pin1Enc1, INPUT_PULLUP);
  pinMode(Pin2Enc1, INPUT_PULLUP);

  
}




//**************************************************************
//**************************************************************
//**************************************************************
//
//                     MAIN PROGRAM
//
// setup()
// loop()
//**************************************************************
//**************************************************************
//**************************************************************


String incoming = "";

void setup() {

   pinMode(LED_BUILTIN, OUTPUT);
   
   noInterrupts();
   initZeroCrossingInterrupt();
   initTimer1ForTRIAC1Fire();
   initTimer2ForTRIAC2Fire();
   initEncoder1();
   interrupts();

   Serial.begin(115200);
   Serial.setTimeout(1);

}

void loop() {
  int16_t pulses;

  
  if (Serial.available() > 0){
    incoming = Serial.readString();
    if(incoming == "LON"){
      digitalWrite(LED_BUILTIN, HIGH);
    }else if(incoming == "LOFF"){
      digitalWrite(LED_BUILTIN, LOW);
    }else if(incoming[0] == 'D'){
      Serial.println(incoming.substring(1).c_str());
      pulses = (unsigned int16_t)atoi(incoming.substring(1).c_str());
      noInterrupts();
      setDelay1(dtable[pulses]);
      interrupts();
    }else if(incoming[0] == 'E'){
      Serial.println(incoming.substring(1).c_str());
      pulses = (unsigned int16_t)atoi(incoming.substring(1).c_str());
      noInterrupts();
      setDelay2(dtable[pulses]);
      interrupts();
    }else if(incoming == "C"){
      Serial.println("Pulse count: ");
      Serial.println(count);
    }
   
  }

  

 
}
