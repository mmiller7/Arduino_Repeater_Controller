//COS sensing bi-directional repeater for Arduino
//Matthew Miller
//KK4NDE
//17-Sept-2016
//
//Changelog:
//cos_repeater_generic - Matthew Miller 17 September 2016
//  Initial version
//cos_repeater_generic2 - Matthew Miller 10 December 2016
//  Bugfix - changed COS function for clarity to match wiring
//  Debug - added Serial deugging enabled with preprocessor flag
//cos_repeater_generic3 - Matthew Miller 10 December 2016
//  Added flag to only ID if radio transmitted
//  Cleaned up some code
//cos_repeater_generic4 - Matthew Miller 17 December 2016
//  Added software-flag to disable a radio
//  Fixed battMon flag changed int to boolean

//This is based on the VOX repeater sketch but modified for more reliable use
//by interfaceing radios equipped with carrier-operated squealch output.

//I have provided example pin numbers that I think would work with the Arduino Uno,
//they can be changed for other boards or ATTINY chips depending on your need.
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
#define voltSensePin 0

//define threshold for low battery
#define lowBattThreshold 11.5

//define threshold below which low battery is not triggered
//this lets you power it with low-voltage and not get any alerts
#define lowNotifyFloor 9.5

//how many milliseconds to ID every
//600000 is 10 minutes in milliseconds
#define idTimeout 600000
#define onlyIdAfterTx true

//define delay for squealch-tail hold over in milliseconds
#define cosDelay 1000

//morse code "dit" base unit length in milliseconds
#define ditLen 60

//the pitch for the dit/dah tone used by the ID
#define tonePitch 800

//uncomment to enable serial debugging
//#define ENABLE_DEBUG
//#define ENABLE_DEBUG_COS_STATE
//#define ENABLE_DEBUG_PTT
//#define ENABLE_DEBUG_RADIO_STATE
//#define ENABLE_DEBUG_NEEDSID

//data structure for radio info
struct Radio
{
  //cosPin - Audio In (for COS) - Digital Pin
  //micPin - MIC Mix (for tone) - Digital Pin
  //pttPin - PTT OUT (for TX)   - Digital Pin
  int micPin, pttPin, cosPin, autoId;
  long lastBattMonTime=idTimeout,
       lastIdTime=idTimeout,
       lastCosTime=0;
  boolean battMon=true,
          needsId=true,
          txAllowed=true;
  #ifdef ENABLE_DEBUG
  String nametag;
  #endif
};

//globals to store radio config
Radio radioA, radioB;

//declarations for functions which use radio struct
void configure(Radio &radio);
void cosCheckAndRepeat(Radio &rxRadio, Radio &txRadio);
void txAutoId(Radio &radio);
void lowBattCheck(Radio &radio);
boolean isBusy(Radio &radio);

//some declarations for debugging
#ifdef ENABLE_DEBUG
void printRadioState(Radio &radio);
long debugRadioStateLastTime=0;
#endif

void setup() {
  Serial.begin(9600);
  
  /******************************************************
  ** Configure radio connection pins and settings here **
  ******************************************************/
  
  //set config for radio A
  //Note, these would all be "digital" pins
  radioA.cosPin=2;
  radioA.micPin=3;
  radioA.pttPin=4;
  radioA.autoId=true;
  radioA.battMon=false;
  radioA.txAllowed=true;
  //for debugging only
  #ifdef ENABLE_DEBUG
  radioA.nametag="radioA";
  #endif

  //set config for radio B
  //Note, these would all be "digital" pins
  radioB.cosPin=5;
  radioB.micPin=6;
  radioB.pttPin=7;
  radioB.autoId=true;
  radioB.battMon=false;
  radioB.txAllowed=true;
  //for debugging only
  #ifdef ENABLE_DEBUG
  radioB.nametag="radioB";
  #endif
  
  //apply config for radios (set pinmode/etc)
  configure(radioA);
  configure(radioB);
  
  //broadcast ID if applicable
  txAutoId(radioA);
  txAutoId(radioB);

  #ifdef ENABLE_DEBUG
  printSetupDebug();
  #endif
}

//configures pinmode and setup for radio
void configure(Radio &radio)
{
  #ifdef ENABLE_DEBUG
  Serial.print("Configuring ");
  Serial.println(radio.nametag);
  #endif
  pinMode(radio.micPin,OUTPUT);
  pinMode(radio.pttPin,OUTPUT);
  digitalWrite(radio.micPin,LOW);
  digitalWrite(radio.pttPin,LOW);
}

void loop()
{
  if(isEnabled(radioA.txAllowed) && !isBusy(radioB)) //if the other radio is transmitting, this one must be receiving so don't key up
  {
    lowBattCheck(radioA);
    txAutoId(radioA);
    cosCheckAndRepeat(radioB,radioA);
  }
    
  if(isEnabled(radioB.txAllowed) && !isBusy(radioA)) //if the other radio is transmitting, this one must be receiving so don't key up 
  {
    lowBattCheck(radioB);
    txAutoId(radioB);
    cosCheckAndRepeat(radioA,radioB);
  }

  #ifdef ENABLE_DEBUG_RADIO_STATE
  if(millis()-debugRadioStateLastTime > 10000)
  {
    printAllRadioState();
    debugRadioStateLastTime=millis();
  }
  #endif
}

//checks if a radio's PTT pin is keyed
boolean isBusy(Radio &radio)
{
  boolean isBusy = digitalRead(radio.pttPin);
  #ifdef ENABLE_DEBUG_VERBOSE
  Serial.print(radio.nametag);
  Serial.print(" isBusy");
  Serial.print("=");
  Serial.println(isBusy);
  #endif
  return isBusy;
}

//checks if feature is enabled (if pin is true/false)
//  In the future this had some grand plan of making it
//  take a pin # and then check if that pin was high
//  or low depending on jumpers/switches but that didn't
//  happen so now this just returns the feature-boolean
//  stored in the radio object.
boolean isEnabled(boolean feature)
{
  return feature; //temp just return true/false coded
  //return digitalRead(pin);
}

//trigger PTT based on COS input and delay
void cosCheckAndRepeat(Radio &rxRadio, Radio &txRadio)
{
  if(isEnabled(txRadio.txAllowed))
  {
    // test if the pin has cos
    if(digitalRead(rxRadio.cosPin) == COS_VALUE_SQL_OPEN)
    {
      #ifdef ENABLE_DEBUG_COS_STATE
      Serial.print("COS squealch open on ");
      Serial.println(rxRadio.nametag);
      #endif
  
      //cos active
      #ifdef ENABLE_DEBUG_PTT
      Serial.print("Turning on TX for ");
      Serial.println(txRadio.nametag);
      #endif
      digitalWrite(txRadio.pttPin,HIGH);
      #ifdef ENABLE_DEBUG_NEEDSID
      Serial.print(txRadio.nametag);
      Serial.println("setting needsId=true in 'cos active' if clause");
      #endif
      txRadio.needsId=true;
      rxRadio.lastCosTime=millis();
    }
    else
    {
      if(millis()-rxRadio.lastCosTime < cosDelay)
      {
        //cos delay
        #ifdef ENABLE_DEBUG_NEEDSID
        Serial.print(txRadio.nametag);
        Serial.println("setting needsId=true in 'cos delay' else-if clause");
        #endif
        txRadio.needsId=true;
      }
      else
      {
        #ifdef ENABLE_DEBUG_PTT
        Serial.print("Turning off TX for ");
        Serial.println(txRadio.nametag);
        #endif
        digitalWrite(txRadio.pttPin,LOW);
      }
    }
  }
}

//broadcast ID if applicable
void txAutoId(Radio &radio)
{
  //If we're only suppsoed to ID after TX
  //and we didn't TX
  if(onlyIdAfterTx && !radio.needsId)
  {
    //keep resetting the last ID time to now
    radio.lastIdTime=millis();
  }
  
  if(isEnabled(radio.txAllowed) && isEnabled(radio.autoId) && (millis()-radio.lastIdTime) > idTimeout)
  {
    #ifdef ENABLE_DEBUG
    Serial.print("Sending autoID on ");
    Serial.println(radio.nametag);
    #endif
  
    boolean tx=digitalRead(radio.pttPin);
    digitalWrite(radio.pttPin,HIGH);
    delay(500);
    morseCode(radio.micPin,CALLSIGN);
    #ifdef ENABLE_DEBUG_NEEDSID
    Serial.print(radio.nametag);
    Serial.println(" txAutoId setting needsId=false");
    #endif
    radio.needsId=false;
    radio.lastIdTime=millis();
    digitalWrite(radio.pttPin,tx);
  }
}

//broadcast low battery if applicable
void lowBattCheck(Radio &radio)
{
  float voltage=getPowerVoltage(voltSensePin);
  if(isEnabled(radio.txAllowed) && isEnabled(radio.battMon) && voltage < lowBattThreshold && voltage > lowNotifyFloor && (millis()-radio.lastBattMonTime) > idTimeout)
  {
    #ifdef ENABLE_DEBUG
    Serial.print("Sending low-battery on ");
    Serial.println(radio.nametag);
    #endif
  
    boolean tx=digitalRead(radio.pttPin);
    digitalWrite(radio.pttPin,HIGH);
    radio.needsId=true;
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
    radio.needsId=true;
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


#ifdef ENABLE_DEBUG
void printSetupDebug()
{
  Serial.println("------ setup() complete ------");
  printProgramConfig();
  Serial.println("");
  printRadioState(radioA);
  Serial.println("");
  printRadioState(radioB);
  Serial.println("------------------------------");
  debugRadioStateLastTime=millis();
}

void printAllRadioState()
{
  Serial.println("------ radio state ------");
  Serial.print("millis=");
  Serial.println(millis());
  Serial.println("");
  printRadioState(radioA);
  Serial.println("");
  printRadioState(radioB);
  Serial.println("-------------------------");
}

void printProgramConfig()
{
  Serial.print("CALLSIGN=");
  Serial.println(CALLSIGN);
  Serial.print("COS_VALUE_SQL_OPEN=");
  Serial.println(COS_VALUE_SQL_OPEN);
  Serial.print("voltSensePin=");
  Serial.println(voltSensePin);
  Serial.print("lowBattThreshold=");
  Serial.println(lowBattThreshold);
  Serial.print("lowNotifyFloor=");
  Serial.println(lowNotifyFloor);
  Serial.print("idTimeout=");
  Serial.print(idTimeout);
  float idTimeoutSec=((float)idTimeout/1000);
  Serial.print(" (");
  Serial.print(idTimeoutSec);
  Serial.println(" seconds)");
  Serial.print("cosDelay=");
  Serial.print("cosDelay");
  Serial.println("ms");
  Serial.print("ditLen=");
  Serial.println(ditLen);
  Serial.print("tonePitch");
  Serial.println(tonePitch);
}

void printRadioState(Radio &radio)
{
  //cosPin - Audio In (for COS) - Digital Pin
  //micPin - MIC Mix (for tone) - Digital Pin
  //pttPin - PTT OUT (for TX)   - Digital Pin
  Serial.print("Radio state debug: ");
  Serial.println(radio.nametag);
  Serial.print("memAddr=");
  Serial.println(reinterpret_cast<int>(&radio));
  Serial.print("micPin=");
  Serial.println(radio.micPin);
  Serial.print("pttPin=");
  Serial.println(radio.pttPin);
  Serial.print("cosPin=");
  Serial.println(radio.cosPin);
  Serial.print("autoId=");
  Serial.println(radio.autoId);
  Serial.print("battMon=");
  Serial.println(radio.battMon);
  Serial.print("lastBattMonTime=");
  Serial.println(radio.lastBattMonTime);
  Serial.print("lastIdTime=");
  Serial.println(radio.lastIdTime);
  Serial.print("lastCosTime=");
  Serial.println(radio.lastCosTime);
  Serial.print("needsId=");
  Serial.println(radio.needsId);
  Serial.print("txAllowed=");
  Serial.println(radio.txAllowed);
}
#endif
