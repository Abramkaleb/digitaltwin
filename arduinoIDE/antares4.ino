#include <Servo.h>
#include "max6675.h" 
#include <AntaresESP32HTTP.h>

#define ACCESSKEY "5fe73ec991d0d8bb:4bd2da90a6b504c5"
#define WIFISSID "monitor"
#define PASSWORD "monitor2021"

#define projectName "Digital-Twin"
#define deviceName "LoRa"

#define Tmax 218

AntaresESP32HTTP antares(ACCESSKEY);

const int trigPin = 33;
const int echoPin = 32;
const int rpmPin  = 26;

const int SO  = 23;
const int CS  = 19;
const int sck = 18;

#define SOUND_SPEED 0.034
#define LED_BUILTIN 5
#define SENSOR  17

long currentMillis = 0;
long previousMillis = 0;
int interval = 1000;
boolean ledState = LOW;
float calibrationFactor = 4.5;
volatile byte pulseCount;
byte pulse1Sec = 0;
float flowRate;
unsigned int flowMilliLitres;
unsigned long totalMilliLitres;

long duration;
float distanceCm;

Servo motorServo; 
MAX6675 module(sck, CS, SO);

const int pinServo=25;
const int pinLed=5;
int valServo;
int Delay=0;

const float pi = 3.14;
const float d =  2.1;

int Volume;
float Tinggi;
int RPMnew;

int RPM;
void ICACHE_RAM_ATTR sensRpm() {
  RPM++;
}

void IRAM_ATTR pulseCounter()
{
  pulseCount++;
}

void setup()
{

  Serial.begin(115200);

  pinMode(rpmPin, INPUT_PULLUP);

  pinMode(pinLed, OUTPUT);
  pinMode(trigPin, OUTPUT); 
  pinMode(echoPin, INPUT);

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(SENSOR, INPUT_PULLUP);


    
  motorServo.attach(pinServo); 
  
  motorServo.write(0); 
  delay(1000); 

  antares.setDebug(false);
  antares.wifiConnection(WIFISSID, PASSWORD);
  
  pulseCount = 0;
  flowRate = 0.0;
  flowMilliLitres = 0;
  totalMilliLitres = 0;
  previousMillis = 0;

  attachInterrupt(digitalPinToInterrupt(SENSOR), pulseCounter, FALLING); 
  attachInterrupt(digitalPinToInterrupt(rpmPin), sensRpm, RISING);   
     
}

boolean statSend=false;

void loop()
{
  currentMillis = millis();
  if (currentMillis - previousMillis > interval) {
    
    pulse1Sec = pulseCount;
    pulseCount = 0;

    flowRate = ((1000.0 / (millis() - previousMillis)) * pulse1Sec) / calibrationFactor;
    previousMillis = millis();

    flowMilliLitres = (flowRate / 60) * 1000;

    totalMilliLitres += flowMilliLitres;
    
    Serial.print("Flow rate: ");
    Serial.print(int(flowRate));  
    Serial.print("L/min");
    Serial.print("\t");   

    Serial.print("Output Liquid Quantity: ");
    Serial.print(totalMilliLitres);
    Serial.print("mL / ");
    Serial.print(totalMilliLitres / 1000);
    Serial.println("L");

    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    
    duration = pulseIn(echoPin, HIGH);
    
    distanceCm = duration * SOUND_SPEED/2;

    //Tinggi=Tmax - distanceCm;
    Tinggi=distanceCm;
    if(Tinggi<=0) Tinggi=0;
    
    Serial.print("Distance (cm): ");
    Serial.println(distanceCm);
    
    //π × (1/2 × d)² × t

    float Volume =pi * (0.5 * d) * (0.5 * d) * Tinggi;
    Serial.print("Volume : ");
    Serial.println(Volume);    
   
    float temperature = module.readCelsius(); 
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.println("°C ");   
    
    //int statusState = digitalRead(rpmPin);
    //Serial.print("status=");  //0 aktif
    //Serial.println(statusState);


  int wings= 3;  // no of wings of rotating object, for disc object use 1 with white tape on one side
  RPMnew = RPM/wings;  //here we used fan which has 3 wings
  Serial.print(RPMnew);
  Serial.print(" Rot/sec :"); 

  Serial.print((RPMnew*60));
  Serial.println("Rot/min. "); 

  RPM=0;
  
    antares.get(projectName, deviceName);
    if(antares.getSuccess()) {
      
      valServo = antares.getInt("device","rpm");
      Serial.println("val Servo: " + String(valServo));      
      motorServo.write(valServo);
           
    }


    Delay++; 
    if(Delay>=4){
      Delay=0;
      Serial.println("Send");
      
      antares.add("device","modelMotor", "Motor Diesel 4 langkah horizontal");
      antares.add("device","jumlahSilinder", 1);
      antares.add("device","dimensi", "672 x 330,5 x " + String(distanceCm));
      antares.add("device","bahanBakar","Solar");
      antares.add("device","rasioKompresi", 17);
      antares.add("device","sistemPendinginan", "Hopper");
      antares.add("device","setRpm", true);
      antares.add("device","exhaust", int(temperature));
      antares.add("device","fuel", int(Volume));
      antares.add("device","cooling",int(flowRate));
      antares.add("device","rpm",RPMnew);
        
      antares.send(projectName, deviceName);  
        
    }  
  }
  


    
}
