/*
  Project: ESP32 Snake Game
  Created by: ProtoSam IoT
  GitHub: https://github.com/ProtoSamIoT
  YouTube: https://www.youtube.com/@protosamiot

  Classic Snake game using ESP32, Joystick module, and OLED display.
  Star ⭐ the repo and subscribe for more ESP32 & IoT projects.
*/
#include <Arduino.h>
#include <U8g2lib.h>

/* ---------- OLED ---------- */
U8G2_SH1106_128X64_NONAME_F_HW_I2C oled(U8G2_R0, U8X8_PIN_NONE);

/* ---------- JOYSTICK ---------- */
#define JOY_X 34
#define JOY_Y 35
#define JOY_SW 32

/* ---------- BUZZER ---------- */
#define BUZZER 25

/* ---------- GAME STATES ---------- */
#define GAME_START 0
#define GAME_PLAY  1
#define GAME_OVER  2

int gameState = GAME_START;

/* ---------- SNAKE VARIABLES ---------- */
#define MAX_LEN 64
struct Point { int x, y; };
Point snake[MAX_LEN];
int snakeLen = 3;
int dirX = 1, dirY = 0; // start moving right
Point apple;
unsigned long lastMove;
int moveDelay = 200; // ms
int score = 0;

/* ---------- SETUP ---------- */
void setup() {
  pinMode(JOY_SW, INPUT_PULLUP);
  pinMode(BUZZER, OUTPUT);

  oled.begin();
  oled.setFont(u8g2_font_6x10_tf);

  randomSeed(analogRead(0));
  spawnApple();
}

/* ---------- LOOP ---------- */
void loop() {
  unsigned long now = millis();
  int joyX = analogRead(JOY_X);
  int joyY = analogRead(JOY_Y);
  bool btn = digitalRead(JOY_SW) == LOW;

  if (gameState == GAME_START) {
    drawStart();
    if (btn) resetGame();
  }
  else if (gameState == GAME_PLAY) {
    readJoystick(joyX, joyY);
    if (now - lastMove > moveDelay) {
      moveSnake();
      lastMove = now;
    }
    drawGame();
  }
  else if (gameState == GAME_OVER) {
    drawGameOver();
    if (btn) resetGame();
  }
}

/* ---------- GAME FUNCTIONS ---------- */
void readJoystick(int x, int y) {
  if (x < 1200 && dirX != 1) { dirX = -1; dirY = 0; }
  else if (x > 3000 && dirX != -1) { dirX = 1; dirY = 0; }
  else if (y < 1200 && dirY != 1) { dirX = 0; dirY = -1; }
  else if (y > 3000 && dirY != -1) { dirX = 0; dirY = 1; }
}

void moveSnake() {
  // Move body
  for (int i = snakeLen - 1; i > 0; i--) {
    snake[i] = snake[i - 1];
  }
  // Move head
  snake[0].x += dirX * 4;
  snake[0].y += dirY * 4;

  // Check collisions with walls
  if (snake[0].x < 0 || snake[0].x >= 128 || snake[0].y < 0 || snake[0].y >= 64) {
    gameState = GAME_OVER;
    tone(BUZZER, 300, 300);
    return;
  }

  // Check collision with self
  for (int i = 1; i < snakeLen; i++) {
    if (snake[0].x == snake[i].x && snake[0].y == snake[i].y) {
      gameState = GAME_OVER;
      tone(BUZZER, 300, 300);
      return;
    }
  }

  // Check apple
  if (snake[0].x == apple.x && snake[0].y == apple.y) {
    if (snakeLen < MAX_LEN) snakeLen++;
    score++;
    tone(BUZZER, 1000, 100);
    spawnApple();
  }
}

void spawnApple() {
  apple.x = (random(0, 32)) * 4; // grid 4x4
  apple.y = (random(0, 16)) * 4;
}

void drawGame() {
  oled.clearBuffer();

  // Draw snake
  for (int i = 0; i < snakeLen; i++) {
    oled.drawBox(snake[i].x, snake[i].y, 4, 4);
  }

  // Draw apple
  oled.drawBox(apple.x, apple.y, 4, 4);

  // Draw score
  oled.setCursor(90, 10);
  oled.print("S:");
  oled.print(score);

  oled.sendBuffer();
}

void drawStart() {
  oled.clearBuffer();
  oled.drawStr(0, 20, "Snake Game");
  oled.drawStr(0, 40, "Push joystick to start");
  oled.sendBuffer();
}

void drawGameOver() {
  oled.clearBuffer();
  oled.drawStr(30, 20, "GAME OVER");
  oled.setCursor(35, 40);
  oled.print("Score:");
  oled.print(score);
  oled.sendBuffer();
}

void resetGame() {
  gameState = GAME_PLAY;
  snakeLen = 3;
  snake[0] = {32, 32};
  snake[1] = {28, 32};
  snake[2] = {24, 32};
  dirX = 1; dirY = 0;
  score = 0;
  spawnApple();
}