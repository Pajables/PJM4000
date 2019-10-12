//X motor // Pump 1
const int asteppin =54;
const int adirpin =55;
const int aenpin = 38;
const int arefpin = 3; //xmin pin ramps

// Y motor // Pump 2
const int bsteppin = 60;
const int bdirpin = 61;
const int benpin = 56;
const int brefpin = 2; //xmax pin ramps

//Z motor // Valve 1
const int avalvstep = 46;
const int avalvdir = 48;
const int avalven = 62;
const int avalvref = 14; //ymin pin ramps

//E0 motor // Valve 2
const int bvalvstep = 26;
const int bvalvdir = 28;
const int bvalven = 24;
const int bvalvref = 15; //ymax pin ramps

//E1 motor //pump 3
const int csteppin = 36;
const int cdirpin = 34;
const int cenpin = 30;
const int crefpin = 18; //zmin pin ramps

// Flow rate formula stuff needed
float P =8; // lead of threaded rod in mm (thread has 4 starts and pitch of 2mm) initialised as float for division 
float SPR = 3200;// steps per revolution // microsteps in this case 
int x[6]={1,2,42,58,20,50};  //!!!!the lengths for 1,2,20 and 50 have not been measured!!!!!! //length of the syringe that accounts for its full volume// eg the length of the 5ml part of the syringe

//speeds (delay in microsecs
long SPDV=500; //time delay for pulses. shorter delay gives faster speed.
long SPDPa=0,SPDPb=0;
long SPDrefill = 200;
int SPDfactor = 0.5; //factor to adjust for delays in code poll rate. Found from time to pump 1ml at 500 microL/min

// Define the variables
long delaya = 240384;
long delayb = 240384;
long delayc = 240384;
int countp1=0, countp2=0, countp3=0;
int countv1=0, countv2=0;
int Aon=0, Bon=0, Con=0; //motor on/off

long reactiontime=0;

int vola=0 , volb = 0, volc = 0;
char inputvola[3];
char inputvolb[3];
char inputvolc[3];
char inputa, inputb, inputc;
int aa, ab, ac, ia;
int ba, bb, bc, ib;
int ca, cb, cc, ic;

float voladdeda=0, voladdedb=0, voladdedc=0;
int flowa=0, flowb=0, flowc=0;
int syra=3, syrb=3, syrc=3;
float xa=0, xb = 0, xc=0; //length of syringes
float syravol=0,syrbvol=0, syrcvol=0;

float corr = 0.993;// correction factor for the volume dispensed
long factor = 375000000;/// 
char getkey;

int syringe[6]={1,2,5,10,20,50}; //syringe volumes
float syringefactor[6]={58.310, 16.616, 8.459, 94.69, 3.200, 1.548}; // this should also account for the volume stored by the one way valve
float syringefactora, syringefactorb, syringefactorc;

//Setting up the display and Keypad
#include <Keyboard.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
LiquidCrystal_I2C lcd(0x27,20,4);// i^2C address is 0x27 and we using a 20x4 character display

#include <Keypad.h>
const byte ROWS=4;
const byte COLS=4;
char keys[ROWS][COLS]={
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'},
};
byte rowPins[ROWS]={33,31,29,27};
byte colPins[COLS]={25,23,17,16};
Keypad keypad=Keypad(makeKeymap(keys),rowPins,colPins,ROWS,COLS);

// Setting up multiflow valve. Each valve connected to a pump has four ports. 1=system, 2=reservoir for refilling, 3=waste and 4=?
int vacount=0;
int vbcount=0;
int pacount=0;
int pbcount=0;

int motorsteps=200;
long refilla=0;
long refillb=0;
long refillcounta=0;
long refillcountb=0;

//// function ReadKeyD// for inputting flow rate and volume required for reaction
int ReadKeyD(int column, int row, int x){
  char keypressed = ' ';
  int ZReturn=0;
  int i=0;
  String SInput=" ";
  char StopChar='-';
  char EnterChar="-";
  lcd.setCursor(column,row);
  lcd.print( '_' );
  column++;
  i++;
  do {
    keypressed= keypad.getKey();
    
    if (isDigit(keypressed)==true){
      if(i<x){
        SInput+=keypressed; // same as SInput=SInput+keypressed;
        lcd.setCursor(column,row);
        lcd.print(keypressed);
        column++;
        i++;
        if (i<x){
          lcd.print("_");
        }
      }
    }
   }while ((keypressed!='#'));

   if (column<20){
    lcd.setCursor(column,row);
    lcd.print(" ");
   }

  EnterChar= '-';
  StopChar= '-';
  return SInput.toInt();
 
}
//// end of ReadKeyD function 

//Main body
void setup() 
{
Serial.begin(9600);

//define the motors
pinMode(asteppin,OUTPUT);
pinMode(adirpin,OUTPUT);
pinMode(aenpin,OUTPUT);
pinMode(arefpin,INPUT);
digitalWrite(aenpin,HIGH);// motor off

pinMode(bsteppin,OUTPUT);
pinMode(bdirpin,OUTPUT);
pinMode(benpin,OUTPUT);
pinMode(brefpin,INPUT);
digitalWrite(benpin,HIGH);//motor off


// define valve motors
pinMode(avalven,OUTPUT);
pinMode(avalvdir,OUTPUT);
pinMode(avalvstep,OUTPUT);
digitalWrite(avalven,HIGH);

pinMode(bvalven,OUTPUT);
pinMode(bvalvdir,OUTPUT);
pinMode(bvalvstep,OUTPUT);
digitalWrite(bvalven,HIGH);

//set up the lcd display
lcd.init();
lcd.backlight();
lcd.clear();
lcd.setCursor(0,0);lcd.print("A-B:Start/Stop");
lcd.setCursor(0,1);lcd.print("1-2: Change/Flow");
lcd.setCursor(0,2);lcd.print("5-6: Reset Volume");
lcd.setCursor(0,3);lcd.print("0: Refresh Display");
}

void loop() {
getkey= keypad.getKey();

// if 0 is pressed then the lcd display refreshes
if(getkey=='0')
{
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("A ");lcd.print( vola);lcd.print("mL  ");lcd.print(flowa);lcd.print(" \344L/min ");
  lcd.setCursor(0,1);
  lcd.print("B ");lcd.print( volb);lcd.print("mL  ");lcd.print( flowb );lcd.print(" \344L/min ");
}
// if key 5 is pressed then reset volume of A
if (getkey=='5')
{
  vola=0;
  lcd.clear();
  lcd.setCursor(0,0); 
  lcd.print("Syringe A ");
  lcd.setCursor(0,1);lcd.print("Volume Reset");
}

//if key 6 is pressed then reset volume of B
if (getkey=='6')
{
  volb=0;
  lcd.clear();
  lcd.setCursor(0,0); 
  lcd.print("Syringe B ");
  lcd.setCursor(0,1);lcd.print("Volume Reset");
}

//AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
////if key A is pressed
if (getkey=='A')
{
  if(Aon==1)
  {
    Aon=0;
    digitalWrite(aenpin,HIGH);
  }
  else
  {
    Aon = 1;
    digitalWrite(aenpin,LOW); 
  }
}
if (digitalRead(arefpin) == 0) //pump should reverse after hitting endstop
{
  digitalWrite(aenpin, HIGH);
  digitalWrite(adirpin, LOW);
  digitalWrite(avalvdir,HIGH);
  digitalWrite(avalven,LOW);
  for(vacount=0; vacount<800 ; vacount++) //index valve to reservoir (2)
  {
    digitalWrite(avalvstep, HIGH);
    delayMicroseconds(SPDV);
    digitalWrite(avalvstep, LOW);
    delayMicroseconds(SPDV);
  }
  digitalWrite(aenpin,LOW);

  while (refilla < refillcounta) //refills syringe at double speed
  {
    digitalWrite(asteppin, HIGH);
    delayMicroseconds(SPDrefill);
    digitalWrite(asteppin, LOW);
    delayMicroseconds(SPDrefill);
    refilla++;
  }
  //reset direction of motors upon refill complete
  digitalWrite(adirpin, HIGH);
  digitalWrite(avalven,LOW);
  digitalWrite(avalvdir,LOW);
  for(vacount=0;vacount<800;vacount++) //index valve back to system (1)
  {
    digitalWrite(avalvstep, HIGH);
    delayMicroseconds(SPDV);
    digitalWrite(avalvstep, LOW);
    delayMicroseconds(SPDV);
   }
  digitalWrite(avalven,HIGH);
  refilla = 0;
  refillcounta =0;
}
if (Aon == 1 && voladdeda < vola)
{
    digitalWrite(adirpin,HIGH);
    digitalWrite(asteppin,HIGH);
    digitalWrite(asteppin,LOW);
    delayMicroseconds(SPDPa);
    refillcounta++;
    countp1++;
    voladdeda =(syravol*((countp1*(P/3200))/xa)); //counts/SPR x pitch gives linear distance. 
                                                  //linear distance/total distance gives prop volume pumped                                            
}
//BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB
  
//key B is pressed
if (getkey=='B')
{
  if(Bon==1)
  {
    Bon=0;
    digitalWrite(benpin,HIGH);
  }
  else
  {
    Bon = 1;
    digitalWrite(benpin,LOW); 
  }
}
if (digitalRead(brefpin) == 0) //pump should reverse after hitting endstop
{
  digitalWrite(benpin, HIGH);
  digitalWrite(bdirpin, LOW);
  digitalWrite(bvalvdir,HIGH);
  digitalWrite(bvalven,LOW);
  for(vbcount=0; vbcount<800 ; vbcount++) //index valve to reservoir (2)
  {
    digitalWrite(bvalvstep, HIGH);
    delayMicroseconds(SPDV);
    digitalWrite(bvalvstep, LOW);
    delayMicroseconds(SPDV);
  }
  digitalWrite(benpin,LOW);

  while (refillb < refillcountb) //refills syringe at double speed
  {
    digitalWrite(bsteppin, HIGH);
    delayMicroseconds(SPDrefill);
    digitalWrite(bsteppin, LOW);
    delayMicroseconds(SPDrefill);
    refillb++;
  }
  //reset direction of motors upon refill complete
  digitalWrite(bdirpin, HIGH);
  digitalWrite(bvalven,LOW);
  digitalWrite(bvalvdir,LOW);
  for(vbcount=0;vbcount<800;vbcount++) //index valve back to system (1)
  {
    digitalWrite(bvalvstep, HIGH);
    delayMicroseconds(SPDV);
    digitalWrite(bvalvstep, LOW);
    delayMicroseconds(SPDV);
   }
  digitalWrite(bvalven,HIGH);
  refillb = 0;
  refillcountb =0;
}
if (Bon == 1 && voladdedb < volb)
{
    digitalWrite(bdirpin,HIGH);
    digitalWrite(bsteppin,HIGH);
    digitalWrite(bsteppin,LOW);
    delayMicroseconds(SPDPb);
    refillcountb++;
    countp2++;
    voladdedb =(syrbvol*((countp2*(P/3200))/xb)); //counts/SPR x pitch gives linear distance. 
                                                  //linear distance/total distance gives prop volume pumped                                            
}

/// if key 1 is pressed 
if (getkey=='1')
{
  lcd.clear();
  do {
          voladdeda=0;
          countp1=0;
           do {
              //Menu 1: syringe volume and pump rate
              getkey=keypad.getKey();
              lcd.setCursor(0,0);
              lcd.print("Pump A");
              lcd.setCursor(9,0);lcd.print("VolA:");lcd.print(vola); lcd.print("ml");
              lcd.setCursor(0,1);
              lcd.print("Syringe: "); lcd.print(syringe[syra]);lcd.print(" mL ");
              lcd.setCursor(0,2);
              lcd.print("Flow: "); lcd.print(flowa); lcd.setCursor(11,2); lcd.print("\344L/min ");
              lcd.setCursor(0,3);
              lcd.print("Next: # Save: *");
          
              if(getkey=='1'){
                syra++;
              }
              if(getkey=='7'){
                syra--;
              }
              if (syra>5){
                syra=5;
              }
              if(syra<0){
                syra=0;
              }
              syravol=syringe[syra];
              xa=x[syra];
           }while(getkey!='#' && getkey!='*');

          delay(10);
            //menu 1: input flow rate (waits for #)
            if (getkey == '#')
            {
              flowa=ReadKeyD(6,2,5);
              lcd.setCursor(7,2);lcd.print(flowa);lcd.print("   \344L/min ");
              delay(10);
            
            
            //menu 2: volume to pump (waits for #)
            lcd.clear();
            lcd.setCursor(0,0); lcd.print("Input");
            lcd.setCursor(11,0); lcd.print("VolA");
          
            lcd.setCursor(0,1); lcd.print("VolA: "); lcd.print(vola); lcd.setCursor(12,1); lcd.print("ml");
            lcd.setCursor(0,2); lcd.print("New Value: ");
                
            vola=ReadKeyD(11,2,4);
            delay(10);
            lcd.clear();
            
            }
            //speed is a function of the pulses per second. Smaller delay will result in faster motor movement.
            
            SPDPa=((6*(10000000000)*P*syravol)/(SPR*xa*flowa));

            Serial.print(SPDPa);Serial.print("\n");
            Serial.print(xa);
  }while (getkey!='*');

   lcd.clear();
   lcd.print("A: ");lcd.print(vola);lcd.print("mL ");lcd.print(flowa); lcd.print("\344L/min ");
   lcd.setCursor(0,1);
   lcd.print("Syringe A: "); lcd.print(syravol); lcd.print("ml");
   lcd.setCursor(0,2);
   lcd.print("B: ");lcd.print(volb);lcd.print("mL ");lcd.print(flowb); lcd.print("\344L/min ");
   lcd.setCursor(0,3);
   lcd.print("Syringe B: "); lcd.print (syrbvol); lcd.print("ml");
}
  
// if key 2 is pressed
if (getkey=='2')
{
  lcd.clear();     
  do{ 
    
    do{
      //menu 1:syringe volume and pump rate
      getkey= keypad.getKey();
      lcd.setCursor(0,0);
      lcd.print("Pump B");
      lcd.setCursor(9,0);lcd.print("VolB:");lcd.print(volb); lcd.print("ml");
      lcd.setCursor(0,1);
      lcd.print("Syringe: "); lcd.print(syringe[syrb]);lcd.print(" mL ");
      lcd.setCursor(0,2);
      lcd.print("Flow: "); lcd.print(flowb); lcd.setCursor(11,2); lcd.print("\344L/min ");
      lcd.setCursor(0,3);
      lcd.print("Next: # Save: *");
  
      if (getkey=='1'){
        syrb++;
      }
      if (getkey=='7'){
        syrb--;
      }
      if (syrb>5){
        syrb=5;
      }
      if (syrb<0){
        syrb=0;
      }
      syrbvol=syringe[syrb];
      xb=x[syrb];
  }while(getkey!='#' && getkey!='*');

    delay(10);
    //menu 1:input flow rate (waits for #)
    if (getkey == '#')
    {
      flowb=ReadKeyD(6,2,5);
      lcd.setCursor(7,2);lcd.print(flowb);lcd.print("   \344L/min ");
      delay(10);

      //menu 2: volume to pump (waits for #)
      lcd.clear();
      lcd.setCursor(0,0); lcd.print("Input");
      lcd.setCursor(11,0); lcd.print("VolB");
      lcd.setCursor(0,1); lcd.print("VolB: "); lcd.print(volb); lcd.setCursor(12,1); lcd.print("ml");
      lcd.setCursor(0,2); lcd.print("New Value: ");
    
      volb=ReadKeyD(11,2,4);
      delay(10);
      lcd.clear();
    }
      SPDPb=((6*(10000000000)*P*syrbvol)/(SPR*xb*flowb));
      delay(10);
              
  }while(getkey!='*');

  lcd.clear();
  lcd.print("A ");lcd.print(vola);lcd.print("mL ");lcd.print(flowa);lcd.print ("\344L/min");
  lcd.setCursor(0,1);
  lcd.print("Syringe A: "); lcd.print(syravol); lcd.print("ml");
  lcd.setCursor(0,2);
  lcd.print("B ");lcd.print(volb);lcd.print("mL ");lcd.print(flowb);lcd.print ("\344L/min");
  lcd.setCursor(0,3);
  lcd.print("Syringe B: "); lcd.print (syrbvol);lcd.print("ml");
}

//instructions page
if (getkey=='D')
{
  lcd.clear();
lcd.setCursor(0,0);lcd.print("A-B:Start/Stop");
lcd.setCursor(0,1);lcd.print("1-2: Change/Flow");
lcd.setCursor(0,2);lcd.print("5-6: Reset Volume");
lcd.setCursor(0,3);lcd.print("0: Refresh Display");
}
}// the void loop ends here
