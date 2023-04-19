#include <Arduino.h>
#include <TFT_eSPI.h>

#define X_FIRST_BLOCK 140
#define Y_FIRST_BLOCK 10

void setupScreen();
void initBlocks();
void printBlocks();
void setBlockStatus();
void findNeighbour();
void setBlinker();
void debugPrintNeighbour();
void printGridInfo();

struct squareBlock {
  int x_pos = 0;
  int y_pos = 0;
  int neighbour_num = 0;
  bool alive = false;
};
squareBlock blocks[8][8];
TFT_eSPI tft = TFT_eSPI();

int generation = 0;
int cellNum = 0;
bool ifRunning = false;

void setup() {
  Serial.begin(921600);
  Serial.println("Start!");
  setupScreen();
  initBlocks();
  setBlinker();
  printBlocks();
  delay(1000);
}

void loop() {
  tft.fillScreen(tft.color565(56, 178, 92)); // 0x38b25c
  findNeighbour();
  setBlockStatus();
  printBlocks();
  debugPrintNeighbour();
  generation ++;
  printGridInfo();
  delay(1000);
  
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
  tft.drawRect(X_FIRST_BLOCK - 3, Y_FIRST_BLOCK - 3 , 92, 92, TFT_WHITE);
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

void setBlockStatus(){
  for(int i = 0; i < 8; i ++){
    for(int j = 0; j < 8; j ++){
      if(blocks[i][j].alive){
        if(blocks[i][j].neighbour_num < 2){
          blocks[i][j].alive = false;
          cellNum --;
        }
        else if(blocks[i][j].neighbour_num > 3){
          blocks[i][j].alive = false;
          cellNum --;
        }
      }else if(!blocks[i][j].alive){
        if(blocks[i][j].neighbour_num == 3){
          blocks[i][j].alive = true;
          cellNum ++;
        }
      }
    }
  }
}

void findNeighbour(){
  for(int i = 0; i < 8; i ++){
    for(int j = 0; j < 8; j ++){
      blocks[i][j].neighbour_num = 0;// clear the number of neighbour
      int neighbourNum = 0;

      if(((i - 1) > 0)&&((j - 1) > 0)){
        if(blocks[i - 1][j - 1].alive){
          neighbourNum ++;
        }
      }
      if((i - 1) > 0){
        if(blocks[i - 1][j].alive){
          neighbourNum ++;
        }
      }
      if(((i - 1) > 0)&&((j + 1) < 8)){
        if(blocks[i - 1][j + 1].alive){
          neighbourNum ++;
        }
      }
      if((j - 1) > 0){
        if(blocks[i][j - 1].alive){
          neighbourNum ++;
        }
      }
      if((j + 1) < 8){
        if(blocks[i][j + 1].alive){
          neighbourNum ++;
        }
      }
      if(((i + 1) < 8)&&((j - 1) > 0)){
        if(blocks[i + 1][j - 1].alive){
          neighbourNum ++;
        }
      }
      if((i + 1) < 8){
        if(blocks[i + 1][j].alive){
          neighbourNum ++;
        }
      }
      if(((i + 1) < 8)&&((j + 1) < 8)){
        if(blocks[i + 1][j + 1].alive){
          neighbourNum ++;
        }
      }

      blocks[i][j].neighbour_num = neighbourNum;

    }
  }
}

void debugPrintNeighbour(){
  for(int i = 0; i < 8; i ++){
    for(int j = 0; j < 8; j ++){
      Serial.print(" ");
      Serial.print(blocks[i][j].neighbour_num);
      Serial.print(" ");
    }
    Serial.println();
  }
  Serial.println();
}

void printGridInfo(){
  tft.setTextFont(2);
  tft.setTextColor(TFT_WHITE);
  tft.drawString("Genation:", 140, 100);
  tft.drawString("Cell Number:", 140, 115);
  tft.setCursor(201, 100);
  tft.print(generation);
  tft.setCursor(221, 115);
  tft.print(cellNum);
  
}

void setBlinker(){
  blocks[3][1].alive = true;
  blocks[3][2].alive = true;
  blocks[3][3].alive = true;
}
