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

/**
 * @brief 双核并行求解器类
 * 
 * 该类实现了基于双核的并行DPLL算法，通过将搜索空间分成两个分支
 * 在不同的核心上并行执行，提高求解效率
 */
class DualCoreSolver {
private:
    SATList* original_cnf;                    // 原始CNF公式
    std::atomic<bool> solution_found{false}; // 原子变量，标记是否找到解
    
public:
    /**
     * @brief 构造函数
     * @param cnf CNF公式链表
     */
    DualCoreSolver(SATList* cnf);
    
    /**
     * @brief 双核并行求解主函数
     * @param split_var 分支变量
     * @param value 变量赋值数组
     * @return true表示可满足，false表示不可满足
     */
    bool solve(int split_var, int value[]);
    
private:
    /**
     * @brief 求解单个分支
     * @param var 分支变量
     * @param assignment 变量赋值（true或false）
     * @return true表示该分支可满足，false表示不可满足
     */
    bool solveBranch(int var, bool assignment);
    
    /**
     * @brief 复制解到结果数组
     * @param solution 求解结果向量
     * @param value 目标数组
     */
    void copySolution(const std::vector<int>& solution, int value[]);
};

/**
 * @brief 双核并行DPLL算法接口函数
 * 
 * 这是对外提供的主要接口，使用双核并行技术求解SAT问题
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
