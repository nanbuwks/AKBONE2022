#define MP3FILENAME "/test.mp3"
#include <Arduino.h>
#include "FS.h"
#include "SD.h" // LGFXより先に読み込む必要がある
#include "SD_MMC.h"
#include "AudioFileSourceSD.h"
#include "AudioFileSourceID3.h"
#include "AudioGeneratorMP3.h"
#include "AudioOutputI2SNoDAC.h"

AudioFileSourceSD *file;
AudioFileSourceID3 *id3;
AudioOutputI2S *out;
AudioGeneratorMP3 *mp3;

// Called when a metadata event occurs (i.e. an ID3 tag, an ICY block, etc.
void MDCallback(void *cbData, const char *type, bool isUnicode, const char *string)
{
  (void)cbData;
  Serial.printf("ID3 callback for: %s = '", type);

  if (isUnicode) {
    string += 2;
  }
  
  while (*string) {
    char a = *(string++);
    if (isUnicode) {
      string++;
    }
    Serial.printf("%c", a);
  }
  Serial.printf("'\n");
  Serial.flush();
}

void setup()
{
  Serial.begin(115200);
  delay(1000);
  Serial.println();
  audioLogger = &Serial;
}
void loop()
{
  char mp3filename[50];
  int size;
  SPIClass spi = SPIClass(HSPI);
  spi.begin(13 /* SCK */, 35 /* MISO */, 23 /* MOSI */, 15 /* SS */);
    
  if (!SD.begin(15 /* SS */, spi, 80000000)) {
      Serial.println("Card Mount Failed");
      return;
  }
  sprintf(mp3filename,MP3FILENAME);
  Serial.print(mp3filename);
/*  File fp=SD.open(filename);
  if (!fp){
      Serial.println("---fail----");
  }
*/
  id3 = NULL; 
  out = new AudioOutputI2S(0,AudioOutputI2S::INTERNAL_DAC);
//  out = new AudioOutputI2SNoDAC();
//  out->SetPinout(25);
  
  mp3 = new AudioGeneratorMP3();
  file = new AudioFileSourceSD(mp3filename);
  id3 = new AudioFileSourceID3(file);
  id3->RegisterMetadataCB(MDCallback, (void*)"ID3TAG");
  mp3->begin(id3, out);
  Serial.printf("Playback of '%s' begins...\n", mp3filename);

  Serial.println("Can't find .mp3 file in SPIFFS");

  while (1) {
    if (mp3->isRunning()) {
      Serial.println("Running");
      if (!mp3->loop()) mp3->stop();
      delay(1000);
    } else {
      Serial.println("MP3 done");
      delay(1000);
    }
  }
}
