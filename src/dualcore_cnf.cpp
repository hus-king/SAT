/**
 * @file dualcore_cnf.cpp
 * @brief 双核并行DPLL求解器实现
 * @author [Hus-King]
 * @date 2025-09-04
 * @version 1.0
 */

#include "dualcore_cnf.h"

// ==================== DualCoreSolver类实现 ====================

DualCoreSolver::DualCoreSolver(SATList* cnf) : original_cnf(cnf) {}

bool DualCoreSolver::solve(int split_var, int value[]) {
    // 重置解发现标志
    solution_found.store(false);
    
    // 启动两个线程分别计算 var=true 和 var=false
    auto future_true = std::async(std::launch::async, [&]() {
        return solveBranch(split_var, true);
    });
    
    auto future_false = std::async(std::launch::async, [&]() {
        return solveBranch(split_var, false);
    });
    
    // 等待结果 - 谁先完成谁赢
    bool result_true = future_true.get();
    bool result_false = future_false.get();
    
    // 如果true分支找到解
    if (result_true) {
        // 从true分支获取解
        OptimizedDPLL solver_true(original_cnf);
        if (solver_true.pushAssignmentWithPropagation(split_var, true)) {
            if (solver_true.solve()) {
                auto solution = solver_true.getSolution();
                copySolution(solution, value);
                return true;
            }
        }
    }
    
    // 如果false分支找到解
    if (result_false) {
        // 从false分支获取解
        OptimizedDPLL solver_false(original_cnf);
        if (solver_false.pushAssignmentWithPropagation(split_var, false)) {
            if (solver_false.solve()) {
                auto solution = solver_false.getSolution();
                copySolution(solution, value);
                return true;
            }
        }
    }
    
    return false;  // 都没找到解
}

bool DualCoreSolver::solveBranch(int var, bool assignment) {
    try {
        // 为每个分支创建独立的求解器
        OptimizedDPLL solver(original_cnf);
        
        // 设置初始赋值
        if (!solver.pushAssignmentWithPropagation(var, assignment)) {
            return false;  // 初始就冲突
        }
        
        bool result = solver.solve();  // 继续求解
        
        // 如果找到解，设置全局标志
        if (result) {
            solution_found.store(true);
        }
        
        return result;
    } catch (...) {
        return false;
    }
}

void DualCoreSolver::copySolution(const std::vector<int>& solution, int value[]) {
    for (int i = 1; i <= boolCount && i < (int)solution.size(); ++i) {
        value[i] = solution[i];
    }
}

// ==================== 全局函数实现 ====================

int DPLL_DualCore(SATList*& cnf, int value[]) {
    if (cnf == nullptr) return 0;
    
    // 选择最优分支变量
    int split_var = selectBestSplitVariable(cnf);
    
    if (split_var == -1) {
        // 所有变量都已赋值，检查是否满足
        SATList* lp;
        SATNode* tp;
        
        for (lp = cnf; lp != nullptr; lp = lp->next) {
            bool clause_satisfied = false;
            for (tp = lp->head; tp != nullptr; tp = tp->next) {
                int var = abs(tp->data);
                if ((tp->data > 0 && value[var] == 1) || 
                    (tp->data < 0 && value[var] == 0)) {
                    clause_satisfied = true;
                    break;
                }
            }
            if (!clause_satisfied) {
                return 0;  // 有子句不满足
            }
        }
        return 1;  // 所有子句都满足
    }
    
    // 启动双核求解
    DualCoreSolver dual_solver(cnf);
    return dual_solver.solve(split_var, value) ? 1 : 0;
}

int selectBestSplitVariable(SATList* cnf) {
    if (cnf == nullptr) return -1;
    
    // 统计每个变量在多少个子句中出现
    std::vector<int> pos_count(boolCount + 1, 0);  // 正文字出现次数
    std::vector<int> neg_count(boolCount + 1, 0);  // 负文字出现次数
    std::vector<bool> assigned(boolCount + 1, false);  // 是否已赋值
    
    SATList* lp;
    SATNode* tp;
    
    // 统计变量出现频率
    for (lp = cnf; lp != nullptr; lp = lp->next) {
        for (tp = lp->head; tp != nullptr; tp = tp->next) {
            int var = abs(tp->data);
            if (var <= boolCount) {
                if (tp->data > 0) {
                    pos_count[var]++;
                } else {
                    neg_count[var]++;
                }
            }
        }
    }
    
    // 选择平衡性最好的变量（正负出现次数都较多且接近）
    int best_var = -1;
    double best_score = -1.0;
    
    for (int i = 1; i <= boolCount; i++) {
        if (!assigned[i] && (pos_count[i] > 0 || neg_count[i] > 0)) {
            // 计算平衡分数：考虑总出现次数和正负平衡
            int total = pos_count[i] + neg_count[i];
            double balance = 1.0 - abs(pos_count[i] - neg_count[i]) / (double)total;
            double score = total * balance;
            
            if (score > best_score) {
                best_score = score;
                best_var = i;
            }
        }
    }
    
    return best_var;
}

double estimateBranchComplexity(SATList* cnf, int var, bool assignment) {
    if (cnf == nullptr) return 0.0;
    
    int remaining_clauses = 0;
    int total_literals = 0;
    
    SATList* lp;
    SATNode* tp;
    
    for (lp = cnf; lp != nullptr; lp = lp->next) {
        bool clause_satisfied = false;
        int clause_literals = 0;
        
        for (tp = lp->head; tp != nullptr; tp = tp->next) {
            clause_literals++;
            
            // 检查当前赋值是否满足这个文字
            if (abs(tp->data) == var) {
                if ((tp->data > 0 && assignment) || (tp->data < 0 && !assignment)) {
                    clause_satisfied = true;
                    break;
                }
            }
        }
        
        if (!clause_satisfied) {
            remaining_clauses++;
            total_literals += clause_literals;
        }
    }
    
    // 复杂度估算：剩余子句数 * 平均文字数
    return remaining_clauses * (total_literals / (double)std::max(1, remaining_clauses));
}
