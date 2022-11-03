#include <ArduinoJson.h>
#include <WiFiClientSecure.h>
#include <GxEPD2_BW.h>
#include <GxEPD2_3C.h>
#include <Fonts/FreeSansBold12pt7b.h>
#include <Fonts/FreeSansBold6pt7b.h>

const char* ssid     = "TheInternet";
const char* password = "dc924b3898";

const char* host = "sats.pw";
const char* invoiceKey = "e24c3f0e71044b93866efeb0985e3135";

String walletBalanceText = "";
String paymentDetails = "";

int walletBalance = 0;

//GxEPD2_3C<GxEPD2_213c, GxEPD2_213c::HEIGHT> display(GxEPD2_213c(/*CS=5*/ SS, /*DC=*/ 22, /*RST=*/ 21, /*BUSY=*/ 4));
// For black and white. Remember to change the _YELLOW colour
GxEPD2_BW<GxEPD2_213_flex, GxEPD2_213_flex::HEIGHT> display(GxEPD2_213_flex(/*CS=5*/ SS, /*DC=*/ 22, /*RST=*/ 21, /*BUSY=*/ 4));

/**
 * Busy - g4
 * Reset - g21
 * D/C - g22
 * CS - g5
 * SCLK - g18
 * SDI-g23
 * GND - gnd
 * VCC - 3.3
 */

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

    delay(100);

    display.init();
    display.setRotation(1);
    display.setFullWindow();
    display.setTextColor(GxEPD_BLACK);
    display.setFont(&FreeSansBold12pt7b);
    display.fillScreen(GxEPD_WHITE);

    delay(1000);
}

void loop()
{
  Serial.println("-----------------");
  getWalletDetails();
  printBalance();
  Serial.println("-----------------");
  getLNURLPayments(5);
  Serial.println("-----------------");
  getLNURLp();
  Serial.println("-----------------");

  display.display(false); // full update

  hibernate(300);
}

void printBalance() {
    display.setCursor(10, 20);
    // const char HelloWorld[] = "Hello World!";
    display.print(walletBalanceText);
}


void getWalletDetails() {
  const String url = "/api/v1/wallet";
  const String line = getEndpointData(url);
  StaticJsonDocument<3000> doc;

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
  walletBalanceText = String(walletBalance) + " sats";
}

/**
 * @brief Get recent LNURL Payments
 * 
 * @param limit 
 */
void getLNURLPayments(int limit) {
  const String url = "/api/v1/payments?limit=" + String(limit);
  const String line = getEndpointData(url);
  StaticJsonDocument<4000> doc;

  DeserializationError error = deserializeJson(doc, line);
  if (error)
  {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.f_str());
  }

  uint16_t yPos = 40;
  String output;
  for (JsonObject areaElems : doc.as<JsonArray>()) {
    if(areaElems["extra"] && areaElems["extra"]["tag"] && areaElems["extra"]["comment"]) {
      const char* tag = areaElems["extra"]["tag"];
      if(strcmp(tag,"lnurlp") == 0) {
        int amount = areaElems["amount"];
        amount = amount / 1000;
        const char* comment = areaElems["extra"]["comment"];
        // Serial.print(comment);
        // Serial.print(F(" - "));
        // Serial.print(amount);
        // Serial.print(F(" sats "));
        // Serial.println();

        display.setFont(&FreeSansBold6pt7b);
        display.setCursor(10, yPos);
        String paymentDetail(comment);
        String paymentAmount(amount);
        // output = paymentDetail.substring(0,20) + " - " + paymentAmount + " sats";
        output = "This sssssssssssssss - 200 sats";
        // output = "sat";
        display.print(output);
        yPos += 15;
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

void hibernate(int sleepTimeSeconds) {
  uint64_t deepSleepTime = (uint64_t)sleepTimeSeconds * (uint64_t)1000 * (uint64_t)1000;
  Serial.println("Going to sleep for seconds");
  Serial.println(deepSleepTime);
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH,   ESP_PD_OPTION_OFF);
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_SLOW_MEM, ESP_PD_OPTION_OFF);
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_FAST_MEM, ESP_PD_OPTION_OFF);
  esp_sleep_pd_config(ESP_PD_DOMAIN_XTAL,         ESP_PD_OPTION_OFF);
  esp_sleep_enable_timer_wakeup(deepSleepTime);
  esp_deep_sleep_start();
}