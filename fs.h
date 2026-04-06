
/**
 * Project name   : RelayESP-FailSafeNet 
 * LittleFS Functions 
 *
 */
 
String getValue(String data, char separator, int index) {
    int found = 0;
    int start = 0;
    int end = -1;

    for (int i = 0; i <= data.length(); i++) {
        if (i == data.length() || data.charAt(i) == separator) {
            if (found == index) {
                end = i;
                break;
            }
            found++;
            start = i + 1;
        }
    }
    if (end == -1) end = data.length();

    if (start >= 0 && end > start) {
        return data.substring(start, end);
    }
    return "";
}

// Get date from Config file.
String getconfigdata(String valuename, String filename) {
    if (!LittleFS.exists(filename)) {
        Serial.println("Config file not found: " + filename);
        return "Config_Not_found";
    }

    File fileConfig = LittleFS.open(filename, "r");
    if (!fileConfig) {
        Serial.println("Unable to open config file: " + filename);
        return "Config_Not_found";
    }

    char buffer[512];
    String valudedata = "";
    int founddata = 0;

    while (fileConfig.available()) {
        int l = fileConfig.readBytesUntil('\n', buffer, sizeof(buffer) - 1);
        buffer[l] = 0;
        if (l > 0 && buffer[l-1] == '\r') {
            buffer[l-1] = 0; // suppression du retour chariot Windows
        }

        Serial.print("Line read: '");
        Serial.print(buffer);
        Serial.println("'");

        String namevalue = getValue(buffer, '=', 0);
        if (namevalue == valuename && founddata == 0) {
            valudedata = getValue(buffer, '=', 1);
            founddata++;
            break;
        }
    }
    fileConfig.close();

    if (founddata == 1) {
        return valudedata;
    } else {
        return "Config_Not_found";
    }
}

bool replaceInFile(const String& searchValue, const String& replaceValue, const char* filepath) {

  File file = LittleFS.open(filepath, "r");
  if (!file || file.isDirectory()) {
    DebugSerialln("[ERROR] Cannot open file for reading: " + String(filepath));
    return false;
  }

  String fileContent = "";
  while (file.available()) {
    String line = file.readStringUntil('\n');
    line.replace(searchValue, replaceValue);
    fileContent += line + "\n";
  }
  file.close();

  // Réouvrir pour écriture, mais **sans redeclarer** 'file'
  file = LittleFS.open(filepath, "w");
  if (!file) {
    DebugSerialln("[ERROR] Cannot open file for writing: " + String(filepath));
    return false;
  }

  file.print(fileContent);
  file.close();

  DebugSerialln("[SUCCESS] Replacement complete in " + String(filepath));
  return true;
}

String getfile_to_string(String filename) {
  String DataFile = "";

  // Ouverture du fichier
  File file = LittleFS.open(filename, "r");
  if (!file) {
    DebugSerialln("[ERROR] Failed to open " + filename);
  } else {
    DebugSerialln("[SUCCESS] Reading " + filename);

    // Lecture caractère par caractère
    while (file.available()) {
      DataFile += (char)file.read();
    }

    // Fermeture du fichier
    file.close();
  }

  return DataFile;
}

fs::Dir getRootDir() {
    fs::Dir root = LittleFS.openDir("/");   // ESP8266: openDir retourne fs::Dir
    // Pas besoin de tester avec 'if (!root)' car fs::Dir n'est pas convertible en bool
    // On peut juste vérifier s'il y a des fichiers plus tard avec root.next()
    return root;
}

