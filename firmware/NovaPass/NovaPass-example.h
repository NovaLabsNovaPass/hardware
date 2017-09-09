#ifdef MAIN
  #define EXTERN
#else
  #define EXTERN extern
#endif

//#define NOPA_USING_SSL
#ifdef NOPA_USING_SSL
  #define NOPA_PORT 443
#else
  #define NOPA_PORT 80
#endif

//
// convention: Prefix globals with NOPA_
//

EXTERN const char      NOPA_WiFi_network_name[]     = "your_network_name";    // SSID of WLAN
EXTERN const char      NOPA_WiFi_network_password[] = "your_network_pw";     // WLAN password
EXTERN const char      NOPA_fallback_server_name[]  = "";                     // Fallback Authentication Server
EXTERN const char      NOPA_api_key[]               = "your_api_key";     // API key. Must match server's
EXTERN const char      NOPA_tool_id[]               = "tool_name";            // this tool's name

EXTERN const char      NOPA_server_name[]           = "your_server_name"; // Authentication Server
EXTERN const uint16_t  NOPA_server_port             = NOPA_PORT
