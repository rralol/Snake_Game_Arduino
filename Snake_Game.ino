#define ROW_1 A0
#define ROW_2 A1
#define ROW_3 A2
#define ROW_4 A3
#define ROW_5 2
#define ROW_6 3
#define ROW_7 4
#define ROW_8 5

#define COL_1 6
#define COL_2 7
#define COL_3 8
#define COL_4 9
#define COL_5 10 
#define COL_6 11 
#define COL_7 12 
#define COL_8 13 
#define joystick_default_x 518
#define joystick_default_y 498

#define up 1
#define down 2
#define left 4
#define right 3
#define sensitivity 20


typedef struct gameObject
{
  uint8_t x, y;
  struct gameObject * next;
} gameObject;

uint8_t dir = right; 
bool dead = false;

const byte rows[] = {
  ROW_1, ROW_2, ROW_3, ROW_4, ROW_5, ROW_6, ROW_7, ROW_8 
};

const byte col[] {
  COL_1, COL_2, COL_3, COL_4, COL_5, COL_6, COL_7, COL_8
};

unsigned int frameCounter = 0;


byte frameBuffer[] = {
  B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000
};

gameObject * pHead = NULL;
gameObject * pTail = NULL;
gameObject * apple = NULL;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  randomSeed(255);
  for (byte i = 2; i <= 13; i++)
  {
    pinMode(i, OUTPUT);
  }
  pinMode(A0, OUTPUT);
  pinMode(A1, OUTPUT);
  pinMode(A2, OUTPUT);
  pinMode(A3, OUTPUT);
  pinMode(A4, INPUT);
  pinMode(A5, INPUT);
}

void spawnApple()
{
  Serial.println("Spawning apple");
  apple->x = random(1,8);
  apple->y = random(1,8);
  if (checkCollision(apple->x, apple->y))
    spawnApple();
}


bool checkCollision(const uint8_t x, const uint8_t y)
{
  gameObject *tempPtr = pTail;
  while(tempPtr != NULL)
  {
    Serial.println(int(tempPtr));
    if(x==tempPtr->x && y==tempPtr->y)
    {
      Serial.println("Collision detected!");
      return true;
    }
    tempPtr = tempPtr->next;
  }
  Serial.println("NO Collision detected!");
  return false;
}

void initGame()
{
  Serial.println("Init game");
  uint8_t x = 3;
  for (int i = 0; i < 2; i++, x++)
  {
    addHead(x, 4);
    if (i == 0)
      pTail = pHead;
  }
  apple = (gameObject*) malloc(sizeof(gameObject));
  apple->next = NULL;
  spawnApple();
  dir = right;
  Serial.println("Init game DONE");
}

void addHead(const uint8_t x, const uint8_t y)
{
  Serial.println("Adding new snake part");
  gameObject *temp;
  temp = (gameObject*) malloc (sizeof(gameObject));
  //Check allocation
  if (temp != NULL)
  {
     temp->x = x;
     temp->y = y;
     temp->next = NULL;
     if (pHead != NULL)
      pHead->next = temp;
     pHead = temp;
  }
  else
  {
    addHead(x,y);
  }
}

void updatePlayer()
{
  Serial.println("Updating player");
  uint8_t new_X = pHead->x;
  uint8_t new_Y = pHead->y;
  if (dir==1)
    new_Y++;
  if (dir==2)
    new_Y--;
  if (dir==3)
    new_X++;
  if (dir==4)
    new_X--;

  if(new_Y > 8)
    new_Y = 1;
  if(new_Y < 1)
    new_Y = 8;
  if(new_X > 8)
    new_X = 1;
  if(new_X < 1)
    new_X = 8;
  Serial.println("Checkdead");
  dead |= checkCollision(new_X, new_Y);

  if (!dead)
  {
    Serial.println("Not dead");
    if(new_X == apple->x && new_Y == apple->y)
    {
      Serial.println("Player reached apple");
      addHead(new_X, new_Y);
      spawnApple();
    }
    else
    {
      Serial.println("Forward player");
      gameObject *tempPtr = pTail;
      pTail =  pTail->next;
      pHead->next = tempPtr;
      pHead = tempPtr;
      pHead->x = new_X;
      pHead->y = new_Y;
      pHead->next = NULL;
    }
  }
}

void clearFramebuffer()
{
  for(int i = 0; i < 8; i++)
  {
    frameBuffer[i] = B00000000;
  }
}

void writeToFrameBuffer()
{
  
  clearFramebuffer();
  gameObject *tempPtr = pTail;
  //Write snake coordinates to framebuffer
  while(tempPtr != NULL)
  {
    frameBuffer[tempPtr->y-1] = frameBuffer[tempPtr->y-1] | (1 << tempPtr->x-1);
    tempPtr = tempPtr->next;
  }
  //write apple to frambuffer
  frameBuffer[apple->y-1] = frameBuffer[apple->y-1] | (1 << apple->x-1);
}

void readPlayerInput()
{
  //Read current joystick values.
  int16_t joystick_y = analogRead(A4);
  int16_t joystick_x = analogRead(A5);
  //Check for highest deviation from default values to decide action.
  if ((abs(joystick_default_y - joystick_y) > abs(joystick_default_x - joystick_x)) & abs(joystick_default_y - joystick_y) > sensitivity)
  {
      if (joystick_y > joystick_default_y)
      {
        dir = down;
      }
      else
      {
        dir = up;
      }
  }
  else if (abs(joystick_default_x - joystick_x) > sensitivity)
  {
    if (joystick_x > joystick_default_x)
    {
      dir = left;
    }
    else
    {
      dir = right;
    }
  }
}

void loop() {
  initGame();
  while (!dead)
  {
    if (frameCounter%15 == 1)
    {
      readPlayerInput();
      updatePlayer();
      writeToFrameBuffer();
    }
    drawScreen(frameBuffer);
    frameCounter++;  
  }
  //Deallocate snake and apple
  gameObject *tempPtr = pTail;
  while(tempPtr != NULL)
  {
    tempPtr = pTail->next;
    free(pTail);
    pTail = tempPtr;
  }
  free(apple);
  apple = NULL;
  pHead = NULL;
  pTail = NULL;
  dead = false;
}
void drawScreen(byte drawBuffer[])
{
  for(byte i = 0; i < 8; i++)
  {
    //Draw row
    digitalWrite(rows[i], HIGH);
    for (byte a = 0; a < 8; a++)
    {
      digitalWrite(col[a], (drawBuffer[i] >> a) & 0x01);
    }
    delay(2);   
    //Reset LEDs before drawing next row   
    digitalWrite(rows[i], LOW);
    for (byte a = 0; a < 8; a++)
    {
      digitalWrite(col[a], 0);
    }
  }
}
