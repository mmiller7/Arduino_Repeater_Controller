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
  if(radio.battMon && voltage < lowBattThreshold && (millis()-radio.lastBattMonTime) > idTimeout)
  {
    boolean tx=digitalRead(radio.pttPin);
    digitalWrite(radio.pttPin,HIGH);
    delay(500);
    //morseCode(radio.micPin,"lb "+ toString(voltage) + " v  ");
    morseCode(radio.micPin,"l batt");
    radio.lastBattMonTime=millis();
    digitalWrite(radio.pttPin,tx);
  }
}

String toString(float input)
{
  int first=(int)input;
  int last=(int)((input-first)*100);
  return String(first)+"."+String(last);
}

void morseCode(int codePin, String message)
{
  // message.trim();
  message.toLowerCase();
  String temp="";
   for(int x=0; x < message.length(); x++)
   {
     char c = message[x];
     switch(message[x])
     {
       case 'a':
                 temp=".-";
                 break;
       case 'b':
                 temp="-...";
                 break;
       case 'c':
                 temp="-.-.";
                 break;
       case 'd':
                 temp="-..";
                 break;
       case 'e':
                 temp=".";
                 break;
       case 'f':
                 temp="..-.";
                 break;
       case 'g':
                 temp="--.";
                 break;
       case 'h':
                 temp="....";
                 break;
       case 'i':
                 temp="..";
                 break;
       case 'j':
                 temp=".---";
                 break;
       case 'k':
                 temp="-.-";
                 break;
       case 'l':
                 temp=".-..";
                 break;
       case 'm':
                 temp="--";
                 break;
       case 'n':
                 temp="-.";
                 break;
       case 'o':
                 temp="---";
                 break;
       case 'p':
                 temp=".--.";
                 break;
       case 'q':
                 temp="--.-";
                 break;
       case 'r':
                 temp=".-.";
                 break;
       case 's':
                 temp="...";
                 break;
       case 't':
                 temp="-";
                 break;
       case 'u':
                 temp="..-";
                 break;
       case 'v':
                 temp="...-";
                 break;
       case 'w':
                 temp=".--";
                 break;
       case 'x':
                 temp="-..-";
                 break;
       case 'y':
                 temp="-.--";
                 break;
       case 'z':
                 temp="--..";
                 break;
       case '0':
                 temp="-----";
                 break;
       case '1':
                 temp=".----";
                 break;
       case '2':
                 temp="..---";
                 break;
       case '3':
                 temp="...--";
                 break;
       case '4':
                 temp="....-";
                 break;
       case '5':
                 temp=".....";
                 break;
       case '6':
                 temp="-....";
                 break;
       case '7':
                 temp="--...";
                 break;
       case '8':
                 temp="---..";
                 break;
       case '9':
                 temp="----.";
                 break;
       case ' ':
                 temp="";
                 delay(7*ditLen);
                 break;
       case '.':
                 temp=".-.-.-";
                 break;
       case '/':
                 temp="-..-.";
                 break;
       case '-':
                 temp="-....-";
                 break;
       case '?':
                 temp="..--..";
                 break;
       default:
                 temp="";
                 break;
     }
     
     for(int y=0; y < temp.length(); y++)
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
