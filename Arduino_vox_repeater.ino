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

//define if PTT TxX is enabled
#define enableTxA true
#define enableTxB true

//define if EOT ID is enabled
//#define eotIdA false
//#define eotIdB false

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
#define ditLen 75

//this stores the last time it ID'd
long lastIdA=idTimeout;
long lastIdB=idTimeout;

//stores the time of the last transmission for VOX delay
long last=0;

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
  if(enableTxA && autoIdA)
    autoId(radioA_micPin,radioA_pttPin,lastIdA);
  if(enableTxA && autoIdB)
    autoId(radioB_micPin,radioB_pttPin,lastIdB);
  
  if(enableTxA && !isBusy(radioB_pttPin))
    vox(radioA_voxPin, radioA_pttPin, last);
    
  if(enableTxB && !isBusy(radioA_pttPin))
    vox(radioB_voxPin, radioB_pttPin, last);
}

boolean isBusy(int pttPin)
{
  return digitalRead(pttPin);
}

void vox(int voxPin,int pttPin,long & last)
{
  // read the input on analog pins
  int voxVal = analogRead(voxPin);
  
  // test if the pin has audio
  if(voxVal>(normVal+devVal) || voxVal<(normVal-devVal))
  {
    //vox active
    digitalWrite(pttPin,HIGH);
    last=millis();
  }
  else
  {
    if(millis()-last < 500)
    {
      //vox delay
    }
    else
    {
      digitalWrite(pttPin,LOW);
      Serial.println();
    }
  }
}


void autoId(int micPin,int pttPin, long & time)
{
  if((millis()-time) > idTimeout)
  {
    boolean tx=digitalRead(pttPin);
    digitalWrite(pttPin,HIGH);
    delay(500);
    kk4nde(micPin);
    time=millis();
    digitalWrite(pttPin,tx);
  }
}


void kk4nde(int codePin) {
  //-.- -.- ....- -. -.. . 
  dah(codePin); dit(codePin); dah(codePin); next();
  dah(codePin); dit(codePin); dah(codePin); next();
  dit(codePin); dit(codePin); dit(codePin); dit(codePin); dah(codePin); next();
  dah(codePin); dit(codePin); next();
  dah(codePin); dit(codePin); dit(codePin); next();
  dit(codePin); next();
}

void dit(int codePin)
{
  tone(codePin,600);
  delay(ditLen);
  noTone(codePin);
  delay(ditLen);
}

void dah(int codePin)
{
  tone(codePin,600);
  delay(3*ditLen);
  noTone(codePin);
  delay(ditLen);
}

void next()
{
  delay(3*ditLen);
}
