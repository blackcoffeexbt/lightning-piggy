#include <ArduinoJson.h>
#include <WiFiClientSecure.h>

const char* ssid     = "";
const char* password = "";

const char* host = "sats.pw";
const char* invoiceKey = "e24c3f0e71044b93866efeb0985e3135";

int walletBalance = 0;

void setup()
{
    Serial.begin(115200);
    delay(100);

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

    delay(1000);
}

void loop()
{
  Serial.println("-----------------");
  getWalletDetails();
  Serial.println("-----------------");
  getLNURLPayments(5);
  Serial.println("-----------------");
  getLNURLp();
  Serial.println("-----------------");

  delay(30000);
}


void getWalletDetails() {
  const String url = "/api/v1/wallet";
  const String line = getEndpointData(url);
  StaticJsonDocument<2000> doc;

  DeserializationError error = deserializeJson(doc, line);
  if (error)
  {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.f_str());
  }

const char* walletName = doc["name"];
  walletBalance = doc["balance"];
  walletBalance = walletBalance / 1000;

  Serial.println(walletName);
  Serial.println(String(walletBalance) + " sats");

  }

/**
 * @brief Get recent LNURL Payments
 * 
 * @param limit 
 */
void getLNURLPayments(int limit) {
  const String url = "/api/v1/payments?limit=" + String(limit);
  const String line = getEndpointData(url);
  StaticJsonDocument<3000> doc;

  DeserializationError error = deserializeJson(doc, line);
  if (error)
  {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.f_str());
  }

  for (JsonObject areaElems : doc.as<JsonArray>()) {
    if(areaElems["extra"] && areaElems["extra"]["tag"] && areaElems["extra"]["comment"]) {
      const char* tag = areaElems["extra"]["tag"];
      if(strcmp(tag,"lnurlp") == 0) {
        int amount = areaElems["amount"];
        amount = amount / 1000;
        const char* comment = areaElems["extra"]["comment"];
        Serial.print(comment);
        Serial.print(" - " + String(amount) + " sats ");
        Serial.println();
      }
    }
  }
}

/**
 * @brief Get the first available LNURLp from the wallet
 * 
 * @return String 
 */
String getLNURLp() {
  // Get the first lnurlp
  String lnurlpData = getEndpointData("/lnurlp/api/v1/links");
  StaticJsonDocument<3000> doc;

  DeserializationError error = deserializeJson(doc, lnurlpData);
  if (error)
  {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.f_str());
  }
  String lnurlpId = doc[0]["id"];

  lnurlpData = getEndpointData("/lnurlp/api/v1/links/" + lnurlpId);
  error = deserializeJson(doc, lnurlpData);
  if (error)
  {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.f_str());
  }
  String lnurlp = doc["lnurl"];
  Serial.println(lnurlp);
}

/**
 * @brief GET data from an LNbits endpoint
 * 
 * @param endpointUrl 
 * @return String 
 */
String getEndpointData(String endpointUrl) {
  WiFiClientSecure client;
  client.setInsecure(); //Some versions of WiFiClientSecure need this

  if (!client.connect(host, 443))
  {
    Serial.println("Server down");
    delay(3000);
  }

  const String request = String("GET ") + endpointUrl + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "User-Agent: LightningPiggy\r\n" +
               "X-Api-Key: " + invoiceKey + " \r\n" +
               "Content-Type: application/json\r\n" +
               "Connection: close\r\n\r\n";
  client.print(request);
  while (client.connected())
  {
    const String line = client.readStringUntil('\n');
    if (line == "\r")
    {
      break;
    }
  }

  const String line = client.readString();
  return line;
}