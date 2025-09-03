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
 * @brief 寻找CNF公式中的单子句
 * @param cnf CNF公式链表
 * @return 找到单子句返回指向该文字的指针，否则返回nullptr
 */
SATNode* unitClause(SATList* cnf);

/**
 * @brief 支持早期停止的DPLL算法
 * @param cnf CNF公式链表
 * @param value 变量赋值数组
 * @return 1表示SAT，0表示UNSAT，-1表示被中断
 */
int DPLL_WithEarlyStop(SATList*& cnf, int value[]);

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

// ==================== 高效数据结构定义 ====================

/**
 * @brief 变量状态枚举
 */
enum VarState {
    VAR_UNASSIGNED = -1,
    VAR_FALSE = 0,
    VAR_TRUE = 1
};

/**
 * @brief 回溯栈项目，记录每次赋值操作
 */
struct TrailItem {
    int var;                    ///< 被赋值的变量
    VarState old_value;         ///< 变量的旧值
    int decision_level;         ///< 决策层级
    bool is_decision;           ///< 是否为决策变量（非传播）
};

/**
 * @brief 高效CNF公式表示
 */
class FastCNF {
public:
    std::vector<std::vector<int>> clauses;      ///< 子句集合，每个子句是文字数组
    std::vector<VarState> assignment;           ///< 变量赋值状态
    std::vector<TrailItem> trail;               ///< 回溯栈
    std::vector<bool> clause_satisfied;         ///< 子句是否已满足
    std::vector<int> clause_watch_count;        ///< 每个子句的未满足文字数
    int num_vars;                               ///< 变量总数
    int decision_level;                         ///< 当前决策层级
    
    FastCNF(int vars = 0) : num_vars(vars), decision_level(0) {
        assignment.resize(vars + 1, VAR_UNASSIGNED);
    }
    
    /**
     * @brief 从传统链表结构转换
     */
    void fromSATList(SATList* cnf);
    
    /**
     * @brief 转换回传统链表结构（兼容性）
     */
    SATList* toSATList() const;
    
    /**
     * @brief 深拷贝（用于并行）
     */
    FastCNF copy() const;
    
    /**
     * @brief 检查是否有空子句
     */
    bool hasEmptyClause() const;
    
    /**
     * @brief 检查是否所有子句都满足
     */
    bool allClausesSatisfied() const;
    
    /**
     * @brief 单子句传播
     */
    bool unitPropagate();
    
    /**
     * @brief 赋值变量并记录到trail
     */
    void assign(int var, VarState value, bool is_decision = false);
    
    /**
     * @brief 回溯到指定层级
     */
    void backtrack(int level);
    
    /**
     * @brief 选择下一个分支变量
     */
    int chooseBranchVariable() const;
    
private:
    void updateClauseStatus();
};

/**
 * @brief 高效双核DPLL求解器
 */
int DPLL_DualCore_Fast(SATList*& cnf, int value[]);

/**
 * @brief 高效DPLL算法实现
 */
bool FastDPLL(FastCNF& cnf);

/**
 * @brief 并行DPLL线程函数
 */
void parallel_dpll_thread(FastCNF cnf_copy, int branch_var, bool branch_value, 
                         std::atomic<bool>& solution_found, 
                         std::atomic<bool>& result_ready,
                         std::vector<VarState>& global_solution,
                         std::mutex& solution_mutex);

#endif // CNF_H