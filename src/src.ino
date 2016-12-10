#include <ModbusMaster.h>
#include <Sodaq_RN2483.h>

#define DEBUG
#define debugSerial SerialUSB
#define loraSerial Serial1
#define growattSerial Serial
#define LED LED_BUILTIN

const int WAKEUP_INTERVAL_S = 900;
const int SLAVE_ID = 2;

ModbusMaster node;
unsigned long Pnow, Etoday;

void onSetup()
{
  growattSerial.begin(9600, SERIAL_8N1);

  // communicate with Modbus slave SLAVE_ID over serial port
  node.begin(SLAVE_ID, growattSerial);
}

void doLoop()
{
  uint8_t result;
  uint16_t data;
    
  //start LED flash
  digitalWrite(LED, HIGH);

  //PYTHON
  //  rr = client.read_input_registers(12,1) #watts delivered by inverter to net
  //value=rr.registers
  //out_watts=float(value[0])/10
  //  rr = client.read_input_registers(27,1) # Total energy production today
  //value=rr.registers
  //Wh_today=float(value[0])*100

  result = node.readInputRegisters(12,1);
  if (result == node.ku8MBSuccess)
  {
      data = node.getResponseBuffer(0);
      Pnow = round(float(data)*100); //convert to milliwats
  }
  
  result = node.readInputRegisters(27,1);
  if (result == node.ku8MBSuccess)
  {
      data = node.getResponseBuffer(0);
      Etoday = round(float(data)/36); //convert to J
  }

  // Print the cumulative total of litres flowed since starting
  logMsg("watts delivered by inverter to net: " + String(Pnow) + "W");
  logMsg("Total energy production today: " + String(Etoday) + "J");
  
  sendMsg();

  digitalWrite(LED, LOW);

  delay(WAKEUP_INTERVAL_S * 1000);
}
  
void sendMsg() {
  //construct payload from Pnow & Etoday
  char payload[8];
  for(int i = 0; i <= 3; i++)
  {
    char c = 0xFF & Pnow >> 8*i;
    payload[3 - i] = c;
  }
  for(int i = 0; i <= 3; i++)
  {
    char c = 0xFF & Etoday >> 8*i;
    payload[7 - i] = c;
  }
  
  logMsg("Sending payload: " + String(payload));
  LoRaBee.send(1, (uint8_t*)payload, 4);
}
