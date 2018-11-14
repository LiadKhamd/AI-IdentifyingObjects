#pragma once

#define _CRT_SECURE_NO_WARNINGS

#include "GLUT.h"
#include <math.h>

#include <stdio.h>
#include <time.h>
#include <Windows.h>

const int W = 600; // window width
const int H = 600; // window height

const int TMPSZ = 512;
const int SCRSZ = 256;
const int INPUT_SZ = SCRSZ + 1;//1-bios
const int HIDDEN_SZ = INPUT_SZ / 2 + 1;//1-bios
const int OUTPUT_SZ = 3;
const int ITEMS = 18;
const static char* FLOWERS[OUTPUT_SZ] = { "chrysanthemum", "rose", "tulip"};

void LoadBitmap(char * filename);
void LoadImage(char* name);
void init();
void DrawSquares();
void HPF();
int MaxOutput();
void Clean();
void FeedForward();
void Backpropagation();
void display();
void idle();
void drag(int x, int y);

void mouse(int button, int state, int x, int y);
void startLearning();