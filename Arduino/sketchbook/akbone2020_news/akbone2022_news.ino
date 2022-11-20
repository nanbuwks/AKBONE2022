// AKBONE2022 AKBONE2022 Yahoo News
// 2022.11.18 K.Ohe New Create
#include "akbone.hpp"
#include <string.h>
#include "YahooNewsLib.hpp"

//#define FONT          &fonts::lgfxJapanMincho_32
#define FONT          &fonts::lgfxJapanMincho_40
#define FONT_SIZE     64                

uint32_t WebGet_LastTime = 0;
static lgfx::LGFX_Sprite cBuf;      // ex [64 * 64] Buffer

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
    cBuf.createSprite(128 + FONT_SIZE, FONT_SIZE);
    cBuf.setTextSize(1.6, 1.6);
    cBuf.setFont(FONT);       
    cBuf.setTextWrap(false);
    cBuf.setTextColor(TFT_YELLOW, TFT_BLUE);
    gfx.clear();
}

//String str = "◇Yahoo!ニュース・トピックス - 主要：Yahoo! JAPANのニュース・トピックスで取り上げている最新の見出しを提供しています。  ◇COP27 交渉難航で会期延長へ◇9カ月間の「闇」ヘルソン住民証言◇森元首相 ゼレンスキー氏を批判◇同時感染を経験「痛みで寝れず」◇ファスト映画賠償5億円 識者解説◇Amazon 来年にかけ人員削減継続◇カタールW杯 会場で酒販売NGに◇C・ロナのマンU批判 波紋広がる";
String str;
size_t textlen = 0;
size_t textpos = 0;
//===========================================================
//	loop() ： arduino main loop
//===========================================================
void loop()
{
    //=====================================================================
    // 180秒(3分)毎にYahoo Newsを取得
    //=====================================================================
    if((millis() - WebGet_LastTime) > 180000){        
        //wifiConnect();       // LGFX 0.4
        str = https_Web_Get("news.yahoo.co.jp", "/rss/topics/top-picks.xml", '\n', "</rss>", "<title>", "</title>", "<description>", "</description>", "◇");
        str = str + "　　　　　　　　　　　　　　　　　　　";
        textlen = str.length();
        WebGet_LastTime = millis();
        //wifiDisconnect();   // LGFX 0.4
    }
    //=====================================================================
    // Newsをスクロール表示
    //=====================================================================
    int32_t cursor_x = cBuf.getCursorX() - 1;   // 現在のカーソル位置を取得し、1ドット左に移動
    if (cursor_x <= 0) // カーソル位置が左端に到達している場合は一周したと判断
    {
        textpos = 0;            // 文字列の読取り位置をリセット
        cursor_x = gfx.width(); // 新たな文字が画面右端に描画されるようにカーソル位置を変更
    }
    cBuf.setCursor(cursor_x, 0); // カーソル位置を更新
    cBuf.scroll(-1, 0);          // キャンバスの内容を1ドット左にスクロール
    while (textpos < textlen && cursor_x <= gfx.width()) // 画面右端に文字が書けるか判定
    {
        //Serial.print(str[textpos]);
        cBuf.print(str[textpos++]);   // 1バイトずつ出力 (マルチバイト文字でもこの処理で動作します)
        cursor_x = cBuf.getCursorX();  // 出力後のカーソル位置を取得
    }

    cBuf.pushSprite(&gfx, 0, 0);
}
