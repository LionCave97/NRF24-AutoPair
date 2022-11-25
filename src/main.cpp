#include <Arduino.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

//Radio Setup
RF24 radio(10, 9);  // CE, CSN
byte sAddresses[][6] = {"BasT","BasR"};

const bool autoPair = true;
String mode = "none";
byte ctrlData[10];
int maxData = 10;

//Radio Pair
typedef struct{
  int id = 0; // Id number for each fighter
  bool paired = false;
}
pair;
pair pairData;


//Controller Data
typedef struct{
  int id = 0;
  int leftSpeed = 0;   
  int rightSpeed = 0;
  bool btn1 = 0;
  bool btn2 = 0;
}
ctrl;
ctrl ctrlData;

boolean pairNow(){
  if (mode == "remote"){
    pairData.id = 0;
    pairData.paired = false;
    radio.begin();
    radio.setChannel(1);
    radio.stopListening();
    radio.openReadingPipe(1, sAddresses[0]);
    radio.startListening();
    radio.write( &pairData, sizeof(pairData) );  

    if (radio.available())
    {
      radio.read(&pairData, sizeof(pairData));
      pairData.paired = true;
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
          pairData.paired = true;
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

void sendData(byte data[]){
  radio.write( &data, sizeof(10) );
}

byte receiveData(){
  byte rdata[10];
  if (radio.available()){
    radio.read(&rdata, sizeof(rdata));
  }
  Serial.println(rdata[2]);
  return rdata;
}

void init(String setMode, int setId){
  mode = setMode;
  pairData.id = setId;
}

byte sData[10];

void setup() {
  boolean paired = false;
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("Init");
  // init("fighter", 10);
  init("remote", 0);

  while (!paired)
  {
    paired = pairNow();
    Serial.println("Not Paired");
  }
  Serial.print("Id: ");
  Serial.println(pairData.id);
  sData[1] = {1};
}


void loop() {
  if (mode == "fighter")
  {
    sData[0] = byte("Hello");
    sData[1] = sData[1] + 1;
    sData[2] = 2;
    sData[3] = 3000;

    Serial.println("sendData");
    Serial.println(sData[1]);
    sendData(sData);
  }

  if (mode == "remote")
  {
    // byte* rsData = receiveData();
    receiveData();
    // Serial.println(rsData[2]);
    
    // for (size_t i = 0; i < 11; i++)
    // {
    //   Serial.println(i);
    //   Serial.println(rsData[i]);
      // delay(500);
    // }
    // delete[] rsData;
    delay(100);
  }
}