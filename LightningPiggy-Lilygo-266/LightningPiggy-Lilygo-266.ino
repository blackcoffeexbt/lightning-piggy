#include <ArduinoJson.h>

#define LILYGO_T5_V266
#include <GxEPD.h>
#include <boards.h>
#include "qrcoded.h"

#include <WiFiClientSecure.h>

#include <GxDEPG0266BN/GxDEPG0266BN.h>    // 2.66" b/w   form DKE GROUP

#include "Fonts/LatoMedium8pt.h"
#include "Fonts/LatoMedium12pt.h"
#include "Fonts/LatoMedium18pt.h"
#include "Fonts/LatoMedium20pt.h"
#include "Fonts/LatoMedium26pt.h"

#include <GxIO/GxIO_SPI/GxIO_SPI.h>
#include <GxIO/GxIO.h>

GxIO_Class io(SPI,  EPD_CS, EPD_DC,  EPD_RSET);
GxEPD_Class display(io, EPD_RSET, EPD_BUSY);

const char* ssid     = "TheInternet";
const char* password = "dc924b3898";

const char* host = "sats.pw";
const char* invoiceKey = "e24c3f0e71044b93866efeb0985e3135";

String walletBalanceText = "";
String paymentDetails = "";

String qrData;
uint8_t *framebuffer;

int walletBalance = 0;

void setup()
{
    Serial.begin(115200);

    SPI.begin(EPD_SCLK, EPD_MISO, EPD_MOSI);
    display.init();

    display.setTextColor(GxEPD_BLACK);
    display.setRotation(1);

    display.fillScreen(GxEPD_WHITE);
    display.setFont(&Lato_Medium_20);
    printTextCentered("Connecting to WiFi");
    display.update();

    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    display.fillScreen(GxEPD_WHITE);
    display.setFont(&Lato_Medium_20);
    printTextCentered("Updating Piggy");
    display.update();

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    delay(100);
}

void loop()
{
  getWalletDetails();

  display.fillScreen(GxEPD_WHITE);
  printBalance();
  getLNURLPayments(5);
  display.update();
  delay(1000);
  // getLNURLp();
  showLNURLpQR();

  // display.display(false); // full update

  hibernate(300);
}

void printBalance() {
  int16_t x1, y1;
    uint16_t w, h;
    display.setFont(&Lato_Medium_26);
    display.getTextBounds(walletBalanceText, 0, 0, &x1, &y1, &w, &h);
    display.setCursor(display.width() / 2 - w / 2, 40);
    display.print(walletBalanceText);
    Serial.println("Printing balance");
    // display.setCursor(20, 40);
    // display.println(walletBalanceText);
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
  const uint8_t maxPaymentDetailStrLength = 30; // The maximum number of chars that should be displayed for each payment
  const String url = "/api/v1/payments?limit=" + String(limit);
  const String line = getEndpointData(url);
  StaticJsonDocument<4000> doc;
  Serial.println("Got payments");
  Serial.println(line);

  DeserializationError error = deserializeJson(doc, line);
  if (error)
  {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.f_str());
  }

  uint16_t yPos = 70;
  String output;
  for (JsonObject areaElems : doc.as<JsonArray>()) {
    if(areaElems["extra"] && areaElems["extra"]["tag"] && areaElems["extra"]["comment"]) {
      const char* tag = areaElems["extra"]["tag"];
      if(strcmp(tag,"lnurlp") == 0) {
        int amount = areaElems["amount"];
        amount = amount / 1000; // millisats to sats
        const char* comment = areaElems["extra"]["comment"];
        Serial.println(comment);

        display.setFont(&Lato_Medium_12);
        display.setCursor(10, yPos);
        String paymentDetail(comment);
        String paymentAmount(amount);
        output = paymentDetail.substring(0, maxPaymentDetailStrLength) + " " + paymentAmount + " sats";

        printTextCenteredX(output, yPos);
        yPos += 19;
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
  qrData = lnurlp;
  Serial.println(lnurlp);
}

void showLNURLpQR() {
  qrData = "LNURL1DP68GURN8GHJ7UMPW3EJUURH9AKXUATJD3CZ7CTSDYHHVVF0D3H82UNV9UEQDZ3CM3";
  const char *qrDataChar = qrData.c_str();
  QRCode qrcoded;

  int qrVersion = getQrCodeVersion();
  int pixSize = getQrCodePixelSize(qrVersion);
  uint8_t qrcodeData[qrcode_getBufferSize(qrVersion)];
  
  qrcode_initText(&qrcoded, qrcodeData, qrVersion, 0, qrDataChar);

  int qrWidth = pixSize * qrcoded.size;
  int qrPosX = ((display.width() - qrWidth) / 2);
  // int qrPosY = ((EPD_HEIGHT - qrWidth) / 2);
  int qrPosY = 20;

  display.fillScreen(GxEPD_WHITE);

  for (uint8_t y = 0; y < qrcoded.size; y++)
  {
    for (uint8_t x = 0; x < qrcoded.size; x++)
    {
      if (qrcode_getModule(&qrcoded, x, y))
      {
        display.fillRect(qrPosX + pixSize * x, qrPosY + pixSize * y, pixSize, pixSize, GxEPD_BLACK);
      }
    }
  }
  // display.setFont(&Lato_Medium_12);
  // printTextCenteredX("Send me sats, please :)", display.height() - 20);
  display.update();
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


void printTextCentered(char* str) {
  int16_t x1, y1;
    uint16_t w, h;

    display.getTextBounds(str, 0, 0, &x1, &y1, &w, &h);
    display.setCursor(display.width() / 2 - w / 2,
                    display.height() / 2 - h / 2 );
    display.print(str);
}

void printTextCenteredX(String str, uint16_t yPos) {
    int16_t x1, y1;
    uint16_t w, h;

    display.getTextBounds(str, 0, 0, &x1, &y1, &w, &h);
    display.setCursor(display.width() / 2 - w / 2, yPos );
    display.print(str);
}

/**
 * @brief Get the size of the qr code to produce
 * 
 * @param qrData 
 * @return int 
 */
int getQrCodeVersion() {
  int qrVersion = 0;
  int stringLength = qrData.length();

  // Using this chart with ECC_LOW https://github.com/ricmoo/QRCode#data-capacities
  if(stringLength <= 17) {
    qrVersion = 1;
  }  
  else if(stringLength <= 32) {
    qrVersion = 2;
  }
  else if(stringLength <= 53) {
    qrVersion = 3;
  }
  else if(stringLength <= 134) {
    qrVersion = 6;
  }
  else if(stringLength <= 367) {
    qrVersion = 11;
  }
  else {
    qrVersion = 28;
  }

  return qrVersion;
}


/**
 * @brief Get the Qr Code Pixel Size object
 * 
 * @param qrCodeVersion The QR code version that is being used
 * @return int The size of the QR code pixels
 */
int getQrCodePixelSize(int qrCodeVersion) {
  int qrDisplayHeight = display.height() - 40; // qr code height in pixels
  // Using https://github.com/ricmoo/QRCode#data-capacities

  // Get the QR code size (blocks not pixels)
  int qrCodeHeight = 0;
  switch(qrCodeVersion) {
    case 1:
      qrCodeHeight = 21;
      break;
    case 2:
      qrCodeHeight = 25;
      break;
    case 3:
      qrCodeHeight = 29;
      break;
    case 4:
      qrCodeHeight = 33;
      break;
    case 5:
      qrCodeHeight = 37;
      break;
    case 6:
      qrCodeHeight = 41;
      break;
    case 7:
      qrCodeHeight = 45;
      break;
    case 8:
      qrCodeHeight = 49;
      break;
    case 9:
      qrCodeHeight = 53;
      break;
    case 10:
      qrCodeHeight = 57;
      break;
    case 11:
      qrCodeHeight = 61;
      break;
    default:
      qrCodeHeight = 129;
      break;
  }
  int pixelHeight = floor(qrDisplayHeight / qrCodeHeight);
  // Serial.println(F("qrCodeHeight pixel height is"));
  // Serial.println(qrCodeHeight);

  // Serial.println(F("Calced pixel height is"));
  // Serial.println(pixelHeight);
  return pixelHeight;
}