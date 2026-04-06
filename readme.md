# RelayESP-FailSafeNet

## Description

RelayESP-FailSafeNet is an ESP8266/ESP01-based project that monitors Internet connectivity and controls a relay in case of network failure.
The device continuously tests WiFi connectivity and performs ping tests to primary and secondary servers. 
If the network is unreachable for a configurable number of attempts, it triggers a relay (e.g., to restart a router or activate an alarm).

The project also provides a web interface to:

- Display system status and failure counts
- View logs (protected)
- Export and import configuration stored in LittleFS (protected) 

## Screenshot 

Interface : 
![Home Interface Photo](https://raw.githubusercontent.com/libre/relayesp-failsafenet/master/images/webui1.png)

Logs : 
![Home Logs Photo](https://raw.githubusercontent.com/libre/relayesp-failsafenet/master/images/webui2.png)

Export / Import configuration :
![Home Export Photo](https://raw.githubusercontent.com/libre/relayesp-failsafenet/master/images/webui3.png)


## Features

- Automatic WiFi connection check
- Ping tests to primary and secondary servers
- Failure counter and relay trigger after configurable threshold
- Non-blocking relay management with configurable duration
- Recovery mode with delay before resuming checks
- Web interface for logs and configuration management
- Secured Backup and restore configuration via LittleFS


## Hardware Requirements

- ESP-01
- Power supply 220v > 5v/500mA. 
- 5v-compatible relay
- Wiring for relay (VCC, GND, IN)
- WiFi access
- Libraries Used
- ESP8266WiFi
- ESP8266WebServer
- LittleFS
- ESP8266Ping

## Installation

Clone or download this repository into your Arduino workspace.
Install the required libraries listed above via Arduino Library Manager.

Configure parameters in conf.txt on LittleFS or via the web interface:

`
SETUPMODE=0				
MONITORACTIVED=1
SSID=YourWiFiSSID
WIRLESSPASSWORD=YourWiFiPassword
PRIMARYPING=8.8.8.8
SECONDARYPING=1.1.1.1
TEST_INTERVAL=30000
MAX_FAIL_COUNT=10
RELAY_DURATION=20000
RECOVERY_DELAY=1500000
NTPSERVER=pool.ntp.org
GMTOFFSET_SEC=3600
DAYLIGHTOFFSET=0
DEBUGSERIAL=1
WEBPASS=changeme
`

Flash the code to your ESP8266.
Connect the relay to the pin defined by RELAY_PIN.


## Configuration file 

- SETUPMODE=1                       // to active AP Setup mode. Device IP is 192.168.4.1 			
- MONITORACTIVED=1					// to active monitoring process
- SSID=YourWiFiSSID					// Your Wireless info 
- WIRLESSPASSWORD=YourWiFiPassword	// YOur Pass Wireless info 
- PRIMARYPING=8.8.8.8				// First Ping test 
- SECONDARYPING=1.1.1.1				// First not responding, use secondary ping test. 
- TEST_INTERVAL=30000				// Test every 30sec 
- MAX_FAIL_COUNT=10					// Test faild after 50x 30sec 
- RELAY_DURATION=20000				// Active relay 10sec 
- RECOVERY_DELAY=1500000			// Recovery delay 25min for restart test processing. 
- NTPSERVER=pool.ntp.org			// NTP server 
- GMTOFFSET_SEC=3600				// GMT Offset 
- DAYLIGHTOFFSET=0					// Day light of set 
- DEBUGSERIAL=1						// Active debug mode (Serial and Log buffer web)  
- WEBPASS=changeme					// Password for Webinterface. 


## Security 

WEBPASS by default user: admin / password: changeme
Store password md5 on the Cookie 

## Detect pass Wifi not work. 

Not good password, disable monitoring and active AP Mode for change setup. 

## Usage

Access the web interface via the ESP8266 IP address (shown in Serial Monitor):

- **/ :** View status and failure counters
- **/logs :** View logs
- **/exportimport :** Export or import configuration

The relay automatically triggers after MAX_FAIL_COUNT consecutive ping or WiFi failures.
The system returns to normal after RECOVERY_DELAY.

## Technical Notes

headerhtml() and footerhtml() functions return HTML as String.
LittleFS is used to save and restore configuration (conf.txt).
Pings use the ESP8266Ping library, which requires const char* arguments.

RELAY use PIN 0 for ESP01 and RelayPCB v 4.0. 

## Mounting usage. 

![Home Mounting Photo](https://raw.githubusercontent.com/libre/relayesp-failsafenet/master/images/mount1.jpg)

![Home Mounting Photo](https://raw.githubusercontent.com/libre/relayesp-failsafenet/master/images/mount2.jpg)

![Home Mounting Photo](https://raw.githubusercontent.com/libre/relayesp-failsafenet/master/images/mount3.jpg)

![Home Mounting Photo](https://raw.githubusercontent.com/libre/relayesp-failsafenet/master/images/mount4.jpg)

![Home Mounting Photo](https://raw.githubusercontent.com/libre/relayesp-failsafenet/master/images/mount5.jpg)

## License

This project is open-source and free to use and modify.