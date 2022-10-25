/*
 *  This sketch sends data via HTTP GET requests to data.sparkfun.com service.
 *
 *  You need to get streamId and privateKey at data.sparkfun.com and paste them
 *  below. Or just customize this script to talk to other HTTP servers.
 *
 */

#include <ArduinoJson.h>

#include <WiFiClientSecure.h>

const char* ssid     = "Maddox-Guest";
const char* password = "MadGuest1";

const char* host = "sats.pw";
const char* invoiceKey = "e24c3f0e71044b93866efeb0985e3135";

void setup()
{
    Serial.begin(115200);
    delay(10);

    // We start by connecting to a WiFi network

    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}

int value = 0;

void loop()
{
    WiFiClientSecure client;

    delay(1000);
    ++value;

    Serial.print("connecting to ");
    Serial.println(host);

  client.setInsecure(); //Some versions of WiFiClientSecure need this

  if (!client.connect(host, 443))
  {
    // error("SERVER DOWN");
    delay(3000);
    // return false;
  }

  const String url = "/api/v1/payments";
  const String request = String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "User-Agent: ESP32\r\n" +
               "X-Api-Key: " + invoiceKey + " \r\n" +
               "Content-Type: application/json\r\n" +
               "Connection: close\r\n\r\n";
Serial.println(request);
  client.print(request);
  while (client.connected())
  {
    Serial.println("Reading line");
    const String line = client.readStringUntil('\n');
    Serial.println(line);
    if (line == "\r")
    {
      break;
    }
  }

Serial.println("Read all lines");
  const String line = client.readString();
  Serial.println(line);
  StaticJsonDocument<2000> doc;

  DeserializationError error = deserializeJson(doc, line);
  if (error)
  {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.f_str());
    // return false;
  }

for (JsonObject areaElems : doc.as<JsonArray>()) {
  Serial.println("Checking payment");
  if(areaElems["extra"] && areaElems["extra"]["tag"] && areaElems["extra"]["comment"]) {
    Serial.println("Got");
    const char* value = areaElems["extra"]["tag"];
    Serial.println(value);
    const char* comment = areaElems["extra"]["comment"];
    Serial.println(comment);
  } else {
    Serial.println("No extra or tag");
  }
}

//   if (doc["paid"])
//   {
//     unConfirmed = false;
//   }

//   return unConfirmed;

delay(15000);

}
