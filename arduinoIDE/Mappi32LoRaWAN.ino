

#include <lorawan.h>
#include <max6675.h>
#include <NewPing.h>

//ABP Credentials
const char *devAddr = "e7ff96be";
const char *nwkSKey = "5fe73ec991d0d8bb0000000000000000";
const char *appSKey = "00000000000000004bd2da90a6b504c5";


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


const unsigned long interval = 10000;    // 10 s interval to send message
unsigned long previousMillis = 0;  // will store last time message sent
unsigned int counter = 0;     // message counter

char myStr[100];
byte outStr[255];
byte recvStatus = 0;
int port, channel, freq;
bool newmessage = false;

String dataSend = ""; // ini ngikutin hilmi gatau bwt apa
String stringout;

const sRFM_pins RFM_pins = {
  .CS = 12, // ini karna pake hspi miso
  .RST = 0,
  .DIO0 = 27,
  .DIO1 = 2,
};


void pulse () // Interrupt function
{
   pulse_freq++;
}


void rpm_fun()
{
  rpmcount++;
}



void setup() {
  // Setup loraid access
  Serial.begin(115200);
  delay(2000);

  //sensor HC-SR04 (Sensor Fuel)
  
  
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


  if (!lora.init()) {
    Serial.println("RFM95 not detected");
    Serial.println("MAX6675 test");
    delay(5000); //ini belum tentu segini
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
}

void loop() {
    lorawanrxtx();
}

void lorawanrxtx(){

// Check interval overflow
  if (millis() - previousMillis > interval) {
    previousMillis = millis();

    sprintf(myStr, "Lora Counter-%d", counter++); //kalo eror ni bang liat punya hilmi

    
    Serial.print("Sending: ");

    //ini sensor jarak fuel
    int distance = sonar.ping_cm(); // Send ping, get distance in cm and print result (0 = outside set distance range)

    Serial.print("Distance: ");
    Serial.print(distance);
    Serial.println("cm");

    //ini sensor temperature
    int temp = thermocouple.readCelsius();

    Serial.print("Temperature = "); 
    Serial.print(temp);
    Serial.println("°C"); 

    

    //ini sensor cooling water
    lastTime = currentTime; 
      // Pulse frequency (Hz) = 7.5Q, Q is flow rate in L/min.
      flow = (pulse_freq / 7.5); 
      pulse_freq = 0; // Reset Counter
      Serial.print(flow, DEC); 
      Serial.println(" L/Min");


    //ini sensor IR RPM
    delay(1000);
    detachInterrupt(0);
    rpm = 60*1000/(millis() - timeold)*rpmcount;
    timeold = millis();
    rpmcount = 0;

    Serial.print("RPM = ");
    Serial.print(rpm);
    //attachInterrupt(0, rpm_fun, FALLING); --> sapa tau butuh


    //read data from the sensor
    float fuel = distance;
    float temperature = temp;
    float flow = flow; 
    float rotation = rpm;

    Serial.println("Sending: ");
    dataSend = "{\"f\": " + String(fuel,2) + ", \"t\": " + String(temperature,2) +", \"f\": " + String(flow,2) +", \"r\": " + String(rotation,2) +"}";
    dataSend.toCharArray(myStr,100);
    Serial.println(myStr);


    lora.sendUplink(myStr, strlen(myStr), 0);
    port = lora.getFramePortTx();
    channel = lora.getChannel();
    freq = lora.getChannelFreq(channel);
    Serial.print(F("fport: "));   Serial.print(port);Serial.print(" ");
    Serial.print(F("Ch: "));      Serial.print(channel);Serial.print(" ");
    Serial.print(F("Freq: "));    Serial.print(freq);Serial.println(" ");

     Serial.print("Fuel: ");      Serial.print(fuel);       Serial.println("cm");
     Serial.print("Temp: ");      Serial.print(temp);       Serial.println("c");
     Serial.print("Flow: ");      Serial.print(flow);       Serial.println("L/m");
     Serial.print("Rpm: ");       Serial.print(rotation);   Serial.println(" ");

  }

  // Check Lora RX
  lora.update();

  recvStatus = lora.readDataByte(outStr);
  if (recvStatus) {
    newmessage = true;
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
  }

}
