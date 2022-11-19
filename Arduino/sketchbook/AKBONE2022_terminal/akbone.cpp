// AKBONE2022 common library
// 2022.11.06 V0.0 K.Ohe New Create
// 2022.11.18 V0.1 select functions
#include <SD.h>
#include <SPIFFS.h>
enum { sd_sck = 13, sd_miso = 35, sd_mosi = 23, sd_ss = 15 };
#include "akbone.hpp"
#if defined(USE_MPU6500) || defined(USE_RTC) || defined(BME280)
    #include <Wire.h>
    #define I2C_SDA 22
    #define I2C_SCL 21
#endif
#if defined(USE_ENCODER)
    #include <Ticker.h>
    #include <RotaryEncoder.h>
    #define SW_A    34
    #define SW_B    36
    #define BUTTON_SW1  39
    #define SW1_COUNT 2
    Ticker ticker1;
    RotaryEncoder encoder(SW_A, SW_B, RotaryEncoder::LatchMode::FOUR0);
    volatile int16_t encoder_pos  = 0;
    volatile int16_t sw1_count    = 0;
    volatile bool sw1_pressed  = 0;
#endif
#if defined(USE_BME280)
    #include <Adafruit_Sensor.h>
    #include <Adafruit_BME280.h>
    #define BME280_ADDR 0x76
    Adafruit_BME280 bme;
#endif
#if defined(USE_RTC)
    #include <RTClib.h>
    RTC_PCF8563 RTC;
#endif
#if defined(USE_MPU6500)
    #include <MPU6500_WE.h>
    #define MPU6500_ADDR 0x68
    MPU6500_WE myMPU6500 = MPU6500_WE(MPU6500_ADDR);
#endif

LGFX_HUB75 gfx;

#if defined(USE_ENCODER)
//===========================================================
//	checkPosition() : ISR for encoder
//===========================================================
IRAM_ATTR void check_encoder()
{
    encoder.tick(); // just call tick() to check the state.
    encoder_pos = - (uint16_t)encoder.getPosition();
}

//===========================================================
//	check_button() : Timer for check button
//===========================================================
void check_button()
{
    int value = digitalRead(BUTTON_SW1);

    if (value == 0) {
        sw1_count++;
        if (sw1_count > SW1_COUNT) {
            sw1_pressed = true;
        } 
    } else {
        sw1_count = 0;
    }
}
#endif

//===========================================================
//	akbone_setup()
//===========================================================
void akbone_setup()
{
    //-------------------------------------------------------
    //	Serial Setup (for terminal)
    //-------------------------------------------------------
    Serial.begin(115200);
#if defined(USE_TERMINAL)    
    Serial2.begin(115200, SERIAL_8N1, 35, 23);
#endif
    //-------------------------------------------------------
    //	setup for encoder
    //-------------------------------------------------------
#if defined(USE_ENCODER)
    analogSetAttenuation(ADC_6db);  //ATT -6dB
    pinMode(BUTTON_SW1, INPUT);
    ticker1.attach_ms(20, check_button);
    attachInterrupt(digitalPinToInterrupt(SW_A), check_encoder, CHANGE);
    attachInterrupt(digitalPinToInterrupt(SW_B), check_encoder, CHANGE);
#endif
    //-------------------------------------------------------
    //	setup for TFCARD
    //-------------------------------------------------------
#if defined(USE_TFCARD)
    SPI.end();
    SPI.begin(sd_sck, sd_miso, sd_mosi, sd_ss); 
    if (!SD.begin(sd_ss, SPI)){
    //if(!SPIFFS.begin()){
        Serial.println("Card Mount Failed");
        return;
    }
#endif 
    //-------------------------------------------------------
    //	setup for panel configration
    //-------------------------------------------------------
#if defined (PANEL_DEPTH_8)
    //色深度は 8bit(RGB332)と 16bit(RGB565) が設定可能
    gfx.setColorDepth(8);
#else 
    gfx.setColorDepth(16);
#endif
    //-------------------------------------------------------
    //	init LovyanGFX
    //-------------------------------------------------------
    gfx.init();
    gfx.setBrightness(160);
    clearLED();
    //-------------------------------------------------------
    //	 setup for I2C functions
    //-------------------------------------------------------
#if defined(USE_MPU6500) || defined(USE_RTC) || defined(BME280)
    Wire.begin(I2C_SDA, I2C_SCL);
#endif
    //-------------------------------------------------------
    //	 setup for I2C BME280
    //-------------------------------------------------------
#if defined(USE_BME280)
    bool status = bme.begin(BME280_ADDR);
    while (!status) {
        Serial.println("BME280 fail!");
    }
#endif
    //-------------------------------------------------------
    //	 setup for RTC
    //-------------------------------------------------------
#if defined(USE_RTC)
    if (! RTC.begin()) {
        Serial.println("Couldn't find RTC");
        Serial.flush();
        while (1) delay(10);
    }
    if (RTC.lostPower()) {
        //RTC.adjust(DateTime(2022, 11, 13, 0, 28, 40));
        RTC.adjust(DateTime(F(__DATE__), F(__TIME__)));                
    }
    RTC.start();
#endif
    //-------------------------------------------------------
    //	 setup for MPU6500
    //-------------------------------------------------------
#if defined(USE_MPU6500)
    if(!myMPU6500.init()){
        Serial.println("MPU6500 does not respond");
    }
    else{
        Serial.println("MPU6500 is connected");
    }    
#endif
}

//===========================================================
//	akbone main loop
//===========================================================
void akbone_loop()
{
#if defined(USE_ENCODER)    
    encoder.tick(); // just call tick() to check the state.
#endif
}

//===========================================================
//	textControlls
//===========================================================
void clearLED()
{
    gfx.setFont(&fonts::Font0);
    gfx.setCursor(0, 0);
    gfx.fillScreen(TFT_BLACK);
    gfx.setTextWrap(false);
    //gfx.setTextWrap(true);
    gfx.setTextScroll(true);
    gfx.setScrollRect(0, 0, gfx.width(), gfx.height());
    gfx.setTextColor(TFT_BLUE, TFT_BLACK);
    gfx.setTextDatum( textdatum_t::top_left );
}

//===========================================================
//	textControlls
//===========================================================
#if defined(USE_TERMINAL)
void akbone_terminal()
{
    clearLED();
    while(1) {
    if (Serial2.available()) {
        int inByte = Serial2.read();
        Serial.write(inByte);
        gfx.print((char)inByte);
    }
    if (Serial.available()) {
        int inByte = Serial.read();
        Serial2.write(inByte);
    }
    delay(1);
    }
}
#endif
//===========================================================
//	panel_test : draw lines & circle
//===========================================================
void panel_test()
{
    clearLED();
    int w = gfx.width();
    int h = gfx.height() >> 3;
    for (int x = 0; x < w; ++x) {
        int c1 = (x << 8) / w;
        int c2 = 255 - c1;
        gfx.drawFastVLine(x, 0 * h, h, gfx.color565(c1,  0,  0));
        gfx.drawFastVLine(x, 1 * h, h, gfx.color565(c2,  0,  0));
        gfx.drawFastVLine(x, 2 * h, h, gfx.color565( 0, c1,  0));
        gfx.drawFastVLine(x, 3 * h, h, gfx.color565( 0, c2,  0));
        gfx.drawFastVLine(x, 4 * h, h, gfx.color565( 0,  0, c1));
        gfx.drawFastVLine(x, 5 * h, h, gfx.color565( 0,  0, c2));
        gfx.drawFastVLine(x, 6 * h, h, gfx.color565(c1, c1, c1));
        gfx.drawFastVLine(x, 7 * h, h, gfx.color565(c2, c2, c2));
    }
    delay(2000);
    for (int i = 1; i < 1000; i++) {
        gfx.fillCircle(rand()%gfx.width(), rand()%gfx.height(), (rand()&7)+3, rand());
        delay(10);
    }
}

#if defined(USE_BME280)
//===========================================================
//	read_bme280()
//===========================================================
void read_bme280(float& temp, float& pressure, float& humi)
{
    temp     = bme.readTemperature();
    pressure = bme.readPressure() / 100.0F;
    humi     = bme.readHumidity();
    Serial.printf("temp = %f4.1, humi = %f4.1 pressure = %f6.1\n", temp, pressure, humi);
}

//===========================================================
//	disp_bme280()
//===========================================================
void disp_bme280()
{
    float temp;
    float pressure;
    float humi;
    uint16_t pos_y;
    float scale;

    read_bme280(temp, pressure, humi);

    clearLED();

#if defined(PANEL_64x32)
    const lgfx::IFont* font1 = &fonts::Font0;
    const lgfx::IFont* font2 = &fonts::lgfxJapanGothicP_8;
    const char h_temp[] = "TE:";
    const char f_temp[] = "ﾟC";

    const char h_humi[] = "HU:";
    const char f_humi[] = "%%";

    const char h_pres[] = "PR:";
    const char f_pres[] = "hP";
    pos_y = 10;
    scale = 1.0;
#else
    const lgfx::IFont* font1 = &fonts::lgfxJapanGothicP_20;
    const lgfx::IFont* font2 = &fonts::lgfxJapanGothicP_20;
    const char h_temp[] = "気温:";
    const char f_temp[] = "ﾟC";

    const char h_humi[] = "湿度:";
    const char f_humi[] = "%%";

    const char h_pres[] = "気圧:";
    const char f_pres[] = "hPa";
    pos_y = 20;
    scale = 0.9;
#endif

    gfx.setFont(font1);
    gfx.setTextSize(1.0, 1.0);
    gfx.setTextWrap(false);

    gfx.setCursor(0, 0);
    gfx.setTextColor(TFT_WHITE, TFT_BLACK);
    gfx.printf(h_temp);
    gfx.setTextColor(TFT_RED);
    gfx.printf("%2.1f",temp);
    gfx.setTextColor(TFT_WHITE);
    gfx.setFont(font2);
    gfx.printf(f_temp);

    gfx.setFont(font1);
    gfx.setCursor(0, pos_y);
    gfx.setTextColor(TFT_WHITE);
    gfx.printf(h_humi);
    gfx.setTextColor(TFT_BLUE);
    gfx.printf("%2.1f",humi);
    gfx.setTextColor(TFT_WHITE);
    gfx.printf(f_humi);

    gfx.setCursor(0, pos_y * 2);
    gfx.setTextColor(TFT_WHITE);
    gfx.printf(h_pres);
    gfx.setTextColor(TFT_GREEN);
    gfx.printf("%4.0f", pressure);
    gfx.setTextSize(scale, 1.0);
    gfx.setTextColor(TFT_WHITE);
    gfx.printf(f_pres);
}
#endif
#if defined(USE_RTC)
//===========================================================
//	read_rtc()
//===========================================================
bool read_rtc(uint16_t& hour, uint16_t& minute, uint16_t& second, uint16_t& year , uint16_t& month, uint16_t& day)
{
    bool ret = false;
    if (RTC.isrunning()) {
        DateTime now = RTC.now();
        hour   = now.hour();
        minute = now.minute();
        second = now.second();
        year   = now.year();
        month  = now.month();
        day    = now.day();

        ret = true;
    }
    return ret;
}

//===========================================================
//	disp_rtc()
//===========================================================
void disp_rtc()
{
    char    buf[64];
    uint16_t hour = 0, minute = 0, second = 0;
    uint16_t year = 0, month  = 0, day    = 0;

    clearLED();
#if defined(PANEL_64x32)
    const lgfx::IFont* font1 = &fonts::Font2;
    const lgfx::IFont* font2 = &fonts::Font0;
    float textsizex=1.0;
    float textsizey=1.0;
#else
    const lgfx::IFont* font1 = &fonts::FreeSerifBoldItalic18pt7b;
    const lgfx::IFont* font2 = &fonts::Font4;
    float textsizex=1.0;
    float textsizey=1.0;
#endif
    gfx.setTextDatum( textdatum_t::baseline_left );
    gfx.setFont(font1);
    gfx.setTextSize(textsizex, textsizey);
    gfx.setTextWrap(false);
    gfx.setTextColor(0xAAAAFFU, TFT_BLACK);

    if (!read_rtc(hour, minute, second, year, month, day)) {
        Serial.println("RTC not find");
    } else {
        gfx.setFont(font1);
        sprintf(buf, "%02d:%02d:%02d", hour, minute, second);
        Serial.print(buf);
        Serial.print(" ");
        gfx.drawString(buf,  0,  gfx.height() / 2);

        gfx.setFont(font2);
        gfx.setTextColor(TFT_BLUE);
        sprintf(buf, "%02d/%02d/%02d", year, month, day);
        Serial.print(buf);
        Serial.println("");
        gfx.drawString(buf,  0,  gfx.height() * 0.9 );
    }
}
#endif
#if defined(USE_MPU6500)
//===========================================================
//	init_mpu6500()
//===========================================================
void init_mpu6500(bool calib)
{
    clearLED();

#if defined(PANEL_64x32)
    const lgfx::IFont* font1 = &fonts::efontJA_12;
    const lgfx::IFont* font2 = &fonts::Font0;
    const lgfx::IFont* font3 = &fonts::TomThumb;
    float textsizex=1.0;
    float textsizey=0.95;
#else
    const lgfx::IFont* font1 = &fonts::efontJA_16;
    const lgfx::IFont* font2 = &fonts::Font4;
    const lgfx::IFont* font3 = &fonts::Font2;
    float textsizex=1.0;
    float textsizey=1.0;
#endif    
    Serial.println("Position you MPU6500 flat and don't move it - calibrating...");
    gfx.setFont(font1);
    gfx.setTextSize(textsizex, textsizey);
    gfx.setTextWrap(true);
    gfx.setTextColor(0xAAAAFFU, TFT_BLACK);
    gfx.setCursor(0,0);
    if (calib == true) {
        gfx.println("画面を上にして,静置してください");
        //delay(3000);
        myMPU6500.autoOffsets();
        delay(1000);
        Serial.println("Done!");
    }
    myMPU6500.enableGyrDLPF();
    myMPU6500.setGyrDLPF(MPU6500_DLPF_6);
    myMPU6500.setSampleRateDivider(5);
    myMPU6500.setGyrRange(MPU6500_GYRO_RANGE_250);
    myMPU6500.enableAccDLPF(true); 
    myMPU6500.setAccDLPF(MPU6500_DLPF_6);
}

//===========================================================
//	read_mpu6500()
//===========================================================
void read_mpu6500(float& acc_x, float& acc_y, float& acc_z, float& gyr_x, float& gyr_y, float& gyr_z, float& temp, float& tanG)
{
    xyzFloat acc     = myMPU6500.getGValues();
    acc_x = acc.x;
    acc_y = acc.y;
    acc_z = acc.z;
    xyzFloat gyr     = myMPU6500.getGyrValues();
    gyr_x = gyr.x;
    gyr_y = gyr.y;
    gyr_z = gyr.z;
    temp  = myMPU6500.getTemperature();
    tanG  = myMPU6500.getResultantG(acc);
}

//===========================================================
//	disp_mpu6500()
//===========================================================
void disp_mpu6500()
{
    uint16_t    pos_y;
#if defined(PANEL_64x32)
    const lgfx::IFont* font1 = &fonts::efontJA_12;
    float textsizex=1.0;
    float textsizey=0.95;
    pos_y = 10;
#else
    const lgfx::IFont* font1 = &fonts::efontJA_16;
    float textsizex=1.0;
    float textsizey=1.0;
    pos_y = 20;
#endif    
    char  buf[32];
    float acc_x, acc_y, acc_z;
    float gyr_x, gyr_y, gyr_z;
    float temp;
    float tanG;

    clearLED();
    read_mpu6500(acc_x, acc_y, acc_z, gyr_x, gyr_y, gyr_z, temp, tanG);
/*
    Serial.println("Acceleration in g (x,y,z):");
    Serial.printf("X:%1.3f\tY:%1.3f\tZ:%1.3f\n", acc_x, acc_y, acc_z);
    Serial.print("Resultant g: ");
    Serial.println(tanG);

    Serial.println("Gyroscope data in degrees/s: ");
    Serial.printf("X:%1.3f\tY:%1.3f\tZ:%1.3f\n", gyr_x, gyr_y, gyr_z);

    Serial.print("Temperature in °C: ");
    Serial.println(temp);
*/
    gfx.setFont(font1);
    gfx.setTextSize(textsizex, textsizey);
    gfx.setTextWrap(false);

    gfx.setTextColor(TFT_WHITE, TFT_BLACK);

    gfx.setCursor(0, 0);
    gfx.setTextColor(TFT_BLUE);
    gfx.printf("X:%5.2f", acc_x);
    gfx.setCursor(0, pos_y);
    gfx.printf("Y:%5.2f", acc_y);
    gfx.setCursor(0, pos_y * 2);
    gfx.printf("Z:%5.2f", acc_z);

    gfx.setCursor(64, 0);
    gfx.setTextColor(TFT_RED);
    gfx.printf("GX:%5.2f", gyr_x);
    gfx.setCursor(64, pos_y);
    gfx.printf("GY:%5.2f", gyr_y);
    gfx.setCursor(64, pos_y * 2);
    gfx.printf("GZ:%5.2f", gyr_z);
}
#endif
#if defined (USE_TFCARD)  
//===========================================================
//	disp_imgfile()
//===========================================================
void disp_imgfile()
{
    gfx.drawPngFile(SD, "/avtokyo.png",0 ,0);
    delay(5000);
    gfx.drawPngFile(SD, "/akbone2022.png",0 ,0);
    delay(5000);
    clearLED();
    gfx.drawBmpFile(SD, "/low.bmp",0 ,0);
    gfx.setFont(&fonts::FreeSans9pt7b);

    gfx.setTextColor(TFT_WHITE, TFT_BLACK);
    gfx.setCursor(64,3);
    gfx.print("LOW");

    gfx.setTextColor(TFT_RED, TFT_BLACK);
    gfx.setCursor(64,18);
    gfx.print("LEVEL");

    gfx.setTextColor(TFT_GREEN, TFT_BLACK);
    gfx.setCursor(64,33);
    gfx.print("STUDY");

    gfx.setTextColor(TFT_BLUE, TFT_BLACK);
    gfx.setTextSize(0.99, 1.0);
    gfx.setCursor(64,48);
    gfx.print("GROUP");
}
#endif
#if defined(USE_ENCODER)
//===========================================================
//	read_encoder()
//===========================================================
int16_t read_encoder(void)
{
    return encoder_pos;
}

//===========================================================
//	read_button()
//===========================================================
bool read_button()
{
    bool ret;

    ret = sw1_pressed;
    sw1_pressed = false;

    return ret;
}
#endif

//===========================================================
//	menu function
//===========================================================
#if defined(USE_MENU)
#define MENU_NUM 5 + 1
const char *menu_str[MENU_NUM] = {
    "",
    "1.PANEL TEST         ",
    "2.DISP IMAGE FILE    ",
    "3.DISP WEATHER       ",
    "4.DISP DATE/TIME     ",
    "5.DISP 6AXIS IMU     "
};

int16_t encoder_old = -32768;

static void _menu(uint16_t mode);
//===========================================================
//	disp_menu()
//===========================================================
uint16_t disp_menu(uint16_t mode)
{
    int16_t  pos = 0, sw1 = 0;
    if (mode == 0) {
        _menu(mode);
    }
    while (!read_button()) {
        pos = read_encoder();
        if (pos < 1) {
            pos = 1;
        } else if (pos >= MENU_NUM) {
            pos = MENU_NUM - 1;
        }
        _menu(pos);
        delay(10);
    }
    return pos;
}

//===========================================================
//	_menu()
//===========================================================
static void _menu(uint16_t mode)
{
    int16_t pos_y;
    static uint16_t old_mode = 0;

#if defined(PANEL_64x32)
    const lgfx::IFont* font1 = &fonts::Font0;
    float textsizex=1.0;
    float textsizey=1.0;
    pos_y = 10;
#else
    const lgfx::IFont* font1 = &fonts::Font0;
    float textsizex=1.0;
    float textsizey=1.0;
    pos_y = 10;
#endif
    if (mode == 0 ) {    
        clearLED();
        gfx.setFont(font1);
        gfx.setTextSize(textsizex, textsizey);
        gfx.setTextWrap(false);
        gfx.setTextColor(TFT_WHITE, TFT_BLUE);
        gfx.setCursor(0, 0);
        gfx.print("** AKBONE2020 MENU **");

        gfx.setTextColor(TFT_WHITE, TFT_BLACK);
        gfx.setCursor(0, 0);

        for (int i = 1; i < MENU_NUM; i++) {
            gfx.setCursor(0, pos_y * (i));
            if (i == mode) {
                gfx.setTextColor(TFT_YELLOW, TFT_WHITE);
            } else {
                gfx.setTextColor(TFT_WHITE, TFT_BLACK);
            }
            gfx.print(menu_str[i]);
        }
    }
    if (old_mode != mode) {
        gfx.setFont(font1);
        gfx.setTextSize(textsizex, textsizey);
        gfx.setTextWrap(false);
        gfx.setCursor(0, pos_y * mode);
        gfx.setTextColor(TFT_RED, TFT_BLACK);
        gfx.print(menu_str[mode]);
        gfx.setCursor(0, pos_y * old_mode);
        gfx.setTextColor(TFT_WHITE, TFT_BLACK);
        if (old_mode) {
            gfx.print(menu_str[old_mode]);
        }
        old_mode = mode;
    }
}
#endif
