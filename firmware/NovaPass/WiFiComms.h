class WiFiComms {
  uint32_t status;
  public:
    void Setup();
    void Connect();
    const char *network_name;
    const char *network_password;
    const char *server_name;
    const char *fallback_server_name;
    IPAddress dns1,dns2;
    boolean got_disconnected;

};
