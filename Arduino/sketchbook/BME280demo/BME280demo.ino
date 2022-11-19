#define LGFX_USE_V1
#include <SPIFFS.h>
#include <LovyanGFX.hpp>


#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

//#define PANEL_64x32
#define PANEL_128x64



Adafruit_BME280 bme;

float temp;
float pressure;
float humi;

struct LGFX_HUB75 : public lgfx::LGFX_Device
{
  static void convertCoordinate(uint_fast16_t &x, uint_fast16_t &y)
  {/*
    if (x & 8) { x = x ^ 7; }
    */
    if ( x < 256)
    {
      x = x ^ 127;
      y = y ^ 63;
    }
  }

  lgfx::Bus_HUB75 _bus_instance;
// 1枚だけで使用する場合はこちら
  lgfx::Panel_HUB75 _panel_instance;

// 2枚以上接続する場合はこちら
// lgfx::Panel_HUB75_Multi _panel_instance;

 
  LGFX_HUB75(void)
  {
    // X 座標が8ドット単位で逆順になるパネルの場合、座標変換関数を使用するよう指定する
   // _panel_instance.convertCoordinate = convertCoordinate;
    
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
      cfg.refresh_rate = 300;

      // パネルの行選択の仕様に応じて指定する
       cfg.address_mode = cfg.address_shiftreg;
      //cfg.address_mode = cfg.address_binary;

      // LEDドライバの初期化コマンドを指定する
      // cfg.initialize_mode = cfg.initialize_none;
      cfg.initialize_mode = cfg.initialize_fm6124;

      // DMA用のタスクの優先度 (FreeRTOSのタスク機能を使用)
      cfg.task_priority = 2;

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
      //  ----- 1枚構成  ここから
      cfg.memory_width  = cfg.panel_width  = 128;
      cfg.memory_height = cfg.panel_height = 64;

      _panel_instance.config(cfg);
      setPanel(&_panel_instance);
      //*/ 1枚構成 ここまで
      
      /* 複数枚構成 ここから 
      
      cfg.memory_width  = cfg.panel_width  = 256;
      cfg.memory_height = cfg.panel_height = 128;

      _panel_instance.config(cfg);
      setPanel(&_panel_instance);
    }
    {
      auto cfg = _panel_instance.config_detail();

      // 構成パネルの総枚数を指定
      cfg.panel_count = 4;

      // パネル1枚の幅を指定
      cfg.single_width = 128;

      // パネル1枚の高さを指定
      cfg.single_height = 64;

      _panel_instance.config_detail(cfg);

      // 各パネルの配置座標を設定する
      _panel_instance.setPanelPosition( 1,   0,   0,2);
      _panel_instance.setPanelPosition( 0,  128,  0,2);
      _panel_instance.setPanelPosition( 2,  0,   64);
      _panel_instance.setPanelPosition( 3,  128,  64);
      */
      // 複数枚構成 ここまで

    }

  }
};

LGFX_HUB75 gfx;

#define CLK_PULSE digitalWrite(cfg.pin_clk, HIGH); digitalWrite(cfg.pin_clk, LOW);

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



//===========================================================
//	drawLGFX() ： LCD Draw Test
//===========================================================
void drawenvironment()
{
  int pos_y;
  int diff;
  
  temp=bme.readTemperature();
  pressure=bme.readPressure() / 100.0F;
  humi=bme.readHumidity();


  
  clearLED();
#if defined(PANEL_64x32)
  const lgfx::IFont* font1 = &fonts::lgfxJapanGothicP_16;
  const lgfx::IFont* font2 = &fonts::lgfxJapanGothicP_12;
  const lgfx::IFont* font3 = &fonts::lgfxJapanGothicP_8;
  pos_y = -2;
  diff = 18;
#else
  const lgfx::IFont* font1 = &fonts::lgfxJapanGothicP_32;
  const lgfx::IFont* font2 = &fonts::lgfxJapanGothicP_24;
  const lgfx::IFont* font3 = &fonts::lgfxJapanGothicP_16;
  pos_y = -5;
  diff = 40;
#endif
  gfx.setFont(font1);
  gfx.setTextSize(1.0, 1.0);
  gfx.setTextWrap(false);
  gfx.setTextColor(TFT_RED, TFT_BLACK);
  gfx.setCursor(0, pos_y);
  gfx.printf("%3.0f",temp);
  gfx.setFont(font3);
  gfx.setCursor(gfx.getCursorX(), pos_y+diff*0.5);
  gfx.setTextSize(0.5, 0.5);
  gfx.printf("o");
  gfx.setTextSize(1.0, 1.0);
  gfx.printf("C");
  gfx.setFont(font1);
  gfx.setCursor(diff * 1.5, pos_y);
  gfx.setTextColor(TFT_BLUE);
  gfx.printf("%3.0f",humi);
  gfx.setFont(font3);
  gfx.setCursor(gfx.getCursorX(), pos_y+diff*0.5);
  gfx.print("%");
  gfx.setCursor(0, pos_y += diff);
  gfx.setFont(font2);
  gfx.setTextSize(1.0, 1.0);
  gfx.setTextColor(TFT_GREEN);
  gfx.printf("  %4.0f",pressure);
  gfx.setFont(font3);
  gfx.setCursor(gfx.getCursorX(), pos_y+diff*0.4);
  gfx.print("hPa");
}

void setup() {
  Serial.begin(115200);
  gfx.init();
  gfx.setBrightness(200);
  gfx.setColorDepth(16);
  bool status;
  Wire.begin(22,21);
  status = bme.begin(0x76);  
  while (!status) {
    Serial.println("BME280 fail!");
    delay(1000);
  }
}

void loop() {

  drawenvironment();
  delay(5000);
}
