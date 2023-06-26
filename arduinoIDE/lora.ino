#include <lorawan.h>
#include <Servo.h>
#include "max6675.h" 

#define SOUND_SPEED 0.034
#define LED_BUILTIN 5
#define SENSOR  17

#define Tmax 218

// ABP Credentials
const char *devAddr = "e7ff96be"; // Digital Twin Dev Address
const char *nwkSKey = "5fe73ec991d0d8bb0000000000000000";
const char *appSKey = "00000000000000004bd2da90a6b504c5";

const int trigPin = 33;
const int echoPin = 32;
const int rpmPin  = 26;
const int pinServo=25;
const int pinLed  =5;

const int SO  = 23;
const int CS  = 19;
const int sck = 18;

const unsigned long interval = 30000; // 30 s interval to send message
unsigned long previousMillis = 0;     // will store last time message sent
unsigned int counter = 0;             // message counter

char myStr[100];
byte outStr[255];
byte recvStatus = 0;
int port, channel, freq;
bool newmessage = false;
String dataSend = "";
String stringout;

float flowRate;
unsigned int flowMilliLitres;
unsigned long totalMilliLitres;

long duration;
float distanceCm;

Servo motorServo; 
MAX6675 module(sck, CS, SO);

boolean ledState = LOW;
float calibrationFactor = 4.5;
volatile byte pulseCount;
byte pulse1Sec = 0;

const float pi = 3.14;
const float d =  2.1;

int Volume;
float Tinggi;
int RPMnew;
int RPM;

const sRFM_pins RFM_pins = {
    .CS = 19,
    .RST = 0,
    .DIO0 = 27,
    .DIO1 = 2,
};

void ICACHE_RAM_ATTR sensRpm() {
  RPM++;
}

void IRAM_ATTR pulseCounter()
{
  pulseCount++;
}


void setup()
{
  // Setup loraid access
  Serial.begin(115200);
  delay(2000);
  pinMode(rpmPin, INPUT_PULLUP);
  pinMode(pinLed, OUTPUT);
  pinMode(trigPin, OUTPUT); 
  pinMode(echoPin, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(SENSOR, INPUT_PULLUP);
  motorServo.attach(pinServo); 
  motorServo.write(0); 
  delay(1000); 
  if (!lora.init())
  {
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
    
  pulseCount = 0;
  flowRate = 0.0;
  flowMilliLitres = 0;
  totalMilliLitres = 0;
  previousMillis = 0;

  attachInterrupt(digitalPinToInterrupt(SENSOR), pulseCounter, FALLING); 
  attachInterrupt(digitalPinToInterrupt(rpmPin), sensRpm, RISING); 
}

void loop()
{
  lorawanrxtx();
}

void lorawanrxtx()
{
  // UPLINK
  if (millis() - previousMillis > interval)
  {
    pulse1Sec = pulseCount;
    pulseCount = 0;
    flowRate = ((1000.0 / (millis() - previousMillis)) * pulse1Sec) / calibrationFactor;

    previousMillis = millis();

    flowMilliLitres = (flowRate / 60) * 1000;

    totalMilliLitres += flowMilliLitres;

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

    float Volume =pi * (0.5 * d) * (0.5 * d) * Tinggi;
    float temperature = module.readCelsius(); 
    int wings= 3;  // no of wings of rotating object, for disc object use 1 with white tape on one side
    RPMnew = RPM/wings;  //here we used fan which has 3 wings

    sprintf(myStr, "Lora Counter-%d\n", counter++);

    Serial.print("Custom Address:");


    // Read the data from the sensor
    float temperature = module.readCelsius(); 
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.println("°C ");  

    
    float volume = getVolume();
    float flowrate = getFlowrate();
    float RPMnew= getRPMnew();

    Serial.print("Sending: ");
    dataSend = "{\"t\": " + String (temperature,2 + ", \"v\": " + String (Volume,2) + ", \"f\": " + String (flowRate,2) + ", \"r\": " + String (RPMnew,2) + "}";
    dataSend.toCharArray(myStr, 100);
    Serial.println(myStr);
    lora.sendUplink(myStr, strlen(myStr), 0);
    port = lora.getFramePortTx();
    channel = lora.getChannel();
    freq = lora.getChannelFreq(channel);
    Serial.print(F("fport: "));
    Serial.print(port);
    Serial.print(" ");
    Serial.print(F("Ch: "));
    Serial.print(channel);
    Serial.print(" ");
    Serial.print(F("Freq: "));
    Serial.print(freq);
    Serial.println(" ");
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.println("Fuel: ");
    Serial.print(Volume);
    Serial.println("Cooling: ");
    Serial.print(flowRate);
    Serial.println("RPM: ");
    Serial.print(RPMnew);
  }

  // Check Lora RX
  lora.update();

  //DOWNLINK   
  recvStatus = lora.readDataByte(outStr);
  if (recvStatus)
  {
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
          Serial.print(outStr[i], HEX);
          Serial.print(" ");
        }
      }
      Serial.println();
      Serial.print(F("fport: "));
      Serial.print(port);
      Serial.print(" ");
      Serial.print(F("Ch: "));
      Serial.print(channel);
      Serial.print(" ");
      Serial.print(F("Freq: "));
      Serial.println(freq);
      Serial.println(" ");
    }
    else
    {
      Serial.print(F("Received Mac Cmd : "));
      for (int i = 0; i < recvStatus; i++)
      {
        Serial.print(outStr[i], HEX);
        Serial.print(" ");
      }
      Serial.println();
      Serial.print(F("fport: "));
      Serial.print(port);
      Serial.print(" ");
      Serial.print(F("Ch: "));
      Serial.print(channel);
      Serial.print(" ");
      Serial.print(F("Freq: "));
      Serial.println(freq);
      Serial.println(" ");
    }

    stringout = (String)outchar;
    Serial.println(stringout);
    if (stringout == "1")
    {
      digitalWrite(RELAY, HIGH);
      Serial.print("RELAY ON");
      stringout = "";
    }
    if (stringout == "0")
    {
      digitalWrite(RELAY, LOW);
      Serial.print("RELAY OFF");
      stringout = "";
    }
  }
}
