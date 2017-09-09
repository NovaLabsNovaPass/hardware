#include <Arduino.h>

#define MAIN // causes vars in NovaPass.h to have storage declared for them
#include "NovaPass.h"

#include <SPI.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <WiFi101.h>
#include <ArduinoJson.h> 

#include "RFIDReaderWiegand.h"
RFIDReaderWiegand rdr; // Card Reader
#include "WiFiComms.h"
WiFiComms wificomms; // Wifi and Server

#include "Disp.h"
// Display
// SH1106 OLED on mkr1000 I2C
U8G2_SH1106_128X64_NONAME_1_SW_I2C u8g2(/*rotation*/U8G2_R0,/*clock*/12,/*data*/11,/*reset*/U8X8_PIN_NONE); 
Disp disp; // display

#include "Timers.h"
Timers timers;

#include "RestClient.h"
RestClient restclient;


uint16_t OneHzFlag;
uint32_t SysUpSecs;

#define USER_NAME_LEN 64
char user_name[USER_NAME_LEN];
#define TOOL_AUTHS_LEN 256
char tool_auths[TOOL_AUTHS_LEN];
boolean tool_on = false;

const char *api_key;
const char *tool_id;

void gen_hms(uint32_t Secs,char *char_buf);
void OneSecChores(void);
void process_card(void);
void connect_wifi(void);
boolean get_authorization(String card_id_str);

void setup()   {                
  // classic arduino setup(). Called once
  SerialUSB.begin(9600);

  timers.Setup();
  OneHzFlag = timers.GetOneHzFlag();
  SysUpSecs = 1;
 
  disp.Setup();

  // card reader
  rdr.ledPin = 6;
  rdr.D0Pin = 7;
  rdr.D1Pin = 8;
  rdr.Setup();

  // WiFi
  wificomms.network_name         = NOPA_WiFi_network_name;
  wificomms.network_password     = NOPA_WiFi_network_password;
  wificomms.server_name          = NOPA_server_name;
  wificomms.fallback_server_name = NOPA_fallback_server_name;

  wificomms.Setup();

  api_key = NOPA_api_key;
  tool_id = NOPA_tool_id;

  restclient.host = NOPA_server_name;
  restclient.port = NOPA_server_port;

  restclient.setContentType("application/json");
  restclient.num_headers = 0;


  disp.Clear();
  //             "1234567890123456"
  disp.Writeln(0,"NOVA PASS");
  disp.Writeln(2,"Ver 0.0");
  disp.Writeln(4,"Resetting..");
  disp.Show();
  delay(3000);
  disp.Clear();

  connect_wifi();

}

void loop() {
  // classic arduino top-level loop

  timers.Chores(); // interrupts will be setting periodic flags
  if (OneHzFlag & timers.OneHzFlags) {
    timers.OneHzFlags &= ~OneHzFlag;
    OneSecChores();
  }

  rdr.Chores(); // tend to any interrupts caused be card reader
  if (rdr.decode_done) {

    process_card();

    rdr.decode_done = false;
    rdr.ReadComplete();
  }
  if (rdr.error_code) {
    rdr.error_code = 0;
    rdr.ReadComplete();

    SerialUSB.println("Reader Decode Error");

    disp.Clear();
    disp.Writeln(0,"READER ERROR");
    disp.Show();
    delay(3000);
    disp.Clear();
  }
}

void gen_hms(uint32_t Secs,char *char_buf) {

  uint16_t DispDays, DispSecs, DispHours,DispMins;

  if (Secs == 0)
    Secs = 1;

  DispDays = Secs/ 86400;
  DispSecs = Secs %= 86400;
  DispHours = DispSecs / 3600;
  DispSecs %= 3600;
  DispMins = DispSecs / 60;
  DispSecs %= 60;

  snprintf(char_buf, 32, "   %04d:%02d:%02d:%02d", DispDays,DispHours,DispMins,DispSecs);
  return;

}

void OneSecChores(void) {
  static uint8_t ThreeHzCtr = 3;
  static uint8_t NinetySecCtr = 1;
  char uptime_buf[32];
  char tmp_buf[32];
  enum state { st_conn_wifi, st_stats, st_stats2 };
  static uint8_t state = st_conn_wifi;

  SysUpSecs++;

  if (!(--ThreeHzCtr)) {
    ThreeHzCtr = 3;

    if (WiFi.status() != WL_CONNECTED)
      state = st_conn_wifi;

    switch(state) { 

      case st_conn_wifi:
        disp.Clear();
        disp.Writeln(0,"WIFI");  
        if (WiFi.status() == WL_CONNECTED) {
          if (wificomms.got_disconnected) {
            disp.Writeln(1,"   RECONNECTED");
            wificomms.got_disconnected = false;
            log_to_server("wifi reconnect");
          } else 
            disp.Writeln(1,"   CONNECTED");
        } else {
          disp.Writeln(1,"   DISCONNECTED");
          wificomms.got_disconnected = true;
          connect_wifi();
        }

        gen_hms(SysUpSecs,uptime_buf);
        disp.Writeln(2,"SYS_UPTIME");
        disp.Writeln(3,uptime_buf);
        disp.Show();

        break;

      case st_stats:
        
        break;

      case st_stats2:
        break;
      default:
        break;
    }


  }

  if (!(--NinetySecCtr)) {
    NinetySecCtr = 15; // XXX shortened for debugging
    if (WiFi.status() == WL_CONNECTED) {
      gen_hms(SysUpSecs,uptime_buf);
      sprintf(tmp_buf,"sys_uptime:%s",uptime_buf);
      log_to_server(tmp_buf);
    }
  }

}



void process_card()
{

  String full_code_str =  String(rdr.facilityCode,DEC) + String(".") + String(rdr.cardCode,DEC);

  // facility code: 8 bits
  // card code: 16 bits
  // full code as in db <facility code>.<card code> example  12.37653
  if (get_authorization(full_code_str)) {
    // TODO: Prompt user to swipe again to turn tool on
    // TODO: If steward card, give option to swipe another card to be authorized
  }

  delay(3000);
  disp.Clear();

}

boolean get_authorization(String card_id_str) {

  char postdata[256];
  char tmp[32];
  char card_id_chbuf[32];


  //
  // Server responses cannot exceed this:
  //                           vvvv 
  DynamicJsonBuffer jsonBuffer(512);

  disp.Clear();
  disp.Writeln(0,"CARD_ID");
  disp.Writeln(1,"   " + card_id_str);
  disp.Writeln(2,"AUTHORIZATION");
  disp.Writeln(3,"   REQUESTING...");
  disp.Show();

  String response;
  strcpy(postdata,"{");
  sprintf(tmp,"\"api_key\":")   ;strcat(postdata, tmp);
  sprintf(tmp,"\"%s\",",api_key);strcat(postdata, tmp);
  sprintf(tmp,"\"tool_id\":")   ;strcat(postdata, tmp);
  sprintf(tmp,"\"%s\",",tool_id);strcat(postdata, tmp);
  sprintf(tmp,"\"card_id\":")   ;strcat(postdata, tmp);
  card_id_str.toCharArray(card_id_chbuf,32);
  sprintf(tmp,"\"%s\"",card_id_chbuf);strcat(postdata,tmp );
  strcat(postdata,"}");

  int statusCode = restclient.post("/novapass/api/v1.0/getauth", postdata, &response);

  if (statusCode == 0) {
    disp.Writeln(3,"   TIMEOUT");
    disp.Show();
    delay(1500);

  } else
    if (statusCode != 200) {
      disp.Writeln(3,"  FAIL");
      disp.Writeln(4,"  " + String(statusCode));
      disp.Show();
      delay(1500);
    } else {

      JsonObject& jsroot = jsonBuffer.parseObject(response); 
      if (!jsroot.success()) {
        disp.Writeln(3,"   FAIL ");
        disp.Writeln(4,"   JSON PARSE");
        disp.Show();
        delay(1500);
      } else {
        uint8_t js_error = jsroot["error"];
        const char * js_mesg = jsroot["message"];

        if (js_error != 0) {
          disp.Writeln(3,"   DENIED");
          disp.Writeln(4," " + String(js_mesg));
          disp.Show();
          delay(1500);
          return false;
        } else {
          disp.Writeln(3,"   GRANTED");
          strncpy(user_name,jsroot["user_name"],USER_NAME_LEN);
          strncpy(tool_auths,jsroot["tool_auths"],USER_NAME_LEN);
          disp.Show();
          delay(750);

          disp.Clear();
          disp.Writeln(0,"USER");
          disp.Writeln(1,"   " + String(user_name));
          disp.Writeln(2,"AUTHORIZATIONS");
          String str = String(tool_auths);
          str.replace(":",",");
          disp.Writeln(3,str);
          disp.Show();
          delay(1500);
          return true;
        }
      }
    }
    return false; // not reached
}

void log_to_server(const char *message) {

  char postdata[256];
  char tmp[31];
  //
  // Server responses cannot exceed this:
  //                           vvvv 
  DynamicJsonBuffer jsonBuffer(512);

  String response;
  //char postdata[] = "{ \"api_key\":\"1ECE22945F7A280E54B80E249D2B957E\", \"card_id\":\"0000000000000012\", \"tool_id\":\"test\" }";
  //int statusCode = restclient.post("/novapass/api/v1.0/getauth", postdata, &response);
  strcpy(postdata,"{");
  sprintf(tmp,"\"api_key\":")   ;strcat(postdata, tmp);
  sprintf(tmp,"\"%s\",",api_key);strcat(postdata, tmp);
  sprintf(tmp,"\"tool_id\":")   ;strcat(postdata, tmp);
  sprintf(tmp,"\"%s\",",tool_id);strcat(postdata, tmp);
  sprintf(tmp,"\"message\":")   ;strcat(postdata, tmp);
  sprintf(tmp,"\"%s\"",message);strcat(postdata, tmp);
  strcat(postdata,"}");

  disp.Clear();
  disp.Writeln(0,"SERVER");
  disp.Writeln(1,String("   ") + String(wificomms.server_name));
  disp.Writeln(2,"STATUS");
  disp.Writeln(3,"   Connecting..");
  disp.Show();

  int statusCode = restclient.post("/novapass/api/v1.0/log", postdata, &response);

  if (statusCode == 0) {
    disp.Writeln(3,"   TIMEOUT");
    disp.Show();
    delay(1500);

  } else
    if (statusCode != 200) {
      disp.Writeln(3,"  FAIL");
      disp.Writeln(4,"  " + String(statusCode));
      disp.Show();
      delay(1500);
    } else {

      JsonObject& jsroot = jsonBuffer.parseObject(response); 
      if (!jsroot.success()) {
        disp.Writeln(3,"   FAIL ");
        disp.Writeln(4,"   JSON PARSE");
        disp.Show();
        delay(1500);
      } else {
        uint8_t js_error = jsroot["error"];
        const char * js_mesg = jsroot["message"];

        if (js_error != 0) {
          disp.Writeln(3,"   ERROR");
          disp.Writeln(4," " + String(js_mesg));
          disp.Show();
          delay(1500);
        } else {
          disp.Writeln(3,"   OK");
          disp.Show();
          delay(1500);
        }
      }
    }

}

void connect_wifi(void) {

  //             "1234567890123456"
  disp.Writeln(0,"WIFI NETWORK");
  disp.Writeln(1,wificomms.network_name);
  disp.Writeln(2,"STATUS");
  disp.Writeln(3," CONNECTING");
  disp.Show();
  wificomms.Connect();

  if (WiFi.status() == WL_CONNECTED) {
    disp.Writeln(3," OK. MY IP");
    IPAddress ipAddress = WiFi.localIP();
    disp.Writeln(4, String(ipAddress[0]) + String(".") +\
        String(ipAddress[1]) + String(".") +\
        String(ipAddress[2]) + String(".") +\
        String(ipAddress[3])  );
    disp.Show();
    delay(1500);

    log_to_server("reset");

  } else {
    disp.Writeln(3,"  FAIL");
    disp.Show();
    delay(3000);
    disp.Clear();
  }
}
