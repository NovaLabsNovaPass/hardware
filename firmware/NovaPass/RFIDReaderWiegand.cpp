
//
// RFIDReaderWiegand.cpp
//

// converted to C++ class

/*
 * HID RFID Reader Wiegand Interface for Arduino Uno
 * Written by Daniel Smith, 2012.01.30
 * www.pagemac.com
 *
 * This program will decode the wiegand data from a HID RFID Reader (or, theoretically,
 * any other device that outputs weigand data).
 * The Wiegand interface has two data lines, DATA0 and DATA1.  These lines are normall held
 * high at 5V.  When a 0 is sent, DATA0 drops to 0V for a few us.  When a 1 is sent, DATA1 drops
 * to 0V for a few us.  There is usually a few ms between the pulses.
 *
 * Your reader should have at least 4 connections (some readers have more).  Connect the Red wire 
 * to 5V.  Connect the black to ground.  Connect the green wire (DATA0) to Digital Pin 2 (INT0).  
 * Connect the white wire (DATA1) to Digital Pin 3 (INT1).  That's it!
 *
 * Operation is simple - each of the data lines are connected to hardware interrupt lines.  When
 * one drops low, an interrupt routine is called and some bits are flipped.  After some time of
 * of not receiving any bits, the Arduino will decode the data.  I've only added the 26 bit and
 * 35 bit formats, but you can easily add more.

*/

#include <Arduino.h>
#include "RFIDReaderWiegand.h"

extern RFIDReaderWiegand rdr; // in some other file thou shalt declare 'RFIDReaderWiegand rdr'

void RFIDReaderWiegand::Setup() {

  void ISR_INT0();
  void ISR_INT1();

  facilityCode = cardCode = error_code = 0;
  decode_done = false;

  pinMode(ledPin, OUTPUT);  // LED
  pinMode(D0Pin, INPUT);     // DATA0 (INT0)
  pinMode(D1Pin, INPUT);     // DATA1 (INT1)
  
  SerialUSB.begin(9600);
  SerialUSB.println("RFID Readers");
  
  // binds the ISR functions to the falling edge of INTO and INT1
  attachInterrupt(digitalPinToInterrupt(7), ISR_INT0, FALLING);  
  attachInterrupt(digitalPinToInterrupt(8), ISR_INT1, FALLING);
  

  weigand_counter = RFIDWR_WAIT_TIME;


}



// interrupt that happens when INTO goes low (0 bit)
void ISR_INT0()
{
  //SerialUSB.print("0");   // uncomment this line to u8g2 raw binary
  rdr.bitCount++;
  rdr.flagDone = 0;
  rdr.weigand_counter = RFIDWR_WAIT_TIME;  
  
}

// interrupt that happens when INT1 goes low (1 bit)
void ISR_INT1()
{
  //SerialUSB.print("1");   // uncomment this line to u8g2 raw binary
  rdr.databits[rdr.bitCount] = 1;
  rdr.bitCount++;
  rdr.flagDone = 0;
  rdr.weigand_counter = RFIDWR_WAIT_TIME;
}


void RFIDReaderWiegand::Chores(void) {

  // attempt to decode card when the ISRs indicate they have collected enough bits

  // bit position. 
  // 01001011101110111000010001
  // 01234567890123456789012345
  //           1111111111222222 

  // This waits to make sure that there have been no more data pulses before processing data
  //SerialUSB.print("l");
  if (!flagDone) {
    if (--weigand_counter == 0)
      flagDone = 1;	
    //SerialUSB.print("d");
  }

  // if we have bits and the weigand counter went out
  if (bitCount > 0 && flagDone) {
    unsigned char i;

    SerialUSB.print("Read ");
    SerialUSB.print(bitCount);
    SerialUSB.print(" bits. ");

    // we will decode the bits differently depending on how many bits we have
    // see www.pagemac.com/azure/data_formats.php for mor info
    if (bitCount == 35)
    {
      // 35 bit HID Corporate 1000 format
      // facility code = bits 2 to 14
      for (i=2; i<14; i++)
      {
        facilityCode <<=1;
        facilityCode |= databits[i];
      }

      // card code = bits 15 to 34
      for (i=14; i<34; i++)
      {
        cardCode <<=1;
        cardCode |= databits[i];
      }

      decode_done = true;
    }
    else if (bitCount == 26)
    {
      // standard 26 bit format
      // facility code = bits 2 to 9
      for (i=1; i<9; i++)
      {
        facilityCode <<=1;
        facilityCode |= databits[i];
      }

      // card code = bits 10 to 23
      for (i=9; i<25; i++)
      {
        cardCode <<=1;
        cardCode |= databits[i];
      }

      decode_done = true;

    }
    else {
      // you can add other formats if you want!
      error_code = 1;
    }

  }
}

void RFIDReaderWiegand::ReadComplete(void) {
  uint8_t i;
  // cleanup and get ready for the next card
  bitCount = 0;
  facilityCode = 0;
  cardCode = 0;
  for (i=0; i<RFIDWR_MAX_BITS; i++) 
  {
    databits[i] = 0;
  }
}
