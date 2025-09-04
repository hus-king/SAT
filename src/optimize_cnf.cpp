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

OptimizedDPLL::OptimizedDPLL(SATList* sat_cnf) 
    : cnf(boolCount, clauseCount), activity_inc(1.0), decay_factor(0.95), decision_count(0) {
    cnf.fromSATList(sat_cnf);
    pos_count.resize(boolCount + 1, 0);
    neg_count.resize(boolCount + 1, 0);
    
    // 初始化VSIDS相关数据结构
    activity.resize(boolCount + 1, 0.0);
    order_heap.reserve(boolCount);
    
    // 构建变量到子句的映射
    buildVarClauseMapping();
    
    // 初始化Two-Watched Literals
    initWatchedLiterals();
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
    // 求解初期使用MOM启发式（快速），后期使用VSIDS（适应性强）
    if (decision_count < cnf.num_vars / 4) {
        // 初期：优先使用MOM启发式
        int mom_var = selectVariableMOM();
        if (mom_var != 0) return mom_var;
    }
    
    // 使用VSIDS快速选择
    int var = selectVariableVSIDS();
    if (var != 0) return var;
    
    // 备用：MOM启发式（更快速）
    return selectVariableMOM();
}

bool OptimizedDPLL::pureLiteralElimination() {
    calculateLiteralCounts();
    
    for (int i = 1; i <= cnf.num_vars; i++) {
        if (!cnf.is_assigned[i]) {
            if (pos_count[i] > 0 && neg_count[i] == 0) {
                if (!pushAssignmentWithPropagation(i, true)) {   // 纯正文字
                    return false; // 传播失败
                }
            } else if (pos_count[i] == 0 && neg_count[i] > 0) {
                if (!pushAssignmentWithPropagation(i, false)) {  // 纯负文字
                    return false; // 传播失败
                }
            }
        }
    }
    
    return true;
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
    // 使用Two-Watched Literals的优化版本
    return true; // 传播已经在pushAssignment中通过propagateWatched处理
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
    if (!pureLiteralElimination()) {
        return false; // 纯文字消除导致冲突
    }
    
    // 重新检查是否所有子句都满足
    updateClauseStatus();
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
    
    // 选择变量进行分支
    int var = selectVariable();
    if (var == 0) {
        // 没有未赋值变量，检查是否全部满足
        return cnf.allClausesSatisfied();
    }
    
    // 增加决策计数
    decision_count++;
    
    // 记录当前决策层级
    size_t decision_level = getCurrentLevel();
    
    // 分支1：var = true
    if (!pushAssignmentWithPropagation(var, true)) {
        // 传播失败，回溯并尝试另一个分支
        backtrack(decision_level);
    } else if (dpllRecursive()) {
        return true;
    } else {
        // 回溯并尝试分支2：var = false
        backtrack(decision_level);
    }
    
    // 分支2：var = false
    if (!pushAssignmentWithPropagation(var, false)) {
        return false; // 两个分支都失败
    }
    
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

bool OptimizedDPLL::pushAssignmentWithPropagation(int var, bool value) {
    // 执行赋值
    pushAssignment(var, value);
    
    // 触发Two-Watched Literals传播
    return propagateWatched(var, value);
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

// ==================== 预处理和VSIDS优化实现 ====================

void OptimizedDPLL::buildVarClauseMapping() {
    var_to_clauses.resize(cnf.num_vars + 1);
    
    for (int clause_idx = 0; clause_idx < static_cast<int>(cnf.clauses.size()); ++clause_idx) {
        for (int literal : cnf.clauses[clause_idx]) {
            int var = abs(literal);
            var_to_clauses[var].push_back({clause_idx, literal});
        }
    }
}

void OptimizedDPLL::bumpActivity(int var) {
    activity[var] += activity_inc;
    
    // 防止数值溢出
    if (activity[var] > 1e100) {
        // 重新归一化所有活跃度
        for (int i = 1; i <= cnf.num_vars; ++i) {
            activity[i] *= 1e-100;
        }
        activity_inc *= 1e-100;
    }
}

void OptimizedDPLL::decayActivity() {
    activity_inc /= decay_factor;
    if (activity_inc > 1e100) {  // 防止溢出
        for (int i = 1; i <= cnf.num_vars; ++i) {
            activity[i] *= 1e-100;
        }
        activity_inc *= 1e-100;
    }
}

int OptimizedDPLL::selectVariableVSIDS() {
    int best_var = 0;
    double max_activity = -1.0;
    
    for (int i = 1; i <= cnf.num_vars; ++i) {
        if (!cnf.is_assigned[i] && activity[i] > max_activity) {
            max_activity = activity[i];
            best_var = i;
        }
    }
    
    return best_var;
}

int OptimizedDPLL::selectVariableMOM() {
    calculateLiteralCounts();
    
    int best_var = 0;
    int max_mom = -1;
    
    for (int i = 1; i <= cnf.num_vars; ++i) {
        if (!cnf.is_assigned[i]) {
            // MOM启发式：(pos_count * neg_count) + pos_count + neg_count
            // 优先选择在正负文字出现频率都高的变量
            int mom_score = pos_count[i] * neg_count[i] + pos_count[i] + neg_count[i];
            if (mom_score > max_mom) {
                max_mom = mom_score;
                best_var = i;
            }
        }
    }
    
    return best_var;
}

void OptimizedDPLL::handleConflict(const std::vector<int>& conflict_clause) {
    // 提升冲突子句中所有变量的活跃度
    for (int literal : conflict_clause) {
        int var = abs(literal);
        bumpActivity(var);
    }
    
    // 定期衰减活跃度
    decayActivity();
}

// ==================== Two-Watched Literals 实现 ====================

int OptimizedDPLL::literalToIndex(int literal) const {
    // 将文字映射到索引：正文字 x 映射到 x，负文字 -x 映射到 num_vars + x
    return literal > 0 ? literal : cnf.num_vars + (-literal);
}

void OptimizedDPLL::initWatchedLiterals() {
    // 初始化watch列表，大小为 2 * num_vars + 1（索引从1开始）
    watches.resize(2 * cnf.num_vars + 1);
    clause_watched.resize(cnf.clauses.size(), {0, 0});
    
    for (int i = 0; i < static_cast<int>(cnf.clauses.size()); ++i) {
        const auto& clause = cnf.clauses[i];
        if (clause.size() >= 2) {
            // 选择前两个文字作为watched literals
            int watch1 = clause[0];
            int watch2 = clause[1];
            
            clause_watched[i] = {watch1, watch2};
            
            // 添加到对应的watch列表
            int idx1 = literalToIndex(watch1);
            int idx2 = literalToIndex(watch2);
            
            watches[idx1].push_back(i);
            watches[idx2].push_back(i);
        } else if (clause.size() == 1) {
            // 单子句特殊处理
            int watch1 = clause[0];
            clause_watched[i] = {watch1, 0};
            
            int idx1 = literalToIndex(watch1);
            watches[idx1].push_back(i);
        }
    }
}

bool OptimizedDPLL::updateWatch(int clause_idx, int old_watch_literal) {
    const auto& clause = cnf.clauses[clause_idx];
    auto& watched = clause_watched[clause_idx];
    
    // 确定另一个watched literal
    int other_watch = (watched.first == old_watch_literal) ? watched.second : watched.first;
    
    // 寻找新的watched literal
    for (int literal : clause) {
        if (literal != old_watch_literal && literal != other_watch) {
            int var_in_lit = abs(literal);
            
            // 检查这个文字是否已经满足子句
            if (cnf.is_assigned[var_in_lit]) {
                bool var_value = cnf.assignment[var_in_lit];
                bool lit_value = (literal > 0) ? var_value : !var_value;
                if (lit_value) {
                    // 子句已满足
                    cnf.clause_satisfied[clause_idx] = true;
                    return true;
                }
            } else {
                // 找到新的未赋值文字作为watched literal
                if (watched.first == old_watch_literal) {
                    watched.first = literal;
                } else {
                    watched.second = literal;
                }
                
                // 更新watch列表
                int old_idx = literalToIndex(old_watch_literal);
                int new_idx = literalToIndex(literal);
                
                // 从旧的watch列表中移除
                auto& old_watch_list = watches[old_idx];
                old_watch_list.erase(
                    std::remove(old_watch_list.begin(), old_watch_list.end(), clause_idx),
                    old_watch_list.end()
                );
                
                // 添加到新的watch列表
                watches[new_idx].push_back(clause_idx);
                
                return true;
            }
        }
    }
    
    return false; // 没有找到新的watched literal
}

bool OptimizedDPLL::propagateWatched(int var, bool value) {
    // 构造被赋值为false的文字
    int false_lit = value ? -var : var;
    int false_idx = literalToIndex(false_lit);
    
    // 创建watch列表的副本，因为在迭代过程中可能会修改
    std::vector<int> watched_clauses = watches[false_idx];
    
    for (int clause_idx : watched_clauses) {
        if (cnf.clause_satisfied[clause_idx]) continue;
        
        const auto& clause = cnf.clauses[clause_idx];
        const auto& watched = clause_watched[clause_idx];
        
        // 尝试更新watch
        if (!updateWatch(clause_idx, false_lit)) {
            // 无法找到新的watch，检查另一个watched literal
            int other_watch = (watched.first == false_lit) ? watched.second : watched.first;
            
            if (other_watch == 0) {
                // 单子句冲突
                handleConflict(clause);
                return false;
            }
            
            int other_var = abs(other_watch);
            if (cnf.is_assigned[other_var]) {
                // 检查是否冲突
                bool other_value = cnf.assignment[other_var];
                bool other_sign = other_watch > 0;
                if ((other_sign && !other_value) || (!other_sign && other_value)) {
                    // 冲突
                    handleConflict(clause);
                    return false;
                }
                // 否则子句已满足
                cnf.clause_satisfied[clause_idx] = true;
            } else {
                // 单子句传播
                bool required_value = other_watch > 0;
                pushAssignment(other_var, required_value);
                
                // 递归传播
                if (!propagateWatched(other_var, required_value)) {
                    return false;
                }
            }
        }
    }
    
    return true;
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
