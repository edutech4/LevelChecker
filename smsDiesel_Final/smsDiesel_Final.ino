//////RECEIVED MESSAGE FORMAT//////////////
//+CMT: "+2348039471950","","20/09/15,12:37:19+04"
//#D.level*

/////EEPROM MEMORY SPACE 0 IS DEDICATED FOR TANK LEVEL THRESHOLD.
/////EEPROM MEMORY SPACE 1 IS DEDICATED FOR STORING FLAG DETECTING WHEN A MSG HAS BEING SENT TO ADMIN.
/////EEPROM MEMORY SPACE 100 TO 105 IS DEDICATED FOR STORING LITTER VALUE.
/////EEPROM MEMORY SPACE 200 TO 215 IS DEDICATED FOR STORING TANK HEIGHT.
/////EEPROM MEMORY SPACE 120 TO 165 IS DEDICATED FOR STORING 3 ADMIN PHONE NUMBERS.
/////EEPROM MEMORY SPACE 120 TO 134 IS DEDICATED FOR STORING ADMIN 1 NUMBER.
/////EEPROM MEMORY SPACE 135 TO 149 IS DEDICATED FOR STORING ADMIN 2 NUMBER.
/////EEPROM MEMORY SPACE 150 TO 164 IS DEDICATED FOR STORING ADMIN 3 NUMBER.
#include <EEPROM.h>
#include <SoftwareSerial.h>
#include <NewPing.h>
#include <MedianFilter.h>
#include <Wire.h>
//#include<LiquidCrystal.h>
//LiquidCrystal lcd(6,7,8,9,10,11);
SoftwareSerial mySerial(8, 9); // RX, TX
String col = "";
int addr = 0;// Threshold level
int tank_Height=0;
int tank_H_index=254;
int error_flag=0;
int temp=0,temp2=0,i=4;
int led=13,levelThreshold=0; 
int cnt=0,nt,sent_flag=0,send_flag_addr=1;
float Depth_Liters,real_depth,Percent;
char str[100];//initially 65
char phoneNumber[15];/// initially 15
char msg[42];
//char str2[15];
int level=0;
unsigned long data_percent=0;
unsigned long data_litter=0;
unsigned long Tank_Litters=0;
String lev_threshold;
float o,uS;
int arr[20];

#define TRIGGER_PIN  12  // Arduino pin tied to trigger pin on the ultrasonic sensor.
#define ECHO_PIN     11  // Arduino pin tied to echo pin on the ultrasonic sensor.
#define MAX_DISTANCE 500 // Maximum distance we want to ping for (in centimeters). Maximum sensor distance is rated at 400-500cm.

NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE); // NewPing setup of pins and maximum distance.
MedianFilter filter(31,0);
void setup()
{
  sent_flag = EEPROM.read(send_flag_addr); 
//  tank_Height = EEPROM.read(254);
  tank_Height = EEPROMReadlong(200);
  delay(2000);
  Serial.begin(9600);
  mySerial.begin(9600);
  delay(2000);
  delay(2000);
  Serial.print("ATE0");  
  delay(200);
  Serial.write(byte(13));
  delay(500);
  Serial.print("AT+CNMI=1,2,0,0,0");
  delay(500);
  Serial.write(byte(13));
  delay(500);
 Serial.print("AT+CMGF=1");  
  delay(500);
  Serial.write(byte(13));
  delay(1000);    
//  mySerial.print("STARTING SYSTEM");
  str[0]='+';
  str[1]='2';
  str[2]='3';
  str[3]='4';
  i=4;
}

void loop()
{
  if(temp==1)
  {
    check();
    temp=0;
    temp2=0;
    i=4;  
    delay(1000);
  }
  cnt=0;
tabulate_depth();
//  real_depth = sonar.ping_cm();
  while(real_depth<=0){
    real_depth = sonar.ping_cm();
    cnt++;
    if(cnt>5){
      break;
    }
    delay(50);
  }
  real_depth = tank_Height-(real_depth);// Tank height is 98cm
//  Depth_Liters = real_depth*10.2;//(1000*d)/98cm
//  //Depth_Liters = Depth_Liters/98;
//  Percent = Depth_Liters*100;
//  Percent = Percent/1000;

  data_litter = real_depth*Tank_Litters;/// using 1000 as current tank Height.
  Depth_Liters = data_litter/tank_Height;
  data_percent = Depth_Liters*100;
  Percent = data_percent/1000;
  
  levelThreshold = EEPROM.read(0);
  if(Percent<= levelThreshold && sent_flag ==0){
  send_init();
  Serial.print("AT+CMGS=\"");
  for(int j=0;j<14;j++){
    Serial.print(EEPROM.read(j+2));
    delay(20);
  }   
  Serial.print("\"");
  delay(200);
  Serial.write(byte(13));
  delay(500);  
  Serial.print("PLEASE REFILL DIESEL.");
   Serial.write(byte(26)); 
  delay(500); 
  EEPROM.write(send_flag_addr, 1);
  sent_flag=1;
  Serial.print("AT+CMGD=1,4");
  delay(500);
  Serial.write(byte(13));
  delay(500);
  }
}

 void serialEvent() 
 {
  while(Serial.available() && temp==0) 
  {
    if(Serial.find("+234"))
    {
      i=4;
      digitalWrite(led, HIGH);
      delay(200);
      digitalWrite(led, LOW);
      delay(200);
//      Serial.print("Just Entered");
      while (Serial.available()) 
      {
          char inChar=Serial.read();  
          
          if(temp==0){
            str[i++]=inChar;
          }
          
            if(inChar=='\"')
          {                       
            temp=2;
          }  
             if(inChar=='#')
          {                       
            temp=0;
          }  
          if(inChar=='*')
          {
            temp=1;            
//            Serial.flush();        
            return;
          }             
      } 
    }

   }
 }

 /////////////////////////EEPROM READ AND WRITE///////////////////////////////

//EEPROM_LONG//
void EEPROMWritelong(int address, unsigned long value)
{
  //Decomposition from a long to 4 bytes by using bitshift.
  //One = Most significant -> Four = Least significant byte
  byte four = (value & 0xFF);
  byte three = ((value >> 8) & 0xFF);
  byte two = ((value >> 16) & 0xFF);
  byte one = ((value >> 24) & 0xFF);
  
  //Write the 4 bytes into the eeprom memory.
  EEPROM.write(address, four);
  EEPROM.write(address + 1, three);
  EEPROM.write(address + 2, two);
  EEPROM.write(address + 3, one);
//  EEPROM.commit();
}

//This function will return a 4 byte (32bit) long from the eeprom
//at the specified address to adress + 3.
unsigned long EEPROMReadlong(long address)
{
  //Read the 4 bytes from the eeprom memory.
  long four = EEPROM.read(address);
  long three = EEPROM.read(address + 1);
  long two = EEPROM.read(address + 2);
  long one = EEPROM.read(address + 3);
  
  //Return the recomposed long by using bitshift.
  return ((four << 0) & 0xFF) + ((three << 8) & 0xFFFF) + ((two << 16) & 0xFFFFFF) + ((one << 24) & 0xFFFFFFFF);
}

 
void send_init(){
//  digitalWrite(13, HIGH);
//  delay(200);
//  digitalWrite(13, LOW);
//  delay(200);
  Serial.print("AT+CNMI=1,2,0,0,0");
  delay(200);
  Serial.write(byte(13));
  delay(500);
  Serial.print("AT+CMGF=1");  
  delay(200);
  Serial.write(byte(13));
  delay(500);  
//  tank_Height = EEPROM.read(254);// Update current set tank height.
  tank_Height = EEPROMReadlong(200);
  Tank_Litters = EEPROMReadlong(100);
  lev_threshold = "";
}
void tabulate_depth(){
 for(int k=0;k<20;k++){
  uS = sonar.ping();
  delay(50);
  filter.in(uS);
  o = filter.out();
  real_depth = o/US_ROUNDTRIP_CM;
  arr[k] = real_depth;
 } 
 nt=0;
 for(int k=0;k<20;k++){
    cnt=0;
  if(nt == 0){
   for(int j=0;j<20;j++){
    if(arr[k] == arr[j]){
      cnt++;
      real_depth=arr[k];
      if(cnt>14){
        nt = 1;
        break;
      }
    }
  } 
  }
 }
}


void check()
{
//   Serial.print("ENTERED LOOP");
  for(int j=0;j<14;j++){
    phoneNumber[j] = str[j];// Filter phone number out
   }
    for(int j=0;j<40;j++){
    msg[j] = str[15+j];  //Filter out message of sms
    if((str[15+j])=='*'){
       break; 
     }
   }

//   Serial.println(" ");
//   for(int j=0;j<60;j++){
//   Serial.print(str[j]); 
//   if((str[j])=='*'){
//    break; 
//   }
//   }
//   Serial.println(" ");
//   for(int j=0;j<14;j++){
//      Serial.print(phoneNumber[j]);
//    } 
//   Serial.println(" "); 
//   for(int j=0;j<sizeof(msg);j++){
//    Serial.print(msg[j]);
//   }
//   Serial.println(" "); 
//   for(int j=8;j<sizeof(msg);j++){//////Filter out Admin Number
//    Serial.print(msg[j]);
//    if(msg[j]=='*'){
//       break; 
//     }
//   }
   
 if(!(strncmp(msg,"D.level*",8)))
    {
  send_init();
  Serial.print("AT+CMGS=\"");
  for(int j=0;j<14;j++){
    Serial.print(phoneNumber[j]);
   }
  Serial.print("\"");
  delay(200);
  Serial.write(byte(13));
  delay(500);  
  Serial.print("DIESEL LEVEL: ");
// for(int j=0;j<14;j++){
//    Serial.print(phoneNumber[j]);
//   }
// for(int j=0;j<9;j++){
//    Serial.print(msg[j]);
//   }

tabulate_depth();
 
//real_depth = sonar.ping_cm();
cnt=0;
while(real_depth ==0 || real_depth<0){
  real_depth = sonar.ping_cm();
  cnt++;
  if(cnt>10){
    break;
  }
  delay(50);
}
//if(!(real_depth>=0 && real_depth<=98)){
//  real_depth = 0;
//}
//Serial.println(real_depth);
real_depth = tank_Height-(real_depth);// Tank height is 98cm

//Depth_Liters = real_depth*10.2;//(1000*d)/98cm
////Depth_Liters = Depth_Liters/98;
//Percent = Depth_Liters*100;
//Percent = Percent/1000;
//Tank_Litters = 
data_litter = real_depth*Tank_Litters;/// using 1000 as current tank Height.
Depth_Liters = data_litter/tank_Height;
data_percent = Depth_Liters*100;
Percent = data_percent/1000;
//Serial.println(Percent);
//  Serial.print(sonar.ping_cm()); // Send ping, get distance in cm and print result (0 = outside set distance range)
  Serial.print(Depth_Liters);
  Serial.print(" Liters");
  Serial.print(",  ");
  Serial.print(Percent);
  Serial.print("%");
  Serial.print(".");
  Serial.write(byte(26)); 
  delay(500); 
  Serial.print("AT+CMGD=1,4");
  delay(500);
  Serial.write(byte(13));
  delay(500);   
    }     
   else if(!(strncmp(msg,"D.set",5)))
    {
//      digitalWrite(13, HIGH);
//      delay(200);
//      digitalWrite(13, LOW);
//      delay(200);
      for(int j=0;j<3;j++){
        //pick level data and save here
        lev_threshold += (msg[6+j]);          
      }
      levelThreshold = lev_threshold.toInt();
      if(levelThreshold>=0 && levelThreshold<=98){
        EEPROM.write(0, levelThreshold);
      }      
    }
       else if(!(strncmp(msg,"D.Tset",6)))///  SET ORIGINAL HEIGHT OF TANK
    {
//      Serial.print("SETTING THE TANK HEIGHT");
      for(int j=0;j<sizeof(msg);j++){
          if(msg[6+j]=='*'){
          break;
        }
        //pick level data and save here
        lev_threshold += (msg[6+j]); 
//      Serial.print(msg[6+j]);         
      }
      levelThreshold = lev_threshold.toInt();
      lev_threshold ="";
      if(levelThreshold>=0 && levelThreshold<=500){// Maximum height of btank is 255.
//        EEPROM.write(254, levelThreshold);// saved in address 254
          EEPROMWritelong(200,levelThreshold);
          delay(100);
      }     

    }

   else if(!(strncmp(msg,"D.Lset",6)))///  SET ORIGINAL HEIGHT OF TANK
    {
//    Serial.print("TANK LITTERS SET Succesful ");
      for(int j=0;j<sizeof(msg);j++){
        //pick level data and save here
        if(msg[6+j]=='*'){
          break;
        }
        lev_threshold += (msg[6+j]); 
//        Serial.print(msg[6+j]);         
      }
      data_litter = lev_threshold.toInt();
      lev_threshold ="";
      if(data_litter>=0 && data_litter<=100000){// Maximum height of Tank Litters is 255.
//          Serial.print("This is a number :");
//       Serial.print(levelThreshold);       
          EEPROMWritelong(100,data_litter);//STORE THE TANK LITTERS
          delay(100);
      }   
 
    }
    
    
//   else if(!(strncmp(msg,"D.Admin*",7)))
//    {
//      Serial.print("Entered Admin Reg2...");
////     for(int j=0;j<sizeof(msg);j++){
////        Serial.print(msg[j]);
////       }
////      if(msg[7]=='1'){
////         Serial.print("FIRST ADMIN DETECTED");
////      }
//      for(int j=2;j<16;j++){
//        EEPROM.write(j, (phoneNumber[j-2]));    
//        delay(20);
//      }
//      
//    }
       else if(!(strncmp(msg,"D.ViewTH*",9)))
    {
//      digitalWrite(13, HIGH);
//      delay(200);
//      digitalWrite(13, LOW); 
//      delay(200);
      send_init();
      Serial.print("AT+CMGS=\"");   
      for(int j=0;j<14;j++){
        Serial.print(phoneNumber[j]);
      }                
      Serial.print("\"");
      delay(200);
      Serial.write(byte(13));
      delay(500);  
      Serial.print("THRESHOLD: ");
      Serial.print(EEPROM.read(0));
      Serial.print("%");
      Serial.write(byte(26)); 
      delay(1000);   
      Serial.print("AT+CMGD=1,4");
      delay(500);
      Serial.write(byte(13));
      delay(500);  
      
    }
  else if(!(strncmp(msg,"D.ViewTK*",9)))
    {
//      digitalWrite(13, HIGH);
//      delay(200);
//      digitalWrite(13, LOW); 
//      delay(200);
      send_init();
      Serial.print("AT+CMGS=\"");   
      for(int j=0;j<14;j++){
        Serial.print(phoneNumber[j]);
      }                
      Serial.print("\"");
      delay(200);
      Serial.write(byte(13));
      delay(500);  
      Serial.print("TANK HEIGHT: ");
//      Serial.print(EEPROM.read(254));
      Serial.print(EEPROMReadlong(200));
      Serial.print("CM");
      Serial.write(byte(26)); 
      delay(1000);   
      Serial.print("AT+CMGD=1,4");
      delay(500);
      Serial.write(byte(13));
      delay(500);  
      
    }
     else if(!(strncmp(msg,"D.ViewTL*",9)))
    {
      send_init();
      Serial.print("AT+CMGS=\"");   
      for(int j=0;j<14;j++){
        Serial.print(phoneNumber[j]);
      }                
      Serial.print("\"");
      delay(200);
      Serial.write(byte(13));
      delay(500);  
      Serial.print("TANK LITERS: ");
      Serial.print(EEPROMReadlong(100));
      Serial.print(" LITERS");
      Serial.write(byte(26)); 
      delay(1000);   
      Serial.print("AT+CMGD=1,4");
      delay(500);
      Serial.write(byte(13));
      delay(500);  
      
    }
//    #D.ViewAd*
//   else if(!(strncmp(msg,"D.ViewAd*",9)))
//    {
////      Serial.print("Tested");
////      digitalWrite(13, HIGH);
////      delay(200);
////      digitalWrite(13, LOW); 
////      delay(200);
//      send_init();
//      Serial.print("AT+CMGS=\"");   
//      for(int j=0;j<14;j++){
//        Serial.print(phoneNumber[j]);
//      }                
//      Serial.print("\"");
//      delay(200);
//      Serial.write(byte(13));
//      delay(500);  
//      Serial.print("ADMIN: ");
//      for(int j=2;j<16;j++){
//        Serial.write(EEPROM.read(j));       
//      }       
//      Serial.write(byte(26)); 
//      delay(1000);  
////      AT+CMGD=1,4 
//      Serial.print("AT+CMGD=1,4");
//      delay(500);
//      Serial.write(byte(13));
//      delay(500);        
//    }
       else if(!(strncmp(msg,"D.Admin1+",9)) || !(strncmp(msg,"D.Admin2+",9)) || !(strncmp(msg,"D.Admin3+",9)))
    {
      if(int(msg[7])== '1'){         
         error_flag = 0;
       for(int j=120;j<134;j++){
//        Serial.print(msg[j-112]);
        if(int(msg[j-112])<'+' || int(msg[j-112])>'9' ){///ASCII FOR +
//          Serial.print("NUMBER ERROR");
          error_flag = 1;
          break;          
        }
//        EEPROM.write(j, (int(msg[j-12])));    
        delay(20);
      }
      if(error_flag == 0){
//        Serial.print("NO NUMBER ERROR");
        for(int j=120;j<134;j++){
          EEPROM.write(j, msg[j-112]);    
          delay(20);
        }
        Serial.print("SUCCESS Admin Reg1...");
      }
      }
      if(int(msg[7])== '2'){
           error_flag = 0;
       for(int j=135;j<149;j++){
//        Serial.print(msg[j-127]);
        if(int(msg[j-127])<'+' || int(msg[j-127])>'9' ){///ASCII FOR +
//          Serial.print("NUMBER ERROR");
          error_flag = 1;
          break;          
        }
//        EEPROM.write(j, (int(msg[j-127])));    
        delay(20);
      }
      if(error_flag == 0){
//        Serial.print("NO NUMBER ERROR");
        for(int j=135;j<149;j++){
          EEPROM.write(j, msg[j-127]);    
          delay(20);
        }
        Serial.print("SUCCESS Admin Reg2...");
      }
      }
      if(int(msg[7])== '3'){
           error_flag = 0;
       for(int j=150;j<164;j++){
//        Serial.print(msg[j-142]);
        if(int(msg[j-142])<'+' || int(msg[j-142])>'9' ){///ASCII FOR +
//          Serial.print("NUMBER ERROR");
          error_flag = 1;
          break;          
        }
//        EEPROM.write(j, (int(msg[j-142])));    
        delay(20);
      }
      if(error_flag == 0){
//        Serial.print("NO NUMBER ERROR");
        for(int j=150;j<164;j++){
          EEPROM.write(j, msg[j-142]);    
          delay(20);
        }
        Serial.print("SUCCESS Admin Reg3...");
      }
      }
      Serial.print("Entered ADMIN");
     
    }
       else if(!(strncmp(msg,"D.ViewAdmin*",12)))
    {
//      Serial.print("Tested");
//      digitalWrite(13, HIGH);
//      delay(200);
//      digitalWrite(13, LOW); 
//      delay(200);
      send_init();
      Serial.print("AT+CMGS=\"");   
      for(int j=0;j<14;j++){
        Serial.print(phoneNumber[j]);
      }           
      Serial.print("\"");
      delay(200);
      Serial.write(byte(13));
      delay(500);  
      Serial.print("ADMIN: ");
      for(int j=120;j<134;j++){
        Serial.write(EEPROM.read(j));       
      }  
        Serial.print(", ");  
      for(int j=135;j<149;j++){
        Serial.write(EEPROM.read(j));       
      }    
        Serial.print(", ");  
      for(int j=150;j<164;j++){
        Serial.write(EEPROM.read(j));       
      }  
        Serial.print(".");       
      Serial.write(byte(26)); 
      delay(1000);  
//      AT+CMGD=1,4 
      Serial.print("AT+CMGD=1,4");
      delay(500);
      Serial.write(byte(13));
      delay(500);        
    }

 

    
   
}
