/*#include "SdFat.h"
SdFs sd;
FsFile file;
*/
#include "FS.h"
#include "SD.h" // LGFXより先に読み込む必要がある
#include "SD_MMC.h"

// video info
#define FRAME_W 128
#define FRAME_H 64
#define FRAMEBUFF_SIZE FRAME_W*FRAME_H*2 
#define FILENAME "/128x64F.rgb"
auto buffer = (uint8_t*)malloc(FRAMEBUFF_SIZE);

//rgb565=buffer;
#define LGFX_USE_V1
#include <LovyanGFX.hpp>

// #define PANEL_64x32
#define PANEL_128x64



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


void setup() {
  Serial.begin(115200);
    //gfx.setColorDepth(8);
  gfx.init();
  gfx.setBrightness(128);
  gfx.setColorDepth(16);
  Serial.println("setup done");
  delay(1000);
}

void loop() {
  char filename[50];
  int size;
    /* test SD SPI Mode at HSPI */
  SPIClass spi = SPIClass(HSPI);
  spi.begin(13 /* SCK */, 35 /* MISO */, 23 /* MOSI */, 15 /* SS */);
  //sd.begin(13 /* SCK */, 35 /* MISO */, 23 /* MOSI */, 15 /* SS */);
  if (!SD.begin(15 /* SS */, spi, 80000000)) {
//   if (!sd.begin(15 /* SS */, 16000000)) {
      Serial.println("Card Mount Failed");
      return;
  }

  
  while (1){
    sprintf(filename,FILENAME);
    Serial.print(filename);
    File fp=SD.open(filename);
    if (!fp){
      Serial.println("---fail----");
      continue;
    }
    for ( int i=1; fp.available(); i++){
      size = fp.read(buffer,FRAMEBUFF_SIZE);     
      Serial.println(String(i)+" "+String(size) + "bytes  " + String(millis()) + "ms ");
     // gfx.setSwapBytes(true);
      gfx.pushImage(0,0,FRAME_W,FRAME_H,(uint16_t *)buffer);
     // 位置を変化するお遊び
     // gfx._panel_instance.setPanelPosition( 3,  i,  64);
    }
    fp.close();
  }
}
