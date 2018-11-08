#pragma once

#include "GLUT.h"
#include <math.h>

#include <stdio.h>
#include <time.h>

const int W = 600; // window width
const int H = 600; // window height

const int SCRSZ = 100;
const int INPUT_SZ = SCRSZ + 1;
const int HIDDEN_SZ = INPUT_SZ / 2 + 1;
const int OUTPUT_SZ = 10;

void init();
void DrawSquares();
void Digitize();
int MaxOutput();
void Clean();
void FeedForward();
void Backpropagation();
void display();
void idle();
void drag(int x, int y);
void mouse(int button, int state, int x, int y);