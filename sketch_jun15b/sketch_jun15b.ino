//Arduino-Wine-Thermostat Project
//Nikos Grivas

//This arduino code is made to toggle an electrovalve,
//which controls the flow of cold water to the outer surface of a cylindrical steel tank 
//to lower the temperature of the wine that is contained in it. A temperature sensor is used to 
//monitor the temperature of the wine, two buttons to set it to the desired temperature and a 
//7segment display to depict the readings. 

//These two libraries are essential for the easy extraction of temperature reading and communication 
//between arduino μC and the temperature sensor DS18B20
#include <OneWire.h>
#include <DallasTemperature.h>

//Data wire of sensor DS18B20 is plugged into port 10 on the Arduino 
#define ONE_WIRE_BUS 10

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

//Set the pins that correspond to each 
//digit for the display
int Digits[3];
const int Dfirst=5;
const int Dlast=7;

//Set the pins that wil be used for the shift register operation and passing of values
const int ser=4;
const int latch=3;
const int clk=2;

//Set the pins that will be used as inputs of the push-down buttons
const int buttonUp=8;
const int buttonDown=9;

//Set the pin that toggles the electrovalve through a relay
const int relayEnable=11;

//The default values for the real-time temperature and the desired temperature
float temperatureCurr;
float temperatureSet;

//Initiate values used for renewing the display
unsigned long lastTime=0;
unsigned long int count=0;


void setup() {
  
  //Set as output all the digits enablers
  for (int i=0;i<3;i++){
    Digits[i]=Dfirst+i;
    pinMode(Digits[i], OUTPUT);
  }
  
  //Start up the Dallas library
  sensors.begin();
  
  //Set as output the serial input pin ser,clk and latch
  pinMode(ser,OUTPUT);
  pinMode(clk,OUTPUT);
  pinMode(latch,OUTPUT);

  //Initialize push down inputs
  pinMode(buttonUp, INPUT);
  pinMode(buttonDown, INPUT);

  //Set and initialize the relay output
  pinMode(relayEnable,OUTPUT);
  digitalWrite(relayEnable,HIGH);

  temperatureCurr=25.5;
  temperatureSet=22.5;
}


int int2dispint(int digit){
  //This function is a conversion of an integer [0-9]
  //to the byte that enables the appropriate LED 
  //segments at a 7-segment display
    switch (digit){
      case 0:
        return int(B00000011);
      case 1:
        return int(B10011111);
      case 2:
        return int(B00100101);
      case 3:
        return int(B00001101);
      case 4:
        return int(B10011001);
      case 5:
        return int(B01001001);
      case 6:
        return int(B01000001);
      case 7:
        return int(B00011111);
      case 8:
        return int(B00000001);
      case 9: 
        return int(B00001001);
    }
  }
  
void write_to_display(float digOut){
  //This function outputs to the 4digit 7-segment display
  //(using 2 integer digits and one decimal) the number digOut that is its input
  
  //Compute the value of each digit for the float number digOut
  int digit[3];
  digit[0]=int(digOut/10);
  digit[1]=int(digOut)%10;
  digit[2]=int(digOut*10)%10;
  
  //Convert the 3 first digits to appropriate byte for display
  for (int i=0;i<3;i++){
    digit[i]=int2dispint(digit[i]);
    
    //Insert the decimal point to the second digit
    if (i==1) digit[i]-=1;  
  }
  //Write/Depict to display each digit 
  for (int i=0;i<3;i++){
    
    //Pass at the shift register the specific digit
    digitalWrite(latch,LOW);
    shiftOut(ser,clk,LSBFIRST,digit[i]);
    digitalWrite(latch,HIGH);
    
    //Pick and enable the right digit
    digitalWrite(Digits[i],HIGH);
    
    //Keep for some msecs the LEDs ON 
    delay(6);
    
    //Disable the digit output
    digitalWrite(Digits[i],LOW);
  }
}


void loop() {
  
  //Read the temperature from the sensor every 500 loops
  count++;
  if (count%500==0){
    
    //Get the current temperature
    sensors.requestTemperatures();
    
    //In case of an error
    if (sensors.getTempCByIndex(0)<-100){
      return;
    }
    else {
      temperatureCurr=sensors.getTempCByIndex(0);
      }
  } 
    
  //Set the temperature using the increment buttons
  unsigned long int timeInterval=500;
  //Enter the condition when the button is pressed and at least timeInterval has passed
  
  //Button up
  if (digitalRead(buttonUp) == HIGH && millis()-lastTime>=timeInterval) {
    
    //Keep track of the last time the temperature was changed
    lastTime=millis();
    
    //Check for digits overflow
    if (temperatureSet<99.5){
      
      //Increment +0.5 
      temperatureSet+=0.5;
    }
  }
  
  //Button down
  if (digitalRead(buttonDown) == HIGH && millis()-lastTime>=timeInterval){
    //Keep track of the last time the temperature was changed
    lastTime=millis();

    //Check for digits underflow
    if (temperatureSet>0.5){
      
      //Decrement -0.5
      temperatureSet-=0.5;
    }
  }
  
  //If in a short time interval the temperature set was changed then show the temperature set,
  //in every other case show the current sensor temperature
  if (millis()-lastTime<=3*timeInterval){
    write_to_display(temperatureSet);
  }
  else{
    write_to_display(temperatureCurr);
  }

  //Check the current sensor temperature and compare it with the set temperature
  //if there is a difference more than 0.5 degrees, then the relay output relayEnable is enabled.
  if (temperatureCurr-temperatureSet>=0.5){
    digitalWrite(relayEnable,HIGH);
  }
  else{
    digitalWrite(relayEnable,LOW);
    }
}
