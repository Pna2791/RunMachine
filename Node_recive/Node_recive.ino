#include <ESP8266WiFi.h>
#include <espnow.h>
#include <VL53L0X.h>
#define led     4   //D2
#define ID      6

uint8_t hostAddress[] = {0x3C, 0x61, 0x05, 0x64, 0xFE, 0x00};
uint8_t thisAddress[] = {0x78, 0x21, 0x84, 0x79, 0x79, 0x22};

VL53LOX sensor;

unsigned long time_out = 0;
unsigned long time_start = 0;

// Variable to store if sending data was successful
String success;
typedef struct struct_message {
    int id;
    int value;
    int distance;
} struct_message;
struct_message sending_data;
struct_message recei_data;

// Callback when data is sent
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
    Serial.print(" Last Packet Send Status: ");
    if (sendStatus == 0)  Serial.println("Delivery success");
    else                  Serial.println("Delivery fail");
}

// Callback when data is received
void OnDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len) {
    memcpy(&recei_data, incomingData, sizeof(recei_data));
    digitalWrite(led, HIGH);
    time_start = millis();
    time_out = time_start + 100 * recei_data.value;
    Serial.println(recei_data.value);
}

 
void setup() {
    pinMode(led, OUTPUT);
    Serial.begin(115200);
    WiFi.mode(WIFI_STA);
    for(int i = 0; i<6; i++)
      thisAddress[i] = hostAddress[i];
    thisAddress[5] = 17*ID;
    wifi_set_macaddr(STATION_IF, &thisAddress[0]);
    Serial.println(WiFi.macAddress());
    WiFi.disconnect();

    // Init ESP-NOW
    if (esp_now_init() != 0) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }
    //set VL53LOX 
    Wire.begin();
    sensor.setTimeout(500);
    if (!sensor.init())
    {
        Serial.println("Failed to detect and initialize sensor!");
        while (1) {}
    }   

    // Set ESP-NOW Role
    esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
    esp_now_register_send_cb(OnDataSent);
    esp_now_add_peer(hostAddress, ESP_NOW_ROLE_COMBO, ID, NULL, 0);
    esp_now_register_recv_cb(OnDataRecv);
    sending_data.id = ID;
    Serial.println("Ready!!!");
}
 
void loop() {
    while(millis() < time_out){
        int temp_distance = readRangeSingleMillimeters();
        if(temp_distance < receiv_data.distance){
        // if(true){
			sending_data.distance = temp_distance;
            sending_data.value = int((millis() - time_start)/100);
            esp_now_send(hostAddress, (uint8_t *) &sending_data, sizeof(sending_data));
            time_out = millis();
            break;
        }
        delay(1);
    }
    digitalWrite(led, LOW);
    delay(1);
}
