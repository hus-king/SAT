/**
 * @file common.h
 * @brief 通用定义和包含文件
 * @author [Hus-King]
 * @date 2025-09-03
 * @version 1.0
 */

#ifndef COMMON_H
#define COMMON_H

#define _CRT_SECURE_NO_WARNINGS
#include <bits/stdc++.h>
#include <string>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <random>
#include <vector>
#include <algorithm>
using namespace std;

// ==================== 常量定义 ====================
#define N 9                     // 数独网格大小 9x9
#define MAX_VAR 10000          // SAT变量最大数量，根据实际需求调整

// ==================== 全局变量 ====================
extern int boolCount;           // CNF公式中的布尔变量数量
extern int clauseCount;         // CNF公式中的子句数量
extern char fileName[100];      // 待处理的CNF文件名

// ==================== 辅助函数 ====================

/**
 * @brief 跨平台清屏函数
 * @details 根据不同操作系统使用相应的清屏命令
 */
void clearScreen();

/**
 * @brief 暂停程序等待用户输入
 * @details 跨平台的暂停函数
 */
void pauseProgram();

#endif // COMMON_H
