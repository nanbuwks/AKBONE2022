#define LGFX_USE_V1
#include <LovyanGFX.hpp>

#include <lgfx/v1/panel/Panel_HUB75.hpp>
#include <lgfx/v1/platforms/esp32/Bus_HUB75.hpp>

#define PANEL_64x32
// #define PANEL_128x64


#include "FS.h"
#include "SD.h"
#include "SD_MMC.h"

#define TEST_FILE_SIZE (4 * 1024 * 1024)


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

#define CLK_PULSE          digitalWrite(cfg.pin_clk, HIGH); digitalWrite(cfg.pin_clk, LOW);





void testWriteFile(fs::FS &fs, const char *path, uint8_t *buf, int len)
{
  unsigned long start_time = millis();
  Serial.printf("Test write %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  int loop = TEST_FILE_SIZE / len;
  while (loop--)
  {
    if (!file.write(buf, len)) {
      Serial.println("Write failed");
      return;
    }
  }
  file.flush();
  file.close();
  unsigned long time_used = millis() - start_time;
  Serial.printf("Write file used: %d ms, %f KB/s\n", time_used, (float)TEST_FILE_SIZE / time_used);
}

void testReadFile(fs::FS &fs, const char *path, uint8_t *buf, int len)
{
  unsigned long start_time = millis();
  Serial.printf("Test read %s\n", path);

  File file = fs.open(path);
  if (!file) {
    Serial.println("Failed to open file for reading");
    return;
  }
  int loop = TEST_FILE_SIZE / len;
  while (loop--)
  {
    if (!file.read(buf, len)) {
      Serial.println("Read failed");
      return;
    }
  }
  file.close();
  unsigned long time_used = millis() - start_time;
  Serial.printf("Read file used: %d ms, %f KB/s\n", time_used, (float)TEST_FILE_SIZE / time_used);
}

void testIO(fs::FS &fs)
{
  /* malloc will not reset all bytes to zero, so it is a random data */
  uint8_t *buf = (uint8_t*)malloc(64 * 1024);

  testWriteFile(fs, "/test_1k.bin", buf, 1024);
  testWriteFile(fs, "/test_2k.bin", buf, 2 * 1024);
  testWriteFile(fs, "/test_4k.bin", buf, 4 * 1024);
  testWriteFile(fs, "/test_8k.bin", buf, 8 * 1024);
  testWriteFile(fs, "/test_16k.bin", buf, 16 * 1024);
  testWriteFile(fs, "/test_32k.bin", buf, 32 * 1024);
  testWriteFile(fs, "/test_64k.bin", buf, 64 * 1024);

  testReadFile(fs, "/test_1k.bin", buf, 1024);
  testReadFile(fs, "/test_2k.bin", buf, 2 * 1024);
  testReadFile(fs, "/test_4k.bin", buf, 4 * 1024);
  testReadFile(fs, "/test_8k.bin", buf, 8 * 1024);
  testReadFile(fs, "/test_16k.bin", buf, 16 * 1024);
  testReadFile(fs, "/test_32k.bin", buf, 32 * 1024);
  testReadFile(fs, "/test_64k.bin", buf, 64 * 1024);
}

void setup() {

  Serial.begin(115200);

// LEDドライバのレジスタ設定
//fm6124init();

  gfx.init();
  gfx.setBrightness(255);



  /* test SD SPI Mode at HSPI */
  SPIClass spi = SPIClass(HSPI);
  spi.begin(13 /* SCK */, 35 /* MISO */, 23 /* MOSI */, 15 /* SS */);
  if (!SD.begin(15 /* SS */, spi, 80000000)) {
    Serial.println("Card Mount Failed");
    return;
  }
  testIO(SD);

}

void loop() {
}
