/**
 * @file optimize_cnf.h
 * @brief 优化的DPLL求解器头文件
 * @author [Hus-King]
 * @date 2025-09-03
 * @version 2.0
 */

#ifndef OPTIMIZE_CNF_H
#define OPTIMIZE_CNF_H

#include "common.h"
#include "cnf.h"

// ==================== 优化版数据结构 ====================

/**
 * @brief 优化的CNF公式表示
 */
class OptimizedCNF {
private:
    std::vector<std::vector<int>> clauses;      ///< 子句集合
    std::vector<int> assignment;                ///< 变量赋值：-1未赋值，0为假，1为真
    std::vector<bool> clause_satisfied;         ///< 子句是否已满足
    int num_vars;                               ///< 变量总数
    int num_clauses;                            ///< 子句总数
    
public:
    OptimizedCNF(int vars = 0, int clauses_count = 0);
    
    /**
     * @brief 从传统链表结构转换
     */
    void fromSATList(SATList* cnf);
    
    /**
     * @brief 检查是否有空子句
     */
    bool hasEmptyClause() const;
    
    /**
     * @brief 检查是否所有子句都满足
     */
    bool allClausesSatisfied() const;
    
    /**
     * @brief 获取变量赋值
     */
    std::vector<int> getAssignment() const { return assignment; }
    
    /**
     * @brief 设置变量赋值
     */
    void setAssignment(int var, int value) { assignment[var] = value; }
    
    /**
     * @brief 获取变量数量
     */
    int getNumVars() const { return num_vars; }
    
    /**
     * @brief 拷贝构造
     */
    OptimizedCNF(const OptimizedCNF& other);
    
    /**
     * @brief 调试函数：打印CNF状态信息
     */
    void printDebugInfo() const;
    
    friend class OptimizedDPLL;
};

/**
 * @brief 优化的DPLL求解器类
 */
class OptimizedDPLL {
private:
    OptimizedCNF cnf;
    std::vector<int> pos_count;                 ///< 正文字计数
    std::vector<int> neg_count;                 ///< 负文字计数
    
    /**
     * @brief 变量选择启发式（MOM + Jeroslow-Wang）
     */
    int selectVariable();
    
    /**
     * @brief 纯文字消除
     */
    bool pureLiteralElimination();
    
    /**
     * @brief 单子句传播
     */
    bool unitPropagation();
    
    /**
     * @brief 简化公式
     */
    bool simplifyFormula(int literal);
    
    /**
     * @brief 更新子句状态
     */
    void updateClauseStatus();
    
    /**
     * @brief 计算文字出现频率
     */
    void calculateLiteralCounts();
    
    /**
     * @brief DPLL递归求解
     */
    bool dpllRecursive();
    
public:
    OptimizedDPLL(SATList* sat_cnf);
    
    /**
     * @brief 求解SAT问题
     */
    bool solve();
    
    /**
     * @brief 获取解
     */
    std::vector<int> getSolution() const { return cnf.getAssignment(); }
    
    /**
     * @brief 调试信息
     */
    void printStats() const;
};

// ==================== 接口函数 ====================

/**
 * @brief 优化版DPLL求解器接口
 */
int DPLL_Optimized(SATList*& cnf, int value[]);

#endif // OPTIMIZE_CNF_H
