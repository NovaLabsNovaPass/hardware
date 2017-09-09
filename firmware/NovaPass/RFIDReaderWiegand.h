//
// RFIDReaderWiegand.h
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

#define RFIDWR_MAX_BITS 100                 // max number of bits 
#define RFIDWR_WAIT_TIME  3000      // time to wait for another weigand pulse.  

class RFIDReaderWiegand {

  public:

  uint8_t databits[RFIDWR_MAX_BITS];    // stores all of the data bits
  uint8_t bitCount;              // number of bits currently captured
  uint8_t flagDone;              // goes low when data is currently being captured
  uint16_t weigand_counter;        // countdown until we assume there are no more bits

  uint32_t facilityCode;        // decoded facility code
  uint32_t cardCode;            // decoded card code

  uint8_t ledPin,D0Pin,D1Pin;

  uint8_t error_code;
  boolean decode_done;

  void Setup(void);
  void Chores(void);
  void ReadComplete(void);

};



