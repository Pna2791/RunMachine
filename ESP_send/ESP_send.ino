#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>

uint8_t hostAddress[8] = {0x78, 0x21, 0x84, 0x79, 0x79, 0x00};
uint8_t nodeAddress[6][8] = {
  {0x78, 0x21, 0x84, 0x79, 0x79, 0x11},
  {0x78, 0x21, 0x84, 0x79, 0x79, 0x22},
  {0x78, 0x21, 0x84, 0x79, 0x79, 0x33},
  {0x78, 0x21, 0x84, 0x79, 0x79, 0x44},
  {0x78, 0x21, 0x84, 0x79, 0x79, 0x55},
  {0x78, 0x21, 0x84, 0x79, 0x79, 0x66}
};

void change_mac(){
    for(int i=0; i<6; i++){
        nodeAddress[i][0] = hostAddress[0];
        nodeAddress[i][1] = hostAddress[1];
        nodeAddress[i][2] = hostAddress[2];
        nodeAddress[i][3] = hostAddress[3];
        nodeAddress[i][4] = hostAddress[4];
        nodeAddress[i][5] = 17*(i+1);
    }
}


esp_now_peer_info_t peerInfo[6];
String success;
typedef struct struct_message {
    int id;
    int value;
} struct_message;
struct_message sending_data;
struct_message recei_data;

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\rLast Packet Send Status:\t");
  Serial.print(String(*mac_addr));
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

// Callback when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
    memcpy(&recei_data, incomingData, sizeof(recei_data));
    Serial.print(recei_data.id);
    Serial.print(" ");
    Serial.println(recei_data.value);
}






void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  Serial.print("Old: ");
  Serial.println(WiFi.macAddress());
  change_mac();
  esp_wifi_set_mac(WIFI_IF_STA, &hostAddress[0]);
  Serial.print("[NEW] ESP32 Board MAC Address:  ");
  Serial.println(WiFi.macAddress());
  
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_register_send_cb(OnDataSent);
  for(int i=0; i<6; i++){
    memcpy(peerInfo[i].peer_addr, nodeAddress[i], 6);
    peerInfo[i].channel = 0;  
    peerInfo[i].encrypt = false;
    esp_now_add_peer(&peerInfo[i]);
  }
  esp_now_register_recv_cb(OnDataRecv);
}



void loop() {
  sending_data.id = 1;
  sending_data.value = 20;
  for(int i=0; i<6; i++){
    esp_now_send(nodeAddress[i], (uint8_t *) &sending_data, sizeof(sending_data));
    delay(500);
  }
  delay(1000);
}
