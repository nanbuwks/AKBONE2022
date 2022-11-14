#define PANEL_TEC-N3.051515-16A2
#include "LovyanGFX_LEDPANEL.h"

#include <MPU6500_WE.h>
#include <Wire.h>
#define MPU6500_ADDR 0x68
MPU6500_WE myMPU6500 = MPU6500_WE(MPU6500_ADDR);



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
