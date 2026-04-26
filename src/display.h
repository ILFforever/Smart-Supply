#pragma once

void displayUpdate();
void Screen1();
void Screen2();
void Screen3();
void BotBar();
void RotAdjScr();
void GraphRec();
void drawGauge(int value);
void drawGraphGrid(int bars, int sideShift, int topShift);
void drawGraphLine(const int* graphBuffer, int sideShift, int topShift, int calibration);
void drawGraphHeader(const char* title, const char* maxLabel, int topShift);
