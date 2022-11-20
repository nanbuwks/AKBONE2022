# AKBONE2022 News

## はじめに
本サンプルソフトではP2高解像度LEDパネル（64x128)を接続したAKBONE2022において、Yahoo Newsを横スクロール表示します。
![](img/2022-11-20-10-47-56.png)

## 事前準備
SSL ( https )ページを取得するには、ブラウザと同じように公開鍵を含んだルート証明書をマイコンに保存しておく必要があります。
private.hppにあらかじめ、以下の記事を参考にYahoo! Japan RSS ニュースサイトのルート証明書を取得しておいてください。    
private.hppには仮の鍵が記載してあります。

https://www.mgo-tec.com/blog-entry-arduino-esp32-ssl-stable-root-ca.html/2

## 使用方法
初回起動時やWiFi環境が異なる場所での利用の場合、以下の画面時にSmartConfig(ESPTouch)でWiFiの設定を行ってください。  

https://www.espressif.com/en/products/software/esp-touch/overview

![](img/![](img/2022-11-20-10-50-03.png).png)
