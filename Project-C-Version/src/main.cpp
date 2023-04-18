#include <Arduino.h>
#include <TFT_eSPI.h>

#define X_FIRST_BLOCK 75
#define Y_FIRST_BLOCK 30

struct squareBlock {
  int x_pos = 0;
  int y_pos = 0;
  bool alive = false;
};
squareBlock blocks[8][8];

void setupScreen();
void initBlocks();
void printBlocks();

TFT_eSPI tft = TFT_eSPI();

void setup() {
  Serial.begin(921600);
  Serial.println("Start!");
  setupScreen();
  initBlocks();

}

void loop() {
  tft.fillScreen(tft.color565(56, 178, 92)); // 0x38b25c
  printBlocks();
  delay(3000);
  
}

void setupScreen()
{
  tft.init();
  tft.setRotation(3);
  tft.setSwapBytes(true);
  tft.setTextColor(TFT_WHITE);
}

void initBlocks(){//set correct initial position for each blocks
  for(int i = 0; i < 8; i ++){
    for(int j = 0; j < 8; j ++){
      blocks[i][j].x_pos = X_FIRST_BLOCK + (11 * i);
      blocks[i][j].y_pos = Y_FIRST_BLOCK + (11 * j);
    }
  }
}

void printBlocks(){
  for(int i = 0; i < 8; i ++){
    for(int j = 0; j < 8; j ++){
      if(blocks[i][j].alive){
        tft.fillRect(blocks[i][j].x_pos, blocks[i][j].y_pos, 9, 9, TFT_WHITE);
      }else{
        tft.drawRect(blocks[i][j].x_pos, blocks[i][j].y_pos, 9, 9, TFT_WHITE);
      }
    }
  }
}