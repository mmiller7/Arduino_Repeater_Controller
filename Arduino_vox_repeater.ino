//Matthew Miller
//12-March-2013

//Analog pin for voltage sense
#define voltSensePin 2

//define threshold for low battery
#define lowBattThreshold 11.5

//how many milliseconds to ID every
//600000 is 10 minutes in milliseconds
#define idTimeout 600000

//define input value for no-audio
#define normVal 511

//define deviation to activate VOX
#define devVal 4

//morse code "dit" base unit length in milliseconds
#define ditLen 60

//the pitch for the tone
#define tonePitch 800

//data structure for radio info
struct Radio
{
  //voxPin - Audio In (for VOX) - Analog Pin
  //micPin - MIC Mix (for tone) - Digital Pin
  //pttPin - PTT OUT (for TX)   - Digital Pin
  int micPin, pttPin, voxPin;
  boolean autoId, battMon;
  long lastBattMonTime, lastIdTime, lastVoxTime;
};

Radio radioA, radioB;

void configure(Radio &radio);
void vox(Radio &radio);
void txAutoId(Radio &radio);
void lowBattCheck(Radio &radio);

void setup() {
  
  radioA.voxPin=0;
  radioA.micPin=2;
  radioA.pttPin=3;
  radioA.autoId=true;
  radioA.battMon=true;
  radioA.lastBattMonTime=idTimeout;
  radioA.lastIdTime=idTimeout;
  radioA.lastVoxTime=0;
  
  radioB.voxPin=1;
  radioB.micPin=12;
  radioB.pttPin=13;
  radioB.autoId=false;
  radioB.battMon=false;
  radioB.lastBattMonTime=idTimeout;
  radioB.lastIdTime=idTimeout;
  radioB.lastVoxTime=0;
  
  configure(radioA);
  configure(radioB);
  
  txAutoId(radioA);
  txAutoId(radioB);
}

void configure(Radio &radio)
{
  pinMode(radio.micPin,OUTPUT);
  pinMode(radio.pttPin,OUTPUT);
  digitalWrite(radio.micPin,LOW);
  digitalWrite(radio.pttPin,LOW);
}

void loop()
{
  if(!isBusy(radioB.pttPin)) //if the other radio is transmitting, this one must be receiving so don't key up
  {
    lowBattCheck(radioA);
    txAutoId(radioA);
    vox(radioA);
  }
    
  if(!isBusy(radioA.pttPin)) //if the other radio is transmitting, this one must be receiving so don't key up 
  {
    lowBattCheck(radioB);
    txAutoId(radioB);
    vox(radioB);
  }
}

boolean isBusy(int pttPin)
{
  return digitalRead(pttPin);
}


void vox(Radio &radio)
{
  // read the input on analog pins
  int voxVal = analogRead(radio.voxPin);
  
  // test if the pin has audio
  if(voxVal>(normVal+devVal) || voxVal<(normVal-devVal))
  {
    //vox active
    digitalWrite(radio.pttPin,HIGH);
    radio.lastVoxTime=millis();
  }
  else
  {
    if(millis()-radio.lastVoxTime < 500)
    {
      //vox delay
    }
    else
    {
      digitalWrite(radio.pttPin,LOW);
    }
  }
}


void txAutoId(Radio &radio)
{
  if(radio.autoId && (millis()-radio.lastIdTime) > idTimeout)
  {
    boolean tx=digitalRead(radio.pttPin);
    digitalWrite(radio.pttPin,HIGH);
    delay(500);
    //kk4nde(micPin);
    morseCode(radio.micPin,"kk4nde");
    radio.lastIdTime=millis();
    digitalWrite(radio.pttPin,tx);
  }
}

void lowBattCheck(Radio &radio)
{
  float voltage=getPowerVoltage(voltSensePin);
  if(radio.battMon && voltage < lowBattThreshold && voltage > 5.5 && (millis()-radio.lastBattMonTime) > idTimeout)
  {
    boolean tx=digitalRead(radio.pttPin);
    digitalWrite(radio.pttPin,HIGH);
    delay(500);
    morseCode(radio.micPin,"lb "+ toString(voltage) + "v");
    //morseCode(radio.micPin,"l batt");
    radio.lastBattMonTime=millis();
    digitalWrite(radio.pttPin,tx);
  }
}

String toString(float input)
{
  String result="";
  if(input > 10)
  {
    result+='1';
    input-=10;
  }
  result+=(char)((int)input+48); //add 48 to shift to ascii value
  input-=(int)input;
  result+='.';
  input*=10;
  if((input-(int)input)>.4) //round (because here we will drop everything after decimal)
    input++;
  result+=(char)((int)input+48); //add 48 to shift to ascii value
  return result;
}

void morseCode(int codePin, String message)
{
  // message.trim();
  message.toLowerCase();
  int code;
  int length;
   for(unsigned int x=0; x < message.length(); x++)
   {
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
                 code=B10001;
                 length=6;
                 break;
       case '?':
                 //strcpy(temp,"..--..");
                 code=B001100;
                 length=6;
                 break;
       default:
                 //strcpy(temp,"");
                 code=B0;
                 length=0;
                 break;
     }
     
     while(length != 0)
     {
       if(code & bitMask(length))
       {
                 tone(codePin,tonePitch);
                 delay(3*ditLen);
                 noTone(codePin);
                 delay(ditLen);
       }
       else
       {
                 tone(codePin,tonePitch);
                 delay(ditLen);
                 noTone(codePin);
                 delay(ditLen);
                 break;
       }
       length--;
     }
     delay(ditLen);
   }
   delay(7*ditLen);
}

int bitMask(int bitNumber)
{
  int value=1;
  for(int x=1; x < bitNumber; x++)
    value*=2;
  return value;
}

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
