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
    std::vector<bool> assignment;               ///< 变量赋值：true为真，false为假
    std::vector<bool> is_assigned;              ///< 变量是否已赋值：true已赋值，false未赋值
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
    std::vector<int> getAssignment() const { 
        std::vector<int> result(num_vars + 1, -1);
        for (int i = 1; i <= num_vars; i++) {
            if (is_assigned[i]) {
                result[i] = assignment[i] ? 1 : 0;
            }
        }
        return result;
    }
    
    /**
     * @brief 设置变量赋值
     */
    void setAssignment(int var, int value) { 
        if (value == -1) {
            is_assigned[var] = false;
        } else {
            is_assigned[var] = true;
            assignment[var] = (value == 1);
        }
    }
    
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
    /**
     * @brief 赋值变更记录，用于增量回溯
     */
    struct AssignmentChange {
        int var;                ///< 变更的变量
        bool old_value;         ///< 原始值
        bool was_assigned;      ///< 原来是否已赋值
    };
    
    OptimizedCNF cnf;
    std::vector<int> pos_count;                 ///< 正文字计数
    std::vector<int> neg_count;                 ///< 负文字计数
    std::vector<AssignmentChange> undo_stack;   ///< 回溯栈
    
    // 预处理优化数据结构
    std::vector<std::vector<std::pair<int, int>>> var_to_clauses; ///< 变量到子句的映射 {clause_idx, literal}
    
    // VSIDS启发式数据结构
    std::vector<double> activity;               ///< 变量活跃度
    std::vector<int> order_heap;                ///< 按活跃度排序的变量堆
    double activity_inc;                        ///< 活跃度增量
    double decay_factor;                        ///< 衰减因子
    int decision_count;                         ///< 决策计数器
    
    // Two-Watched Literals 数据结构
    std::vector<std::vector<int>> watches;      ///< 每个文字对应的watched子句列表
    std::vector<std::pair<int, int>> clause_watched; ///< 每个子句的两个watched文字索引
    
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
     * @brief 构建变量到子句的映射
     */
    void buildVarClauseMapping();
    
    /**
     * @brief 提升变量活跃度
     */
    void bumpActivity(int var);
    
    /**
     * @brief 衰减所有变量活跃度
     */
    void decayActivity();
    
    /**
     * @brief VSIDS变量选择启发式
     */
    int selectVariableVSIDS();
    
    /**
     * @brief MOM启发式变量选择（Maximum Occurrences in clauses of Minimum size）
     */
    int selectVariableMOM();
    
    /**
     * @brief 在冲突时提升相关变量的活跃度
     */
    void handleConflict(const std::vector<int>& conflict_clause);
    
    /**
     * @brief 初始化Two-Watched Literals数据结构
     */
    void initWatchedLiterals();
    
    /**
     * @brief 文字到索引的映射（处理正负文字）
     */
    int literalToIndex(int literal) const;
    
    /**
     * @brief 基于Two-Watched Literals的传播
     */
    bool propagateWatched(int var, bool value);
    
    /**
     * @brief 更新某个子句的watched literal
     */
    bool updateWatch(int clause_idx, int old_watch_literal);
    
    /**
     * @brief 记录变量赋值变更，用于回溯
     */
    void pushAssignment(int var, bool value);
    
    /**
     * @brief 回溯到指定层级
     */
    void backtrack(size_t target_level);
    
    /**
     * @brief 获取当前决策层级
     */
    size_t getCurrentLevel() const { return undo_stack.size(); }
    
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
     * @brief 记录变量赋值并触发Two-Watched Literals传播
     */
    bool pushAssignmentWithPropagation(int var, bool value);
    
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
