// #include <ESP8266WiFi.h>     // For ESP8266
#include <WiFi.h> // For ESP32
#include <WebSocketsClient.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include <ArduinoJson.h>

#define BUZZER_PIN 2      // Connect the piezo buzzer to this GPIO pin.
#define CLICK_DURATION 10 // Duration in milliseconds.

const char *ssid = "[SSID]";
const char *password = "[PASSWD]";

WebSocketsClient webSocket;

// setup ttgo tft
TFT_eSPI tft = TFT_eSPI(135, 240);


void setup()
{
  Serial.begin(115200);

  pinMode(BUZZER_PIN, OUTPUT); // Set the buzzer pin as an output.

  // init tft
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(2);
  tft.setTextColor(TFT_GREEN);

  // click buzzer at boot
  click(100);

  // Connect to Wi-Fi
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.println(".");
  }
  Serial.println("Connected to WiFi!");
  // beep
  click(500);
  delay(100);
  click(500);

  // Setup WebSocket
  webSocket.beginSSL("mempool.space", 443, "/api/v1/ws");
  webSocket.onEvent(webSocketEvent);
}

unsigned long lastPingTime = 0;
const unsigned long pingInterval = 10000;  // 10 seconds
bool awaitingPong = false;

void loop()
{
  webSocket.loop();

  if (millis() - lastPingTime > pingInterval) {
        // Send a PING frame
        webSocket.sendPing();
        awaitingPong = true;
        lastPingTime = millis();
  }
}

void click(int period)
{
  Serial.println("NEW BLOCK Click!");
  for (int i = 0; i < CLICK_DURATION; i++)
  {
    digitalWrite(BUZZER_PIN, HIGH);
    delayMicroseconds(period); // Half period of 1000Hz tone.
    digitalWrite(BUZZER_PIN, LOW);
    delayMicroseconds(period); // Other half period of 1000Hz tone.
  }
}

void webSocketEvent(WStype_t type, uint8_t *payload, size_t length)
{
  switch (type)
  {
  case WStype_DISCONNECTED:
    Serial.println("Disconnected!");
    // Attempt to reconnect
    webSocket.beginSSL("mempool.space", 443, "/api/v1/ws");
    break;
  case WStype_CONNECTED:
    Serial.println("Connected to WebSocket");
    // Send message on connection
    click(250);
    delay(100);
    click(250);
    delay(100);
    click(250);

    webSocket.sendTXT("{\"action\": \"want\", \"data\": [\"blocks\"]}");
    // tft show "connected"
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.setTextDatum(MC_DATUM);
    tft.setTextSize(2);
    tft.drawString("Awaiting new block",  tft.width() / 2, tft.height() / 2 );

    break;
  case WStype_TEXT:
    Serial.println("Received: ");
    Serial.println((char *)payload);
    // if payload includes the word "block" then click
    if (strstr((char *)payload, "height"))
    {
      click(100);
      // get height from this payload: {"block": {"id":"00000000000000000003390f9da5e76dba42f8b3f147bcfdfc8092b4ba8007ac","height":810613,"version":648273920,"timestamp":1696424982,"bits":386197775,"nonce":1711818532,"difficulty":57321508229258.04,"merkle_root":"2f4d206b4ae8fa000f8a4bca8e204c7cfbe2401e796d19c31587fc5c9e9d8ef1","tx_count":2064,"size":1288841,"weight":3992804,"previousblockhash":"00000000000000000000af83bffb46f1de8a6b22e3822bb6cb0699ce0bd2ee0d","mediantime":1696420600,"stale":false,"extras":{"reward":647328442,"coinbaseRaw":"03755e0c0416641d652f466f756e6472792055534120506f6f6c202364726f70676f6c642f0d01c0361f01000000000000","orphans":[],"medianFee":16.050393700787403,"feeRange":[13.022222222222222,14.733532934131736,16.52953537240252,28.53715775749674,31.137777777777778,40.47144152311877,469.97389033942557],"totalFees":22328442,"avgFee":10823,"avgFeeRate":22,"utxoSetChange":-1647,"avgTxSize":624.29,"totalInputs":7045,"totalOutputs":5398,"totalOutputAmt":1519350914914,"segwitTotalTxs":1755,"segwitTotalSize":718500,"segwitTotalWeight":1711548,"feePercentiles":null,"virtualSize":998201,"coinbaseAddress":"bc1qxhmdufsvnuaaaer4ynz88fspdsxq2h9e9cetdj","coinbaseSignature":"OP_0 OP_PUSHBYTES_20 35f6de260c9f3bdee47524c473a6016c0c055cb9","coinbaseSignatureAscii":"\u0003u^\f\u0004\u0016d\u001de/Foundry USA Pool #dropgold/\r\u0001À6\u001f\u0001\u0000\u0000\u0000\u0000\u0000\u0000","header":"00e0a3260deed20bce9906cbb62b82e3226b8adef146fbbf83af00000000000000000000f18e9d9e5cfc8715c3196d791e40e2fb7c4c208eca4b8a0f00fae84a6b204d2f16641d650fe9041724470866","utxoSetSize":null,"totalInputAmt":null,"pool":{"id":111,"name":"Foundry USA","slug":"foundryusa"},"matchRate":100,"expectedFees":22854570,"expectedWeight":3991874,"similarity":0.9598855088210204}}}
      // parse json
      StaticJsonDocument<1024> doc;
      deserializeJson(doc, (char *)payload);
      // get height
      int height = doc["block"]["height"];
      // show height on tft
      tft.fillScreen(TFT_BLACK);
      tft.setTextColor(TFT_GREEN);
      tft.setTextSize(3);
      tft.drawString(String(height), tft.width() / 2, tft.height() / 2);
    }
    break;
  case WStype_BIN:
  case WStype_ERROR:
  case WStype_FRAGMENT_TEXT_START:
  case WStype_FRAGMENT_BIN_START:
  case WStype_FRAGMENT:
  case WStype_FRAGMENT_FIN:
    // Handle or ignore other cases
    break;
  }
}
