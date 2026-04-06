/**
 * Project name   : RelayESP-FailSafeNet 
 * Web server HTTP 
 *
 */
String getCookieFailSafeNet(String cookie) {

  int start = cookie.indexOf("FAILSAFENET=");
  if (start == -1) return "";

  start += strlen("FAILSAFENET=");

  int end = cookie.indexOf(";", start);
  if (end == -1) end = cookie.length();

  String value = cookie.substring(start, end);
  value.trim();  // supprime espace éventuel

  return value;
}

String headerhtml() {
  String headerhtml = R"rawliteral(
<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>FailSafeNet</title>

  <link href="css/bootstrap.min.css" rel="stylesheet">
  <link href="css/main.css" rel="stylesheet">
</head>

<body>
<nav class="navbar navbar-expand-md navbar-dark fixed-top bg-dark">
        <a class="navbar-brand" href="#">FailSafeNet</a>
        <button class="navbar-toggler" type="button" data-toggle="collapse" data-target="#navbarCollapse" aria-controls="navbarCollapse" aria-expanded="false" aria-label="Toggle navigation">
          <span class="navbar-toggler-icon"></span>
        </button>
        <div class="collapse navbar-collapse" id="navbarCollapse">
          <ul class="navbar-nav mr-auto">
			<li class="nav-item">
				<a class="nav-link" href="/">Home</a>
			</li>		
			<li class="nav-item">
				<a class="nav-link" href="/logs">Logs</a>
			</li>
          </ul>
		<a class="btn btn-light" href="/exportimport">Configure</a>	
        </div>
      </nav>)rawliteral";
return headerhtml; 
}	

String footerhtml() {
  String footerhtml = R"rawliteral(
<footer class="footer">
      <div class="container">
        <span class="text-muted">Copyleft RelayESP-FailSafeNet | Author : <a target="_blank" href="https://github.com/libre/relayesp-failsafenet">Github/libre</a></span>
      </div>
    </footer>
	<script src="https://code.jquery.com/jquery-3.2.1.slim.min.js" integrity="sha384-KJ3o2DKtIkvYIK3UENzmM7KCkRr/rE9/Qpg6aAZGJwFDMVNA/GpGFF93hXpG5KkN" crossorigin="anonymous"></script>
	<script src="js/bootstrap.min.js"></script>
	<script src="js/popper.min.js"></script>
</body>
</html>)rawliteral";
  return footerhtml; 
}

// Set CrossOrigin security
void setCrossOrigin(){
   server.sendHeader(F("Access-Control-Allow-Origin"), F("*"));
   server.sendHeader(F("Access-Control-Max-Age"), F("600"));
   server.sendHeader(F("Access-Control-Allow-Methods"), F("PUT,POST,GET,OPTIONS"));
   server.sendHeader(F("Access-Control-Allow-Headers"), F("*"));
};

// Set CrossOrigin security
void sendCrossOriginHeader(){
   Serial.println(F("sendCORSHeader"));
   server.sendHeader(F("access-control-allow-credentials"), F("false"));
   setCrossOrigin();
   server.send(204);
}

// Root page
void handleRoot() {
	DebugSerialln("Enter Root page");
	String page = headerhtml();
	page += R"rawliteral(
	<div class="container mt-4">
	  <h3>Internet Monitoring</h3>
	  <p>
	  <b>Wirless :</b> )rawliteral" + String(ssid) + R"rawliteral(
	  <b>State:</b> )rawliteral" + String(currentState) + R"rawliteral(
	  <b>Fail count:</b> )rawliteral" + String(pingFailCounter) + R"rawliteral(
	  <b>Date:</b> )rawliteral" + getTimeString() + R"rawliteral(
	  </p>
	</div>)rawliteral";
	page += footerhtml();
	server.send(200, "text/html", page);
}

// Authentification Webapp.
bool is_authentified() {
  DebugSerialln("Enter is_authentified");
  String CookiePass = "";
  if (server.hasHeader("Cookie")) {
    DebugSerial("Found cookie: ");
    String cookie = server.header("Cookie");
    DebugSerialln(cookie);
    CookiePass = getCookieFailSafeNet(cookie);
    DebugSerialln("CookiePass :" + CookiePass);
    if (CookiePass.equals(md5(webpass))) {
      DebugSerialln("Authentification Successful");
      return true;
    }
  }
  DebugSerialln("PassMD5 :" + md5(webpass));
  DebugSerialln("PassMD5 Cookie :" + CookiePass);
  DebugSerialln("Authentification Failed");
  return false;
}


// Log page
void handleLogs() {
	if (!is_authentified()){
	  server.sendHeader("Location","login");
	  server.sendHeader("Cache-Control","no-cache");
	  server.send(301);
	  return;
	}
	DebugSerialln("Enter handleLogs");
	server.send(200, "text/plain", logBuffer);
	}

// Page of Backup/Restore (Auth only)
void handleImportExportPage() {
	if (!is_authentified()){
	  server.sendHeader("Location","login");
	  server.sendHeader("Cache-Control","no-cache");
	  server.send(301);
	  return;
	}
	DebugSerialln("Enter handleImportExportPage");
	String header;
	String content = headerhtml();
	content += "<header class=\"jumbotron text-left\" style=\"padding-top: 8rem\">";
	content += "<div class=\"container\">";
	content += "<h3>Backup configuration</h3>";
	content += "<a href=\"export_config.txt\" class=\"btn btn-default mb-2\"><i class=\"fa fa-wrench\"></i>&nbsp; Export Config</a>";
	content += "<h3>Restore configuration</h3>";
	content += "<form id=\"1\" method=\"post\" action='fupload' enctype=\"multipart/form-data\">";
	content += "    <input type=\"file\" name=\"name\">";
	content += "    <input class=\"button\" type=\"submit\" value=\"Upload\">";
	content += "</form>";
	content += "</div></div></header>";
	content += footerhtml();
	server.send(200, "text/html", content);
}

// IOT Process for restore config file.  (Auth only)
void handleImportConfig() {
	if (!is_authentified()){
	  server.sendHeader("Location","login");
	  server.sendHeader("Cache-Control","no-cache");
	  server.send(301);
	  return;
	}
	DebugSerialln("Enter handleImportConfig");
	String header;
	HTTPUpload& upload = server.upload();
	if(upload.status == UPLOAD_FILE_START){
	  String filename = upload.filename;
	  if(!filename.startsWith("/")) filename = "/newconf.txt";
		Serial.print("handleImportConfig Name: "); Serial.println(filename);
		fsUploadFile = LittleFS.open(filename, "w");            // Open the file for writing in LittleFS (create if it doesn't exist)
		filename = String();
	  } else if(upload.status == UPLOAD_FILE_WRITE){
		if(fsUploadFile)
		fsUploadFile.write(upload.buf, upload.currentSize); // Write the received bytes to the file
	  } else if(upload.status == UPLOAD_FILE_END){
		if(fsUploadFile) {                                    // If the file was successfully created
		  fsUploadFile.close();                               // Close the file again
		  DebugSerial("handleImportConfig Size: ");
		  DebugSerialln(String(upload.totalSize));
		  // File file = LittleFS.open("/cft/rw.raw", "r");
		  LittleFS.remove("/conf.txt");
		  LittleFS.rename("/newconf.txt", "/conf.txt");
		  server.sendHeader("Location","success");      // Redirect the client to the success page
		  server.send(303);
		  DebugSerialln("Restarting node");
		  delay(1000);
		  ESP.restart();
	  } else {
		server.send(500, "text/plain", "500: couldn't create file");
	  }
	}

}

// Export configuration file. (Auth only)
void handleExportConfig() {
	if (!is_authentified()){
	  server.sendHeader("Location","login");
	  server.sendHeader("Cache-Control","no-cache");
	  server.send(301);
	  return;
	}
	DebugSerialln("Enter handleExportConfig");
	String header;
	String str = "";
    File f = LittleFS.open("/conf.txt", "r");
    if (!f) {
	  DebugSerialln("Can't open LittleFS file !\r\n");
    }
    else 
    {
	  server.streamFile(f, "application/octet-stream");
	  f.close();
	  server.sendHeader("Location","dashboard");
	  server.sendHeader("Cache-Control","no-cache");
	  server.send(301);
	}
}

// Page of Login
void handleLogin(){
 setCrossOrigin();
 String msg;
 String header;
 if (server.hasHeader("Cookie")){
   DebugSerial("Found cookie: ");
   String cookie = server.header("Cookie");
   DebugSerialln(cookie);
 }
 if (server.hasArg("DISCONNECT")){
   DebugSerialln("Disconnection");
   server.sendHeader("Location","login");
   server.sendHeader("Cache-Control","no-cache");
   server.sendHeader("Set-Cookie","ESPSESSIONID=0");
   server.send(301);
   return;
 }
 if (server.hasArg("USERNAME") && server.hasArg("PASSWORD")){
    // String RecevedPassword = md5(server.arg("PASSWORD"));
	String RecevedPassword = server.arg("PASSWORD");
    // Replace par une vérification de la valeur global à l'initalisation.
	if ( server.arg("USERNAME") == "admin" && RecevedPassword.equals(webpass)) {
	  server.sendHeader("Location","/");
	  server.sendHeader("Cache-Control","no-cache");
	  String ContructCookie = "FAILSAFENET=";
	  ContructCookie += md5(webpass);
	  server.sendHeader("Set-Cookie", ContructCookie);
	  server.send(302,"text/plain","");
	  Serial.println("Log in Successful");
	  return;
	}
    msg = "Wrong username/password! try again.";
    DebugSerialln("Password receved and md5 parsed:");
    DebugSerialln(RecevedPassword);
    DebugSerialln("Log in Failed");
 }
 String content = headerhtml();
 content += "<header class=\"jumbotron text-center\" style=\"padding-top: 8rem\">";
 content += "<div class=\"container\">";
 content += "<br>" + msg + "<br>";
 content += "<br>";
 content += "<form action='login' method='IOTT'>";
 content += "<label>Username</label><br>";
 content += "<input type='text' name='USERNAME' placeholder='admin'><br>";
 content += "<label>Password</label><br>";
 content += "<input type='password' name='PASSWORD' placeholder='password'><br>";
 content += "<input type='submit' name='SUBMIT' value='Submit'></form>";
 content += "</body></center></html>";
 content += "<div></div></header>";
 content += footerhtml();
 server.send(200, "text/html", content);
}

