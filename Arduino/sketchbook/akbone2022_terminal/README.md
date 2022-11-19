# AKBONE2022 Terminal

## はじめに
本サンプルソフトではP2高解像度LEDパネル（64x128)とLinuxボード（LicheeNano）を接続したAKBONE2022において、コンソール画面の表示とシリアル入出力を実現します。

## AKBONEの設定
- Lichee NANOを搭載
- TFCARDスロットは搭載しない
- JP1、JP2は 2-3をショートしTX2、RX2を選択

## 使用方法
LicheeNanoのシリアル出力は、LEDパネルとUSBシリアルに出力されます。
USBシリアルからの入力はLicheeNanoのシリアルに出力されます。
USBシリアルにTerminalソフトを115200bpsに設定して接続してください。


