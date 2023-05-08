#include <Arduino.h>
#include <TFT_eSPI.h>

#define X_FIRST_BLOCK 140
#define Y_FIRST_BLOCK 10
#define ENCODER_ONE_A 25
#define ENCODER_ONE_B 26
#define ENCODER_ONE_SWITCH 27
#define ENCODER_TWO_A 36
#define ENCODER_TWO_B 37
#define ENCODER_TWO_SWITCH 32

void setupScreen();
void initBlocks();
void printBlocks();
void setBlockStatus();
void findNeighbour();
void setBlinker();
void debugPrintNeighbour();
void printGridInfo();
void findCellNum();
void printMenu();
void blinkSelectedBlock();

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

int flag_1_encoder = 0;
int flag_2_encoder = 0;

int row = 0;
int column = 0;
bool menu = true;
bool editMode = false;
bool running = false;
bool blinkFlag = false;

long debouncingDelay = 500;
volatile long debouncingFlagSwitchA = 0;
volatile long debouncingFlagSwitchB = 0;
long runningDelay = 1000;
long runningDelayFlag = 0;
long runningDelayLevel = 0;

void setup()
{
  Serial.begin(921600);
  Serial.println("");
  Serial.println("Start!");

  pinMode(ENCODER_ONE_A, INPUT);
  pinMode(ENCODER_ONE_B, INPUT);
  pinMode(ENCODER_ONE_SWITCH, INPUT_PULLUP);
  pinMode(ENCODER_TWO_A, INPUT);
  pinMode(ENCODER_TWO_B, INPUT);
  pinMode(ENCODER_TWO_SWITCH, INPUT_PULLUP);

  attachInterrupt(ENCODER_ONE_A, ISR_ENCODER_ONE, CHANGE);
  attachInterrupt(ENCODER_ONE_SWITCH, ISR_SWITCH_ONE, FALLING);

  attachInterrupt(ENCODER_TWO_A, ISR_ENCODER_TWO, CHANGE);
  attachInterrupt(ENCODER_TWO_SWITCH, ISR_SWITCH_TWO, FALLING);

  setupScreen();
  initBlocks();
  // setBlinker();
  printBlocks();
  findCellNum();
  printMenu();
  delay(1000);
}

void loop()
{
  tft.fillScreen(tft.color565(56, 178, 92)); // 0x38b25c
  printMenu();

  if ((running) && (millis() - runningDelay > runningDelayFlag))
  {
    runningDelayFlag = millis();
    findNeighbour();
    setBlockStatus();
    debugPrintNeighbour();
    generation++;
  }
  printBlocks();
  if (editMode)
  {
    blinkSelectedBlock();
  }
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

      if ((i - 1 >= 0) && (j - 1 >= 0))
      {
        if (blocks[i - 1][j - 1].alive)
        {
          neighbourNum++;
        }
      }
      if (i - 1 >= 0)
      {
        if (blocks[i - 1][j].alive)
        {
          neighbourNum++;
        }
      }
      if ((i - 1 >= 0) && (j + 1 < 8))
      {
        if (blocks[i - 1][j + 1].alive)
        {
          neighbourNum++;
        }
      }
      if (j - 1 >= 0)
      {
        if (blocks[i][j - 1].alive)
        {
          neighbourNum++;
        }
      }
      if (j + 1 < 8)
      {
        if (blocks[i][j + 1].alive)
        {
          neighbourNum++;
        }
      }
      if ((i + 1 < 8) && (j - 1 >= 0))
      {
        if (blocks[i + 1][j - 1].alive)
        {
          neighbourNum++;
        }
      }
      if (i + 1 < 8)
      {
        if (blocks[i + 1][j].alive)
        {
          neighbourNum++;
        }
      }
      if ((i + 1 < 8) && (j + 1 < 8))
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
  tft.setCursor(196, 100);
  if (generation > 9999)
  {
    tft.print("9999+");
  }
  else
  {
    tft.print(generation);
  }

  tft.setCursor(216, 115);
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
    if (editMode)
    {
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

void blinkSelectedBlock()
{
  if (blinkFlag)
  {
    tft.fillRect(blocks[row][column].x_pos, blocks[row][column].y_pos, 9, 9, TFT_WHITE);
  }
  else
  {
    tft.drawRect(blocks[row][column].x_pos, blocks[row][column].y_pos, 9, 9, TFT_WHITE);
    tft.fillRect(blocks[row][column].x_pos + 1, blocks[row][column].y_pos + 1, 7, 7, tft.color565(56, 178, 92));
    // drawRect can only draw hollow rectangles, if we try to draw a hollow rectangle above the other solid rectangle that has the same color,
    // then we need to use a smaller solid rectangle to fill the hollow part with different color
  }
  blinkFlag = !blinkFlag;
}

void findCellNum()
{
  cellNum = 0;
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
  if (millis() - debouncingDelay > debouncingFlagSwitchA)
  {
    debouncingFlagSwitchA = millis();
    if (menu)
    {
      running = !running;
    }
    else
    {
      editMode = !editMode;
      running = false; // pause the game when entering the edit mode
    }
  }
}

void IRAM_ATTR ISR_SWITCH_TWO()
{
  if (millis() - debouncingDelay > debouncingFlagSwitchB)
  {
  if (editMode)
  {
    blocks[row][column].alive = !blocks[row][column].alive;
    findCellNum();
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

  if (A_B) // to find out which pin to read
  {
    pin_A = ENCODER_ONE_A;
    pin_B = ENCODER_ONE_B;
  }
  else
  {
    pin_A = ENCODER_TWO_A;
    pin_B = ENCODER_TWO_B;
  }

  if (flag_1_encoder == 0 && digitalRead(pin_A) == LOW)
  {
    flag_2_encoder = 0;
    if (digitalRead(pin_B))
    {
      flag_2_encoder = 1;
    }
    flag_1_encoder = 1;
  }
  if (flag_1_encoder && digitalRead(pin_A))
  {
    if (digitalRead(pin_B) == LOW && flag_2_encoder == 1)
    {
      if (A_B)
      {
        if (!editMode)
        {
          menu = !menu;
        }
        else
        {
          if (row == 8)
          {
            row = 8;
          }
          else
          {
            row++;
          }
        }
      }
      else
      {
        if (editMode)
        {
          if (column == 8)
          {
            column = 8;
          }
          else
          {
            column++;
          }
        }
      }
    }
    if (digitalRead(pin_B) && flag_2_encoder == 0)
    {
      if (A_B)
      {
        if (!editMode)
        {
          menu = !menu;
        }
        else
        {
          if (editMode)
          {
            if (row == 0)
            {
              row = 0;
            }
            else
            {
              row--;
            }
          }
        }
      }
      else
      {
        Serial.println(column);
        if (column == 0)
        {
          column = 0;
        }
        else
        {
          column--;
        }
      }
    }
    flag_1_encoder = 0;
  }
}

void setBlinker()
{
  blocks[3][1].alive = true;
  blocks[3][2].alive = true;
  blocks[3][3].alive = true;
}
