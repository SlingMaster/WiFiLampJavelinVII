/* TelnetManager.ino */
#if defined(GENERAL_DEBUG) && GENERAL_DEBUG_TELNET

void handleTelnetClient() {
  if (telnetServer.hasClient()) {
    if (!telnet || !telnet.connected()) {
      if (telnet) {
        telnet.stop();
        telnetGreetingShown = false;
      }
      telnet = telnetServer.available();
    } else {
      telnetServer.available().stop();
      telnetGreetingShown = false;
    }
  }

  if (telnet && telnet.connected() && telnet.available()) {
    if (!telnetGreetingShown) {
      telnet.println("Підключення до пристрою по протоколу telnet встановлено\n\r");
      telnet.println("Connection to the device via telnet protocol established\n\r");
      telnetGreetingShown = true;
    }
  }
}

#endif
