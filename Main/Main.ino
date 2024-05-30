
#include <EEPROM.h>
#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>

#define pinUp 34
#define pinEn 39
#define pinDo 36

#define latchPin 19
#define clockPin 22
#define dataPin 23
#define TONE_OUTPUT_PIN 18

#include "MCUFRIEND_kbv.h"
MCUFRIEND_kbv tft;
#include "logo.c"
#include "FontMaker.h"
#include <FreeDefaultFonts.h>
void setpx(int16_t x,int16_t y,uint16_t color){
    tft.drawPixel(x,y,color); //Thay đổi hàm này thành hàm vẽ pixel mà thư viện led bạn dùng cung cấp
}
MakeFont myfont(&setpx);


#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define BLACK  0x0000
#define WHITE  0xFFFF


// 78:21:84:79:79:00    {0x78, 0x21, 0x84, 0x79, 0x79, 0x00}    Máy đầu tiên ở Quảng Ngãi
// 3C:61:05:64:FE:58    {0x3C, 0x61, 0x05, 0x64, 0xFE, 0x00|    May 03             

uint8_t hostAddress[8] = {0x78, 0x21, 0x84, 0x79, 0x79, 0x00};
bool led_stt[6] = {0, 0, 0, 0, 0, 0};
int idx[3][6] = {
  {1,6,0,0,0,0},
  {1,3,4,6,0,0},
  {1,2,3,4,5,6}
};
int time_speed[10] = {1800, 1600, 1400, 1200, 1000, 800, 600, 400, 200, 0};
int time_dura[10] = {255,   192,  144,  108,  81,   60,   45,   34, 25, 19};



void led_change(){
    int value = 0;
    for(int i=0; i<6; i++)
        value = 2*value + led_stt[i];
    value *= 2;

    digitalWrite(latchPin, 0);
    shiftOut(dataPin, clockPin, LSBFIRST, byte(value));
    digitalWrite(latchPin, 1);
}
void led_write(int index, int stt){
    led_stt[index] = stt;
    led_change();
}
void around(){
    Serial.println("Around");
    int time_delay = 100;
    for(int i=0; i<6; i++)
        led_stt[i] = 0;
    led_change();
    for(int i=0; i<6; i++){
        led_stt[i] = 1;
        led_change();
        sendding(i+1, 6);
        delay(time_delay);
    }
    for(int i=0; i<6; i++){
        led_stt[i] = 0;
        led_change();
        delay(time_delay);
    }
}

#define debounce 200
bool pressed(){
    static long pre_press = millis();
    if(!digitalRead(pinEn) && millis() > pre_press + debounce){
        pre_press = millis();
        return true;
    }else
        return false;
}
bool down(){
    if(digitalRead(pinDo))
        return false;
    else
        return true;
}
bool up(){
    if(digitalRead(pinUp))
        return false;
    else
        return true;
}


esp_now_peer_info_t peerInfo[6];
uint8_t nodeAddress[6][8] = {
  {0x78, 0x21, 0x84, 0x79, 0x79, 0x11},
  {0x78, 0x21, 0x84, 0x79, 0x79, 0x22},
  {0x78, 0x21, 0x84, 0x79, 0x79, 0x33},
  {0x78, 0x21, 0x84, 0x79, 0x79, 0x44},
  {0x78, 0x21, 0x84, 0x79, 0x79, 0x55},
  {0x78, 0x21, 0x84, 0x79, 0x79, 0x66}
};

typedef struct parameter {
    int speed=4;
    int capacity=10;
    bool type;
    int size=6;
} parameter;
parameter params;

String success;
typedef struct struct_message {
    int id;
    int value;
} struct_message;
struct_message sending_data;
struct_message recei_data;


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
void wifi_setup(){
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



void sendding(int id, int value){
  recei_data.id = 0;
  recei_data.value = time_dura[params.speed];
  sending_data.id = id;
  sending_data.value = value;
  esp_now_send(nodeAddress[id-1], (uint8_t *) &sending_data, sizeof(sending_data));
}
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  // Serial.print("\rLast Packet Send Status:\t");
  // Serial.print(String(*mac_addr));
  // Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
    memcpy(&recei_data, incomingData, sizeof(recei_data));
    Serial.print(recei_data.id);
    Serial.print(" ");
    Serial.println(recei_data.value);
}

void load_setup(){
    EEPROM.begin(128);
    params.speed = EEPROM.read(0);
    params.capacity = 10*EEPROM.read(1);
    if(EEPROM.read(2))  params.type = 1;
    else                params.type = 0;
    params.size = EEPROM.read(3);
    EEPROM.end();
}
void save_setup(){
    EEPROM.begin(128);
    EEPROM.write(0, params.speed);
    EEPROM.write(1, params.capacity/10);
    EEPROM.write(2, params.type);
    EEPROM.write(3, params.size);
    EEPROM.end();
}


void startup_screen(){
    tft.setRotation(3);
    tft.fillScreen(WHITE);
    tft.drawRGBBitmap(80, 0, logo, 240, 240);
    around();
    while(true){
        if(!digitalRead(pinUp) || !digitalRead(pinDo) || !digitalRead(pinEn))
            break;
        delay(100);
    }
}

const int TONE_PWM_CHANNEL = 0;
void sound(int value){
    if(value == 0){
        ledcWriteNote(TONE_PWM_CHANNEL, NOTE_F, 3);
        delay(200);
    }else{
        ledcWriteNote(TONE_PWM_CHANNEL, NOTE_F, 5);
        delay(100);
    }
    ledcWriteTone(TONE_PWM_CHANNEL, 0);
}
void setup() {
    uint16_t ID = 0x9327;
    Serial.begin(115200);
    pinMode(pinUp, INPUT);
    pinMode(pinEn, INPUT);
    pinMode(pinDo, INPUT);
    pinMode(latchPin, OUTPUT);
    pinMode(clockPin, OUTPUT);
    pinMode(dataPin, OUTPUT);

    tft.begin(ID);
    
        

    ledcAttachPin(TONE_OUTPUT_PIN, TONE_PWM_CHANNEL);
    load_setup();

    wifi_setup();
    startup_screen();
    myfont.set_font(Arial_20);
}

int box_height = 48;

void show_home(float pos){
    if(pos == 0)  Serial.println("[In home] practice now");
    if(pos == 1)  Serial.println("[In home] setup");

    pos += 0.8;
    tft.fillScreen(BLACK);
    tft.fillRect(20, pos*box_height, 340, box_height, BLUE);

    myfont.print(100, 1*box_height, "Bắt đầu bài tập", YELLOW, 0);
    myfont.print(100, 2*box_height, "Thiết lập", YELLOW, 0);

    String mess = "Tốc độ: " + String(params.speed) + "     " + String(params.capacity);
    if(params.type)  mess += " lần ";
    else            mess += " giây ";
    mess += "    " + String(params.size) + " góc";
  
    myfont.print(40, 4*box_height, mess, YELLOW, 0);
}

int in_home(){
    int pos = 0;
    // 0 practice now
    // 1 setup
    show_home(pos);
    around();
    while(true){
        if(pressed()) return pos;
        if(up() || down()){
            pos = 1 - pos;
            show_home(pos);
            delay(300);
        }
    }
}

void print_params(){
    String mess = "";
    mess += "Speed: " + String(params.speed);
    mess += " Cap: " + String(params.capacity);
    mess += " Unit: " + String(params.type);
    mess += " Num: " + String(params.size);
    Serial.println(mess);
}
void draw_line(int index, String mess, String value){
    int y = index * box_height + 10;
    myfont.print(40, y, mess, YELLOW, 0);
    myfont.print(300, y, value, YELLOW, 0);
}
void show_setting(int pos){
    tft.fillScreen(BLACK);
    tft.fillRect(240, pos*box_height, 150, box_height, BLUE);

    draw_line(0, "Tốc độ:", String(params.speed));
    draw_line(1, "Thời lượng:", String(params.capacity));
    
    if(params.type) draw_line(2, "Đơn vị:", "lần");
    else            draw_line(2, "Đơn vị:", "giây");

    draw_line(3, "Số góc:", String(params.size));
    draw_line(4, "Trở lại:", "<==");
    print_params();
    delay(200);
}
void set_value(int pos){
    show_setting(pos);
    while(true){
        if(pressed()) return;
        int delta = 0;
        if(up())    delta = 1;
        if(down())  delta = -1;
        if(delta != 0){
            if(pos == 0){
                params.speed += delta;
                if(params.speed < 0)  params.speed = 0;
                if(params.speed > 9)  params.speed = 9;
            }
            if(pos == 1){
                params.capacity += 10*delta;
                if(params.capacity < 10)  params.capacity = 10;
                if(params.capacity > 990)  params.capacity = 990;
            }
            if(pos == 3){
                params.size += 2*delta;
                if(params.size < 2)  params.size = 2;
                if(params.size > 6)  params.size = 6;
            }
            show_setting(pos);
        }
    }
}
void show_setup(int pos){
    if(pos == 0)  Serial.println("[In setup] set speed");
    if(pos == 1)  Serial.println("[In setup] set capacity");
    if(pos == 2)  Serial.println("[In setup] set type");
    if(pos == 3)  Serial.println("[In setup] set num");
    if(pos == 4)  Serial.println("[In setup] back to home");
    
    tft.fillScreen(BLACK);
    tft.fillRect(20, pos*box_height, 360, box_height, BLUE);

    draw_line(0, "Tốc độ:", String(params.speed));
    draw_line(1, "Thời lượng:", String(params.capacity));
    
    if(params.type) draw_line(2, "Đơn vị:", "lần");
    else            draw_line(2, "Đơn vị:", "giây");

    draw_line(3, "Số góc:", String(params.size));
    draw_line(4, "Trở lại:", "<==");
    delay(200);
}
void in_setup(){
    int pos = 0;
    // 0. toc do
    // 1. so luong
    // 2. don vi
    // 3. so cam bien
    // 4 return
    show_setup(pos);
    while(true){
        if(pressed()){
            if(pos == 0)  set_value(pos);
            if(pos == 1)  set_value(pos);
            if(pos == 2){
                if(params.type) params.type = false;
                else            params.type = true;
            }
            if(pos == 3)  set_value(pos);
            if(pos == 4)  break;
            show_setup(pos);
        }
        int delta = 0;
        if(up())    delta = -1;
        if(down())  delta = 1;

        pos += delta;
        if(pos < 0)   pos = 4;
        if(pos > 4)   pos = 0;
        if(delta != 0)  show_setup(pos);
        
    }
    save_setup();
    Serial.println("Saved Setup");
}


void show_detail(String mess, int score){
    tft.fillScreen(BLACK);
    mess += " ";
    const int font_width = 13;
    myfont.print(400-font_width*mess.length(), 10, mess, GREEN, 0);

    tft.setFont(&FreeSevenSegNumFont);
    tft.setTextSize(3);
    tft.setTextColor(YELLOW);

    const int y = 200;
    int first = score/100;
    int mid = (score/10) % 10;
    int last = score % 10;
    if(first){
        tft.setCursor(10, y);
        tft.print(first);
    }
    tft.setCursor(110, y);
    tft.print(mid);
    
    myfont.set_font(SVN_60);
    myfont.print(210, 90, ".", YELLOW, 0);
    myfont.print(330, 90, "s", YELLOW, 0);
    tft.setCursor(230, y);
    tft.print(last);
    myfont.set_font(Arial_20);
}

void show_stop(float pos){
    pos += 0.8;
    tft.fillScreen(BLACK);
    tft.fillRect(20, pos*box_height, 340, box_height, BLUE);

    myfont.print(150, 1*box_height, "Tiếp Tục", YELLOW, 0);
    myfont.print(150, 2*box_height, "Kết Thúc", YELLOW, 0);
    delay(300);
}


bool in_stop(){
    int pos = 0;
    // 0 continue
    // 1 stop
    show_stop(pos);
    while(true){
        if(pressed()){
            if(pos) return true;
            else    return false;
        }
        if(up() || down()){
            pos = 1-pos;
            show_stop(pos);
        }
    }
}
void in_practice(){
    int total_score = 0;
    int good_score=0, count=0, score=0;
    String mess;
    // type==1: lan, type==0: second
    around();
    delay(500);
    around();
    delay(500);
    if(params.type){
        score = 0;
        mess = "0/" + String(params.capacity);
        show_detail(mess, score);
        for(int i=0; i<params.capacity; i++){
            long t_out = millis() + time_speed[params.speed];
            while(millis() < t_out)
                if(pressed()){
                    if(in_stop()) return;
                    show_detail(mess, score);
                }

            int index = t_out % params.size;
            int id = idx[params.size/2-1][index];

            led_write(id-1, 1);
            sendding(id, time_dura[params.speed]);
            t_out = millis() + time_dura[params.speed]*100;

            while(millis() < t_out){
                if(recei_data.id == id)  break;
                if(pressed()){
                    if(in_stop()) return;
                    show_detail(mess, score);
                }
            }
            total_score += recei_data.value;
            led_write(id-1, 0);
            int pass = 0;
            if(2 * recei_data.value < time_dura[params.speed])  pass = 1;
            good_score += pass;
            sound(pass);
            score = recei_data.value;
            mess = String(good_score) + "/" + String(i+1) + "/" + String(params.capacity);
            show_detail(mess, score);
        }
        total_score /= params.capacity;
        score = total_score;
        mess = "DONE   " + String(good_score) + "/" + String(params.capacity);
        show_detail(mess, score);
    }else{
        score = 0;
        mess = "0/" + String(params.capacity);
        show_detail(mess, score);
        long t_finish = millis() + 1000 * params.capacity;
        while(millis() < t_finish){
          count++;
            long t_out = millis() + time_speed[params.speed];
            while(millis() < t_out)
                if(pressed()){
                    if(in_stop()) return;
                    show_detail(mess, score);
                }

            int index = t_out % params.size;
            int id = idx[params.size/2-1][index];

            led_write(id-1, 1);
            sendding(id, time_dura[params.speed]);
            t_out = millis() + time_dura[params.speed]*100;

            while(millis() < t_out){
                if(recei_data.id == id)  break;
                if(pressed()){
                    if(in_stop()) return;
                    show_detail(mess, score);
                }
            }
            total_score += recei_data.value;
            led_write(id-1, 0);
            int pass = 0;
            if(2 * recei_data.value < time_dura[params.speed])  pass = 1;
            good_score += pass;
            sound(pass);
            score = recei_data.value;
            mess = String(good_score) + "/" + String(count) + "  " + String(int((t_finish-millis())/1000)) + "/" + String(params.capacity);
            show_detail(mess, score);
        }
        total_score /= params.capacity;
        score = total_score;
        mess = "DONE   " + String(good_score) + "/" + String(count) + "  " + String(params.capacity);
        show_detail(mess, score);
    }
    // Start exercise at here
    while(true){
        break;
    }
    
    around();
    while(!pressed())
      delay(1);
}
void loop() {
    int value = in_home();
    if(value == 0)  in_practice();
    if(value == 1)  in_setup();
}
