#include <Sodaq_RN2483.h>
#include <Sodaq_wdt.h>
#include <math.h>

#define DEBUG
#ifdef ARDUINO_ARCH_AVR
  #define debugSerial Serial
  #define LED LED1
#elif ARDUINO_ARCH_SAMD
  #define debugSerial SerialUSB
  #define LED LED_BUILTIN
#endif
#define loraSerial Serial1

//global constants
const uint8_t DEV_ADDR[4] = { 0x14, 0x20, 0x37, 0x02 };
const uint8_t NWK_SKEY[16] = { 0x43, 0x60, 0xde, 0xdc, 0x84, 0x29, 0x0c, 0xe7, 0x1c, 0x08, 0x82, 0x88, 0x7d, 0xbe, 0xa8, 0x10 };
const uint8_t APP_SKEY[16] = { 0xea, 0xcc, 0x28, 0x68, 0xe7, 0xc0, 0xc3, 0xed, 0x94, 0x8c, 0x77, 0xe7, 0x64, 0x12, 0x04, 0x86 };

//globar vars
bool softReset = false;

void setup()
{
  //disable watchdog to prevent startup loop after soft reset
  sodaq_wdt_disable();

  // Setup LED, on by default
  pinMode(LED, OUTPUT);

  //supply power to lora bee
  digitalWrite(BEE_VCC, HIGH);
  
#ifdef DEBUG
  //Wait for debugSerial or 10 seconds
  while ((!debugSerial) && (millis() < 10000));
  debugSerial.begin(9600);
  LoRaBee.setDiag(debugSerial);
#endif

  //setup LORA connection
  while(!loraSerial);
  loraSerial.begin(LoRaBee.getDefaultBaudRate());
  
  if (!LoRaBee.initABP(loraSerial, DEV_ADDR, APP_SKEY, NWK_SKEY))
  {
    logMsg("Connection to the network failed!");
    softReset = true;
  }
  else
  {
    logMsg("Connection to the network successful.");
    setDataRate(0); //HACK because no acknoledgements received
    //sendTestPacket();
  }

  onSetup();
}

void loop()
{
   doLoop();
}

void sendTestPacket() {
  const uint8_t testPayload[] = { 0x30, 0x31, 0xFF, 0xDE, 0xAD };
  logMsg("Sending test packet...");
  LoRaBee.send(1, testPayload, 5);
}

void setDataRate(int dr) {
  loraSerial.print("mac set dr 0\r\n");
  String result = loraSerial.readString();
  logMsg("Setting data rate to " + String(dr) + ": " + result);
}

void logMsg(String msg)
{
  #ifdef DEBUG
  debugSerial.println(msg);
  #endif
}
