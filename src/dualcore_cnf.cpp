/**
 * @file dualcore_cnf.cpp
 * @brief 双核并行DPLL求解器实现
 * @author [Hus-King]
 * @date 2025-09-04
 * @version 1.0
 */

#include "dualcore_cnf.h"

// ==================== 全局函数实现 ====================

int DPLL_DualCore(SATList*& cnf, int value[]) {
    if (cnf == nullptr) return 0;
    
    // 选择第一个未赋值变量（简单快速）
    int split_var = selectBestSplitVariable(cnf);
    
    if (split_var == -1) {
        // 所有变量已赋值，直接检查
        OptimizedDPLL temp_solver(cnf);
        return temp_solver.solve() ? 1 : 0;
    }
    
    // 使用 std::promise 实现真正的并行竞争
    std::promise<std::vector<int>> promise_true, promise_false;
    std::future<std::vector<int>> future_true = promise_true.get_future();
    std::future<std::vector<int>> future_false = promise_false.get_future();
    
    std::atomic<bool> solution_found{false};
    
    // 分支1: var = true
    std::thread thread_true([&](std::promise<std::vector<int>>&& prom) {
        if (!solution_found.load()) {
            OptimizedDPLL solver(cnf);
            if (solver.pushAssignmentWithPropagation(split_var, true) && 
                solver.solve() && !solution_found.exchange(true)) {
                prom.set_value(solver.getSolution());
            } else {
                prom.set_value({});
            }
        }
    }, std::move(promise_true));
    
    // 分支2: var = false
    std::thread thread_false([&](std::promise<std::vector<int>>&& prom) {
        if (!solution_found.load()) {
            OptimizedDPLL solver(cnf);
            if (solver.pushAssignmentWithPropagation(split_var, false) && 
                solver.solve() && !solution_found.exchange(true)) {
                prom.set_value(solver.getSolution());
            } else {
                prom.set_value({});
            }
        }
    }, std::move(promise_false));
    
    // 等待第一个完成的结果
    std::vector<int> result;
    if (future_true.wait_for(std::chrono::milliseconds(5000)) == std::future_status::ready) {
        result = future_true.get();
        if (!result.empty()) {
            thread_false.detach();  // 直接分离，让其自行结束
            thread_true.join();
            // 复制结果
            for (int i = 1; i <= boolCount && i < (int)result.size(); ++i) {
                value[i] = result[i];
            }
            return 1;
        }
    }
    
    if (future_false.wait_for(std::chrono::milliseconds(1)) == std::future_status::ready) {
        result = future_false.get();
        if (!result.empty()) {
            thread_true.detach();
            thread_false.join();
            // 复制结果
            for (int i = 1; i <= boolCount && i < (int)result.size(); ++i) {
                value[i] = result[i];
            }
            return 1;
        }
    }
    
    // 等待所有线程完成
    thread_true.join();
    thread_false.join();
    
    return 0;
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
