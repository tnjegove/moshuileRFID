#include <ESP8266WiFi.h>
#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN         0          // Configurable, see typical pin layout above
#define SS_PIN          2         // Configurable, see typical pin layout above


#ifndef STASSID
#define STASSID "setSSID"
#define STAPSK  "changeme"
#endif

const char* ssid     = STASSID;
const char* password = STAPSK;
const uint ServerPort = 23;
byte readCard[4];
byte readCardSize;
WiFiServer Server(ServerPort);
WiFiClient RemoteClient;
MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance
void CheckForConnections()
{
  if (Server.hasClient())
  {
    // If we are already connected to another computer, 
    // then reject the new connection. Otherwise accept
    // the connection. 
    if (RemoteClient.connected())
    {
      Serial.println("Connection rejected");
      Server.available().stop();
    }
    else
    {
      Serial.println("Connection accepted");
      RemoteClient = Server.available();
      RemoteClient.println("You are now connected to rfid reader...");
      Serial.println("RemoteClient connected. Sending a message...");
    }
  }
}



void setup() {
  Serial.begin(115200);
  while (!Serial);
  SPI.begin();      // Init SPI bus
  mfrc522.PCD_Init();   // Init MFRC522
  delay(4);       // Optional delay. Some board do need more time after init to be ready, see Readme
  mfrc522.PCD_DumpVersionToSerial();  // Show details of PCD - MFRC522 Card Reader details
  Serial.println(F("Scan PICC to see UID, SAK, type, and data blocks..."));

  // We start by connecting to a WiFi network

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  /* Explicitly set the ESP8266 to be a WiFi-client, otherwise, it by default,
     would try to act as both a client and an access-point and could cause
     network-issues with your other WiFi-devices on your WiFi-network. */
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Server.begin();
  Serial.println("WIFI server started.");
}

void loop() {
  // put your main code here, to run repeatedly:
  CheckForConnections();
  getID();
  
}

uint8_t getID() {
  // Getting ready for Reading PICCs
  if ( ! mfrc522.PICC_IsNewCardPresent()) { //If a new PICC placed to RFID reader continue
    return 0;
  }
  if ( ! mfrc522.PICC_ReadCardSerial()) {   //Since a PICC placed get Serial and continue
    return 0;
  }
  // There are Mifare PICCs which have 4 byte or 7 byte UID care if you use 7 byte PICC
  // I think we should assume every PICC as they have 4 byte UID
  // Until we support 7 byte PICCs
  Serial.println(F("Scanned PICC's UID:"));
  for ( uint8_t i = 0; i < 4; i++) {  //
    readCard[i] = mfrc522.uid.uidByte[i];
    Serial.print(readCard[i], HEX);
  }
  readCardSize = sizeof(readCard);
   char dest[9];

  // initialise character array
  memset(dest, 0, sizeof(dest));
  // test the size of the character array
  if((sizeof(dest) - 1) / 2 < readCardSize)
  {
    // display error message
    Serial.println("Character buffer too small");
    // never continue
    for(;;);
  }

  for (int cnt = 0; cnt < readCardSize; cnt++)
  {
    // convert byte to its ascii representation
    sprintf(&dest[cnt * 2], "%02X", readCard[cnt]);
  }
  if (RemoteClient.connected()) {
    RemoteClient.println(dest);
  }
  Serial.println("");
  mfrc522.PICC_HaltA(); // Stop reading
  return 1;
}
