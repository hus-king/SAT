/**
 * @file optimize_cnf.cpp
 * @brief 优化的DPLL求解器实现
 * @author [Hus-King]
 * @date 2025-09-03
 * @version 2.0
 */

#include "optimize_cnf.h"
#include <algorithm>
#include <cstring>

// ==================== OptimizedCNF类实现 ====================

OptimizedCNF::OptimizedCNF(int vars, int clauses_count) 
    : num_vars(vars), num_clauses(clauses_count) {
    assignment.resize(vars + 1, false);      // 使用vector<bool>存储
    is_assigned.resize(vars + 1, false);     // 跟踪是否已赋值
    clause_satisfied.resize(clauses_count, false);
}

OptimizedCNF::OptimizedCNF(const OptimizedCNF& other) {
    clauses = other.clauses;
    assignment = other.assignment;
    is_assigned = other.is_assigned;
    clause_satisfied = other.clause_satisfied;
    num_vars = other.num_vars;
    num_clauses = other.num_clauses;
}

void OptimizedCNF::fromSATList(SATList* cnf) {
    clauses.clear();
    num_clauses = 0;
    
    for (SATList* clause_ptr = cnf; clause_ptr != nullptr; clause_ptr = clause_ptr->next) {
        std::vector<int> clause;
        for (SATNode* literal_ptr = clause_ptr->head; literal_ptr != nullptr; literal_ptr = literal_ptr->next) {
            clause.push_back(literal_ptr->data);
        }
        if (!clause.empty()) {
            clauses.push_back(clause);
            num_clauses++;
        }
    }
    
    clause_satisfied.resize(num_clauses, false);
}

bool OptimizedCNF::hasEmptyClause() const {
    for (size_t i = 0; i < clauses.size(); ++i) {
        if (clause_satisfied[i]) continue;
        
        bool all_false = true;
        for (int literal : clauses[i]) {
            int var = abs(literal);
            
            if (!is_assigned[var]) {  // 未赋值
                all_false = false;
                break;
            }
            
            bool lit_true = (literal > 0) ? assignment[var] : !assignment[var];
            if (lit_true) {
                all_false = false;
                break;
            }
        }
        if (all_false) return true;
    }
    return false;
}

bool OptimizedCNF::allClausesSatisfied() const {
    for (size_t i = 0; i < clauses.size(); ++i) {
        if (clause_satisfied[i]) continue;
        
        bool satisfied = false;
        for (int literal : clauses[i]) {
            int var = abs(literal);
            
            if (is_assigned[var]) {  // 已赋值
                bool lit_true = (literal > 0) ? assignment[var] : !assignment[var];
                if (lit_true) {
                    satisfied = true;
                    break;
                }
            }
        }
        
        if (!satisfied) {
            // 检查是否是单子句且未满足
            int unassigned_count = 0;
            for (int literal : clauses[i]) {
                int var = abs(literal);
                if (!is_assigned[var]) {
                    unassigned_count++;
                }
            }
            if (unassigned_count == 0) {
                return false;  // 空子句，不满足
            }
            return false;  // 有未满足的子句
        }
    }
    return true;
}

void OptimizedCNF::printDebugInfo() const {
    std::cout << "=== CNF Debug Info ===" << std::endl;
    std::cout << "Variables: " << num_vars << ", Clauses: " << num_clauses << std::endl;
    
    for (size_t i = 0; i < clauses.size(); ++i) {
        std::cout << "Clause " << i << " (satisfied=" << clause_satisfied[i] << "): ";
        for (int lit : clauses[i]) {
            std::cout << lit << " ";
        }
        std::cout << std::endl;
    }
    
    std::cout << "Assignments: ";
    for (size_t i = 1; i < assignment.size(); ++i) {
        if (is_assigned[i]) {
            std::cout << "x" << i << "=" << (assignment[i] ? 1 : 0) << " ";
        } else {
            std::cout << "x" << i << "=-1 ";
        }
    }
    std::cout << std::endl;
}

// ==================== OptimizedDPLL类实现 ====================

OptimizedDPLL::OptimizedDPLL(SATList* sat_cnf) : cnf(boolCount, clauseCount) {
    cnf.fromSATList(sat_cnf);
    pos_count.resize(boolCount + 1, 0);
    neg_count.resize(boolCount + 1, 0);
}

void OptimizedDPLL::calculateLiteralCounts() {
    std::fill(pos_count.begin(), pos_count.end(), 0);
    std::fill(neg_count.begin(), neg_count.end(), 0);
    
    for (size_t i = 0; i < cnf.clauses.size(); ++i) {
        if (cnf.clause_satisfied[i]) continue;
        
        for (int literal : cnf.clauses[i]) {
            int var = abs(literal);
            if (var <= cnf.num_vars && !cnf.is_assigned[var]) {
                if (literal > 0) {
                    pos_count[var]++;
                } else {
                    neg_count[var]++;
                }
            }
        }
    }
}

int OptimizedDPLL::selectVariable() {
    calculateLiteralCounts();
    
    int best_var = 0;
    double max_score = -1.0;
    
    for (int i = 1; i <= cnf.num_vars; i++) {
        if (!cnf.is_assigned[i]) {  // 未赋值
            // Jeroslow-Wang启发式：考虑子句长度权重
            double score = 0.0;
            
            for (size_t j = 0; j < cnf.clauses.size(); ++j) {
                if (cnf.clause_satisfied[j]) continue;
                
                int clause_size = 0;
                bool contains_var = false;
                
                for (int literal : cnf.clauses[j]) {
                    int var = abs(literal);
                    if (!cnf.is_assigned[var]) {
                        clause_size++;
                        if (var == i) contains_var = true;
                    }
                }
                
                if (contains_var && clause_size > 0) {
                    score += 1.0 / (1 << clause_size);  // 2^(-clause_size)
                }
            }
            
            // 结合MOM启发式：Maximum Occurrences in clauses of Minimum size
            score += (pos_count[i] + neg_count[i]) * 0.1;
            
            if (score > max_score) {
                max_score = score;
                best_var = i;
            }
        }
    }
    
    return best_var;
}

bool OptimizedDPLL::pureLiteralElimination() {
    calculateLiteralCounts();
    bool changed = false;
    
    for (int i = 1; i <= cnf.num_vars; i++) {
        if (!cnf.is_assigned[i]) {
            if (pos_count[i] > 0 && neg_count[i] == 0) {
                pushAssignment(i, true);   // 纯正文字
                changed = true;
            } else if (pos_count[i] == 0 && neg_count[i] > 0) {
                pushAssignment(i, false);  // 纯负文字
                changed = true;
            }
        }
    }
    
    if (changed) {
        updateClauseStatus();
    }
    
    return changed;
}

void OptimizedDPLL::updateClauseStatus() {
    for (size_t i = 0; i < cnf.clauses.size(); ++i) {
        if (cnf.clause_satisfied[i]) continue;
        
        // 检查子句是否已满足
        for (int literal : cnf.clauses[i]) {
            int var = abs(literal);
            
            if (cnf.is_assigned[var]) {  // 变量已赋值
                bool lit_true = (literal > 0) ? cnf.assignment[var] : !cnf.assignment[var];
                if (lit_true) {
                    cnf.clause_satisfied[i] = true;
                    break;
                }
            }
        }
    }
}

bool OptimizedDPLL::unitPropagation() {
    bool changed = true;
    
    while (changed) {
        changed = false;
        updateClauseStatus();
        
        for (size_t i = 0; i < cnf.clauses.size(); ++i) {
            if (cnf.clause_satisfied[i]) continue;
            
            int unassigned_count = 0;
            int unassigned_literal = 0;
            bool is_satisfied = false;
            
            // 检查已赋值的文字
            for (int literal : cnf.clauses[i]) {
                int var = abs(literal);
                
                if (cnf.is_assigned[var]) {  // 已赋值
                    bool lit_true = (literal > 0) ? cnf.assignment[var] : !cnf.assignment[var];
                    if (lit_true) {
                        is_satisfied = true;
                        break;
                    }
                } else {  // 未赋值
                    unassigned_count++;
                    unassigned_literal = literal;
                }
            }
            
            // 如果子句已满足，跳过
            if (is_satisfied) {
                cnf.clause_satisfied[i] = true;
                continue;
            }
            
            // 检查冲突（所有文字都已赋值但都不满足）
            if (unassigned_count == 0) {
                return false;  // 冲突
            }
            
            // 单子句传播
            if (unassigned_count == 1) {
                int var = abs(unassigned_literal);
                bool value = (unassigned_literal > 0);
                
                if (!cnf.is_assigned[var]) {
                    pushAssignment(var, value);
                    changed = true;
                    // 立即更新相关子句状态
                    updateClauseStatus();
                } else {
                    // 检查赋值冲突
                    bool current_value = cnf.assignment[var];
                    bool required_value = (unassigned_literal > 0);
                    if (current_value != required_value) {
                        return false;  // 赋值冲突
                    }
                }
            }
        }
    }
    
    return true;
}

bool OptimizedDPLL::dpllRecursive() {
    // 单子句传播
    if (!unitPropagation()) {
        return false;
    }
    
    // 更新子句状态
    updateClauseStatus();
    
    // 检查是否已解决
    bool all_satisfied = true;
    for (size_t i = 0; i < cnf.clauses.size(); ++i) {
        if (!cnf.clause_satisfied[i]) {
            all_satisfied = false;
            break;
        }
    }
    
    if (all_satisfied) {
        return true;
    }
    
    // 纯文字消除
    if (pureLiteralElimination()) {
        updateClauseStatus();
        // 重新检查
        bool all_satisfied_after_pure = true;
        for (size_t i = 0; i < cnf.clauses.size(); ++i) {
            if (!cnf.clause_satisfied[i]) {
                all_satisfied_after_pure = false;
                break;
            }
        }
        if (all_satisfied_after_pure) {
            return true;
        }
    }
    
    // 选择变量进行分支
    int var = selectVariable();
    if (var == 0) {
        // 没有未赋值变量，检查是否全部满足
        return cnf.allClausesSatisfied();
    }
    
    // 记录当前决策层级
    size_t decision_level = getCurrentLevel();
    
    // 分支1：var = true
    pushAssignment(var, true);
    if (dpllRecursive()) {
        return true;
    }
    
    // 回溯到决策层级，尝试分支2：var = false
    backtrack(decision_level);
    pushAssignment(var, false);
    
    return dpllRecursive();
}

bool OptimizedDPLL::solve() {
    return dpllRecursive();
}

void OptimizedDPLL::printStats() const {
    std::cout << "CNF统计信息:\n";
    std::cout << "变量数: " << cnf.num_vars << "\n";
    std::cout << "子句数: " << cnf.num_clauses << "\n";
    
    int satisfied_count = 0;
    for (bool sat : cnf.clause_satisfied) {
        if (sat) satisfied_count++;
    }
    std::cout << "已满足子句数: " << satisfied_count << "/" << cnf.num_clauses << "\n";
    
    int assigned_count = 0;
    for (int i = 1; i <= cnf.num_vars; i++) {
        if (cnf.is_assigned[i]) assigned_count++;
    }
    std::cout << "已赋值变量数: " << assigned_count << "/" << cnf.num_vars << "\n";
}

// ==================== 增量回溯实现 ====================

void OptimizedDPLL::pushAssignment(int var, bool value) {
    // 记录变更前的状态
    AssignmentChange change;
    change.var = var;
    change.was_assigned = cnf.is_assigned[var];
    change.old_value = cnf.assignment[var];
    
    // 将变更记录推入栈
    undo_stack.push_back(change);
    
    // 执行赋值
    cnf.is_assigned[var] = true;
    cnf.assignment[var] = value;
}

void OptimizedDPLL::backtrack(size_t target_level) {
    // 回溯到指定层级
    while (undo_stack.size() > target_level) {
        AssignmentChange change = undo_stack.back();
        undo_stack.pop_back();
        
        // 恢复变量状态
        cnf.is_assigned[change.var] = change.was_assigned;
        cnf.assignment[change.var] = change.old_value;
    }
    
    // 重新计算子句状态
    std::fill(cnf.clause_satisfied.begin(), cnf.clause_satisfied.end(), false);
    updateClauseStatus();
}

// ==================== 接口函数实现 ====================

int DPLL_Optimized(SATList*& cnf, int value[]) {
    if (cnf == nullptr) return 0;
    
    try {
        OptimizedDPLL solver(cnf);
        
        if (solver.solve()) {
            std::vector<int> solution = solver.getSolution();
            
            // 复制解到输出数组
            for (int i = 1; i <= boolCount; i++) {
                value[i] = (solution[i] == 1) ? 1 : 0;
            }
            
            return 1;
        } else {
            return 0;
        }
    } catch (const std::exception& e) {
        std::cout << "求解器异常: " << e.what() << std::endl;
        return 0;
    }
}
