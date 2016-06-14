//Matthew Miller
//12-March-2013


//Radio A:
//Audio In (for VOX) - Analog Pin
#define radioA_voxPin 0
//MIC Mix (for tone) - Digital Pin
#define radioA_micPin 2
//PTT OUT (for TX)   - Digital Pin
#define radioA_pttPin 3

//Radio B:
//Audio In (for VOX) - Analog Pin
#define radioB_voxPin 1
//MIC Mix (for tone) - Digital Pin
#define radioB_micPin 12
//PTT OUT (for TX)   - Digital Pin
#define radioB_pttPin 13

//define if auto ID is enabled
#define autoIdA true
#define autoIdB true

//how many milliseconds to ID every
//600000 is 10 minutes in milliseconds
#define idTimeout 600000

//define input value for no-audio
#define normVal 511

//define deviation to activate VOX
#define devVal 4

//morse code "dit" base unit length in milliseconds
#define ditLen 60

//this stores the last time it ID'd
long lastIdA=idTimeout;
long lastIdB=idTimeout;

//stores the time of the last transmission for VOX delay
long lastVoxTime=0;

void setup() {
  pinMode(radioA_micPin,OUTPUT);
  pinMode(radioA_pttPin,OUTPUT);
  pinMode(radioB_micPin,OUTPUT);
  pinMode(radioB_pttPin,OUTPUT);
  digitalWrite(radioA_micPin,LOW);
  digitalWrite(radioA_pttPin,LOW);
  digitalWrite(radioB_micPin,LOW);
  digitalWrite(radioB_pttPin,LOW);

  autoId(radioA_micPin,radioA_pttPin,lastIdA);
  autoId(radioB_micPin,radioB_pttPin,lastIdB);
}

void loop()
{  
  if(!isBusy(radioB_pttPin)) //if the other radio is transmitting, this one must be receiving so don't key up
  {
    if(autoIdA)
      autoId(radioA_micPin,radioA_pttPin,lastIdA);
    vox(radioA_voxPin, radioA_pttPin, lastVoxTime);
  }
    
  if(!isBusy(radioA_pttPin)) //if the other radio is transmitting, this one must be receiving so don't key up 
  {
    if(autoIdB)
      autoId(radioB_micPin,radioB_pttPin,lastIdB);
    vox(radioB_voxPin, radioB_pttPin, lastVoxTime);
  }
}

boolean isBusy(int pttPin)
{
  return digitalRead(pttPin);
}


void vox(int voxPin,int pttPin,long & lastVoxTime)
{
  // read the input on analog pins
  int voxVal = analogRead(voxPin);
  
  // test if the pin has audio
  if(voxVal>(normVal+devVal) || voxVal<(normVal-devVal))
  {
    //vox active
    digitalWrite(pttPin,HIGH);
    lastVoxTime=millis();
  }
  else
  {
    if(millis()-lastVoxTime < 500)
    {
      //vox delay
    }
    else
    {
      digitalWrite(pttPin,LOW);
    }
  }
}


void autoId(int micPin,int pttPin, long & lastIdTime)
{
  if((millis()-lastIdTime) > idTimeout)
  {
    boolean tx=digitalRead(pttPin);
    digitalWrite(pttPin,HIGH);
    delay(500);
    //kk4nde(micPin);
    morseCode(micPin,"kk4nde");
    lastIdTime=millis();
    digitalWrite(pttPin,tx);
  }
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
                 delay(3*ditLen);
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
                 tone(codePin,600);
                 delay(ditLen);
                 noTone(codePin);
                 delay(ditLen);
                 break;
       case '-':
                 tone(codePin,600);
                 delay(3*ditLen);
                 noTone(codePin);
                 delay(ditLen);
                 break;
       default:
                 break;
       }
     }

   }
}
