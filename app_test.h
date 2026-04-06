
/**
 * Project name   : RelayESP-FailSafeNet 
 * Tests  
 *
 */

// PING 
void pingTest(unsigned long now) {
  bool ok = false;

  addLog("Ping test");

  if (Ping.ping(primaryPing.c_str(), 3)) {
    addLog("Ping OK primary");
	DebugSerialln("Ping OK primary");
    ok = true;
  }
  else if (Ping.ping(secondaryPing.c_str(), 3)) {
    addLog("Ping OK secondary");
	DebugSerialln("Ping OK secondary");
    ok = true;
  }
  else {
    addLog("Ping FAIL");
	DebugSerialln("Ping FAIL");
  }

  if (ok) {
    pingFailCounter = 0;
    currentState = STATE_OK;
  } else {
    pingFailCounter++;

    if (pingFailCounter >= MAX_FAIL_COUNT) {
      addLog("Max fail reached -> RELAY");
	  DebugSerialln("Max fail reached -> RELAY");
      triggerRelay(now);
      currentState = STATE_RECOVERY;
      stateStartTime = now;
    } else {
      currentState = STATE_FAILING;
    }
  }
}


// Test Wifi 
void handleWiFi(unsigned long now) {
  static unsigned long lastReconnect = 0;
  const unsigned long RECONNECT_DELAY = 10000; // 10s

  bool ok = (WiFi.status() == WL_CONNECTED);

  addLog("Wifi test");

  if (ok) {
    addLog("Wifi OK");
	DebugSerialln("Wifi OK");
    // Log IP
    String ipStr = "IP Address: " + WiFi.localIP().toString();
    addLog(ipStr.c_str());  // This will also show up in your logBuffer
	DebugSerialln(ipStr);
    // NTP une seule fois
    static bool ntpConfigured = false;
    if (!ntpConfigured) {
      configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

      unsigned long start = millis();
      while (!time(nullptr) && millis() - start < 3000) {
        delay(200);
      }

      ntpConfigured = true;
      addLog("NTP synchronized");
	  DebugSerialln("NTP synchronized");
    }

    // reset fail global seulement si tu veux être strict
    wifiFailCounter = 0;
  }
  else {
    addLog("Wifi FAIL");
	DebugSerialln("Wifi FAIL");
	
	// Password ESSID Fail, Starting AP Mode. 
	if (WiFi.status() == WL_CONNECT_FAILED) {
		addLog("Password not good");
		DebugSerialln("Password not good");	
		monitoring = "0";
		setupmode = "1";
		ssid = "APSetup-FailSafeNET";
		WiFi.mode(WIFI_AP);
		WiFi.softAP(ssid);	
		return; 
	}
	
    // reconnect throttlé
    if (now - lastReconnect > RECONNECT_DELAY) {
      lastReconnect = now;

      WiFi.disconnect();
	  delay(150);
      WiFi.begin(ssid, password);
	  DebugSerialln("Reconnecting WiFi...");
      addLog("Reconnecting WiFi...");
    }

    wifiFailCounter++;

    if (wifiFailCounter >= MAX_FAIL_COUNT) {
      addLog("Max fail reached -> RELAY");
	  DebugSerialln("Max fail reached -> RELAY");
      triggerRelay(now);

      currentState = STATE_RECOVERY;
      stateStartTime = now;
    } else {
      currentState = STATE_FAILING;
    }
  }
}
