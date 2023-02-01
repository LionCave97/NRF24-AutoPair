#include <Arduino.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

//Radio Setup
RF24 radio(10, 9);  // CE, CSN
byte sAddresses[][6] = {"BasT","BasR"};

const bool autoPair = true;
String mode = "none";
int maxRetry = 3;
int retry = 0;

//Radio Pair
typedef struct{
  int id = 0; // Id number for each fighter
}
pair;
pair pairData;


//Controller Data
typedef struct{
  int id = 0;
  String state = "none";
  bool btn1 = 0;
  bool btn2 = 0;
  bool btn3 = 0;
  bool btn4 = 0;
  int boost = 0;
  int remoteVoltage = 0;
  int fighterVoltage = 0;
}
ctrl;
ctrl ctrlData;

boolean pairNow(){
  if (mode == "remote"){
    pairData.id = 0;
    radio.begin();
    radio.setChannel(1);
    radio.stopListening();
    radio.openReadingPipe(1, sAddresses[0]);
    radio.startListening();
    radio.write( &pairData, sizeof(pairData) );  

    if (radio.available())
    {
      radio.read(&pairData, sizeof(pairData));
      radio.setChannel(pairData.id);
      radio.stopListening();
      radio.openWritingPipe(sAddresses[0]);
      return true;      
    }
  } else if (mode == "fighter"){
    radio.begin();
    radio.setChannel(1);
    radio.openWritingPipe(sAddresses[0]);  
    radio.stopListening();
    // radio.setPALevel(RF24_PA_LOW); //Default Max Power
    radio.setRetries(3,5); // delay, count

    bool rslt;
    rslt = radio.write( &pairData, sizeof(pairData) );

    if (rslt) {
          radio.setChannel(pairData.id);
          radio.stopListening();
          radio.openReadingPipe(1, sAddresses[0]);
          radio.startListening();
          return true;
      }
      else {
          return false;
      }
      return false;
  } else{
    return false;
  }
  return false;
}

void init(String setMode, int setId){
  mode = setMode;
  pairData.id = setId;
  ctrlData.id = setId;
}

void setup() {
  boolean paired = false;
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("Init");
  init("fighter", 10);
  // init("remote", 0);

  while (!paired)
  {
    paired = pairNow();
    Serial.println("Not Paired");
  }
  Serial.print("Id: ");
  Serial.println(pairData.id);
  delay(2000);
}


void loop() {
  if (mode == "fighter")
  {
    Serial.println("readData");
    if (radio.available()){
      radio.read(&ctrlData, sizeof(ctrlData));
      Serial.print("Boost:");
      Serial.println(ctrlData.boost);
    } else{

    }
  }

  if (mode == "remote")
  {
    ctrlData.boost = ctrlData.boost + 1;
    Serial.println("sendData");
    radio.write( &ctrlData, sizeof(ctrlData) );
  }
}