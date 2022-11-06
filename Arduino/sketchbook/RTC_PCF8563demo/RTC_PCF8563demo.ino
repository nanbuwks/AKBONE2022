#define LGFX_USE_V1
#include <SPIFFS.h>
#include <LovyanGFX.hpp>
#include <lgfx/v1/panel/Panel_HUB75.hpp>
#include <lgfx/v1/platforms/esp32/Bus_HUB75.hpp>


#include <Wire.h>
#include <RTClib.h>

//#define PANEL_64x32
#define PANEL_128x64



RTC_PCF8563 RTC;

struct LGFX_HUB75 : public lgfx::LGFX_Device
{
  struct Panel_Custom_HUB75 : public lgfx::Panel_HUB75
  {
// X 座標が8ドット単位で逆順になる場合の対策が必要な場合はこのoverrideを有効にする
/*
    void _draw_pixel_inner(uint_fast16_t x, uint_fast16_t y, uint32_t rawcolor) override
    {
      Panel_HUB75::_draw_pixel_inner((x & 8) ? x : (x ^ 7), y, rawcolor);
   // Panel_HUB75::_draw_pixel_inner((x & 8) ? (x ^ 7) : x, y, rawcolor);
    }
//*/
  };

Panel_Custom_HUB75 _panel_instance;
  lgfx::Bus_HUB75 _bus_instance;

  LGFX_HUB75(void)
  {
    {
      auto cfg = _bus_instance.config();
      cfg.port = 1;
   /*   
HUB75 Standard set RGB configuration
     +-+-+
   R1|o|o|G1
   G1|o|o|GND
   R2|o|o|G2
   G2 o|o|GND
   A  o|o|B
   C |o|o|D
  CLK|o|o|LAT
   OE|o|o|GND
     +-+-+

HUB75E + BRG panel set BRG configuration
     +-+-+
   B1|o|o|R1
   G1|o|o|GND
   B2|o|o|R2
   G2 o|o|E
   A  o|o|B
   C |o|o|D
  CLK|o|o|LAT
   OE|o|o|GND
     +-+-+

some panel has no D,E line, please set RGB configuration
     +-+-+
   R1|o|o|G1
   G1|o|o|GND
   R2|o|o|G2
   G2 o|o|GND
   A  o|o|B
   C |o|o|D
  CLK|o|o|LAT
   OE|o|o|GND
     +-+-+
     
*/
   
   // RGB configuration
      cfg.pin_r1 = GPIO_NUM_33; // R1
      cfg.pin_r2 = GPIO_NUM_18; // R2
      cfg.pin_g1 = GPIO_NUM_32; // G1
      cfg.pin_g2 = GPIO_NUM_19; // G2
      cfg.pin_b1 = GPIO_NUM_25; // B1
      cfg.pin_b2 = GPIO_NUM_5 ; // B2
  /*/ // BRG configuration
      cfg.pin_r1 = GPIO_NUM_32; // R1
      cfg.pin_r2 = GPIO_NUM_19; // R2
      cfg.pin_g1 = GPIO_NUM_25; // G1
      cfg.pin_g2 = GPIO_NUM_5 ; // G2
      cfg.pin_b1 = GPIO_NUM_33; // B1
      cfg.pin_b2 = GPIO_NUM_18; // B2
  //*/
      cfg.pin_lat = GPIO_NUM_17; // LAT
      cfg.pin_oe  = GPIO_NUM_16; // OE
      cfg.pin_clk = GPIO_NUM_4 ; // CLK

      cfg.pin_addr_a = GPIO_NUM_12;
      cfg.pin_addr_b = GPIO_NUM_14;
      cfg.pin_addr_c = GPIO_NUM_27;
      cfg.pin_addr_d = GPIO_NUM_26;
      cfg.pin_addr_e = GPIO_NUM_2;

      // 1秒間の更新回数を設定
      cfg.refresh_rate = 200;

      // パネルの行選択の仕様に応じて指定する
      cfg.address_mode = cfg.address_shiftreg;
      //cfg.address_mode = cfg.address_binary;

      // LEDドライバの初期化コマンドを指定する
      //cfg.initialize_mode = cfg.initialize_none;
      cfg.initialize_mode = cfg.initialize_fm6124;

      // DMA用のタスクの優先度 (FreeRTOSのタスク機能を使用)
      cfg.task_priority = 1;

      // DMA用のタスクに使用するCPUコア設定 (FreeRTOSのタスク機能を使用)
      cfg.task_pinned_core = PRO_CPU_NUM;
      // cfg.task_pinned_core = APP_CPU_NUM;

      _bus_instance.config(cfg);
      _panel_instance.setBus(&_bus_instance);
    }

    {
      auto cfg = _panel_instance.config();

      // ここでパネルサイズを指定する
      // 複数枚並べる場合は全体の縦横サイズを指定
      cfg.memory_width  = cfg.panel_width  = 128;
      cfg.memory_height = cfg.panel_height = 64;

      _panel_instance.config(cfg);
      setPanel(&_panel_instance);
    }
    {
      auto cfg = _panel_instance.config_detail();

      // 構成パネルの総枚数を指定
      cfg.panel_count = 1;

      // 横方向のパネル枚数を指定
      cfg.x_panel_count = 1;

      // 縦方向のパネル枚数を指定
      cfg.y_panel_count = 1;

      _panel_instance.config_detail(cfg);
    }
  }
};

LGFX_HUB75 gfx;

//===========================================================
//	clearLED() ： clear LED panel
//===========================================================
void clearLED()
{
  gfx.setFont(&fonts::Font0);
  gfx.setCursor(0, 0);
  gfx.fillScreen(TFT_BLACK);
  gfx.setTextWrap(false);
  gfx.setTextScroll(true);
  gfx.setTextColor(TFT_WHITE, TFT_BLACK);
  gfx.setTextSize(1.0, 1.0);

}

/* font size(y)
TomThumb 5dot
Font0 8dot
Font2 10dot
Font4 16dot
Font6 37dot
Font7 48dot
Font8 about 70dot
FreeFont0 8dot
FreeSansBold9pt7b 13dot
FreeSerifBoldItalic18pt7b 26dot
*/
void drawtime()
{
  int pos_y;
  int diff;
  int center_x;
  char buffer[64];
  clearLED();
#if defined(PANEL_64x32)
  const lgfx::IFont* font1 = &fonts::Font2;
  const lgfx::IFont* font2 = &fonts::Font0;
  const lgfx::IFont* font3 = &fonts::TomThumb;
  float textsizex=1.0;
  float textsizey=1.0;
  gfx.setTextDatum( textdatum_t::baseline_center );
#else
  const lgfx::IFont* font1 = &fonts::FreeSerifBoldItalic18pt7b;
  const lgfx::IFont* font2 = &fonts::Font4;
  const lgfx::IFont* font3 = &fonts::Font2;
  float textsizex=1.0;
  float textsizey=1.0;
  gfx.setTextDatum( textdatum_t::baseline_center );
#endif
  gfx.setFont(font1);
  gfx.setTextSize(textsizex, textsizey);
  gfx.setTextWrap(false);
  gfx.setTextColor(0xAAAAFFU, TFT_BLACK);

  
  if (!RTC.isrunning()) {
    gfx.print("RTC not find");
    Serial.println("RTC not find");
  }else{
    DateTime now = RTC.now();
    gfx.setFont(font1);
    sprintf(buffer,"%02d:%02d:%02d",now.hour(),now.minute(),now.second());
    Serial.print(buffer);
    Serial.print(" ");
    gfx.drawString(buffer,  gfx.width() / 2,  gfx.height() / 2);
    gfx.setFont(font2);
    gfx.setTextColor(TFT_BLUE);
    sprintf(buffer,"%02d/%02d/%02d",now.year(),now.month(),now.day());
    Serial.print(buffer);
    Serial.println("");
    gfx.drawString(buffer,  gfx.width() / 2,  gfx.height()*0.9 );
  }
}

void setup() {
  Serial.begin(115200);
  gfx.init();
  gfx.setBrightness(200);
  gfx.setColorDepth(16);
  Wire.begin(22,21);
  RTC.begin();
 // RTC.adjust(DateTime(2022, 11, 6, 20, 36, 20));
}

void loop() {

  drawtime();
  delay(1000);
}