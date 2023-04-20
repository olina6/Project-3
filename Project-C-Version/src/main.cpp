#include <Arduino.h>
#include <TFT_eSPI.h>

#define X_FIRST_BLOCK 140
#define Y_FIRST_BLOCK 10
#define ENCODER_ONE_A 25
#define ENCODER_ONE_B 26
#define ENCODER_ONE_SWITCH 27
#define ENCODER_TWO_A 36
#define ENCODER_TWO_B 37
#define ENCODER_TWO_SWITCH 38

void setupScreen();
void initBlocks();
void printBlocks();
void setBlockStatus();
void findNeighbour();
void setBlinker();
void debugPrintNeighbour();
void printGridInfo();
void setInitCellNum();
void printMenu();

void IRAM_ATTR readQuadrature(bool A_B);
void IRAM_ATTR ISR_SWITCH_ONE();
void IRAM_ATTR ISR_SWITCH_TWO();
void IRAM_ATTR ISR_ENCODER_ONE();
void IRAM_ATTR ISR_ENCODER_TWO();
struct squareBlock
{
  int x_pos = 0;
  int y_pos = 0;
  int neighbour_num = 0;
  bool alive = false;
};
squareBlock blocks[8][8];
TFT_eSPI tft = TFT_eSPI();

int generation = 0;
int cellNum = 0;

int int_nu = 0;
int flag = 0;

int row = 0;
int column = 0;
bool menu = true;
bool editMode = false;
bool running = false;

long debouncingTime = 500;
long debouncingFlagSwitchA = 0;

void setup()
{
  Serial.begin(921600);
  Serial.println("Start!");

  pinMode(ENCODER_ONE_A, INPUT);
  pinMode(ENCODER_ONE_B, INPUT);
  pinMode(ENCODER_ONE_SWITCH, INPUT_PULLUP);
  pinMode(ENCODER_TWO_A, INPUT);
  pinMode(ENCODER_TWO_B, INPUT);
  pinMode(ENCODER_TWO_SWITCH, INPUT_PULLUP);

  attachInterrupt(ENCODER_ONE_A, ISR_ENCODER_ONE, CHANGE);
  attachInterrupt(ENCODER_ONE_SWITCH, ISR_SWITCH_ONE, FALLING);

  // attachInterrupt(ENCODER_TWO_A, )
  // attachInterrupt(ENCODER_TWO_SWITCH, )

  setupScreen();
  initBlocks();
  setBlinker();
  printBlocks();
  setInitCellNum();
  printMenu();
  delay(1000);
}

void loop()
{
  tft.fillScreen(tft.color565(56, 178, 92)); // 0x38b25c
  printMenu();
  if (menu)
  {
    if (running)
    {
      findNeighbour();
      setBlockStatus();
      printBlocks();
      debugPrintNeighbour();
      generation++;
      printGridInfo();
      delay(1000);
    }
    else
    {
      printBlocks();
      printGridInfo();
      delay(1000);
    }
  }
  else
  {
    printBlocks();
    printGridInfo();
    delay(1000);
  }
}

void setupScreen()
{
  tft.init();
  tft.setRotation(3);
  tft.setSwapBytes(true);
  tft.setTextColor(TFT_WHITE);
}

void initBlocks()
{ // set correct initial position for each blocks
  for (int i = 0; i < 8; i++)
  {
    for (int j = 0; j < 8; j++)
    {
      blocks[i][j].x_pos = X_FIRST_BLOCK + (11 * i);
      blocks[i][j].y_pos = Y_FIRST_BLOCK + (11 * j);
    }
  }
}

void printBlocks()
{
  tft.drawRect(X_FIRST_BLOCK - 3, Y_FIRST_BLOCK - 3, 92, 92, TFT_WHITE);
  for (int i = 0; i < 8; i++)
  {
    for (int j = 0; j < 8; j++)
    {
      if (blocks[i][j].alive)
      {
        tft.fillRect(blocks[i][j].x_pos, blocks[i][j].y_pos, 9, 9, TFT_WHITE);
      }
      else
      {
        tft.drawRect(blocks[i][j].x_pos, blocks[i][j].y_pos, 9, 9, TFT_WHITE);
      }
    }
  }
}

void setBlockStatus()
{
  for (int i = 0; i < 8; i++)
  {
    for (int j = 0; j < 8; j++)
    {
      if (blocks[i][j].alive)
      {
        if (blocks[i][j].neighbour_num < 2)
        {
          blocks[i][j].alive = false;
          cellNum--;
        }
        else if (blocks[i][j].neighbour_num > 3)
        {
          blocks[i][j].alive = false;
          cellNum--;
        }
      }
      else if (!blocks[i][j].alive)
      {
        if (blocks[i][j].neighbour_num == 3)
        {
          blocks[i][j].alive = true;
          cellNum++;
        }
      }
    }
  }
}

void findNeighbour()
{
  for (int i = 0; i < 8; i++)
  {
    for (int j = 0; j < 8; j++)
    {
      blocks[i][j].neighbour_num = 0; // clear the number of neighbour
      int neighbourNum = 0;

      if (((i - 1) > 0) && ((j - 1) > 0))
      {
        if (blocks[i - 1][j - 1].alive)
        {
          neighbourNum++;
        }
      }
      if ((i - 1) > 0)
      {
        if (blocks[i - 1][j].alive)
        {
          neighbourNum++;
        }
      }
      if (((i - 1) > 0) && ((j + 1) < 8))
      {
        if (blocks[i - 1][j + 1].alive)
        {
          neighbourNum++;
        }
      }
      if ((j - 1) > 0)
      {
        if (blocks[i][j - 1].alive)
        {
          neighbourNum++;
        }
      }
      if ((j + 1) < 8)
      {
        if (blocks[i][j + 1].alive)
        {
          neighbourNum++;
        }
      }
      if (((i + 1) < 8) && ((j - 1) > 0))
      {
        if (blocks[i + 1][j - 1].alive)
        {
          neighbourNum++;
        }
      }
      if ((i + 1) < 8)
      {
        if (blocks[i + 1][j].alive)
        {
          neighbourNum++;
        }
      }
      if (((i + 1) < 8) && ((j + 1) < 8))
      {
        if (blocks[i + 1][j + 1].alive)
        {
          neighbourNum++;
        }
      }

      blocks[i][j].neighbour_num = neighbourNum;
    }
  }
}

void debugPrintNeighbour()
{
  for (int i = 0; i < 8; i++)
  {
    for (int j = 0; j < 8; j++)
    {
      Serial.print(" ");
      Serial.print(blocks[i][j].neighbour_num);
      Serial.print(" ");
    }
    Serial.println();
  }
  Serial.println();
}

void printGridInfo()
{
  tft.setTextFont(2);
  tft.setTextColor(TFT_WHITE);
  tft.drawString("Genation:", 138, 100);
  tft.drawString("Cell Number:", 138, 115);
  tft.setCursor(198, 100);
  if (generation > 9999)
  {
    tft.print("9999+");
  }
  else
  {
    tft.print(generation);
  }

  tft.setCursor(218, 115);
  tft.print(cellNum);
}

void printMenu()
{
  tft.setTextFont(2);
  if (menu)
  {
    tft.setTextColor(tft.color565(56, 178, 92), TFT_WHITE);
    tft.drawString("Run/Pause", 10, 10);
    tft.setTextColor(TFT_WHITE, tft.color565(56, 178, 92));
    tft.drawString("Edit Mode", 10, 30);
  }
  else
  {
    tft.setTextColor(TFT_WHITE, tft.color565(56, 178, 92));
    tft.drawString("Run/Pause", 10, 10);
    tft.setTextColor(tft.color565(56, 178, 92), TFT_WHITE);
    tft.drawString("Edit Mode", 10, 30);
    if(editMode){
      tft.fillCircle(85, 37, 5, TFT_WHITE);
    }
  }
  if (running)
  {
    tft.fillRect(80, 10, 5, 15, TFT_WHITE); // draw the left vertical bar
    tft.fillRect(90, 10, 5, 15, TFT_WHITE);
  }
  else
  {
    tft.fillTriangle(80, 10, 80, 25, 90, 17, TFT_WHITE); // draw a triangle
  }
}

void setInitCellNum()
{
  for (int i = 0; i < 8; i++)
  {
    for (int j = 0; j < 8; j++)
    {
      if (blocks[i][j].alive)
      {
        cellNum++;
      }
    }
  }
}

void IRAM_ATTR ISR_SWITCH_ONE()
{
  if (millis() - debouncingTime > debouncingFlagSwitchA)
  {
    debouncingFlagSwitchA = millis();
    Serial.println("Pressed!");
    if (menu)
    {
      running = !running;
    }
    else
    {
      editMode = !editMode;
    }
  }
}

void IRAM_ATTR ISR_ENCODER_ONE()
{
  readQuadrature(true);
}
void IRAM_ATTR ISR_ENCODER_TWO()
{
  readQuadrature(false);
}
void IRAM_ATTR readQuadrature(bool A_B)
{
  int pin_A;
  int pin_B;
  // u8g2.setPowerSave(1);
  // lcdStandbyTime = millis();
  // menuStandbyTime = millis();
  if (A_B)
  {
    pin_A = ENCODER_ONE_A;
    pin_B = ENCODER_ONE_B;
  }
  else
  {
    pin_A = ENCODER_TWO_A;
    pin_B = ENCODER_TWO_B;
  }
  if (int_nu == 0 && digitalRead(pin_A) == LOW)
  {
    flag = 0;
    if (digitalRead(pin_B))
    {
      flag = 1;
    }
    int_nu = 1;
  }
  if (int_nu && digitalRead(pin_A))
  {
    if (digitalRead(pin_B) == LOW && flag == 1)
    {
      if (A_B)
      {
        if (!editMode)
        {
          menu = !menu;
        }
      }
    }
    if (digitalRead(pin_B) && flag == 0)
    {
      if (A_B)
      {
        if (!editMode)
        {
          menu = !menu;
        }
      }
    }
    int_nu = 0;
  }
}

void setBlinker()
{
  blocks[3][1].alive = true;
  blocks[3][2].alive = true;
  blocks[3][3].alive = true;
}
