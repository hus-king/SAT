/**
 * @file common.cpp
 * @brief 通用函数的实现
 * @author [Hus-King]
 * @date 2025-09-03
 * @version 1.0
 */

#include "../include/common.h"

// ==================== 全局变量定义 ====================
int boolCount = 0;              ///< CNF公式中的布尔变量数量
int clauseCount = 0;            ///< CNF公式中的子句数量
char fileName[100] = "";        ///< 待处理的CNF文件名

// ==================== 跨平台函数实现 ====================

/**
 * @brief 跨平台清屏函数
 * @details 根据不同操作系统使用相应的清屏命令
 */
void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");  // Unix/Linux/macOS
#endif
}

/**
 * @brief 暂停程序等待用户输入
 * @details 跨平台的暂停函数
 */
void pauseProgram() {
    std::cout << "按回车键继续...";
    std::cin.ignore();
    std::cin.get();
}
