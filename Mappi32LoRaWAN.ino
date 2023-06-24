

#include <lorawan.h>
#include <max6675.h>


//ABP Credentials
const char *devAddr = "e7ff96be";
const char *nwkSKey = "5fe73ec991d0d8bb0000000000000000";
const char *appSKey = "00000000000000004bd2da90a6b504c5";



// defines pins numbers for Ultrasonic Sensor HC-SR04 (Sensor Fuel) 
const int trigPin = 33;
const int echoPin = 32;

// defines variables
long duration;
int distance;

// defines pins numbers for Thermocouple Sensor MAX6675 (Sensor Temperature) 
int thermoDO = 23; //ini masi nyontek joki, blm tau soalnya beda
int thermoCS = 19;
int thermoCLK = 18;

MAX6675 thermocouple(thermoCLK, thermoCS, thermoDO);

// defines pins numbers for Flow rate of Water YF-S201 (Sensor Cooling Water)
double flow; //Water flow L/Min 
int flowsensor = 2; 
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



void setup() {
  // Setup loraid access
  Serial.begin(115200);
  delay(2000);

  //sensor HC-SR04 (Sensor Fuel)
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input
  

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
     // Clears the trigPin
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    // Sets the trigPin on HIGH state for 10 micro seconds
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    // Reads the echoPin, returns the sound wave travel time in microseconds
    duration = pulseIn(echoPin, HIGH);
    // Calculating the distance
    distance = duration * 0.034 / 2;
    // Prints the distance on the Serial Monitor
    Serial.print("Distance: ");
    Serial.println(distance);

    //ini sensor temperature
    Serial.print("Temperature = "); 
    Serial.print(thermocouple.readCelsius());
    Serial.println("°C"); 

    //read data from the sensor
    float 
   

    Serial.println(myStr);
    lora.sendUplink(myStr, strlen(myStr), 0);
    port = lora.getFramePortTx();
    channel = lora.getChannel();
    freq = lora.getChannelFreq(channel);
    Serial.print(F("fport: "));    Serial.print(port);Serial.print(" ");
    Serial.print(F("Ch: "));    Serial.print(channel);Serial.print(" ");
    Serial.print(F("Freq: "));    Serial.print(freq);Serial.println(" ");

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
