/**
 * Project name   : RelayESP-FailSafeNet 
 * Utils Functions 
 *
 */
 
// Get time to String. 
String getTimeString() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) return "no-time";

  char buf[32];
  strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &timeinfo);
  return String(buf);
}

// Add to log 
void addLog(const char* msg) {
  char timeBuf[32];
  struct tm timeinfo;

  if (!getLocalTime(&timeinfo)) {
    strcpy(timeBuf, "no-time");
  } else {
    strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%d %H:%M:%S", &timeinfo);
  }

  char line[128];
  snprintf(line, sizeof(line), "%s | %s\n", timeBuf, msg);

  for (size_t i = 0; i < strlen(line); i++) {
    logBuffer[logIndex++] = line[i];

    if (logIndex >= LOG_SIZE - 1) {
      logIndex = 0;
    }
  }

  logBuffer[logIndex] = '\0'; // terminaison
}

// Reboot
void restart_device() {
  delay(1000);
  ESP.restart();
}

// STATE 
void handleState(unsigned long now) {
  if (currentState == STATE_RECOVERY) {
	if (now - stateStartTime >= RECOVERY_DELAY) {
		addLog("Recovery done");
		pingFailCounter = 0;
		wifiFailCounter = 0;
		currentState = STATE_OK;
		lastTestTime = now;   // IMPORTANT
    }
  } 

  // gestion non bloquante du relais
  if (relayActive && (now - relayStart >= RELAY_DURATION)) {
    digitalWrite(RELAY_PIN, HIGH);
    relayActive = false;
  }
}

// Genereting MD5 password
String md5(String str) {
  _md5.begin();
  _md5.add(String(str));
  _md5.calculate();
  return _md5.toString();
}

/**
 *
 * Debug mode 
 *
 */
 
void DebugSerial(const String& value) {
  if ( global_debugserial.equals("1")) {
    Serial.print(value);
  }
}

void DebugSerialln(const String& value) {
  if ( global_debugserial.equals("1")) {
    Serial.println(value);
  }
}
