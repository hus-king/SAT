/**
 * @file dualcore_cnf.h
 * @brief 双核并行DPLL求解器头文件
 * @author [Hus-King]
 * @date 2025-09-04
 * @version 1.0
 */

#ifndef DUALCORE_CNF_H
#define DUALCORE_CNF_H

#include "common.h"
#include "cnf.h"
#include "optimize_cnf.h"
#include <future>
#include <atomic>
#include <thread>
#include <vector>
#include <chrono>

/**
 * @brief 双核并行DPLL算法接口函数
 * 
 * 这是对外提供的主要接口，使用双核并行技术求解SAT问题
 * 优化特性：
 * - 真正的并行竞争机制
 * - 避免双重求解
 * - 快速变量选择
 * - 线程间原子同步
 * 
 * @param cnf CNF公式链表（引用传递）
 * @param value 变量赋值数组，用于存储求解结果
 * @return 1表示可满足，0表示不可满足
 */
int DPLL_DualCore(SATList*& cnf, int value[]);

/**
 * @brief 选择最优分支变量
 * 
 * 使用启发式方法选择最适合并行分支的变量
 * 
 * @param cnf CNF公式链表
 * @return 分支变量编号
 */
int selectBestSplitVariable(SATList* cnf);

/**
 * @brief 估算分支复杂度
 * 
 * 评估每个分支的预期计算复杂度，用于负载均衡
 * 
 * @param cnf CNF公式链表
 * @param var 变量编号
 * @param assignment 赋值
 * @return 复杂度估算值
 */
double estimateBranchComplexity(SATList* cnf, int var, bool assignment);

#endif // DUALCORE_CNF_H
