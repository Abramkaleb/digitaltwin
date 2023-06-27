#include <lorawan.h>
#include <max6675.h>
#include <NewPing.h>
#include <Servo.h>



//Buat Antares
const char *devAddr = "e7ff96be";
const char *nwkSKey = "5fe73ec991d0d8bb0000000000000000";
const char *appSKey = "00000000000000004bd2da90a6b504c5";

const sRFM_pins RFM_pins = {
  .CS = 5,
  .RST = 0,
  .DIO0 = 27,
  .DIO1 = 2,
};


 const unsigned long interval = 30000;    // 30 s interval to send message
 unsigned long previousMillis = 0;  // will store last time message sent
 unsigned int counter = 0;     // message counter

  char myStr[100];
  byte outStr[255];
  byte recvStatus = 0;
  int port, channel, freq;
  bool newmessage = false;
  String dataSend = "";
  String stringout;




// defines pins numbers for Ultrasonic Sensor HC-SR04 (Sensor Fuel) 
#define TRIGGER_PIN  33
#define ECHO_PIN     32
#define MAX_DISTANCE 400 // Maximum distance we want to measure (in centimeters).
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE); // NewPing setup of pins and maximum distance.


// defines pins numbers for Thermocouple Sensor MAX6675 (Sensor Temperature) 
int thermoDO = 23; //ini masi nyontek joki, blm tau soalnya beda
int thermoCS = 19;
int thermoCLK = 18;


// defines pins number for IR Sensor (Sensor RPM)
const int rpmPin = 26;
volatile byte rpmcount;
unsigned int rpm;
unsigned long timeold;
MAX6675 thermocouple(thermoCLK, thermoCS, thermoDO);


// defines pins numbers for Flow rate of Water YF-S201 (Sensor Cooling Water)
double flow; //Water flow L/Min 
int flowsensor = 17; 
unsigned long currentTime;
unsigned long lastTime;
unsigned long pulse_freq;
unsigned long millisBefore;
volatile int objects;


Servo motorServo;
const int pinServo=25;
const int pinLed=5;
int valServo;
int Delay=0;



void pulse () // Interrupt function
{
   pulse_freq++;
}



void rpm_fun()
{
  rpmcount++;
}





void setup() {
  // put your setup code here, to run once:


 // Setup loraid access
  Serial.begin(115200);
  delay(2000);
  if (!lora.init()) {
    Serial.println("RFM95 not detected");
    delay(5000);
    return;
  }

  // Set LoRaWAN Class change CLASS_A or CLASS_C
  lora.setDeviceClass(CLASS_C);

  // Set Data Rate
  lora.setDataRate(SF10BW125);

  // Set FramePort Tx
  lora.setFramePortTx(5);

  // set channel to random
  lora.setChannel(MULTI);

  // Set TxPower to 15 dBi (max)
  lora.setTxPower(15);

  // Put ABP Key and DevAddress here
  lora.setNwkSKey(nwkSKey);
  lora.setAppSKey(appSKey);
  lora.setDevAddr(devAddr);

  
  
  //sensor YF-S201 (Sensor Cooling Water)
   pinMode(flowsensor, INPUT);
   Serial.begin(9600);
   attachInterrupt(0, pulse, RISING); // Setup Interrupt
   currentTime = millis();
   lastTime = currentTime;


  //sensor IR rpm

  attachInterrupt(rpmPin, rpm_fun, FALLING);
  rpmcount = 0;
  rpm = 0;
  timeold = 0;



  //motor servo
  motorServo.attach(pinServo); 
  motorServo.write(0); 
   

}





void loop() {
  // put your main code here, to run repeatedly:

    if (millis() - previousMillis > interval) {
    previousMillis = millis();

    sprintf(myStr, "Lora Counter-%d", counter++);


    Serial.print("Sending: ");
    Serial.println(myStr);
    lora.sendUplink(myStr, strlen(myStr), 0);
    port = lora.getFramePortTx();
    channel = lora.getChannel();
    freq = lora.getChannelFreq(channel);


    delay(1000);
    Serial.println("Sending: ");

    //ini sensor jarak fuel
    int distance = sonar.ping_cm(); // Send ping, get distance in cm and print result (0 = outside set distance range)
    int volume = 3.65 * (0.5 * 3.75) * (0.5 * 3.75) * (30 - distance);

    
    Serial.print("     Volume: ");
    Serial.print(volume);
    Serial.println("mL");

    //ini sensor temperature
    int temp = thermocouple.readCelsius();

    Serial.print("     Temp = "); 
    Serial.print(temp);
    Serial.println("°C"); 

    

    //ini sensor cooling water
      lastTime = currentTime; 
      // Pulse frequency (Hz) = 7.5Q, Q is flow rate in L/min.
      flow = (pulse_freq / 7.5); 
      pulse_freq = 0; // Reset Counter
     
      Serial.print("     Flow = ");
      Serial.print(flow); 
      Serial.println(" L/Min");
     

    //ini sensor IR RPM
    if (millis() - millisBefore > 1000) {
    rpm = (objects / 1.0)*60;
    objects = 1;
    millisBefore = millis();
    }

    delay(100);
    objects++;

    Serial.print("     RPM = ");
    Serial.println(rpm);
    attachInterrupt(0, rpm_fun, FALLING);


    //Data Send
    dataSend = "{\"v\": " + String(volume,4)  + ", \"t\": " + String(temp,4) +",    \"f\": " + String(flow,4)  +", \"r\": " + String(rpm,4)    +"}";
    dataSend.toCharArray(myStr,100); 


}

 lora.update();

 recvStatus = lora.readDataByte(outStr);
  if (recvStatus) {
    newmessage = true;
    char outchar[255] = {};
    int counter = 0;
    port = lora.getFramePortRx();
    channel = lora.getChannelRx();
    freq = lora.getChannelRxFreq(channel);

    for (int i = 0; i < recvStatus; i++)
    {
      if (((outStr[i] >= 32) && (outStr[i] <= 126)) || (outStr[i] == 10) || (outStr[i] == 13))
        counter++;
    }
    if (port != 0)
    {
      if (counter == recvStatus)
      {
        Serial.print(F("Received String : "));
        for (int i = 0; i < recvStatus; i++)
        {
          Serial.print(char(outStr[i]));
          outchar[i] = outStr[i];
        }
      }
      else
      {
        Serial.print(F("Received Hex: "));
        for (int i = 0; i < recvStatus; i++)
        {
          Serial.print(outStr[i], HEX); Serial.print(" ");
        }
      }
      Serial.println();
      Serial.print(F("fport: "));    Serial.print(port);Serial.print(" ");
      Serial.print(F("Ch: "));    Serial.print(channel);Serial.print(" ");
      Serial.print(F("Freq: "));    Serial.println(freq);Serial.println(" ");
    }
    else
    {
      Serial.print(F("Received Mac Cmd : "));
      for (int i = 0; i < recvStatus; i++)
      {
        Serial.print(outStr[i], HEX); Serial.print(" ");
      }
      Serial.println();
      Serial.print(F("fport: "));    Serial.print(port);Serial.print(" ");
      Serial.print(F("Ch: "));    Serial.print(channel);Serial.print(" ");
      Serial.print(F("Freq: "));    Serial.println(freq);Serial.println(" ");
    }
  

    //downlink

      stringout = (String)outchar;
      Serial.println(stringout);
      if (stringout == "1")
      {
        //digitalWrite(valServo);
        Serial.print("Val Servo: " + valServo);
        stringout="";
        //motorServo.write(valServo);
      }
      
    }



}
