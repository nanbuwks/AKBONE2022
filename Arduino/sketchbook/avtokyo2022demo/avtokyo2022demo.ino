#define LGFX_USE_V1
#include <SPIFFS.h>
#include <LovyanGFX.hpp>
#include <lgfx/v1/panel/Panel_HUB75.hpp>
#include <lgfx/v1/platforms/esp32/Bus_HUB75.hpp>

#define PANEL_64x32
//#define   PANEL_128x64


struct LGFX_HUB75 : public lgfx::LGFX_Device
{
  struct Panel_Custom_HUB75 : public lgfx::Panel_HUB75
  {
// X 座標が8ドット単位で逆順になる場合の対策が必要な場合はこのoverrideを有効にする

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
      //cfg.address_mode = cfg.address_shiftreg;
      cfg.address_mode = cfg.address_binary;

      // LEDドライバの初期化コマンドを指定する
      cfg.initialize_mode = cfg.initialize_none;
      //cfg.initialize_mode = cfg.initialize_fm6124;

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
      cfg.memory_width  = cfg.panel_width  = 64;
      cfg.memory_height = cfg.panel_height = 32;

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
//  clearLED() ： clear LED panel
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
//===========================================================
//  drawFile() ： Draw BMP File to LCD
//===========================================================
bool drawFile(const char *filename)
{
  bool ret = false;
  int err_count = 0;
  Serial.println("Start Drawing BMP File");
  delay(1000);
  do {
    ret = gfx.drawBmpFile(SPIFFS, filename,0 ,0);
    if (err_count++ > 10) {
          Serial.println("File not EXIST!");
      break;
    }
  } while (!ret);
  return ret;
}
//===========================================================
//  lcdTest() ： LCD Draw Test
//===========================================================
void lcdTest(void)
{
  delay(2000);
  gfx.setCursor(0, 0);
  gfx.setFont(&fonts::Font0);
  gfx.setTextSize(1.0, 1.0);
  gfx.setTextColor(TFT_WHITE, TFT_BLACK);
  gfx.println("AKBONE2022 1234567890");
  gfx.setTextColor(TFT_RED, TFT_BLACK);
  gfx.println("AKBONE2022 1234567890");
  gfx.setTextColor(TFT_GREEN, TFT_BLACK);
  gfx.println("AKBONE2022 1234567890");
  gfx.setTextColor(TFT_BLUE, TFT_BLACK);
  gfx.println("AKBONE2022 1234567890");
}
//===========================================================
//  drawLowLevel() ： LCD Draw Test
//===========================================================
void drawLowLevel()
{
  clearLED();
  int pos_x;
  int pos_y = 0;
  int diff;
#if defined(PANEL_64x32)
  pos_x = 33;
  diff = 8;
  gfx.setFont(&fonts::Font0);
#else
  pos_x = 65;
  diff = 16;
  gfx.setFont(&fonts::Font2);
  gfx.setTextSize(1.5, 1.0);
#endif  
  gfx.setTextColor(TFT_WHITE, TFT_BLACK);
  gfx.setCursor(pos_x, pos_y);
  gfx.print("LOW");
  gfx.setTextColor(TFT_RED, TFT_BLACK);
  gfx.setCursor(pos_x, pos_y += diff);
  gfx.print("LEVEL");
  gfx.setTextColor(TFT_GREEN, TFT_BLACK);
  gfx.setCursor(pos_x,pos_y += diff);
  gfx.print("STUDY");
  gfx.setTextColor(TFT_BLUE, TFT_BLACK);
  gfx.setCursor(pos_x,pos_y += diff);
  gfx.print("GROUP");
  gfx.drawBmpFile(SPIFFS, "/low.bmp",0 ,0);
}
//===========================================================
//  drawLGFX() ： LCD Draw Test
//===========================================================
void drawLGFX()
{
  int pos_y;
  int diff;
  clearLED();
#if defined(PANEL_64x32)
  pos_y = 0;
  diff = 16;
  gfx.setFont(&fonts::Font2);
  gfx.setTextSize(0.95, 1.0);
  pos_y = 0;
#else
  pos_y = 4;
  diff = 32;
  gfx.setFont(&fonts::FreeSansOblique12pt7b);
  //gfx.setTextSize(1.5, 1.0);
#endif  
  gfx.setTextWrap(false);
  gfx.setTextColor(TFT_BLUE, TFT_BLACK);
  gfx.setCursor(0, pos_y);
  gfx.println("Powered by");
  gfx.setCursor(0, pos_y += diff);
  gfx.setTextColor(TFT_RED);
  gfx.println("LovyanGFX");
}
void setup() {
  Serial.begin(115200);
  if(!SPIFFS.begin()){
      Serial.println("Card Mount Failed");
      return;
  }    
  gfx.init();
  gfx.setBrightness(128);
  gfx.setColorDepth(16);
//#ifdef PANEL_P4
//  gfx.createSprite(64, 32); // パネルの面積
// #else
//  gfx.createSprite(128, 64); // パネルの面積
//#endif
}
void loop() {
  //lcdTest();
#if defined(PANEL_64x32)
  gfx.drawPngFile(SPIFFS, "/avtokyo_64x32.png",0 ,0);
#else
  gfx.drawPngFile(SPIFFS, "/avtokyo.png",0 ,0);
#endif  
  delay(5000);
  drawLowLevel();
  delay(5000);
#if defined(PANEL_64x32)
  gfx.drawPngFile(SPIFFS, "/akbone_64x32_B.png",0 ,0);
#else
  gfx.drawPngFile(SPIFFS, "/akbone2022.png",0 ,0);
#endif
  delay(5000);
  drawLGFX();
  delay(5000);
}
