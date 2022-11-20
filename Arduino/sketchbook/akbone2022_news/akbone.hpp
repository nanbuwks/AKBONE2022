#pragma once
// AKBONE2022 common library
// V0.0 2022.11.06 K.Ohe New Create
// V0.2 2022.11.17 K.Ohe Support Multi Panel

#define  LGFX_USE_V1
#include <LovyanGFX.hpp>

//===========================================================
//	LED Panel difinition
//===========================================================
// support panels
// P2 1nd K716-V3.2 , HUB75E,BRG, 128x64pixel , 256x128mm,Y座標パラレル指定
// P2 2nd P2-128X64-32S-V6.1 ,HUB75E,RGB, 128x64pixel  , 256x128mm,Y座標シフトレジスタ
// P2 3rd J1-02064128CB-SMV6.1  ,HUB75E,RGB, 128x64pixel  , 256x128mm,Y座標シフトレジスタ,Bピンディスチャージ,初期設定コマンド必要, AVTOKYO で頒布
// P3 2nd TEC-N3.051515-16A2  ,HUB75,RGB, 64x32pixel  , 192x96mm,Y座標パラレル X座標ZIGGAZ形式 AVTOKYO で頒布 TECNONブランド 
// P3 3rd P3(2121)64X64-32S-M5.1 ,HUB75E,RGB, 64x64pixel  , 192x192mm,Y座標パラレル 未頒布 
// P4 1st P4-2121-64*32-16S-HL1  ,HUB75,RGB, 64x32pixel  , 256x128mm,Y座標パラレル  AVTOKYO で頒布

//
// select one of panel panel 
//
//#define PANEL_P2_1st
//#define PANEL_P2_2nd
#define PANEL_P2_3rd
//#define PANEL_P3_1st
//#define PANEL_P3_2nd
//#define PANEL_P3_3rd
//#define PANEL_P4_1st

//
//  select panel type and configration
//
//#define PANEL_MULTI         // Multi panel support 
#define PANEL_DEPTH_8       // RGB332
//#define PANEL_DEPTH_16        // RGB565
#define PANEL_BRIGHTNESS     160
#define PANEL_REFRESH_RATE   300
//#define PANEL_REFRESH_RATE  80

//
//  select functions
//
#define USE_WIFI
//#define USE_TERMINAL
//#define USE_ENCODER
//#define USE_TFCARD
//#define USE_MPU6500
//#define USE_RTC
//#define USE_BME280
//#define USE_MENU

//===========================================================
//	prototypes
//===========================================================
struct LGFX_HUB75;

extern LGFX_HUB75 gfx;
extern void clearLED(void);
extern void akbone_setup(void);
extern void akbone_loop(void);
extern void akbone_terminal(void);
extern void panel_test(void);
extern void read_bme280(float& temp, float& pressure, float& humi);
extern void disp_bme280(void);
extern bool read_rtc(uint16_t& hour, uint16_t& minute, uint16_t& second, uint16_t& year , uint16_t& month, uint16_t& day);
extern void disp_rtc(void);
extern void init_mpu6500(bool calib = false);
extern void read_mpu6500(float& acc_x, float& acc_y, float& acc_z, float& gyr_x, float& gyr_y, float& gyr_z, float& temp, float& tanG);
extern void disp_mpu6500(void);
extern void disp_imgfile(void);
extern int16_t read_encoder(void);
extern bool read_button(void);
extern uint16_t disp_menu(uint16_t mode = 0);

//===========================================================
//	defines
//===========================================================
#if defined(PANEL_P2_1st)
    #define PANEL_128x64
    #define YADDRESS_MODE_BINARY
    #define LED_INIT_NONE
#elif defined(PANEL_P2_2nd)
    #define PANEL_128x64
    #define YADDRESS_MODE_SHIFTREG
    #define LED_INIT_NONE
#elif defined(PANEL_P2_3rd)
    #define PANEL_128x64
    #define YADDRESS_MODE_SHIFTREG
    #define LED_INIT_FM6124
#elif defined(PANEL_P3_1st)
    #define PANEL_64x32
    #define YADDRESS_MODE_BINARY
    #define LED_INIT_NONE
#elif defined(PANEL_P3_2nd)
    #define PANEL_64x32
    #define YADDRESS_MODE_BINARY
    #define XADDRESS_MODE_ZIGZAG
    #define LED_INIT_NONE
#elif defined(PANEL_P3_3rd)
    #define PANEL_64x64
    #define YADDRESS_MODE_BINARY
    #define LED_INIT_NONE
#elif defined(PANEL_P4_1st)
    #define PANEL_64x32
    #define YADDRESS_MODE_BINARY
    #define LED_INIT_NONE
#endif

//===========================================================
//	HUB75 pin assign
//===========================================================
/*   
HUB75 Standard set RGB configuration
(PANEL_P4_1st, PANEL_P3_1st, PANEL_P3_2nd, PANEL_P3_3rd, PANEL_P2_3rd)
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
HUB75E + BRG panel set BRG configuration (PANEL_P2_1st)
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
some panel has no D,E line, please set RGB configuration(PANEL_P2_2nd)
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

//===========================================================
//	LGFX difinition
//===========================================================
struct LGFX_HUB75 : public lgfx::LGFX_Device {
    // 座標を8ドット単位で逆順にする関数
    static void convertCoordinate(uint_fast16_t &x, uint_fast16_t &y)
    {
        if (x & 8) { x = x ^ 7; }
    }
    lgfx::Bus_HUB75 _bus_instance;

    // 1枚だけで使用する場合はこちら
    lgfx::Panel_HUB75 _panel_instance;

    // 2枚以上接続する場合はこちら
    //lgfx::Panel_HUB75_Multi _panel_instance;

    //Panel_Custom_HUB75 _panel_instance;
    //lgfx::Bus_HUB75 _bus_instance;

    LGFX_HUB75(void)
    {
#if defined(XADDRESS_MODE_ZIGZAG)

        // X 座標が8ドット単位で逆順になるパネルの場合、座標変換関数を使用するよう指定する
        _panel_instance.convertCoordinate = convertCoordinate;
#endif
        {
            auto cfg = _bus_instance.config();
            // I2Sのポート番号を指定する。もし同時にI2S_DACを使用したい場合は 1に設定する。
            // (※ I2S_DACはポート0専用なので、HUB75ではポート1を使用することでDAC用にポート0を空けておく)
            cfg.port = 1;

        #if defined(PANEL_P4_1st) || defined(PANEL_P3_1st) || defined(PANEL_P3_2nd) || defined(PANEL_P3_3rd) || defined(PANEL_P2_3rd)
            cfg.pin_r1 = GPIO_NUM_33; // R1
            cfg.pin_r2 = GPIO_NUM_18; // R2
            cfg.pin_g1 = GPIO_NUM_32; // G1
            cfg.pin_g2 = GPIO_NUM_19; // G2
            cfg.pin_b1 = GPIO_NUM_25; // B1
            cfg.pin_b2 = GPIO_NUM_5 ; // B2
        #elif defined(PANEL_P2_1st)   // 128/64パネル用
            cfg.pin_r1 = GPIO_NUM_32; // R1
            cfg.pin_r2 = GPIO_NUM_19; // R2
            cfg.pin_g1 = GPIO_NUM_25; // G1
            cfg.pin_g2 = GPIO_NUM_5 ; // G2
            cfg.pin_b1 = GPIO_NUM_33; // B1
            cfg.pin_b2 = GPIO_NUM_18; // B2
        #elif defined(PANEL_P2_2nd)     // new 128x64
            cfg.pin_r1 = GPIO_NUM_25; // R1 32
            cfg.pin_r2 = GPIO_NUM_5;  // R2 19
            cfg.pin_g1 = GPIO_NUM_32; // G1 25 
            cfg.pin_g2 = GPIO_NUM_19 ;// G2 5
            cfg.pin_b1 = GPIO_NUM_33; // B1
            cfg.pin_b2 = GPIO_NUM_18; // B2

        #endif

            cfg.pin_lat = GPIO_NUM_17; // LAT
            cfg.pin_oe  = GPIO_NUM_16; // OE
            cfg.pin_clk = GPIO_NUM_4 ; // CLK

            cfg.pin_addr_a = GPIO_NUM_12;
            cfg.pin_addr_b = GPIO_NUM_14;
            cfg.pin_addr_c = GPIO_NUM_27;
            cfg.pin_addr_d = GPIO_NUM_26;
        #if defined(PANEL_P4_1st) || defined(PANEL_P3_3rd)
            cfg.pin_addr_e = GPIO_NUM_2;
        #else 
            cfg.pin_addr_e = -1;
        #endif

            // 1秒間の更新回数を設定
            cfg.refresh_rate = PANEL_REFRESH_RATE;

        #if defined(YADDRESS_MODE_SHIFTREG)
            cfg.address_mode = cfg.address_shiftreg;
        #else
            cfg.address_mode = cfg.address_binary;
        #endif

        #if defined(LED_INIT_FM6124)
            //  FM6124のレジスタを設定 
            cfg.initialize_mode = cfg.initialize_fm6124;
        #else
            cfg.initialize_mode = cfg.initialize_none;
        #endif
            // DMA用のタスクの優先度 (FreeRTOSのタスク機能を使用)
            cfg.task_priority = 2;

            // DMA用のタスクに使用するCPUコア設定 (FreeRTOSのタスク機能を使用)
        #if defined(USE_WIFI)
            cfg.task_pinned_core = APP_CPU_NUM;
        #else
            cfg.task_pinned_core = PRO_CPU_NUM;
        #endif
            _bus_instance.config(cfg);
            _panel_instance.setBus(&_bus_instance);
        }

        {
            auto cfg = _panel_instance.config();

            // パネルサイズを指定する
            // 複数枚使用する場合(Panel_HUB75_Multi使用時) は表示領域全体の縦横サイズを指定

        #if defined(PANEL_128x64)
            cfg.memory_width  = cfg.panel_width  = 128;
            cfg.memory_height = cfg.panel_height =  64;
        #elif defined(PANEL_64x64)
            cfg.memory_width  = cfg.panel_width  = 64;
            cfg.memory_height = cfg.panel_height = 64;
        #else 
            cfg.memory_width  = cfg.panel_width  =  64;
            cfg.memory_height = cfg.panel_height =  32;
        #endif
            _panel_instance.config(cfg);
            setPanel(&_panel_instance);
        }
#if defined(PANEL_MULTI)
// 複数枚使用する場合(Panel_HUB75_Multi使用時) この設定を行う
        {
            auto cfg = _panel_instance.config_detail();

            // 構成パネルの総枚数を指定
            cfg.panel_count = 3;

            // パネル1枚の幅を指定
            cfg.single_width = 64;

            // パネル1枚の高さを指定
            cfg.single_height = 32;

            _panel_instance.config_detail(cfg);

            // 各パネルの配置座標を設定する
            _panel_instance.setPanelPosition( 0,   0,   0);
            _panel_instance.setPanelPosition( 1,   0,  32);
            _panel_instance.setPanelPosition( 2,  64,   0);
        }
#endif
    }
};


