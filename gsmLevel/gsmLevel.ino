#include<LiquidCrystal.h>
//LiquidCrystal lcd(6,7,8,9,10,11);

#define Fan 3
#define Light 4
#define TV 5
int temp=0,i=0;
int led=13; 
char str[15];
int level=0;
#include <NewPing.h>

#define TRIGGER_PIN  12  // Arduino pin tied to trigger pin on the ultrasonic sensor.
#define ECHO_PIN     11  // Arduino pin tied to echo pin on the ultrasonic sensor.
#define MAX_DISTANCE 500 // Maximum distance we want to ping for (in centimeters). Maximum sensor distance is rated at 400-500cm.

NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE); // NewPing setup of pins and maximum distance.

void setup()
{
  Serial.begin(9600);
  pinMode(led, OUTPUT);
   pinMode(Fan, OUTPUT);
    pinMode(Light, OUTPUT);
    pinMode(TV, OUTPUT); 
  delay(2000);
  delay(2000);
  Serial.print("AT+CNMI=1,2,0,0,0");
  delay(200);
  Serial.write(byte(13));
  delay(500);
 Serial.print("AT+CMGF=1");  
  delay(200);
  Serial.write(byte(13));
  delay(1000);    
}

void loop()
{
  if(temp==1)
  {
    check();
    temp=0;
    i=0;
    delay(1000);
  }
 
}

 void serialEvent() 
 {
  while(Serial.available()) 
  {
    if(Serial.find("#A."))
    {
      digitalWrite(led, HIGH);
      delay(1000);
      digitalWrite(led, LOW);
      while (Serial.available()) 
      {
      char inChar=Serial.read();
      str[i++]=inChar;
      if(inChar=='*')
      {
        temp=1;
        return;
      } 
      } 
    }
   }
 }

void check()
{
   if(!(strncmp(str,"level",5)))
    {
      digitalWrite(TV, HIGH);
//      lcd.setCursor(13,1); 
//      lcd.print("ON    ");
   digitalWrite(led, HIGH);
      delay(1000);
      digitalWrite(led, LOW);
      delay(200);
  Serial.print("AT+CNMI=1,2,0,0,0");
  delay(200);
  Serial.write(byte(13));
  delay(500);
  Serial.print("AT+CMGF=1");  
  delay(200);
  Serial.write(byte(13));
  delay(500);
  Serial.print("AT+CMGS=\"+2348057471307\"");
  delay(200);
  Serial.write(byte(13));
  delay(500);  
  Serial.print("LEVEL: ");
  Serial.print(sonar.ping_cm()); // Send ping, get distance in cm and print result (0 = outside set distance range)
//  Serial.print(level);
  Serial.print("cm");
  Serial.write(byte(26)); 
  delay(500);    
    }  
   
   else if(!(strncmp(str,"tv off",6)))
    {
      digitalWrite(TV, LOW);
//      lcd.setCursor(13,1); 
//      lcd.print("OFF    ");
      delay(200);
    }
  
    else if(!(strncmp(str,"fan on",5)))
    {
      digitalWrite(Fan, HIGH);
//      lcd.setCursor(0,1); 
//      lcd.print("ON   ");
      delay(200);
    }
 
    else if(!(strncmp(str,"fan off",7)))
    {
      digitalWrite(Fan, LOW);
//      lcd.setCursor(0,1); 
//      lcd.print("OFF    ");
      delay(200);
    }
 
    else if(!(strncmp(str,"light on",8)))
    {
      digitalWrite(Light, HIGH);
//      lcd.setCursor(7,1); 
//      lcd.print("ON    ");
      delay(200);
    }
 
    else if(!(strncmp(str,"light off",9)))
    {
      digitalWrite(Light, LOW);
//      lcd.setCursor(7,1); 
//      lcd.print("OFF    ");
      delay(200);
    } 
    
    else if(!(strncmp(str,"all on",6)))
    {
      digitalWrite(Light, HIGH);
      digitalWrite(Fan, HIGH);
      digitalWrite(TV, HIGH);
//      lcd.setCursor(0,1); 
//      lcd.print("ON     ON    ON  ");
      delay(200);
    }
 
    else if(!(strncmp(str,"all off",7)))
    {
      digitalWrite(Light, LOW);
      digitalWrite(Fan, LOW);
      digitalWrite(TV, LOW);
//      lcd.setCursor(0,1); 
//      lcd.print("OFF   OFF    OFF  ");
      delay(200);
    }     
}
