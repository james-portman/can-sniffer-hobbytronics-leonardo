/*
This is for a hobbytronics leonardo can board

ATmega32U4 Fuse settings
If you reprogram the bootloader onto this board or use a programming language other than Arduino, you must set the L Fuse to 0xBF. This activates the CLK OUT pin which is used by the MCP2525 chip. Without this fuse setting, the Can Bus chips on the board will not work.


fuel level:
apparently 30 ohm full to 230 (or 270) ohm for empty

330ohm resistor would give
full @30 ohm  - 0.416v (8.32% of 5v) (85 on analog read?)
maybe empty @230 ohm - 2.054v (41.08% of 5v) (420 on analog read?)
definitely empty @270 ohm - 2.25v (45% of 5v) (460 on analog read?)

initial test with a very full fuel tank shows 112% full
that was with the fuel up to the filler neck

low level to be seen..



TODO:
change the CAN bus speed reading code,
change to run a CAN parser if we are not busy reading fuel level or sending our can packet out
can parse OBD requests in there


*/

#include <EEPROM.h>
#include <SPI.h>
#include <mcp2515.h>

const int SPI_CS_PIN = 17;
const int LED_PIN = 23;

const bool DEBUG = true;

MCP2515 mcp2515(17);

void setup() {

  Serial.begin(500000);
  while(!Serial) {}
  delay(1000);
  Serial.println("Up");

//  Serial.println("resetting can module...");
  mcp2515.reset();
//  Serial.println("setting bit rate...");
  mcp2515.setBitrate(CAN_500KBPS);


//  bool ext = false; // extended?
//  uint32_t filterMask = 0xFFFFFFFF;
//  uint32_t filter = 0x0000000F;
//  if (mcp2515.setFilterMask(MCP2515::MASK0, true, filterMask) != MCP2515::ERROR_OK) {
////    if (mcp2515.setFilterMask(MCP2515::MASK0, ext, filterMask) != MCP2515::ERROR_OK) {
//    Serial.println("Failed setting filter mask");
//  }
//  if (mcp2515.setFilter(MCP2515::RXF0, ext, filter) != MCP2515::ERROR_OK) {
//    Serial.println("Failed setting filter mask");
//  }

//  mcp2515.setFilterMask(MCP2515::MASK0, false, 0x07FF0000);
//  mcp2515.setFilterMask(MCP2515::MASK0, false, 0xFFFFFFFF);
//  mcp2515.setFilter(MCP2515::RXF0, false, 0x000007A0);
//  mcp2515.setFilter(MCP2515::RXF1, false, 0x000007A1);
//  mcp2515.setFilterMask(MCP2515::MASK1, false, 0xFFFFFFFF);
//  mcp2515.setFilter(MCP2515::RXF2, false, 0x00000050);
//  mcp2515.setFilter(MCP2515::RXF3, false, 0x00000051);
//  mcp2515.setFilter(MCP2515::RXF4, false, 0x00000053);
//  mcp2515.setFilter(MCP2515::RXF5, false, 0x00000000);

//  Serial.println("setting normal mode...");
  mcp2515.setNormalMode();
//  Serial.println("done with can setup.");

}

struct can_frame canMsg;
struct can_frame canMsgOut;

void loop() {

  if (mcp2515.readMessage(&canMsg) == MCP2515::ERROR_OK) {
    if (canMsg.can_id < 0x300 || canMsg.can_id > 0x30F) {
      Serial.write(canMsg.can_id >> 8);
      Serial.write(canMsg.can_id & 0xFF);
      Serial.write(canMsg.can_dlc);
      for (int i = 0; i<canMsg.can_dlc; i++)  {
        Serial.write(canMsg.data[i]);
      }
    }
  }

  if (Serial.available() >= 1) {
    canMsgOut.can_id = (Serial.read() << 8) + Serial.read();
    canMsgOut.can_dlc = Serial.read();
    for (int i = 0; i< canMsgOut.can_dlc; i++)  {
      canMsgOut.data[i] = Serial.read();
    }
    if (canMsgOut.can_dlc < 8) {
      for (int i=canMsgOut.can_dlc; i<8; i++) {
        canMsgOut.data[i] = 0;
      }
    }
    mcp2515.sendMessage(&canMsgOut);
    // dummy reply to say we accepted command
    Serial.write(int(0));
    Serial.write(int(0));
    Serial.write(int(1));
    Serial.write(int(0));
  }

}
