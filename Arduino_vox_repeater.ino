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
  char temp[6];
   for(unsigned int x=0; x < message.length(); x++)
   {
     switch(message[x])
     {
       case 'a':
                 strcpy(temp,".-");
                 break;
       case 'b':
                 strcpy(temp,"-...");
                 break;
       case 'c':
                 strcpy(temp,"-.-.");
                 break;
       case 'd':
                 strcpy(temp,"-..");
                 break;
       case 'e':
                 strcpy(temp,".");
                 break;
       case 'f':
                 strcpy(temp,"..-.");
                 break;
       case 'g':
                 strcpy(temp,"--.");
                 break;
       case 'h':
                 strcpy(temp,"....");
                 break;
       case 'i':
                 strcpy(temp,"..");
                 break;
       case 'j':
                 strcpy(temp,".---");
                 break;
       case 'k':
                 strcpy(temp,"-.-");
                 break;
       case 'l':
                 strcpy(temp,".-..");
                 break;
       case 'm':
                 strcpy(temp,"--");
                 break;
       case 'n':
                 strcpy(temp,"-.");
                 break;
       case 'o':
                 strcpy(temp,"---");
                 break;
       case 'p':
                 strcpy(temp,".--.");
                 break;
       case 'q':
                 strcpy(temp,"--.-");
                 break;
       case 'r':
                 strcpy(temp,".-.");
                 break;
       case 's':
                 strcpy(temp,"...");
                 break;
       case 't':
                 strcpy(temp,"-");
                 break;
       case 'u':
                 strcpy(temp,"..-");
                 break;
       case 'v':
                 strcpy(temp,"...-");
                 break;
       case 'w':
                 strcpy(temp,".--");
                 break;
       case 'x':
                 strcpy(temp,"-..-");
                 break;
       case 'y':
                 strcpy(temp,"-.--");
                 break;
       case 'z':
                 strcpy(temp,"--..");
                 break;
       case '0':
                 strcpy(temp,"-----");
                 break;
       case '1':
                 strcpy(temp,".----");
                 break;
       case '2':
                 strcpy(temp,"..---");
                 break;
       case '3':
                 strcpy(temp,"...--");
                 break;
       case '4':
                 strcpy(temp,"....-");
                 break;
       case '5':
                 strcpy(temp,".....");
                 break;
       case '6':
                 strcpy(temp,"-....");
                 break;
       case '7':
                 strcpy(temp,"--...");
                 break;
       case '8':
                 strcpy(temp,"---..");
                 break;
       case '9':
                 strcpy(temp,"----.");
                 break;
       case ' ':
                 strcpy(temp,"");
                 delay(7*ditLen);
                 break;
       case '.':
                 strcpy(temp,".-.-.-");
                 break;
       case '/':
                 strcpy(temp,"-..-.");
                 break;
       case '-':
                 strcpy(temp,"-....-");
                 break;
       case '?':
                 strcpy(temp,"..--..");
                 break;
       default:
                 strcpy(temp,"");
                 break;
     }
     
     for(unsigned int y=0; y < strlen(temp); y++)
     {
       switch(temp[y])
       {
       case '.':
                 tone(codePin,tonePitch);
                 delay(ditLen);
                 noTone(codePin);
                 delay(ditLen);
                 break;
       case '-':
                 tone(codePin,tonePitch);
                 delay(3*ditLen);
                 noTone(codePin);
                 delay(ditLen);
                 break;
       default:
                 break;
       }
     }
     delay(ditLen);
   }
   delay(7*ditLen);
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
