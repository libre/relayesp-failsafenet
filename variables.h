#ifndef GLOBAL_VARIABLES_H
#define GLOBAL_VARIABLES_H

extern String ssid; // Freebox-78F543
extern String password;
extern String primaryPing;
extern String secondaryPing;
extern unsigned long TEST_INTERVAL;
extern int MAX_FAIL_COUNT;
extern unsigned long RELAY_DURATION;
extern unsigned long RECOVERY_DELAY;
extern String ntpServer;
extern unsigned long gmtOffset_sec;
extern int daylightOffset_sec;
extern String global_debugserial;
extern String webpass;
#define LOG_SIZE 1024
extern char logBuffer[LOG_SIZE];
extern int logIndex;
extern unsigned long lastTestTime;
extern unsigned long stateStartTime;
extern unsigned long relayStart;
extern bool relayActive;
extern int wifiFailCounter;
extern int pingFailCounter;
extern String CookiePass;
extern String Cookie;
extern String monitoring;
extern String setupmode;
#endif