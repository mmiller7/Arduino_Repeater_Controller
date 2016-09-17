//COS sensing bi-directional repeater for Arduino
//Matthew Miller
//KK4NDE
//17-Sept-2016
//This is based on the VOX repeater sketch but modified for more reliable use
//by interfaceing radios equipped with carrier-operated squealch output.
//DISCLAIMER: This code has not been personally tested for operation but is
//based off working code only changing the detection parameters and renaming
//a few variables.  Use at your own risk.

//Here are some common pins you may want to use depending on if you have an Arduino or ATTINY
//configure the #define at the beginning of the code
//configure the radioA.value radioB.value lines in the setup() function
//NOTE: voltSensePin, radioA.cosPin, and radioB.cosPin should be DIGITAL inputs
//if you don't want to use the voltSensePin just tie that pin to +V and it will always assume the battery is full

//Callsign to ID with, use lowercase only
#define CALLSIGN "xx0xxx"

//Sets the COS (Carrier Operated Squealch) value to represent "squealch open"
//If your COS is +V when carrier present/squealch open, use HIGH
//If your COS is 0V when carrier present/squealch open, use LOW
#define COS_VALUE_SQL_OPEN LOW

//Analog pin for voltage sense
#define voltSensePin 6

//define threshold for low battery
#define lowBattThreshold 11.5

//define threshold below which low battery is not triggered
//this lets you power it with low-voltage and not get any alerts
#define lowNotifyFloor 9.5

//how many milliseconds to ID every
//600000 is 10 minutes in milliseconds
#define idTimeout 600000

//define delay for squealch-tail hold over in milliseconds
#define cosDelay 1000

//morse code "dit" base unit length in milliseconds
#define ditLen 60

//the pitch for the dit/dah tone used by the ID
#define tonePitch 800

//data structure for radio info
struct Radio
{
  //cosPin - Audio In (for COS) - Digital Pin
  //micPin - MIC Mix (for tone) - Digital Pin
  //pttPin - PTT OUT (for TX)   - Digital Pin
  int micPin, pttPin, cosPin, autoId, battMon;
  long lastBattMonTime, lastIdTime, lastCosTime;
};

//globals to store radio config
Radio radioA, radioB;

//declarations for functions which use radio struct
void configure(Radio &radio);
void cosCheck(Radio &radio);
void txAutoId(Radio &radio);
void lowBattCheck(Radio &radio);
boolean isBusy(Radio &radio);

void setup() {
  
  /******************************************************
  ** Configure radio connection pins and settings here **
  ******************************************************/
  
  //set config for radio A
  radioA.cosPin=2;
  radioA.micPin=7;
  radioA.pttPin=0;
  radioA.autoId=true;
  radioA.battMon=false;
  radioA.lastBattMonTime=idTimeout;
  radioA.lastIdTime=idTimeout;
  radioA.lastCosTime=0;

  //set config for radio B
  radioB.cosPin=1;
  radioB.micPin=10;
  radioB.pttPin=1;
  radioB.autoId=true;
  radioB.battMon=true;
  radioB.lastBattMonTime=idTimeout;
  radioB.lastIdTime=idTimeout;
  radioB.lastCosTime=0;
  
  //apply config for radios (set pinmode/etc)
  configure(radioA);
  configure(radioB);
  
  //broadcast ID if applicable
  txAutoId(radioA);
  txAutoId(radioB);
}

//configures pinmode and setup for radio
void configure(Radio &radio)
{
  pinMode(radio.micPin,OUTPUT);
  pinMode(radio.pttPin,OUTPUT);
  digitalWrite(radio.micPin,LOW);
  digitalWrite(radio.pttPin,LOW);
}

void loop()
{
  if(!isBusy(radioB)) //if the other radio is transmitting, this one must be receiving so don't key up
  {
    lowBattCheck(radioA);
    txAutoId(radioA);
    cosCheck(radioA);
  }
    
  if(!isBusy(radioA)) //if the other radio is transmitting, this one must be receiving so don't key up 
  {
    lowBattCheck(radioB);
    txAutoId(radioB);
    cosCheck(radioB);
  }
}

//checks if a radio's PTT pin is keyed
boolean isBusy(Radio &radio)
{
  return digitalRead(radio.pttPin);
}

//checks if feature is enabled (if pin is true/false)
boolean isEnabled(int pin)
{
  return pin; //temp just return true/false coded
  //return digitalRead(pin);
}

//trigger PTT based on COS input and delay
void cosCheck(Radio &radio)
{
  
  // test if the pin has cos
  if(digitalRead(radio.cosPin) == COS_VALUE_SQL_OPEN)
  {
    //cos active
    digitalWrite(radio.pttPin,HIGH);
    radio.lastCosTime=millis();
  }
  else
  {
    if(millis()-radio.lastCosTime < cosDelay)
    {
      //cos delay
    }
    else
    {
      digitalWrite(radio.pttPin,LOW);
    }
  }
}

//broadcast ID if applicable
void txAutoId(Radio &radio)
{
  if(isEnabled(radio.autoId) && (millis()-radio.lastIdTime) > idTimeout)
  {
    boolean tx=digitalRead(radio.pttPin);
    digitalWrite(radio.pttPin,HIGH);
    delay(500);
    morseCode(radio.micPin,CALLSIGN);
    radio.lastIdTime=millis();
    digitalWrite(radio.pttPin,tx);
  }
}

//broadcast low battery if applicable
void lowBattCheck(Radio &radio)
{
  float voltage=getPowerVoltage(voltSensePin);
  if(isEnabled(radio.battMon) && voltage < lowBattThreshold && voltage > lowNotifyFloor && (millis()-radio.lastBattMonTime) > idTimeout)
  {
    boolean tx=digitalRead(radio.pttPin);
    digitalWrite(radio.pttPin,HIGH);
    delay(500);
    
    //encode low battery morse code message
//    char temp[]="lb ";
//    strcat(temp,voltage);
//    strcat(temp,"v");
//    morseCode(radio.micPin,temp);
    morseCode(radio.micPin,"lb");
    //morseCode(radio.micPin,"lb "+ toString(voltage) + "v");
    
    radio.lastBattMonTime=millis();
    digitalWrite(radio.pttPin,tx);
  }
}

//for floats from ~1 to ~19
void strcat(char * appendTo, float input)
{
  char temp[]="x";
  if(input > 10)
  {
    strcat(appendTo,"1");
    input-=10;
  }
  temp[0]=(char)((int)input+48); //add 48 to shift to ascii value
  strcat(appendTo,temp); //append to output
  input-=(int)input; //take off whole value
  strcat(appendTo,".");
  input*=10; //iterate to first place past decimal
  if((input-(int)input)>.4) //round (because here we will drop everything after decimal)
    input++;
  temp[0]=(char)((int)input+48); //add 48 to shift to ascii value
  strcat(appendTo,temp); //append to output
}

//send morse code message by applying tone to codePin
void morseCode(int codePin, char* message)
{
  // message.trim();
  //message.toLowerCase();
  int code;
  int length;
   for(unsigned int x=0; x < strlen(message); x++)
   {
     //shift case
     if(message[x] < 91 && message[x] > 64)
       message[x]+=32;
       
     //encode morse code
     switch(message[x])
     {
       case 'a':
                 //strcpy(temp,".-");
                 code=B01;
                 length=2;
                 break;
       case 'b':
                 //strcpy(temp,"-...");
                 code=B1000;
                 length=4;
                 break;
       case 'c':
                 //strcpy(temp,"-.-.");
                 code=B1010;
                 length=4;
                 break;
       case 'd':
                 //strcpy(temp,"-..");
                 code=B100;
                 length=3;
                 break;
       case 'e':
                 //strcpy(temp,".");
                 code=B0;
                 length=1;
                 break;
       case 'f':
                 //strcpy(temp,"..-.");
                 code=B0010;
                 length=4;
                 break;
       case 'g':
                 //strcpy(temp,"--.");
                 code=B110;
                 length=3;
                 break;
       case 'h':
                 //strcpy(temp,"....");
                 code=B0000;
                 length=4;
                 break;
       case 'i':
                 //strcpy(temp,"..");
                 code=B00;
                 length=2;
                 break;
       case 'j':
                 //strcpy(temp,".---");
                 code=B0111;
                 length=4;
                 break;
       case 'k':
                 //strcpy(temp,"-.-");
                 code=B101;
                 length=3;
                 break;
       case 'l':
                 //strcpy(temp,".-..");
                 code=B0100;
                 length=4;
                 break;
       case 'm':
                 //strcpy(temp,"--");
                 code=B11;
                 length=2;
                 break;
       case 'n':
                 //strcpy(temp,"-.");
                 code=B10;
                 length=2;
                 break;
       case 'o':
                 //strcpy(temp,"---");
                 code=B111;
                 length=3;
                 break;
       case 'p':
                 //strcpy(temp,".--.");
                 code=B0110;
                 length=4;
                 break;
       case 'q':
                 //strcpy(temp,"--.-");
                 code=B1101;
                 length=4;
                 break;
       case 'r':
                 //strcpy(temp,".-.");
                 code=B010;
                 length=3;
                 break;
       case 's':
                 //strcpy(temp,"...");
                 code=B000;
                 length=3;
                 break;
       case 't':
                 //strcpy(temp,"-");
                 code=B1;
                 length=1;
                 break;
       case 'u':
                 //strcpy(temp,"..-");
                 code=B001;
                 length=3;
                 break;
       case 'v':
                 //strcpy(temp,"...-");
                 code=B0001;
                 length=4;
                 break;
       case 'w':
                 //strcpy(temp,".--");
                 code=B011;
                 length=3;
                 break;
       case 'x':
                 //strcpy(temp,"-..-");
                 code=B1001;
                 length=4;
                 break;
       case 'y':
                 //strcpy(temp,"-.--");
                 code=B1011;
                 length=4;
                 break;
       case 'z':
                 //strcpy(temp,"--..");
                 code=B1100;
                 length=4;
                 break;
       case '0':
                 //strcpy(temp,"-----");
                 code=B11111;
                 length=5;
                 break;
       case '1':
                 //strcpy(temp,".----");
                 code=B01111;
                 length=5;
                 break;
       case '2':
                 //strcpy(temp,"..---");
                 code=B00111;
                 length=5;
                 break;
       case '3':
                 //strcpy(temp,"...--");
                 code=B00011;
                 length=5;
                 break;
       case '4':
                 //strcpy(temp,"....-");
                 code=B00001;
                 length=5;
                 break;
       case '5':
                 //strcpy(temp,".....");
                 code=B00000;
                 length=5;
                 break;
       case '6':
                 //strcpy(temp,"-....");
                 code=B10000;
                 length=5;
                 break;
       case '7':
                 //strcpy(temp,"--...");
                 code=B11000;
                 length=5;
                 break;
       case '8':
                 //strcpy(temp,"---..");
                 code=B11100;
                 length=5;
                 break;
       case '9':
                 //strcpy(temp,"----.");
                 code=B11110;
                 length=5;
                 break;
       case ' ':
                 //strcpy(temp,"");
                 code=B0;
                 length=0;
                 delay(7*ditLen);
                 break;
       case '.':
                 //strcpy(temp,".-.-.-");
                 code=B010101;
                 length=6;
                 break;
       case '/':
                 //strcpy(temp,"-..-.");
                 code=B10010;
                 length=5;
                 break;
       case '-':
                 //strcpy(temp,"-....-");
                 code=B100001;
                 length=6;
                 break;
       case '?':
                 //strcpy(temp,"..--..");
                 code=B001100;
                 length=6;
                 break;
       case '@': //debug symbol
                 code=0;
                 length=0;
                 break;
       case '%': //debug symbol
                 code=0;
                 length=0;
                 break;
       default:
                 //strcpy(temp,"");
                 code=B0;
                 length=0;
                 break;
     }
     
     while(length > 0)
     {
       //determine if it's a dit or a dot
       if(code & bitMask(length))
       {
         //1 is a dit
         tone(codePin,tonePitch);
         delay(3*ditLen);
         noTone(codePin);
         delay(ditLen);
       }
       else
       {
         //0 is a dot
         tone(codePin,tonePitch);
         delay(ditLen);
         noTone(codePin);
         delay(ditLen);
       }
       length--;
     }
     delay(ditLen);
   }
}

//generates a "bit mask" for getting nth bit from int
int bitMask(int bitNumber)
{
  int value=1;
  for(int x=1; x < bitNumber; x++)
    value*=2;
  return value;
}

//gets voltage from analog input pin
float getPowerVoltage(int pin)
{
  // R1 = 2200 (Vin to midpoint)
  // R2 = 1000 (midpoint to gnd)
  // put 5.1v protectioin zener in parallel for R2 to protect arduino input against overvolt
  // formula:
  //     ( value/1023 * (arduino vcc) ) / (  R2   /     (R2 + R1)    ) + (Vin diode drop)
  //return ((analogRead(pin)/1023.0)*5.0) / (1000.0 / (2200.0 + 1000.0)) + 0.76 ;
  return (analogRead(pin)*0.0156402737047898) + 0.76; //simplified
}
