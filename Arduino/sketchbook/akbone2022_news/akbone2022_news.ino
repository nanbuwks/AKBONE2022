// AKBONE2022 AKBONE2022 Yahoo News
// 2022.11.18 K.Ohe New Create
// 2022.12.12 K.Ohe for P3 2nd Panel
#include "akbone.hpp"
#include <string.h>
#include "YahooNewsLib.hpp"

#if defined(PANEL_128x64)
    #define FONT          &fonts::lgfxJapanMincho_40
    #define FONT_SIZE     64
    #define FONT_MULTI   1.6
    #define SPRITE_SIZE  128
#else
    #define FONT          &fonts::lgfxJapanMincho_32
    #define FONT_SIZE     32
    #define SPRITE_SIZE   64
    #define FONT_MULTI    1.0
#endif
uint32_t WebGet_LastTime = 0;

//===========================================================
//	setup() ： arduino setup
//===========================================================
void setup() {
    WebGet_LastTime = 200000; //起動時に記事をGETするために、多めの数値で初期化しておく  

    akbone_setup();
    clearLED();
    gfx.setTextColor(TFT_BLUE, TFT_BLACK);
    wifiConnect();
    clearLED();
}

//===========================================================
//	loop() ： arduino main loop
//===========================================================
void loop()
{
    String str;
    uint16_t dly = 5;
    //=====================================================================
    // 180秒(3分)毎にYahoo Newsを取得
    //=====================================================================
    if((millis() - WebGet_LastTime) > 180000){        
        str = https_Web_Get("news.yahoo.co.jp", "/rss/topics/top-picks.xml", '\n', "</rss>", "<title>", "</title>", "<description>", "</description>", "◇");
        str = str + "　　　　　　　　　　　　　　　　　　　";
        WebGet_LastTime = millis();
    }
    //=====================================================================
    // Newsをスクロール表示
    //=====================================================================
    disp_scrollText(str, dly);
}
