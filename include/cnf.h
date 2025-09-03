/**
 * @file cnf.h
 * @brief CNF公式相关的数据结构和函数声明
 * @author [Hus-King]
 * @date 2025-09-03
 * @version 1.0
 */

#ifndef CNF_H
#define CNF_H

#include "common.h"

// ==================== 数据结构定义 ====================

/**
 * @brief SAT子句中文字节点结构
 * 用于构建单个子句的文字链表
 */
struct SATNode {
    int data;                   // 文字值（正数表示正文字，负数表示负文字）
    SATNode* next;             // 指向下一个文字节点
};

/**
 * @brief SAT子句链表结构
 * 用于构建CNF公式的子句链表
 */
struct SATList {
    SATNode* head;             // 指向子句中第一个文字节点
    SATList* next;             // 指向下一个子句
};
// ==================== 函数声明 ====================

// ---------- SAT求解器核心函数 ----------

/**
 * @brief 从CNF文件读取SAT问题
 * @param cnf 输出参数，存储读取的CNF公式链表
 * @return 成功返回1，失败返回0
 */
int ReadFile(SATList*& cnf);

/**
 * @brief 销毁CNF公式链表，释放内存
 * @param cnf 要销毁的CNF公式链表
 */
void destroyClause(SATList*& cnf);

/**
 * @brief 判断子句是否为单子句（只包含一个文字）
 * @param cnf 待判断的子句
 * @return 是单子句返回1，否则返回0
 */
int isUnitClause(SATNode* cnf);

/**
 * @brief 在给定赋值下计算子句的真值
 * @param cnf 待计算的子句
 * @param v 变量赋值数组
 * @return 子句为真返回1，为假返回0
 */
int evaluateClause(SATNode* cnf, int v[]);

/**
 * @brief 从CNF公式中删除指定子句
 * @param cnf 要删除的子句
 * @param root CNF公式链表的根节点
 * @return 删除成功返回1，失败返回0
 */
int removeClause(SATList*& cnf, SATList*& root);

/**
 * @brief 从子句中删除指定文字节点
 * @param cnf 要删除的文字节点
 * @param head 子句的头节点
 * @return 删除成功返回1，失败返回0
 */
int removeNode(SATNode*& cnf, SATNode*& head);

/**
 * @brief 向CNF公式中添加新子句
 * @param cnf 要添加的子句
 * @param root CNF公式链表的根节点
 * @return 添加成功返回1，失败返回0
 */
int addClause(SATList* cnf, SATList*& root);

/**
 * @brief 检查CNF公式中是否包含空子句
 * @param cnf CNF公式链表
 * @return 包含空子句返回1，否则返回0
 */
int emptyClause(SATList* cnf);
/**
 * @brief DPLL算法主函数，求解SAT问题
 * @param cnf CNF公式链表
 * @param value 变量赋值数组
 * @return 有解返回1，无解返回0
 */
int DPLL(SATList*& cnf, int value[]);

/**
 * @brief DPLL算法双核优化版本
 * @param cnf CNF公式链表
 * @param value 变量赋值数组
 * @return 有解返回1，无解返回0
 * @details 使用两个线程分别尝试第一个分支变量的正负赋值，提高求解效率
 */
int DPLL_DualCore(SATList*& cnf, int value[]);

/**
 * @brief 深拷贝CNF公式链表
 * @param a 目标链表（输出参数）
 * @param b 源链表
 */
void CopyClause(SATList*& a, SATList* b);

/**
 * @brief 将求解结果写入.res文件
 * @param result 求解结果（1表示有解，0表示无解）
 * @param time 求解耗时
 * @param value 变量赋值数组
 * @return 写入成功返回1，失败返回0
 */
int WriteFile(int result, double time, int value[]);

#endif // CNF_H