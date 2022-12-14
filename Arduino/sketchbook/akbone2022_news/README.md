# AKBONE2022 News

## はじめに
本サンプルソフトではP2高解像度LEDパネル(64x128)を接続したAKBONE2022において、Yahoo Newsを横スクロール表示します。
![](img/2022-11-20-10-47-56.png)

## 事前準備
SSL ( https )ページを取得するには、ブラウザと同じように公開鍵を含んだルート証明書をマイコンに保存しておく必要があります。
private.hppにあらかじめ、以下の記事を参考にYahoo! Japan RSS ニュースサイトのルート証明書を取得しておいてください。  
private.hppには仮の鍵が記載してあります。

https://www.mgo-tec.com/blog-entry-arduino-esp32-ssl-stable-root-ca.html/2

上記記事から、Chromeの証明書ビューアの表示と操作方法が変更になっています。証明書の取得は、詳細タブの証明書のフィールドで、サブジェクトの公開鍵を選択後、エクスポートボタンを押下してください。

![](img/2022-11-20-11-14-04.png)

## ビルド時の注意事項
スケッチが1MBを超えてしまうので、Arduino IDEのPartition SizeをNo OTAなどに変更し2MBに拡大しておく必要があります。  
![](img/2022-11-20-17-48-19.png)

PlatformIOの場合は、platformio.iniに以下を追加してください。  
```
board_build.partitions = no_ota.csv
```  

## 使用方法
初回起動時やWiFi環境が異なる場所での利用の場合、以下の画面時にSmartConfig(ESPTouch)でWiFiの設定を行ってください。  

https://www.espressif.com/en/products/software/esp-touch/overview

![](img/![](img/2022-11-20-10-50-03.png).png)
