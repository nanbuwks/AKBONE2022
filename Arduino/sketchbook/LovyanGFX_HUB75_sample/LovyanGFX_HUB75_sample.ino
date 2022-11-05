#define LGFX_USE_V1
#include <LovyanGFX.hpp>

#include <lgfx/v1/panel/Panel_HUB75.hpp>
#include <lgfx/v1/platforms/esp32/Bus_HUB75.hpp>

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

  /* // 64x32パネル用
      cfg.pin_r1 = GPIO_NUM_33; // R1
      cfg.pin_r2 = GPIO_NUM_18; // R2
      cfg.pin_g1 = GPIO_NUM_32; // G1
      cfg.pin_g2 = GPIO_NUM_19; // G2
      cfg.pin_b1 = GPIO_NUM_25; // B1
      cfg.pin_b2 = GPIO_NUM_5 ; // B2
  /*/ // 128/64パネル用
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
      // cfg.address_mode = cfg.address_binary;

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

#define CLK_PULSE          digitalWrite(cfg.pin_clk, HIGH); digitalWrite(cfg.pin_clk, LOW);

void fm6124init(void) {

    auto cfg = gfx._bus_instance.config();


    bool REG1[16] = {0,0,0,0,0, 0,1,1,1,1,1, 0,0,0,0,0};    // this sets global matrix brightness power
    bool REG2[16] = {0,0,0,0,0, 0,0,0,0,1,0, 0,0,0,0,0};    // a single bit enables the matrix output
    for (uint8_t _pin:{ cfg.pin_r1
                      , cfg.pin_r2
                      , cfg.pin_g1
                      , cfg.pin_g2
                      , cfg.pin_b1
                      , cfg.pin_b2
                      , cfg.pin_clk
                      , cfg.pin_lat
                      , cfg.pin_oe}){
        pinMode(_pin, OUTPUT);
        digitalWrite(_pin, LOW);
    }

    digitalWrite(cfg.pin_oe, HIGH); // Disable Display

size_t PIXELS_PER_ROW = 128;
    // Send Data to control register REG1
    // this sets the matrix brightness actually
    for (int l = 0; l < PIXELS_PER_ROW; l++){
        for (uint8_t _pin:{cfg.pin_r1, cfg.pin_r2, cfg.pin_g1, cfg.pin_g2, cfg.pin_b1, cfg.pin_b2})
          digitalWrite(_pin, REG1[l%16]);   // we have 16 bits shifters and write the same value all over the matrix array

        if (l > PIXELS_PER_ROW - 12){         // pull the latch 11 clocks before the end of matrix so that REG1 starts counting to save the value
            digitalWrite(cfg.pin_lat, HIGH);
        }
        CLK_PULSE
    }

    // drop the latch and save data to the REG1 all over the FM6124 chips
    digitalWrite(cfg.pin_lat, LOW);

    // Send Data to control register REG2 (enable LED output)
    for (int l = 0; l < PIXELS_PER_ROW; l++){
        for (uint8_t _pin:{cfg.pin_r1, cfg.pin_r2, cfg.pin_g1, cfg.pin_g2, cfg.pin_b1, cfg.pin_b2})
          digitalWrite(_pin, REG2[l%16]);   // we have 16 bits shifters and we write the same value all over the matrix array

        if (l > PIXELS_PER_ROW - 13){       // pull the latch 12 clocks before the end of matrix so that reg2 stars counting to save the value
            digitalWrite(cfg.pin_lat, HIGH);
        }
        CLK_PULSE
    }

    // drop the latch and save data to the REG1 all over the FM6126 chips
    digitalWrite(cfg.pin_lat, LOW);

    // blank data regs to keep matrix clear after manipulations
    for (uint8_t _pin:{cfg.pin_r1, cfg.pin_r2, cfg.pin_g1, cfg.pin_g2, cfg.pin_b1, cfg.pin_b2})
       digitalWrite(_pin, LOW);

    for (int l = 0; l < PIXELS_PER_ROW; ++l){
        CLK_PULSE
    }

    digitalWrite(cfg.pin_lat, HIGH);
    CLK_PULSE
    digitalWrite(cfg.pin_lat, LOW);
    digitalWrite(cfg.pin_oe, LOW); // Enable Display
    CLK_PULSE
}

void setup() {

// LEDドライバのレジスタ設定
// fm6124init();

  gfx.init();
  gfx.setBrightness(90);

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
}

void loop() {
  delay(16);
  gfx.fillCircle(rand()%gfx.width(), rand()%gfx.height(), (rand()&7)+3, rand());
}
