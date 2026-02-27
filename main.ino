#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <SPI.h>
#include <vector>
#include <time.h>
#include <LittleFS.h>
#include <WiFiUdp.h>
#include <Adafruit_GFX.h>         
#include <U8g2_for_Adafruit_GFX.h> 


extern const uint8_t u8g2_font_wqy12_t_gb2312[] U8G2_FONT_SECTION("u8g2_font_wqy12_t_gb2312");


// CONFIG

const char* WIFI_SSID = "SSID";
const char* WIFI_PASS = "PASSWORD";

const char* PC_HOTSPOT_IP = "192.168.137.1"; 
const int UDP_PORT = 8080;
WiFiUDP udp;

const char* API_BASE = "API_BASE"; 
const char* API_KEY  = "API_KEY";

const char* TIMEZONE = "EST5EDT,M3.2.0,M11.1.0"; 

#define LCD_WIDTH  480
#define LCD_HEIGHT 320

#define THEME_BG     0x2106
#define THEME_ACCENT 0xDCF2
#define WHITE        0xFFFF
#define GRAY         0xD69A
#define RED          0xF800
#define GREEN        0x07E0
#define YELLOW       0xFFE0 

// PINS
#define I2C_SDA_PIN  48
#define I2C_SCL_PIN  47
#define BUTTON_PIN   52

// MOTOR PINS
#define EN_PIN       31
#define N1_PIN       29
#define N2_PIN       28
#define N3_PIN       30

// LCD PINS
#define LCD_CS  26
#define LCD_RST 32
#define LCD_DC  27
#define LCD_BL  33
#define LCD_SCK 22
#define LCD_MOSI 21
#define LCD_MISO 20

// CONSTANTS
#define POLE_PAIRS 7
#define VOLTAGE_LIMIT 0.30 
#define MAX_TRACKS 1500 
#define FLING_THRESH 13.0 
#define STOP_THRESH  2.0 
#define MOMENTUM_ASSIST 0.03


// FONT 

const uint8_t font8x8_basic[] PROGMEM = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x3C, 0x3C, 0x18, 0x18, 0x00, 0x18, 0x00, 
    0x66, 0x66, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 0x6C, 0x6C, 0xFE, 0x6C, 0xFE, 0x6C, 0x6C, 0x00, 
    0x18, 0x3E, 0x60, 0x3C, 0x06, 0x7C, 0x18, 0x00, 0x00, 0xC6, 0x1C, 0xC6, 0x38, 0x6C, 0xC6, 0x00, 
    0x3C, 0x66, 0x38, 0x38, 0x66, 0x3C, 0x00, 0x00, 0x18, 0x18, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x0C, 0x18, 0x30, 0x30, 0x30, 0x18, 0x0C, 0x00, 0x30, 0x18, 0x0C, 0x0C, 0x0C, 0x18, 0x30, 0x00, 
    0x00, 0x66, 0x3C, 0xFF, 0x3C, 0x66, 0x00, 0x00, 0x00, 0x18, 0x18, 0x7E, 0x18, 0x18, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x30, 0x00, 0x00, 0x00, 0x7E, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00, 0x06, 0x0C, 0x18, 0x30, 0x60, 0xC0, 0x80, 0x00, 
    0x3C, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x00, 0x18, 0x38, 0x18, 0x18, 0x18, 0x18, 0x3C, 0x00, 
    0x3C, 0x66, 0x06, 0x0C, 0x30, 0x60, 0x7E, 0x00, 0x3C, 0x66, 0x06, 0x1C, 0x06, 0x66, 0x3C, 0x00, 
    0x0C, 0x1C, 0x3C, 0x6C, 0xCC, 0xFE, 0x0C, 0x00, 0x7E, 0x60, 0x7C, 0x06, 0x06, 0x66, 0x3C, 0x00, 
    0x3C, 0x60, 0x60, 0x7C, 0x66, 0x66, 0x3C, 0x00, 0x7E, 0x06, 0x0C, 0x18, 0x30, 0x30, 0x30, 0x00, 
    0x3C, 0x66, 0x66, 0x3C, 0x66, 0x66, 0x3C, 0x00, 0x3C, 0x66, 0x66, 0x3E, 0x06, 0x06, 0x3C, 0x00, 
    0x00, 0x18, 0x18, 0x00, 0x18, 0x18, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00, 0x18, 0x18, 0x30, 0x00, 
    0x06, 0x18, 0x60, 0xC0, 0x60, 0x18, 0x06, 0x00, 0x00, 0x00, 0x7E, 0x00, 0x7E, 0x00, 0x00, 0x00, 
    0x60, 0x18, 0x06, 0x03, 0x06, 0x18, 0x60, 0x00, 0x3C, 0x66, 0x06, 0x0C, 0x18, 0x00, 0x18, 0x00, 
    0x3C, 0x66, 0x6E, 0x6E, 0x60, 0x62, 0x3C, 0x00, 0x18, 0x3C, 0x66, 0x66, 0x7E, 0x66, 0x66, 0x00, 
    0x7C, 0x66, 0x66, 0x7C, 0x66, 0x66, 0x7C, 0x00, 0x3C, 0x66, 0x60, 0x60, 0x60, 0x66, 0x3C, 0x00, 
    0x78, 0x6C, 0x66, 0x66, 0x66, 0x6C, 0x78, 0x00, 0x7E, 0x60, 0x60, 0x78, 0x60, 0x60, 0x7E, 0x00, 
    0x7E, 0x60, 0x60, 0x78, 0x60, 0x60, 0x60, 0x00, 0x3C, 0x66, 0x60, 0x6E, 0x66, 0x66, 0x3C, 0x00, 
    0x66, 0x66, 0x66, 0x7E, 0x66, 0x66, 0x66, 0x00, 0x3C, 0x18, 0x18, 0x18, 0x18, 0x18, 0x3C, 0x00, 
    0x1E, 0x0C, 0x0C, 0x0C, 0xCC, 0xCC, 0x78, 0x00, 0x66, 0x6C, 0x78, 0x70, 0x78, 0x6C, 0x66, 0x00, 
    0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x7E, 0x00, 0x63, 0x77, 0x7F, 0x7F, 0x6B, 0x63, 0x63, 0x00, 
    0x63, 0x73, 0x7B, 0x6F, 0x67, 0x63, 0x63, 0x00, 0x3C, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x00, 
    0x7C, 0x66, 0x66, 0x7C, 0x60, 0x60, 0x60, 0x00, 0x3C, 0x66, 0x66, 0x66, 0x6A, 0x6C, 0x36, 0x00, 
    0x7C, 0x66, 0x66, 0x7C, 0x6C, 0x66, 0x63, 0x00, 0x3C, 0x66, 0x60, 0x3C, 0x06, 0x66, 0x3C, 0x00, 
    0x7E, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x00, 
    0x66, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x18, 0x00, 0x63, 0x63, 0x63, 0x6B, 0x7F, 0x77, 0x63, 0x00, 
    0x66, 0x66, 0x3C, 0x18, 0x3C, 0x66, 0x66, 0x00, 0x66, 0x66, 0x66, 0x3C, 0x18, 0x18, 0x18, 0x00, 
    0x7E, 0x06, 0x0C, 0x18, 0x30, 0x60, 0x7E, 0x00, 0x3C, 0x30, 0x30, 0x30, 0x30, 0x30, 0x3C, 0x00, 
    0x00, 0x60, 0x30, 0x18, 0x0C, 0x06, 0x00, 0x00, 0x3C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x3C, 0x00, 
    0x08, 0x1C, 0x36, 0x63, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 
    0x0C, 0x0C, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x06, 0x3E, 0x66, 0x3E, 0x00, 
    0x60, 0x60, 0x7C, 0x66, 0x66, 0x66, 0x7C, 0x00, 0x00, 0x00, 0x3C, 0x60, 0x60, 0x60, 0x3C, 0x00, 
    0x06, 0x06, 0x3E, 0x66, 0x66, 0x66, 0x3E, 0x00, 0x00, 0x00, 0x3C, 0x66, 0x7E, 0x60, 0x3C, 0x00, 
    0x1C, 0x36, 0x30, 0x78, 0x30, 0x30, 0x30, 0x00, 0x00, 0x00, 0x3E, 0x66, 0x66, 0x3E, 0x06, 0x7C, 
    0x60, 0x60, 0x7C, 0x66, 0x66, 0x66, 0x66, 0x00, 0x18, 0x00, 0x38, 0x18, 0x18, 0x18, 0x3C, 0x00, 
    0x06, 0x00, 0x06, 0x06, 0x06, 0x06, 0x06, 0x3C, 0x60, 0x60, 0x66, 0x6C, 0x78, 0x6C, 0x66, 0x00, 
    0x38, 0x18, 0x18, 0x18, 0x18, 0x18, 0x3C, 0x00, 0x00, 0x00, 0xEC, 0xFE, 0xD6, 0xD6, 0xD6, 0x00, 
    0x00, 0x00, 0xDC, 0x66, 0x66, 0x66, 0x66, 0x00, 0x00, 0x00, 0x3C, 0x66, 0x66, 0x66, 0x3C, 0x00, 
    0x00, 0x00, 0xDC, 0x66, 0x66, 0x7C, 0x60, 0xF0, 0x00, 0x00, 0x3E, 0x66, 0x66, 0x3E, 0x06, 0x0F, 
    0x00, 0x00, 0xDC, 0x62, 0x60, 0x60, 0xF0, 0x00, 0x00, 0x00, 0x3C, 0x60, 0x3C, 0x06, 0x7C, 0x00, 
    0x30, 0x30, 0x78, 0x30, 0x30, 0x36, 0x1C, 0x00, 0x00, 0x00, 0x66, 0x66, 0x66, 0x66, 0x3E, 0x00, 
    0x00, 0x00, 0x66, 0x66, 0x66, 0x3C, 0x18, 0x00, 0x00, 0x00, 0xD6, 0xD6, 0xD6, 0xFE, 0x6C, 0x00, 
    0x00, 0x00, 0x66, 0x3C, 0x18, 0x3C, 0x66, 0x00, 0x00, 0x00, 0x66, 0x66, 0x66, 0x3E, 0x06, 0x7C, 
    0x00, 0x00, 0x7E, 0x0C, 0x18, 0x30, 0x7E, 0x00, 0x0E, 0x18, 0x18, 0x70, 0x18, 0x18, 0x0E, 0x00, 
    0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00, 0x70, 0x18, 0x18, 0x0E, 0x18, 0x18, 0x70, 0x00, 
    0x76, 0xDC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  
};

// DRIVERS

class LCD_ST7796 : public Adafruit_GFX {
public:
    LCD_ST7796() : Adafruit_GFX(LCD_WIDTH, LCD_HEIGHT) {}

    void begin() {
        pinMode(LCD_CS, OUTPUT);
        pinMode(LCD_RST, OUTPUT);
        pinMode(LCD_DC, OUTPUT);
        pinMode(LCD_BL, OUTPUT);
        digitalWrite(LCD_CS, HIGH);
        
        SPI.begin(LCD_SCK, LCD_MISO, LCD_MOSI, LCD_CS);
        SPI.setFrequency(40000000); 

        digitalWrite(LCD_RST, LOW); delay(100);
        digitalWrite(LCD_RST, HIGH); delay(10);

        writeCmd(0x11); delay(120); 
        writeCmd(0x36); writeData(0x28); 
        writeCmd(0x3A); writeData(0x05); 
        writeCmd(0xB4); writeData(0x01);
        writeCmd(0xB7); writeData(0xC6);
        writeCmd(0x21); 
        writeCmd(0x29); 
        setBrightness(100);
    }

    void setBrightness(uint8_t percent) { analogWrite(LCD_BL, map(percent, 0, 100, 0, 255)); }
    
    void writeCmd(uint8_t cmd) { 
        digitalWrite(LCD_DC, LOW); digitalWrite(LCD_CS, LOW); 
        SPI.transfer(cmd); 
        digitalWrite(LCD_CS, HIGH); 
    }
    
    void writeData(uint8_t data) { 
        digitalWrite(LCD_DC, HIGH); digitalWrite(LCD_CS, LOW); 
        SPI.transfer(data); 
        digitalWrite(LCD_CS, HIGH); 
    }
    
    void setWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
        writeCmd(0x2A); writeData(x >> 8); writeData(x & 0xFF); writeData((x+w-1) >> 8); writeData((x+w-1) & 0xFF);
        writeCmd(0x2B); writeData(y >> 8); writeData(y & 0xFF); writeData((y+h-1) >> 8); writeData((y+h-1) & 0xFF);
        writeCmd(0x2C);
    }
    
    void drawChar(char c, uint16_t x, uint16_t y, uint16_t color, uint16_t bg) {
        if(c < 32 || c > 127) return; 
        uint16_t w = 8, h = 8; 
        int fontIdx = (c - 32) * 8; 
        
        uint8_t buf[128]; 
        int idx = 0;
        
        uint8_t hi = color >> 8; uint8_t lo = color & 0xFF;
        uint8_t bgHi = bg >> 8; uint8_t bgLo = bg & 0xFF;

        for(int row = 0; row < h; row++) {
            uint8_t rowByte = pgm_read_byte(&font8x8_basic[fontIdx + row]);
            for(int col = 0; col < w; col++) {
                bool on = (rowByte & (0x80 >> col));
                buf[idx++] = on ? hi : bgHi;
                buf[idx++] = on ? lo : bgLo;
            }
        }
        setWindow(x, y, w, h);
        digitalWrite(LCD_DC, HIGH); digitalWrite(LCD_CS, LOW);
        SPI.writeBytes(buf, 128);
        digitalWrite(LCD_CS, HIGH);
    }

    void drawPixel(int16_t x, int16_t y, uint16_t color) override {
        if((x < 0) ||(x >= _width) || (y < 0) || (y >= _height)) return;
        setWindow(x, y, 1, 1);
        digitalWrite(LCD_DC, HIGH); digitalWrite(LCD_CS, LOW);
        uint8_t data[] = { (uint8_t)(color >> 8), (uint8_t)(color & 0xFF) };
        SPI.writeBytes(data, 2);
        digitalWrite(LCD_CS, HIGH);
    }

    void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) override {
        if (w <= 0 || h <= 0) return;
        if((x >= _width) || (y >= _height)) return;
        if((x + w - 1) >= _width)  w = _width  - x;
        if((y + h - 1) >= _height) h = _height - y;

        setWindow(x, y, w, h);
        digitalWrite(LCD_DC, HIGH); digitalWrite(LCD_CS, LOW);
        
        uint8_t hi = color >> 8; 
        uint8_t lo = color & 0xFF;
        uint8_t lineBuf[128]; 
        int chunkWidth = min((int)w, 64);
        
        for(int i=0; i<chunkWidth; i++) { 
            lineBuf[i*2] = hi; 
            lineBuf[i*2+1] = lo; 
        }

        uint32_t totalBytes = (uint32_t)w * h * 2;
        uint32_t chunkBytes = chunkWidth * 2;

        while(totalBytes > 0) {
            uint32_t writeSize = (totalBytes > chunkBytes) ? chunkBytes : totalBytes;
            SPI.writeBytes(lineBuf, writeSize);
            totalBytes -= writeSize;
        }
        digitalWrite(LCD_CS, HIGH);
    }
    
    void drawImageRaw(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint8_t* data, size_t len) {
        if (!data) return;
        setWindow(x, y, w, h);
        digitalWrite(LCD_DC, HIGH); digitalWrite(LCD_CS, LOW); 
        SPI.writeBytes(data, len); 
        digitalWrite(LCD_CS, HIGH);
    }
};

// STRUCTS

struct ListItem {
    String name;
    String id;
};

struct SharedState {
    DynamicJsonDocument spotify{4096};
    DynamicJsonDocument weather{1024}; 
    DynamicJsonDocument calendar{1024};
    DynamicJsonDocument flights{8192}; 
    DynamicJsonDocument stats{1024};
    DynamicJsonDocument artist_stats{512};
    
    std::vector<ListItem> playlists;
    
    int track_count = 0;
    String current_playlist_id = "";
    
    std::vector<uint32_t> track_offsets;
    
    bool looper_loading = false;
    
    bool new_weather = false;
    bool new_calendar = false;
    bool new_flights = false;
    bool new_stats = false;
    bool new_cover_ready = false;
    
    String current_cover_url = "";
    String current_artist_id = "";
    std::vector<uint8_t> cover_data;
    uint32_t last_spotify_tick = 0;
    
    bool shuffle_state = false;
    String repeat_state = "off";
    
    int volume = 50;
    bool is_browsing = false; 
};

SharedState* state = nullptr;
SemaphoreHandle_t mutex;
volatile int encoder_diff = 0;


class HapticMotor {
    uint8_t _addr = 0x36;
    float _zero_offset = 0.0f;
    float _target_angle = 0.0f;
    float _prev_error = 0.0f;
    float _last_angle_metric = 0.0f; 
    int _current_slot = 0;
    
    int _detent_count = 12;
    float _p_gain = 0.08;
    float _d_gain = 0.01;
    bool _smart_freewheel_enabled = false;
    
    bool _is_freewheeling = false;
    SemaphoreHandle_t _lock = NULL;
    
public:
    void begin() {
        _lock = xSemaphoreCreateMutex();
        pinMode(EN_PIN, OUTPUT); digitalWrite(EN_PIN, HIGH);
        ledcAttach(N1_PIN, 25000, 10);
        ledcAttach(N2_PIN, 25000, 10);
        ledcAttach(N3_PIN, 25000, 10);
        setPhaseVoltage(VOLTAGE_LIMIT, 0); delay(1000);
        _zero_offset = readAngle(); _last_angle_metric = _zero_offset;
        setPwm(0, 0, 0);
        _target_angle = 0; _current_slot = 0;
    }
    
    void configure(int detents, float p_gain, bool smart_enable) {
        if (!_lock) return;
        xSemaphoreTake(_lock, portMAX_DELAY);
        if (_detent_count == detents && fabs(_p_gain - p_gain) < 0.001 && _smart_freewheel_enabled == smart_enable) {
            xSemaphoreGive(_lock); return;
        }
        _detent_count = detents; _p_gain = p_gain; _smart_freewheel_enabled = smart_enable;
        _is_freewheeling = false; 
        xSemaphoreGive(_lock);
    }

    float readAngle() {
        Wire.beginTransmission(_addr); Wire.write(0x0C); Wire.endTransmission();
        Wire.requestFrom(_addr, (uint8_t)2);
        if (Wire.available() >= 2) {
            uint8_t hi = Wire.read(); uint8_t lo = Wire.read();
            uint16_t raw = (hi << 8) | lo;
            return (raw / 4096.0f) * 2 * PI;
        }
        return 0;
    }

    void setPwm(float u, float v, float w) {
        ledcWrite(N1_PIN, (int)(u * 1023));
        ledcWrite(N2_PIN, (int)(v * 1023));
        ledcWrite(N3_PIN, (int)(w * 1023));
    }

    void setPhaseVoltage(float voltage, float angle_el) {
        if (voltage > VOLTAGE_LIMIT) voltage = VOLTAGE_LIMIT; if (voltage < 0) voltage = 0;
        float center = 0.5f;
        float u = voltage * sin(angle_el) + center;
        float v = voltage * sin(angle_el + 2.0944f) + center;
        float w = voltage * sin(angle_el + 4.1888f) + center;
        setPwm(u, v, w);
    }

    void loop(float dt) {
        if (dt <= 0.0001f) return;
        int local_detents; float local_p, local_d; bool local_smart;
        if (_lock) {
             if(xSemaphoreTake(_lock, 0)) { 
                 local_detents = _detent_count; local_p = _p_gain; local_d = _d_gain; local_smart = _smart_freewheel_enabled;
                 xSemaphoreGive(_lock);
             } else {
                 local_detents = _detent_count; local_p = _p_gain; local_d = _d_gain; local_smart = _smart_freewheel_enabled;
             }
        } else return;

        float real_angle = readAngle();
        float delta = real_angle - _last_angle_metric;
        if (delta > PI) delta -= 2*PI; else if (delta < -PI) delta += 2*PI;
        float velocity = delta / dt;
        _last_angle_metric = real_angle;
        
        if (local_smart) {
            float speed = fabs(velocity);
            if (_is_freewheeling) {
                if (speed < STOP_THRESH) {
                    _is_freewheeling = false;
                    float slot_rad = (2 * PI) / local_detents;
                    int nearest = round(real_angle / slot_rad);
                    _target_angle = nearest * slot_rad;
                }
            } else { if (speed > FLING_THRESH) _is_freewheeling = true; }
        } else { _is_freewheeling = false; }

        if (_is_freewheeling) {
            float electrical_angle = real_angle * POLE_PAIRS;
            float dir = (velocity > 0) ? 1.0f : -1.0f;
            float phase_angle = electrical_angle + (dir * PI/2);
            setPhaseVoltage(MOMENTUM_ASSIST, phase_angle);
            _target_angle = real_angle; 
        } else {
            float error = _target_angle - real_angle;
            while (error > PI) error -= 2 * PI; while (error < -PI) error += 2 * PI;
            float error_rate = (error - _prev_error) / dt; _prev_error = error;
            float control_signal = (error * local_p) + (error_rate * local_d);
            float voltage = fabs(control_signal);
            float electrical_angle = real_angle * POLE_PAIRS;
            float phase_angle = (control_signal > 0) ? (electrical_angle + PI/2) : (electrical_angle - PI/2);
            if (fabs(error) < 0.05f) voltage = 0; 
            setPhaseVoltage(voltage, phase_angle);
        }

        float slot_rad = (2 * PI) / local_detents;
        int nearest_slot = round(real_angle / slot_rad);
        int normalized_slot = nearest_slot % local_detents;
        if (normalized_slot < 0) normalized_slot += local_detents;

        if (!_is_freewheeling) _target_angle = nearest_slot * slot_rad;
        
        if (normalized_slot != _current_slot) {
            int diff = normalized_slot - _current_slot;
            if (diff > local_detents/2) diff -= local_detents;
            else if (diff < -local_detents/2) diff += local_detents;
            encoder_diff += diff; 
            _current_slot = normalized_slot;
        }
    }
};

HapticMotor haptics;

void hapticsTaskCode(void * parameter) {
    haptics.begin();
    uint32_t last_us = micros();
    while(true) {
        uint32_t now = micros();
        float dt = (now - last_us) / 1000000.0f;
        if (dt > 0) { haptics.loop(dt); last_us = now; }
        delay(1); 
    }
}

// API HELPERS

String fetchString(String endpoint) {
    if (WiFi.status() != WL_CONNECTED) return "";
    
    WiFiClientSecure *client = new WiFiClientSecure;
    client->setInsecure();
    client->setTimeout(10000); 
    
    HTTPClient http;
    String url = String(API_BASE) + endpoint;
    String payload = "";
    
    if (http.begin(*client, url)) { 
        http.addHeader("API-Key", API_KEY);
        int code = http.GET();
        if (code > 0) {
            payload = http.getString();
        }
        http.end();
    }
    
    client->stop();
    delete client; 
    return payload;
}

void sendAsyncCmd(String endpoint) { fetchString(endpoint); }

// UI

enum LooperMode { L_OFF, L_PLAYLISTS, L_TRACKS };

class Dashboard {
    LCD_ST7796* lcd;
    U8G2_FOR_ADAFRUIT_GFX u8g2; 

public:
    int current_page = 0;
    String last_song = "";
    bool last_is_playing = false;
    int last_bar_width = -1;
    int last_volume = -1;
    int last_nav_index = -1;
    bool last_vol_mode = false;
    bool last_active_status = false;
    bool force_redraw_controls = false;
    bool needs_redraw_left = true;
    
    String last_drawn_song = "";
    String last_drawn_artist = "";
    String last_drawn_time = "";
    
    int last_left_tick = 0; 
    
    bool last_shuffle = false;
    String last_repeat = "";
    
    LooperMode l_mode = L_OFF;
    int l_scroll = 0;
    int l_range_start = -1;
    int l_range_end = -1;
    int last_l_scroll = -1;

    Dashboard(LCD_ST7796* _lcd) : lcd(_lcd) {}

    void initUI() {
        u8g2.begin(*lcd);
        u8g2.setFont(u8g2_font_wqy12_t_gb2312); 
        u8g2.setFontMode(1); 

        lcd->fillRect(0, 0, 480, 320, THEME_BG);
        lcd->fillRect(0, 318, 480, 2, THEME_ACCENT); 
        lcd->fillRect(0, 9, 2, 311, THEME_ACCENT);   
        lcd->fillRect(478, 9, 2, 311, THEME_ACCENT); 
        lcd->fillRect(160, 9, 2, 311, THEME_ACCENT); 
        drawSectionHeader("SPOTIFY", 5, 160, 320); 
        drawLeftWidgets();
    }
    
    int getTextWidth(String text) {
        int width = 0;
        int len = text.length();
        int i = 0;
        while (i < len) {
            uint8_t c = text[i];
            if (c < 128) {
                width += 8;
                i++;
            } else {
                int seqLen = 1;
                if ((c & 0xE0) == 0xC0) seqLen = 2;
                else if ((c & 0xF0) == 0xE0) seqLen = 3;
                else if ((c & 0xF8) == 0xF0) seqLen = 4;
                
                char buf[5] = {0};
                for(int k=0; k<seqLen && (i+k)<len; k++) buf[k] = text[i+k];
                width += u8g2.getUTF8Width(buf);
                i += seqLen;
            }
        }
        return width;
    }

    void drawText(String text, int x, int y, uint16_t color, uint16_t bg = THEME_BG, int maxW = 480) {
        int cursorX = x;
        int len = text.length();
        int i = 0;
        
        while (i < len) {
            uint8_t c = text[i];
            
            if (c < 128) {
                if (cursorX + 8 > x + maxW) break; 
                lcd->drawChar(c, cursorX, y, color, bg); 
                cursorX += 8;
                i++;
            } 
            else {
                int seqLen = 1;
                if ((c & 0xE0) == 0xC0) seqLen = 2;
                else if ((c & 0xF0) == 0xE0) seqLen = 3;
                else if ((c & 0xF8) == 0xF0) seqLen = 4;
                
                char buf[5] = {0};
                for(int k=0; k<seqLen && (i+k)<len; k++) buf[k] = text[i+k];
                
                int w = u8g2.getUTF8Width(buf);
                if (cursorX + w > x + maxW) break; 
                
                u8g2.setForegroundColor(color);
                u8g2.setBackgroundColor(bg);
                u8g2.drawUTF8(cursorX, y + 8, buf); 
                
                cursorX += w;
                i += seqLen;
            }
            yield();
        }
    }
    
    void drawTextBold(String text, int x, int y, uint16_t color, uint16_t bg = THEME_BG) {
        drawText(text, x, y, color, bg);
        drawText(text, x+1, y, color, bg);
    }

    void drawBox(int x, int y, int w, int h, uint16_t color) {
        lcd->fillRect(x, y, w, 1, color);
        lcd->fillRect(x, y + h - 1, w, 1, color);
        lcd->fillRect(x, y, 1, h, color);
        lcd->fillRect(x + w - 1, y, 1, h, color);
    }
    
    void drawSectionHeader(String text, int y, int x_start, int width) {
        int textWidth = text.length() * 8; 
        int lineY = y + 4; 
        lcd->fillRect(x_start + 2, y, width - 4, 8, THEME_BG);
        lcd->fillRect(x_start + 2, lineY, 8, 2, THEME_ACCENT); 
        drawText(text, x_start + 15, y, THEME_ACCENT, THEME_BG);
        int rightLineStart = x_start + 15 + textWidth + 5;
        int rightLineEnd = x_start + width;
        if (rightLineStart < rightLineEnd) {
             lcd->fillRect(rightLineStart, lineY, rightLineEnd - rightLineStart, 2, THEME_ACCENT);
        }
    }

    String padTo(String s, int len) {
        while(s.length() < len) s += " ";
        return s;
    }

    String getScrollText(String text, int maxChars) {
        int utf8Len = 0;
        for(int i=0; i<text.length(); ) {
             uint8_t c = text[i];
             int s = 1;
             if ((c & 0xE0) == 0xC0) s = 2;
             else if ((c & 0xF0) == 0xE0) s = 3;
             else if ((c & 0xF8) == 0xF0) s = 4;
             i += s;
             utf8Len++;
        }

        if (utf8Len <= maxChars) return text;

        String pad = "   ";
        String full = text + pad + text; 
        int tick = millis() / 300;
        int startChar = tick % (utf8Len + 3); 
        
        int byteStart = 0;
        for(int i=0; i<startChar; i++) {
             if (byteStart >= full.length()) break;
             uint8_t c = full[byteStart];
             int s = 1;
             if ((c & 0xE0) == 0xC0) s = 2;
             else if ((c & 0xF0) == 0xE0) s = 3;
             else if ((c & 0xF8) == 0xF0) s = 4;
             byteStart += s;
        }

        int byteEnd = byteStart;
        for(int i=0; i<maxChars; i++) {
             if (byteEnd >= full.length()) break;
             uint8_t c = full[byteEnd];
             int s = 1;
             if ((c & 0xE0) == 0xC0) s = 2;
             else if ((c & 0xF0) == 0xE0) s = 3;
             else if ((c & 0xF8) == 0xF0) s = 4;
             byteEnd += s;
        }
        
        return full.substring(byteStart, byteEnd);
    }

    void drawLeftWidgets() {
        lcd->fillRect(2, 9, 158, 307, THEME_BG);
        last_drawn_time = "";
        
        if (l_mode != L_OFF) {
            drawLooper(true); 
            return;
        }
        
        if(current_page == 0) {
            drawSectionHeader("CALENDAR", 5, 0, 160);
            drawSectionHeader("WEATHER", 100, 0, 160);
            drawSectionHeader("AIRSPACE", 195, 0, 160);
            xSemaphoreTake(mutex, portMAX_DELAY);
            state->new_weather = true; state->new_calendar = true; state->new_flights = true;
            xSemaphoreGive(mutex);
        } else {
            drawSectionHeader("STATS", 5, 0, 160);
            drawSectionHeader("ARTISTS", 100, 0, 160);
            drawSectionHeader("SONGS", 195, 0, 160);
            xSemaphoreTake(mutex, portMAX_DELAY);
            state->new_stats = true;
            xSemaphoreGive(mutex);
        }
        needs_redraw_left = true;
    }

    void drawLooper(bool full_redraw) {
        xSemaphoreTake(mutex, portMAX_DELAY);
        bool loading = state->looper_loading;
        int list_size = (l_mode == L_PLAYLISTS) ? state->playlists.size() : state->track_count;
        xSemaphoreGive(mutex);
        
        int total_items = list_size + 2; 

        if (full_redraw) {
            lcd->fillRect(2, 9, 158, 290, THEME_BG);
            drawSectionHeader(l_mode == L_PLAYLISTS ? "PLAYLISTS" : "TRACKS", 5, 0, 160);
        } else {
            lcd->fillRect(2, 17, 158, 280, THEME_BG);
        }
        
        if (loading) {
            drawText("Loading...", 40, 150, WHITE);
            return;
        }
        
        int visible_lines = 13; 
        int start_idx = l_scroll - (visible_lines / 2);
        if (start_idx > total_items - visible_lines) start_idx = max(0, total_items - visible_lines);
        if (start_idx < 0) start_idx = 0;

        File file;
        if (l_mode == L_TRACKS) {
            file = LittleFS.open("/tracks.txt", FILE_READ);
            if (file) {
                int first_line_to_read = start_idx - 1; 
                if (first_line_to_read < 0) first_line_to_read = 0;
                
                uint32_t seek_pos = 0;
                xSemaphoreTake(mutex, portMAX_DELAY);
                if (first_line_to_read >= 0 && first_line_to_read < state->track_offsets.size()) {
                    seek_pos = state->track_offsets[first_line_to_read];
                }
                xSemaphoreGive(mutex);
                
                file.seek(seek_pos);
            }
        }

        int y = 25; 
        
        for(int i=0; i<visible_lines; i++) {
            int view_idx = start_idx + i; 
            if (view_idx >= total_items) break;
            
            String name = "";
            bool is_back_btn = false;

            if (view_idx == 0 || view_idx == total_items - 1) {
                name = "[..] BACK";
                is_back_btn = true;
            } else {
                int vec_idx = view_idx - 1; 
                if (l_mode == L_PLAYLISTS) {
                    xSemaphoreTake(mutex, portMAX_DELAY);
                    name = state->playlists[vec_idx].name;
                    xSemaphoreGive(mutex);
                } else if (file) {
                    if (!file.available()) break;
                    String line = file.readStringUntil('\n');
                    int delim = line.indexOf('|');
                    if (delim != -1) name = line.substring(delim + 1);
                }
            }
            
            bool selected = (view_idx == l_scroll);
            uint16_t col = selected ? WHITE : GRAY;
            
            if (is_back_btn) {
                if (selected) col = THEME_ACCENT;
            } else {
                if (l_mode == L_TRACKS && l_range_start != -1) {
                    int current_item_idx = view_idx - 1;
                    int scroll_item_idx = l_scroll - 1;
                    int r_end = (l_range_end == -1) ? scroll_item_idx : l_range_end;
                    int r_min = min(l_range_start, r_end);
                    int r_max = max(l_range_start, r_end);
                    
                    if (current_item_idx >= r_min && current_item_idx <= r_max) col = YELLOW; 
                    if (selected) col = WHITE; 
                }
            }

            if (selected) drawText(">", 2, y, THEME_ACCENT);
            drawText(name, 12, y, col, THEME_BG, 140);
            
            yield(); 
            y += 20; 
        }
        
        if (file) file.close();
    }

    void drawClock() {
        struct tm timeinfo;
        if(!getLocalTime(&timeinfo)) return;
        char buf[32];
        strftime(buf, sizeof(buf), "%b-%d %H:%M", &timeinfo);
        String timeStr = String(buf);
        if (timeStr != last_drawn_time) {
            lcd->fillRect(5, 305, 150, 10, THEME_BG);
            drawText(timeStr, 5, 305, THEME_ACCENT);
            last_drawn_time = timeStr;
        }
    }

    void drawPlayIcon(int x, int y, uint16_t color) {
        lcd->fillRect(x-2, y-2, 16, 16, THEME_BG); 
        for(int i=0; i<8; i++) lcd->fillRect(x+i, y+i, 2, 16-(i*2), color);
    }
    
    void drawPauseIcon(int x, int y, uint16_t color) {
        lcd->fillRect(x-2, y-2, 16, 16, THEME_BG);
        lcd->fillRect(x, y, 4, 12, color); 
        lcd->fillRect(x+8, y, 4, 12, color);
    }
    
    void drawNextIcon(int x, int y, uint16_t color) {
        for(int i=0; i<8; i++) lcd->fillRect(x+i, y+i, 1, 16-(i*2), color);
        for(int i=0; i<8; i++) lcd->fillRect(x+8+i, y+i, 1, 16-(i*2), color);
    }
    void drawPrevIcon(int x, int y, uint16_t color) {
         for(int i=0; i<8; i++) lcd->fillRect(x+(8-i), y+i, 1, 16-(i*2), color);
         for(int i=0; i<8; i++) lcd->fillRect(x+8+(8-i), y+i, 1, 16-(i*2), color);
    }
    
    void updateWeather(bool full_redraw) {
        if(current_page != 0 || l_mode != L_OFF) return;
        xSemaphoreTake(mutex, portMAX_DELAY);
        JsonObject w = state->weather.as<JsonObject>();
        if(w.isNull()) { xSemaphoreGive(mutex); return; }
        
        if (full_redraw) {
            lcd->fillRect(5, 115, 150, 70, THEME_BG);
            String temp = String(w["temp_c"].as<float>(), 1) + "C";
            String feels = "Feels: " + String(w["feels_like_c"].as<float>(), 1) + "C";
            String hum = "Humidity: " + String(w["humidity"].as<String>());
            String precip = "Precip: " + String(w["precip_mm"].as<float>(), 1) + "mm";
            String wind = "Wind: " + String(w["wind_kph"].as<float>(), 1) + "kph";
            
            drawTextBold(temp, 5, 115, WHITE);
            drawText(feels, 5, 130, GRAY);
            drawText(hum, 5, 145, GRAY);
            drawText(wind, 5, 160, GRAY);
            drawText(precip, 5, 175, GRAY);
        }
        xSemaphoreGive(mutex);
    }

    void updateCalendar(bool full_redraw) {
        if(current_page != 0 || l_mode != L_OFF) return;
        xSemaphoreTake(mutex, portMAX_DELAY);
        JsonArray events = state->calendar["events"].as<JsonArray>();
        
        if (full_redraw) lcd->fillRect(5, 20, 150, 75, THEME_BG); 
        int y = 20;
        if(events.size() == 0 && full_redraw) drawText("No Events", 10, y, GRAY);
        
        for(JsonVariant v : events) {
            if(y > 90) break;
            String summary = v["summary"].as<String>();
            String start = v["start"].as<String>(); 
            
            if (full_redraw) {
                String dateStr = start.substring(5, 10); 
                String timeStr = "";
                if(start.indexOf('T') > 0) timeStr = start.substring(11, 16);
                drawText(dateStr + " " + timeStr, 5, y, GRAY);
            }

            if (summary.length() > 14 || full_redraw) {
                if (!full_redraw) lcd->fillRect(5, y+10, 150, 10, THEME_BG); 
                drawText(getScrollText(summary, 14), 5, y+10, WHITE, THEME_BG, 140); // LIMIT WIDTH
            }
            y += 25;
        }
        xSemaphoreGive(mutex);
    }

    void updateFlights(bool full_redraw) {
        if(current_page != 0 || l_mode != L_OFF) return;
        xSemaphoreTake(mutex, portMAX_DELAY);
        
        JsonArray f;
        if (state->flights.is<JsonArray>()) {
            f = state->flights.as<JsonArray>();
        } else if (state->flights.containsKey("flights")) {
            f = state->flights["flights"].as<JsonArray>();
        }
        
        if (full_redraw) lcd->fillRect(5, 210, 150, 80, THEME_BG);
        int y = 210;
        
        if(f.isNull() || f.size() == 0) {
             if (full_redraw) drawText("No Traffic", 10, y, GRAY);
        } else {
            int maxLen = 0;
            for(JsonVariant v : f) {
                String s = v["type"].as<String>();
                if(s.length() > 10) maxLen = max(maxLen, (int)s.length());
            }

            for(JsonVariant v : f) {
                if(y > 290) break;
                String type = v["type"].as<String>();
                String callsign = v["callsign"].as<String>();
                
                if (type.length() > 0) {
                     if (full_redraw) drawText(callsign, 5, y, GRAY);
                     
                     if (type.length() > 10 || full_redraw) {
                         if (!full_redraw) lcd->fillRect(65, y, 90, 16, THEME_BG); 
                         drawText(getScrollText(type, 10), 65, y, WHITE, THEME_BG, 85); 
                     }
                     y += 15;
                }
            }
        }
        xSemaphoreGive(mutex);
    }

    void updateStats(bool full_redraw) {
        if(current_page != 1 || l_mode != L_OFF) return;
        xSemaphoreTake(mutex, portMAX_DELAY);
        JsonObject m = state->stats["month"].as<JsonObject>();
        JsonObject art = state->artist_stats.as<JsonObject>();
        
        if (full_redraw) {
            lcd->fillRect(5, 15, 150, 80, THEME_BG);
            drawText("MINUTES MONTH", 5, 20, GRAY);
            drawTextBold(String((int)m["minutes"]), 5, 30, WHITE);
            if (!art.isNull()) {
                drawText("ARTIST PLAYS", 5, 45, GRAY);
                drawTextBold(String((int)art["count"]), 5, 55, WHITE);
                drawText("DISCOVERED", 5, 70, GRAY);
                drawTextBold(art["first"].as<String>().substring(0, 10), 5, 80, WHITE);
            }
        }
        
        if (full_redraw) lcd->fillRect(5, 115, 150, 75, THEME_BG);
        int y = 115;
        if(!m.isNull()) {
            JsonArray arr = m["artists"].as<JsonArray>();
            for(JsonVariant v : arr) {
                 if(y > 185) break;
                 String disp = v["name"].as<String>() + " (" + v["count"].as<String>() + ")";
                 if (disp.length() > 18 || full_redraw) {
                     if (!full_redraw) lcd->fillRect(5, y - 5, 150, 20, THEME_BG); 
                     drawText(getScrollText(disp, 18), 5, y, WHITE, THEME_BG, 140); 
                 }
                 y += 20; 
            }
        }
        
        if (full_redraw) lcd->fillRect(5, 210, 150, 80, THEME_BG);
        y = 210;
        if(!m.isNull()) {
            JsonArray arr = m["tracks"].as<JsonArray>();
            for(JsonVariant v : arr) {
                 if(y > 290) break;
                 String disp = v["name"].as<String>() + " (" + v["count"].as<String>() + ")";
                 if (disp.length() > 18 || full_redraw) {
                     if (!full_redraw) lcd->fillRect(5, y - 5, 150, 20, THEME_BG); 
                     drawText(getScrollText(disp, 18), 5, y, WHITE, THEME_BG, 140); 
                 }
                 y += 20; 
            }
        }
        xSemaphoreGive(mutex);
    }

    String formatTime(int ms) {
        int sec = ms / 1000;
        return String(sec/60) + ":" + ((sec%60 < 10) ? "0" : "") + String(sec%60);
    }

    void updateSpotify(int nav_index, bool in_vol_mode) {
        if (!state) return; 
        xSemaphoreTake(mutex, portMAX_DELAY);
        JsonObject data = state->spotify.as<JsonObject>();
        
        int vol_x = 440, vol_y = 20, vol_w = 12, vol_h = 180;
        uint16_t vol_color = in_vol_mode ? RED : THEME_ACCENT;
        int current_fill_h = (state->volume / 100.0) * vol_h;
        
        bool sel_changed = (nav_index != last_nav_index) || (in_vol_mode != last_vol_mode);
        
        bool active = data["is_active"];
        if (active != last_active_status) {
             lcd->fillRect(162, 15, 316, 302, THEME_BG); 
             last_active_status = active;
             force_redraw_controls = true;
             last_volume = -1; 
        }

        if (!active) {
             int txtW = 16 * 8; 
             drawText("Spotify Inactive", 320 - (txtW/2), 150, GRAY);
             xSemaphoreGive(mutex);
             return;
        }

        String song = data["song_name"].as<String>();
        String artist = data["artist"].as<String>();
        int progress = data["progress_ms"];
        int duration = data["duration_ms"];
        bool is_playing = data["is_playing"];
        bool shuffle = data["shuffle_state"];
        String repeat = data["repeat_state"].as<String>();

        if (sel_changed) {
            uint16_t box_col = (nav_index == 6) ? WHITE : THEME_BG;
            drawBox(vol_x-3, vol_y-3, vol_w+6, vol_h+6, box_col);
            drawText("VOL", 434, vol_y+vol_h+5, THEME_ACCENT);
        }

        if (state->volume != last_volume || in_vol_mode != last_vol_mode || force_redraw_controls) {
            lcd->fillRect(vol_x, vol_y, vol_w, vol_h, GRAY);
            lcd->fillRect(vol_x, vol_y + (vol_h - current_fill_h), vol_w, current_fill_h, vol_color);
            last_volume = state->volume;
        }

        if (state->new_cover_ready) {
            lcd->drawImageRaw(220, 20, 200, 200, state->cover_data.data(), state->cover_data.size());
            state->new_cover_ready = false;
        }

        if (is_playing) {
            long dt = millis() - state->last_spotify_tick;
            if (dt > 3000) dt = 3000;
            progress += dt;
        }
        if (progress > duration) progress = duration;

        int maxLen = 0;
        if (song.length() > 22) maxLen = max(maxLen, (int)song.length());
        if (artist.length() > 22) maxLen = max(maxLen, (int)artist.length());
        
        if (song.length() > 22) song = padTo(song, maxLen);
        if (artist.length() > 22) artist = padTo(artist, maxLen);

        String dSong = getScrollText(song, 22); 
        String dArtist = getScrollText(artist, 22);
        
        int songW = getTextWidth(dSong);
        int artW = getTextWidth(dArtist);
        
        int s_x = 320 - (songW / 2);
        int a_x = 320 - (artW / 2);
        
        if (s_x < 162) s_x = 162;
        if (a_x < 162) a_x = 162;

        int song_y = 230;
        int artist_y = 242;

        if (dSong != last_drawn_song || force_redraw_controls) {
            lcd->fillRect(162, song_y - 4, 270, 16, THEME_BG); 
            drawTextBold(dSong, s_x, song_y, WHITE);
            last_drawn_song = dSong;
        }

        if (dArtist != last_drawn_artist || force_redraw_controls) {
            lcd->fillRect(162, artist_y - 4, 270, 16, THEME_BG); 
            drawText(dArtist, a_x, artist_y, GRAY);
            last_drawn_artist = dArtist;
        }
        
        int bar_w = 200;
        int bar_x = 220; 
        int bar_y = 260;
        float pct = (float)progress / (float)duration;
        int w = (int)(bar_w * pct);
        
        if (w != last_bar_width) {
             lcd->fillRect(bar_x, bar_y, bar_w, 4, GRAY);
             lcd->fillRect(bar_x, bar_y, w, 4, THEME_ACCENT);
             drawText(formatTime(progress), bar_x - 45, bar_y - 2, WHITE);
             drawText(formatTime(duration), bar_x + bar_w + 5, bar_y - 2, WHITE);
             last_bar_width = w;
        }
        
        int ctrl_y = 280;
        int center_x = 320; 
        
        if (shuffle != last_shuffle || repeat != last_repeat || sel_changed || is_playing != last_is_playing || force_redraw_controls) {
             
             int lib_box_x = 166; 
             uint16_t lCol = (nav_index == 0) ? WHITE : THEME_BG;
             drawBox(lib_box_x, ctrl_y-4, 30, 24, lCol);
             lcd->fillRect(lib_box_x+2, ctrl_y-2, 26, 20, THEME_BG);
             drawText("LIB", lib_box_x+3, ctrl_y+4, (l_mode != L_OFF) ? GREEN : GRAY, THEME_BG);

             int shf_box_x = 202; 
             uint16_t sCol = (nav_index == 1) ? WHITE : THEME_BG;
             drawBox(shf_box_x, ctrl_y-4, 30, 24, sCol);
             lcd->fillRect(shf_box_x+2, ctrl_y-2, 26, 20, THEME_BG);
             drawText("SHF", shf_box_x+3, ctrl_y+4, shuffle ? GREEN : GRAY, THEME_BG);
             
             int prev_box_x = center_x - 12 - 50 - 4; 
             uint16_t pCol = (nav_index == 2) ? WHITE : THEME_BG;
             drawBox(prev_box_x, ctrl_y-4, 24, 24, pCol); 
             drawPrevIcon(prev_box_x + 4, ctrl_y, WHITE);

             int play_box_x = center_x - 12; 
             uint16_t plCol = (nav_index == 3) ? WHITE : THEME_BG;
             drawBox(play_box_x, ctrl_y-4, 24, 24, plCol);
             lcd->fillRect(play_box_x+2, ctrl_y-2, 20, 20, THEME_BG); 
             if(is_playing) drawPauseIcon(play_box_x+6, ctrl_y+2, WHITE);
             else drawPlayIcon(play_box_x+8, ctrl_y+2, WHITE);

             int next_box_x = center_x - 12 + 50 - 4; 
             uint16_t nCol = (nav_index == 4) ? WHITE : THEME_BG;
             drawBox(next_box_x, ctrl_y-4, 24, 24, nCol);
             drawNextIcon(next_box_x + 4, ctrl_y, WHITE);

             int rpt_box_x = 400; 
             uint16_t rCol = (nav_index == 5) ? WHITE : THEME_BG;
             drawBox(rpt_box_x, ctrl_y-4, 30, 24, rCol);
             String rText = "RPT";
             uint16_t rTxtCol = GRAY;
             if (repeat == "context") { rText = "ALL"; rTxtCol = GREEN; }
             else if (repeat == "track") { rText = "ONE"; rTxtCol = GREEN; }
             lcd->fillRect(rpt_box_x+2, ctrl_y-2, 26, 20, THEME_BG);
             drawText(rText, rpt_box_x + 3, ctrl_y+4, rTxtCol, THEME_BG);
             
             last_shuffle = shuffle;
             last_repeat = repeat;
             last_is_playing = is_playing;
        }
        
        force_redraw_controls = false; 
        last_nav_index = nav_index;
        last_vol_mode = in_vol_mode;
        xSemaphoreGive(mutex);
    }
};

// API CALLS

void fetchPlaylists() {
    String json = fetchString("/playlists?t=" + String(millis()));
    DynamicJsonDocument doc(16384);
    deserializeJson(doc, json);
    
    xSemaphoreTake(mutex, portMAX_DELAY);
    state->playlists.clear();
    JsonArray arr = doc["playlists"].as<JsonArray>();
    for (JsonVariant v : arr) {
        state->playlists.push_back({v["name"].as<String>(), v["id"].as<String>()});
    }
    state->looper_loading = false;
    xSemaphoreGive(mutex);
}

void buildTrackIndex() {
    std::vector<uint32_t> temp_offsets;
    temp_offsets.reserve(MAX_TRACKS + 2); 
    temp_offsets.push_back(0); 
    
    File f = LittleFS.open("/tracks.txt", FILE_READ);
    if (f) {
        uint8_t buf[256];
        uint32_t current_pos = 0;
        
        while(f.available()) {
            int bytesRead = f.read(buf, sizeof(buf));
            for (int i = 0; i < bytesRead; i++) {
                current_pos++;
                if (buf[i] == '\n') {
                    temp_offsets.push_back(current_pos);
                }
            }
            delay(1); 
        }
        f.close();
    }
    
    xSemaphoreTake(mutex, portMAX_DELAY);
    state->track_offsets = temp_offsets;
    xSemaphoreGive(mutex);
}

void checkAndFetchTracks(String playlist_id) {
    if (WiFi.status() != WL_CONNECTED) {
        xSemaphoreTake(mutex, portMAX_DELAY);
        state->looper_loading = false;
        xSemaphoreGive(mutex);
        return;
    }

    String saved_pid = "";
    String saved_snapshot = "";
    int saved_count = 0;
    
    if (LittleFS.exists("/meta.txt")) {
        File meta = LittleFS.open("/meta.txt", FILE_READ);
        if (meta) {
            saved_pid = meta.readStringUntil('\n'); saved_pid.trim();
            saved_snapshot = meta.readStringUntil('\n'); saved_snapshot.trim();
            saved_count = meta.readStringUntil('\n').toInt();
            meta.close();
        }
    }

    String check_url = "/tracks/check?playlist_id=" + playlist_id + "&t=" + String(millis());
    String check_resp = fetchString(check_url);
    
    StaticJsonDocument<256> check_doc;
    deserializeJson(check_doc, check_resp);
    String fetched_snapshot = check_doc["snapshot_id"].as<String>();

    if (fetched_snapshot != "" && saved_pid == playlist_id && saved_snapshot == fetched_snapshot && LittleFS.exists("/tracks.txt")) {
        Serial.println("CACHE HIT! Skipping download.");
        buildTrackIndex(); 
        
        xSemaphoreTake(mutex, portMAX_DELAY);
        state->track_count = saved_count;
        state->current_playlist_id = playlist_id;
        state->looper_loading = false;
        xSemaphoreGive(mutex);
        return;
    }

    Serial.println("DOWNLOADING TO FLASH...");
    delay(100); 
    
    WiFiClientSecure *client = new WiFiClientSecure;
    client->setInsecure();
    client->setTimeout(15000); 
    
    HTTPClient http;
    String url = String(API_BASE) + "/tracks?playlist_id=" + playlist_id + "&snapshot_id=" + fetched_snapshot + "&t=" + String(millis());
    
    if (http.begin(*client, url)) {
        http.addHeader("API-Key", API_KEY);
        int httpCode = http.GET();
        
        if (httpCode > 0) {
            WiFiClient* stream = http.getStreamPtr();
            
            File file = LittleFS.open("/tracks.txt", FILE_WRITE);
            if (!file) {
                 Serial.println("Failed to open file for writing");
                 http.end();
                 client->stop();
                 delete client;
                 return;
            }

            if (stream->find("[")) {
                int count = 0;
                while (http.connected() || stream->available()) {
                    if (!stream->available()) { delay(1); continue; }
                    
                    while (stream->available() && (stream->peek() == ',' || isspace(stream->peek()))) {
                        stream->read();
                    }
                    if (stream->peek() == ']') break; 

                    StaticJsonDocument<512> doc;
                    StaticJsonDocument<64> filter;
                    filter["track_name"] = true;
                    filter["id"] = true;
                    
                    DeserializationError error = deserializeJson(doc, *stream, DeserializationOption::Filter(filter));
                    
                    if (!error) {
                        String id = doc["id"].as<String>();
                        String name = doc["track_name"].as<String>();
                        
                        name.replace("\n", " "); 
                        name.replace("|", "");
                        
                        file.print(id);
                        file.print("|");
                        file.println(name);
                        
                        count++;
                        if (count >= MAX_TRACKS) break;
                    } 
                    delay(1); 
                }
                file.close();
                
                File metaWrite = LittleFS.open("/meta.txt", FILE_WRITE);
                if (metaWrite) {
                    metaWrite.println(playlist_id);
                    metaWrite.println(fetched_snapshot);
                    metaWrite.println(count);
                    metaWrite.close();
                }

                buildTrackIndex(); 
                
                xSemaphoreTake(mutex, portMAX_DELAY);
                state->track_count = count;
                state->current_playlist_id = playlist_id;
                xSemaphoreGive(mutex);
                
                Serial.println("Saved Tracks to Flash: " + String(count));
            } else {
                file.close();
            }
        }
        http.end();
    }
    
    client->stop();
    delete client; 
    
    xSemaphoreTake(mutex, portMAX_DELAY);
    state->looper_loading = false;
    xSemaphoreGive(mutex);
}

void spotifyTask(void* parameter) {
    while(true) {
        if (state) {
            String json = fetchString("/update-data");
            if (json.length() > 0) {
                xSemaphoreTake(mutex, portMAX_DELAY);
                state->spotify.clear();
                deserializeJson(state->spotify, json);
                
                String currentID = state->spotify["artist_id"].as<String>();
                String currentCover = state->spotify["cover_art"].as<String>();
                bool newArtist = (currentID != "" && currentID != state->current_artist_id);
                bool newCover = (currentCover != "" && currentCover != state->current_cover_url);
                bool isBrowsing = state->is_browsing;
                xSemaphoreGive(mutex);

                if (newArtist) {
                    String aStat = fetchString("/artist/stats?artist_id=" + currentID);
                    if(aStat.length() > 0) {
                        xSemaphoreTake(mutex, portMAX_DELAY);
                        state->artist_stats.clear();
                        deserializeJson(state->artist_stats, aStat);
                        state->current_artist_id = currentID;
                        state->new_stats = true; 
                        xSemaphoreGive(mutex);
                    }
                }

                if (newCover && !isBrowsing) {
                    WiFiClientSecure *imgClient = new WiFiClientSecure;
                    imgClient->setInsecure();
                    
                    HTTPClient http;
                    String imgUrl = String(API_BASE) + "/spotify/image?width=200&height=200&url=" + currentCover;
                    if(http.begin(*imgClient, imgUrl)) {
                         http.addHeader("API-Key", API_KEY);
                         if (http.GET() == 200) {
                             int len = http.getSize();
                             WiFiClient *stream = http.getStreamPtr();
                             std::vector<uint8_t> buf; 
                             if(len>0) buf.resize(len);
                             int total = 0;
                             while(http.connected() && (len>0 || len==-1)) {
                                 size_t size = stream->available();
                                 if(size) {
                                     if(len==-1) buf.resize(total+size);
                                     stream->readBytes(buf.data()+total, size);
                                     total+=size;
                                     if(len>0) len-=size;
                                 }
                                 delay(1);
                             }
                             xSemaphoreTake(mutex, portMAX_DELAY);
                             state->cover_data = buf;
                             state->current_cover_url = currentCover;
                             state->new_cover_ready = true;
                             xSemaphoreGive(mutex);
                         }
                         http.end();
                    }
                    imgClient->stop();
                    delete imgClient; 
                }
            }
        }
        delay(2000);
    }
}

void secondaryTask(void* parameter) {
    bool first_run = true;
    while(true) {
        if (state) {
            String f = fetchString("/flights/overhead?city=Toronto&radius_km=40&on_ground=false");
            if(f.length()>0) {
                 xSemaphoreTake(mutex, portMAX_DELAY);
                 state->flights.clear(); 
                 deserializeJson(state->flights, f);
                 state->new_flights = true; 
                 xSemaphoreGive(mutex);
            }
            delay(10);
            
            String c = fetchString("/calendar/events?count=3");
            if(c.length()>0) {
                 xSemaphoreTake(mutex, portMAX_DELAY);
                 state->calendar.clear(); deserializeJson(state->calendar, c); state->new_calendar = true;
                 xSemaphoreGive(mutex);
            }
            delay(10);
            
            String w = fetchString("/weather?city=London%20Ontario");
            if(w.length()>0) {
                DynamicJsonDocument localDoc(8192);
                if (!deserializeJson(localDoc, w)) {
                    struct tm timeinfo;
                    if(getLocalTime(&timeinfo)) {
                        JsonArray forecast = localDoc["forecast"].as<JsonArray>();
                        for(JsonVariant v : forecast) {
                            String t = v["time"].as<String>(); 
                            int hour = t.substring(11, 13).toInt();
                            if(hour == timeinfo.tm_hour) {
                                xSemaphoreTake(mutex, portMAX_DELAY);
                                state->weather.clear();
                                state->weather.set(v); 
                                state->new_weather = true;
                                xSemaphoreGive(mutex);
                                break;
                            }
                        }
                    }
                }
            }
            delay(10);
            
            String s = fetchString("/stats");
            if(s.length()>0) {
                 xSemaphoreTake(mutex, portMAX_DELAY);
                 state->stats.clear(); deserializeJson(state->stats, s); state->new_stats = true;
                 xSemaphoreGive(mutex);
            }
        }
        if (!first_run) delay(60000); 
        first_run = false;
    }
}

// SETUP & LOOP

LCD_ST7796 lcd;
Dashboard dash(&lcd);

void setup() {
    Serial.begin(115200);
    state = new SharedState();
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
    
    if (!LittleFS.begin(true)) {
        Serial.println("LittleFS Mount Failed");
    }
    
    xTaskCreatePinnedToCore(hapticsTaskCode, "Haptics", 4096, NULL, 10, NULL, 0);
    
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    lcd.begin();
    
    lcd.setCursor(170, 150);
    lcd.setTextColor(WHITE);
    lcd.setTextSize(1);
    lcd.print("Connecting WiFi...");
    
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    while (WiFi.status() != WL_CONNECTED) { delay(500); }
    
    configTime(0, 0, "pool.ntp.org");
    setenv("TZ", TIMEZONE, 1);
    tzset();
    
    mutex = xSemaphoreCreateMutex();
    dash.initUI();
    
    xTaskCreatePinnedToCore(spotifyTask, "Spotify", 12288, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(secondaryTask, "Secondary", 12288, NULL, 1, NULL, 1);
}

int nav_index = 3;
bool in_vol_mode = false;
bool last_btn = HIGH;
uint32_t last_btn_time = 0;
uint32_t last_page_switch = 0;

enum HapticState { H_MENU, H_VOLUME, H_LIST };
HapticState currentHapticState = H_MENU;

void loop() {
    int delta = 0;
    if (encoder_diff != 0) {
        noInterrupts();
        delta = encoder_diff;
        encoder_diff = 0;
        interrupts();
    }
    
    HapticState targetState = H_MENU;
    bool smartEnable = false;
    
    if (in_vol_mode) {
        targetState = H_VOLUME;
        smartEnable = false;
    } else if (dash.l_mode != L_OFF) {
        targetState = H_LIST;
        smartEnable = true;
    }
    
    if (targetState != currentHapticState) {
        if (targetState == H_VOLUME) {
            haptics.configure(50, 0.02, false);
        } else if (targetState == H_LIST) {
            haptics.configure(24, 0.05, true); 
        } else {
            haptics.configure(12, 0.08, false);
        }
        currentHapticState = targetState;
    }
    
    if (dash.l_mode == L_OFF) {
        if (state->is_browsing) {
            xSemaphoreTake(mutex, portMAX_DELAY);
            state->is_browsing = false;
            
            state->playlists.clear();
            state->playlists.shrink_to_fit();
            state->track_offsets.clear();
            state->track_offsets.shrink_to_fit();
            
            xSemaphoreGive(mutex);
        }
        
        if (delta != 0) {
            if (in_vol_mode) {
                xSemaphoreTake(mutex, portMAX_DELAY);
                state->volume = (delta > 0) ? min(100, state->volume+2) : max(0, state->volume-2);
                xSemaphoreGive(mutex);
                
                // SERIAL VOLUME CONTROL FOR PC SCRIPT
                udp.beginPacket(PC_HOTSPOT_IP, UDP_PORT);
                udp.print(delta);
                udp.endPacket();
                
            } else {
                nav_index += delta;
                if (nav_index < 0) nav_index = 6; 
                if (nav_index > 6) nav_index = 0;
            }
        }
    } else {
        if (!state->is_browsing) {
            xSemaphoreTake(mutex, portMAX_DELAY);
            state->is_browsing = true;
            xSemaphoreGive(mutex);
        }
        
        if (delta != 0) {
            xSemaphoreTake(mutex, portMAX_DELAY);
            int list_size = (dash.l_mode == L_PLAYLISTS) ? state->playlists.size() : state->track_count;
            int total_size = list_size + 1;
            xSemaphoreGive(mutex);
            
            dash.l_scroll += delta;
            if (dash.l_scroll < 0) dash.l_scroll = 0;
            if (dash.l_scroll > total_size) dash.l_scroll = total_size; 
        }
    }
    
    if (digitalRead(BUTTON_PIN) == LOW && last_btn == HIGH && (millis() - last_btn_time > 300)) {
        last_btn_time = millis();
        
        if (dash.l_mode == L_OFF) {
            if (in_vol_mode) in_vol_mode = false;
            else {
                 if (nav_index == 6) in_vol_mode = true; 
                 else if (nav_index == 0) { 
                     dash.l_mode = L_PLAYLISTS;
                     dash.l_scroll = 0;
                     xSemaphoreTake(mutex, portMAX_DELAY);
                     state->looper_loading = true;
                     xSemaphoreGive(mutex);
                     dash.drawLeftWidgets(); 
                     fetchPlaylists(); 
                     dash.drawLeftWidgets(); 
                 }
                 else if (nav_index == 3) { 
                     xSemaphoreTake(mutex, portMAX_DELAY);
                     bool isPlaying = state->spotify["is_playing"];
                     xSemaphoreGive(mutex);
                     if(isPlaying) sendAsyncCmd("/spotify/pause");
                     else sendAsyncCmd("/spotify/play");
                 }
                 else if (nav_index == 4) sendAsyncCmd("/spotify/next");
                 else if (nav_index == 2) sendAsyncCmd("/spotify/prev");
                 else if (nav_index == 1) { 
                     xSemaphoreTake(mutex, portMAX_DELAY);
                     bool current = state->spotify["shuffle_state"];
                     xSemaphoreGive(mutex);
                     sendAsyncCmd("/spotify/shuffle?state=" + String(current ? "false" : "true"));
                 }
                 else if (nav_index == 5) { 
                     xSemaphoreTake(mutex, portMAX_DELAY);
                     String current = state->spotify["repeat_state"].as<String>();
                     xSemaphoreGive(mutex);
                     String next = "off";
                     if(current == "off") next = "context";
                     else if(current == "context") next = "track";
                     sendAsyncCmd("/spotify/repeat?state=" + next);
                 }
            }
        } else {
            xSemaphoreTake(mutex, portMAX_DELAY);
            int list_size = (dash.l_mode == L_PLAYLISTS) ? state->playlists.size() : state->track_count;
            xSemaphoreGive(mutex);
            
            if (dash.l_scroll == 0 || dash.l_scroll == list_size + 1) {
                if (dash.l_mode == L_TRACKS) {
                    dash.l_mode = L_PLAYLISTS;
                    dash.l_scroll = 0;
                    dash.l_range_start = -1;
                    dash.l_range_end = -1;
                    
                    xSemaphoreTake(mutex, portMAX_DELAY);
                    state->track_offsets.clear();
                    state->track_offsets.shrink_to_fit();
                    xSemaphoreGive(mutex);
                } else {
                    dash.l_mode = L_OFF;
                }
                dash.drawLeftWidgets(); 
            } else {
                int idx = dash.l_scroll - 1;
                if (dash.l_mode == L_PLAYLISTS) {
                    String pid = state->playlists[idx].id;
                    Serial.println("SELECTED PLAYLIST: " + pid);
                    dash.l_mode = L_TRACKS;
                    dash.l_scroll = 0;
                    xSemaphoreTake(mutex, portMAX_DELAY);
                    state->looper_loading = true;
                    xSemaphoreGive(mutex);
                    dash.drawLeftWidgets(); 
                    checkAndFetchTracks(pid); 
                    dash.drawLeftWidgets(); 
                } else if (dash.l_mode == L_TRACKS) {
                    if (dash.l_range_start == -1) {
                        dash.l_range_start = idx; 
                    } else {
                        dash.l_range_end = idx; 
                        String ids = "";
                        int start = min(dash.l_range_start, dash.l_range_end);
                        int end = max(dash.l_range_start, dash.l_range_end);
                        
                        File file = LittleFS.open("/tracks.txt", FILE_READ);
                        if (file) {
                            uint32_t seek_pos = 0;
                            xSemaphoreTake(mutex, portMAX_DELAY);
                            if (start >= 0 && start < state->track_offsets.size()) {
                                seek_pos = state->track_offsets[start];
                            }
                            xSemaphoreGive(mutex);
                            file.seek(seek_pos);
                            
                            for(int i=start; i<=end; i++) {
                                if (!file.available()) break;
                                String line = file.readStringUntil('\n');
                                int delim = line.indexOf('|');
                                if (delim != -1) {
                                    if (ids.length() > 0) ids += "%20";
                                    ids += line.substring(0, delim);
                                }
                            }
                            file.close();
                        }
                        
                        dash.l_mode = L_OFF;
                        dash.l_range_start = -1; 
                        dash.l_range_end = -1;
                        dash.drawLeftWidgets();
                        
                        sendAsyncCmd("/start_playback?selected_songs=" + ids);
                    }
                }
            }
        }
    }
    last_btn = digitalRead(BUTTON_PIN);

    if(millis() - last_page_switch > 15000 && !in_vol_mode && dash.l_mode == L_OFF) {
        dash.current_page = (dash.current_page == 0) ? 1 : 0;
        dash.drawLeftWidgets(); 
        last_page_switch = millis();
    }
    
    dash.updateSpotify(nav_index, in_vol_mode);
    
    xSemaphoreTake(mutex, portMAX_DELAY);
    bool nw = state->new_weather, nc = state->new_calendar, nf = state->new_flights, ns = state->new_stats;
    xSemaphoreGive(mutex);

    int tick = millis() / 300;
    bool scroll = (tick != dash.last_left_tick);
    
    if (dash.l_mode != L_OFF) {
        if (dash.l_scroll != dash.last_l_scroll) {
            dash.drawLooper(false); 
            dash.last_l_scroll = dash.l_scroll;
        }
    } 
    else if(dash.current_page == 0) {
        if(nc || dash.needs_redraw_left) { dash.updateCalendar(true); state->new_calendar = false; }
        else if (scroll) { dash.updateCalendar(false); }
        
        if(nf || dash.needs_redraw_left) { dash.updateFlights(true); state->new_flights = false; }
        else if (scroll) { dash.updateFlights(false); }
        
        if(nw || dash.needs_redraw_left) { dash.updateWeather(true); state->new_weather = false; }
    } else {
        if(ns || dash.needs_redraw_left) { dash.updateStats(true); state->new_stats = false; }
        else if (scroll) { dash.updateStats(false); }
    }
    
    if(scroll) dash.last_left_tick = tick;
    dash.needs_redraw_left = false;
    
    dash.drawClock();
    
    delay(20);
}