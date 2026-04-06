/**
 *
 * Project name   : RelayESP-FailSafeNet 
 * Board real     : ESP01 + Relay V4.0  
 * Boardconfig    : Generic ESP8266 
 * Flash size  	  : 1Mb -> 512Kb FS / 246Kb OTA 
 * Drivers 	UART  : CH340 v3.4 
 * PCB Relay 	  : v4.0 
 * Author         : https://github.com/libre 
 * Projct URL     : https://github.com/libre/relayesp-failsafenet 
 *
 */

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <FS.h>
#include <LittleFS.h>   				// ESP8266 LittleFS
#include <time.h>
#include <ESP8266Ping.h>   				// https://github.com/libre/ESP8266Ping
#include <MD5Builder.h>     			// https://github.com/esp8266/Arduino/blob/master/cores/esp8266/MD5Builder.h
#define RELAY_PIN 0

File fsUploadFile;              		// a File object to temporarily store the received file
String getContentType(String filename); // convert the file extension to the MIME type
void handleImportConfig();              // upload a new file to the SPIFFS

MD5Builder _md5;

// Gobale variables. 
#include "variables.h"

// ================= CONFIG INIT =================
String ssid = "default"; // Freebox-78F543
String password = "xxxx"; // 49545qmd3wqr3f65q2hz4k
String primaryPing = "8.8.8.8";
String secondaryPing = "1.1.1.1";
unsigned long TEST_INTERVAL = 30000;
int MAX_FAIL_COUNT = 10;
unsigned long RELAY_DURATION = 20000;
unsigned long RECOVERY_DELAY = 1500000;
String ntpServer = "pool.ntp.org";
unsigned long gmtOffset_sec = 3600;
int daylightOffset_sec = 0;
String global_debugserial = "0";
String webpass = "changeme";
String CookiePass = "";
String Cookie = "";
String monitoring = "0";
String setupmode = "1";
// ================= SERVER HTTP =================
ESP8266WebServer server(80);

// ================= LOG =========================
#define LOG_SIZE 1024

char logBuffer[LOG_SIZE];
int logIndex = 0;

// ================= STATE =======================
unsigned long lastTestTime = 0;
unsigned long stateStartTime = 0;

unsigned long relayStart = 0;
bool relayActive = false;

int wifiFailCounter = 0;
int pingFailCounter = 0;

enum State {
  STATE_OK,
  STATE_FAILING,
  STATE_RECOVERY
};

State currentState = STATE_OK;


// Utils
#include "utils.h"

// FS
#include "fs.h"

// Webserver 
#include "webserver.h"

// Master App 
#include "app_relay.h"

#include "app_test.h"

/**
 *
 * Master Setup  
 *
 */
 
void setup() {
	
	if (LittleFS.begin()) {
		Serial.println(F("[Init] SPIFFS system mounted with success"));
	} else {
		Serial.println(F("[Init] SPIFFS mount failed, formatting..."));
		if (LittleFS.format()) {  // Formate le FS
			Serial.println(F("[Init] SPIFFS formatted successfully, rebooting..."));
			delay(5000);
			ESP.restart();
		} else {
			Serial.println(F("[Init] SPIFFS format failed, cannot continue."));
			while (true) delay(1000); // bloquer
		}
	}

	// Check file exist.
	File fileCfg = LittleFS.open("/conf.txt", "r");
	if (!fileCfg) {
		Serial.println("INIT FILES FAILD, Wait reboot.");
		delay(15000);
		ESP.restart();
	}


	fs::Dir root = getRootDir();
	if (!root.next()) {
		Serial.println("No files found in root");
		delay(15000);
		ESP.restart();		
	}
	while (root.next()) {     // retourne true tant qu'il y a un fichier
		Serial.print("File: ");
		Serial.print(root.fileName());
		Serial.print(" | Size: ");
		Serial.println(root.fileSize());
	}
	// File fileConfig = LittleFS.open("/conf.txt", "r");
    // Loading config. 
    ssid = getconfigdata("SSID", "/conf.txt");
    password = getconfigdata("WIRLESSPASSWORD", "/conf.txt");
    primaryPing = getconfigdata("PRIMARYPING", "/conf.txt");
    secondaryPing = getconfigdata("SECONDARYPING", "/conf.txt");
    TEST_INTERVAL = getconfigdata("TEST_INTERVAL", "/conf.txt").toInt();;
    MAX_FAIL_COUNT = getconfigdata("MAX_FAIL_COUNT", "/conf.txt").toInt();
    RELAY_DURATION = getconfigdata("RELAY_DURATION", "/conf.txt").toInt();
    RECOVERY_DELAY = getconfigdata("RECOVERY_DELAY", "/conf.txt").toInt();
    ntpServer = getconfigdata("NTPSERVER", "/conf.txt");
    gmtOffset_sec = getconfigdata("GMTOFFSET_SEC", "/conf.txt").toInt();
    daylightOffset_sec = getconfigdata("DAYLIGHTOFFSET", "/conf.txt").toInt();
	global_debugserial = getconfigdata("DEBUGSERIAL", "/conf.txt");
	webpass = getconfigdata("WEBPASS", "/conf.txt");
	monitoring = getconfigdata("MONITORACTIVED", "/conf.txt"); 
	setupmode = getconfigdata("SETUPMODE", "/conf.txt"); 
	// Init Relay pinmode 
	pinMode(RELAY_PIN, OUTPUT);
	digitalWrite(RELAY_PIN, HIGH);
	DebugSerialln("Config file loaded");
	Serial.begin(115200);

	if ( setupmode.equals("1")) {
		const char* ssid     = "APSetup-FailSafeNET";
		WiFi.mode(WIFI_AP);
		WiFi.softAP(ssid);	
	}
	else 
	{
		WiFi.begin(ssid.c_str(), password.c_str());
		// ne pas bloquer ici
		addLog("WiFi connecting...");
		// NTP
		configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
		// attendre synchro NTP
		unsigned long start = millis();
		while (!time(nullptr) && millis() - start < 5000) {
		  delay(200);
		}
	}

	// INIT HTTP
	server.on("/", handleRoot);
	server.on("/logs", handleLogs);
	server.on("/login", handleLogin);
	server.on("/exportimport", handleImportExportPage);
	server.on("/export_config.txt", handleExportConfig);
	// Upload Restore config.
	server.on("/fupload", HTTP_POST,  // if the client posts to the upload page
	[](){ server.send(200); },                // Send status 200 (OK) to tell the client we are ready to receive
	handleImportConfig                        // Receive and save the file
	);
	server.on("/css/bootstrap.min.css", []()
		{ // serves css file
			if (LittleFS.exists("/bs.css"))
			{
			  File file = LittleFS.open("/bs.css", "r");
			  size_t sent = server.streamFile(file, "text/css");
			  file.close();
			}
			else
			{
			  server.send(404, "text/html", "what file?");
			}
		}
	);	
	server.on("/css/main.css", []()
		{ // serves css file
			if (LittleFS.exists("/main.css"))
			{
			  File file = LittleFS.open("/main.css", "r");
			  size_t sent = server.streamFile(file, "text/css");
			  file.close();
			}
			else
			{
			  server.send(404, "text/html", "what file?");
			}
		}
	);			
	server.on("/js/bootstrap.min.js", []()
		{ // serves css file
			if (LittleFS.exists("/bs.js"))
			{
			  File file = LittleFS.open("/bs.js", "r");
			  size_t sent = server.streamFile(file, "text/css");
			  file.close();
			}
			else
			{
			  server.send(404, "text/html", "what file?");
			}
		}
	);		
	server.on("/js/popper.min.js", []()
		{ // serves css file
			if (LittleFS.exists("/pop.js"))
			{
			  File file = LittleFS.open("/pop.js", "r");
			  size_t sent = server.streamFile(file, "text/css");
			  file.close();
			}
			else
			{
			  server.send(404, "text/html", "what file?");
			}
		}
	);		
	const char * headerkeys[] = {"Cookie"};
	size_t headerkeyssize = 1;

	server.collectHeaders(headerkeys, headerkeyssize);
	server.begin();
	addLog("System started");
	DebugSerialln("System started");
}


/**
 *
 * Master Loop
 * 
 */
 
void loop() {
    unsigned long now = millis();
    server.handleClient();
	if ( monitoring.equals("1") && setupmode.equals("0")) {
		handleState(now);

		// Bloquer les tests pendant la recovery
		if (currentState == STATE_RECOVERY) return;

		// Test WiFi + Ping selon TEST_INTERVAL
		if (now - lastTestTime >= TEST_INTERVAL) {
			lastTestTime = now;

			handleWiFi(now); // test WiFi et mise à jour wifiFailCounter

			if (WiFi.status() == WL_CONNECTED) {
				addLog("WiFi OK -> Ping test");
				DebugSerialln("WiFi OK -> Ping test");
				pingTest(now); // ping seulement si WiFi OK
			} else {
				addLog("Skip ping (no WiFi)");
				DebugSerialln("Skip ping (no WiFi)");
				// Vérification explicite : WiFi KO depuis MAX_FAIL_COUNT tests ?
				if (wifiFailCounter >= MAX_FAIL_COUNT) {
					addLog("WiFi unreachable -> trigger RELAY");
					DebugSerialln("WiFi unreachable -> trigger RELAY");
					triggerRelay(now);
					currentState = STATE_RECOVERY;
					stateStartTime = now;
				}
			}
		}
	}

}
