/**
 * @file sudoku.h
 * @brief 数独相关函数的声明
 * @author [Hus-King]
 * @date 2025-09-03
 * @version 1.0
 */

#ifndef SUDOKU_H
#define SUDOKU_H

#include "cnf.h"
#include "optimize_cnf.h"

// ==================== 数独相关函数 ====================

// 百分号数独特殊窗口位置定义
extern int upperWindow[9][2];
extern int lowerWindow[9][2];

/**
 * @brief 检查在数独网格指定位置放置数字是否合法
 * @param grid 数独网格
 * @param row 行号（0-8）
 * @param col 列号（0-8）
 * @param num 要放置的数字（1-9）
 * @return 合法返回true，否则返回false
 */
bool isSafe(int grid[N][N], int row, int col, int num);

/**
 * @brief 使用回溯算法填充完整的数独网格
 * @param grid 要填充的数独网格
 * @param row 当前填充的行号，默认从0开始
 * @param col 当前填充的列号，默认从0开始
 * @return 填充成功返回true，失败返回false
 */
bool fillGrid(int grid[N][N], int row = 0, int col = 0);

/**
 * @brief 从完整数独中生成谜题，确保解的唯一性
 * @param full 完整的数独解
 * @param puzzle 生成的数独谜题（输出参数）
 * @param clues 要保留的提示数字个数
 */
void generatePuzzle(int full[N][N], int puzzle[N][N], int clues);

/**
 * @brief 打印数独网格到控制台
 * @param grid 要打印的数独网格
 */
void printSudoku(int grid[N][N]);

/**
 * @brief 将数独坐标和数字转换为SAT变量索引
 * @param row 行号（0-8）
 * @param col 列号（0-8）
 * @param num 数字（1-9）
 * @return SAT变量索引
 */
int varIndex(int row, int col, int num);

/**
 * @brief 设置数独求解的全局变量（boolCount和clauseCount）
 */
void setSudokuGlobals();

/**
 * @brief 将数独谜题转换为CNF公式
 * @param puzzle 数独谜题
 * @param cnf 输出的CNF公式（输出参数）
 */
void sudokuToCNF(int puzzle[N][N], SATList*& cnf);

/**
 * @brief 检查用户输入的数独解是否正确
 * @param solved 正确的数独解
 * @param ans 用户输入的答案
 * @return 正确返回true，错误返回false
 */
bool check(int solved[9][9], int ans[9][9]);
/**
 * @brief 计算数独谜题的解的数量
 * @param puzzle 数独谜题，0表示空格
 * @param maxSolutions 最大解数限制，默认为2
 * @return 解的数量（最多计算到maxSolutions个）
 * @note 该函数使用手写栈实现的搜索算法，避免了递归调用可能导致的栈溢出
 */
int countSolutions(int puzzle[N][N], int maxSolutions = 2);

#endif // SUDOKU_H
