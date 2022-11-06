#define LGFX_USE_V1
#include <LovyanGFX.hpp>

#include <lgfx/v1/panel/Panel_HUB75.hpp>
#include <lgfx/v1/platforms/esp32/Bus_HUB75.hpp>

#define PANEL_64x32
// #define PANEL_128x64

#include <MPU6500_WE.h>
#include <Wire.h>
#define MPU6500_ADDR 0x68
MPU6500_WE myMPU6500 = MPU6500_WE(MPU6500_ADDR);

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
      // cfg.address_mode = cfg.address_shiftreg;
      cfg.address_mode = cfg.address_binary;

      // LEDドライバの初期化コマンドを指定する
      cfg.initialize_mode = cfg.initialize_none;
      // cfg.initialize_mode = cfg.initialize_fm6124;

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
static LGFX_Sprite sprite(&gfx);
static LGFX_Sprite sprite2(&sprite);
#if defined(PANEL_64x32)
  const lgfx::IFont* font1 = &fonts::efontJA_12;
  const lgfx::IFont* font2 = &fonts::Font0;
  const lgfx::IFont* font3 = &fonts::TomThumb;
  float textsizex=1.0;
  float textsizey=0.95;
#else
  const lgfx::IFont* font1 = &fonts::efontJA_24;
  const lgfx::IFont* font2 = &fonts::Font4;
  const lgfx::IFont* font3 = &fonts::Font2;
  float textsizex=1.0;
  float textsizey=0.95;
#endif


void setup() {
  Serial.begin(115200);
  gfx.setBrightness(255);
#if defined(PANEL_64x32)
  sprite.createSprite(64,32);
#else
  sprite.createSprite(128,64);
#endif
  sprite2.createSprite(20,20);
  sprite2.fillRect(5,5,10,10,65535);

  Wire.begin(22,21);
  gfx.init();
  if(!myMPU6500.init()){
    Serial.println("MPU6500 does not respond");
  }
  else{
    Serial.println("MPU6500 is connected");
  }
  Serial.println("Position you MPU6500 flat and don't move it - calibrating...");
  //gfx.setTextDatum( textdatum_t::baseline_center );
  gfx.setFont(font1);
  gfx.setTextSize(textsizex, textsizey);
  gfx.setTextWrap(true);
  gfx.setTextColor(0xAAAAFFU, TFT_BLACK);
  gfx.setCursor(0,0);
  gfx.println("画面を上にして静置してください");
  delay(3000);
  myMPU6500.autoOffsets();
  delay(1000);


  Serial.println("Done!");
  myMPU6500.enableGyrDLPF();
  myMPU6500.setGyrDLPF(MPU6500_DLPF_6);
  myMPU6500.setSampleRateDivider(5);
  myMPU6500.setGyrRange(MPU6500_GYRO_RANGE_250);
  myMPU6500.enableAccDLPF(true); 
  myMPU6500.setAccDLPF(MPU6500_DLPF_6);
}
void loop() {
  static int i =0;
  static float x = gfx.width()/2;
  static float y = gfx.height()/2;
  static float xv = 0;
  static float yv = 0;
  xyzFloat gValue = myMPU6500.getGValues();
  xyzFloat gyr = myMPU6500.getGyrValues();
  float temp = myMPU6500.getTemperature();
  float resultantG = myMPU6500.getResultantG(gValue);

  // ジャイロの取付向きで選択 AKBONE2022 を横長に配置
  
  xv = xv + gValue.y;
  yv = yv - gValue.x;
/*
  // ジャイロの取付向きで選択 AKBONE2022 を縦長に配置
  xv = xv + gValue.x;
  yv = yv + gValue.y;
*/
  x = x+xv;
  y = y+yv;
  if ( x < 0 ) { x = 0; xv=0;}
  if ( y < 0 ) { y = 0; yv=0;}
  if ( gfx.width() < x ) { x = gfx.width(); xv=0;}
  if ( gfx.height() < y ) { y = gfx.height(); yv=0; }
  
  Serial.println("Acceleration in g (x,y,z):");
  Serial.print(gValue.x);
  Serial.print("   ");
  Serial.print(gValue.y);
  Serial.print("   ");
  Serial.println(gValue.z);
  Serial.print("Resultant g: ");
  Serial.println(resultantG);

  Serial.println("Gyroscope data in degrees/s: ");
  Serial.print(gyr.x);
  Serial.print("   ");
  Serial.print(gyr.y);
  Serial.print("   ");
  Serial.println(gyr.z);

  Serial.print("Temperature in °C: ");
  Serial.println(temp);

  Serial.println("********************************************");

  sprite2.setPivot(8, 8);
  sprite2.pushRotateZoom(x, y, i, 1.0, 1.0);
  sprite.pushSprite(0,0);
  i++;
  delay(100);
  
}
