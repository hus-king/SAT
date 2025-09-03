#include "cnf.h"
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>
/**
 * @brief 从CNF文件读取SAT问题实现
 * @param cnf 输出参数，存储读取的CNF公式链表
 * @return 成功返回1，失败返回0
 * @details 读取标准DIMACS CNF格式文件：
 *          - 以'c'开头的行为注释行，会被跳过
 *          - 包含"p cnf variables clauses"格式的问题行
 *          - 每个子句以0结尾
 */
int ReadFile(SATList*& cnf)
{
    // 打开指定的CNF文件
    FILE* fp = fopen(fileName, "r");
    if (!fp) {
        printf("File open failed!\n");
        return 0;
    }

    char ch;
    // 跳过注释行，CNF文件中以'c'开头的行为注释
    while ((ch = getc(fp)) == 'c') {
        while ((ch = getc(fp)) != '\n');  // 读到行末
    }

    // 回退一个字符，准备读取问题行"p cnf"
    ungetc(ch, fp);
    char tmp[10];
    fscanf(fp, "%s", tmp);  // 读取"p"
    fscanf(fp, "%s", tmp);  // 读取"cnf"

    // 读取变量数和子句数
    fscanf(fp, "%d %d", &boolCount, &clauseCount);

    cnf = nullptr;                      // 初始化CNF链表头
    SATList* prevClause = nullptr;      // 用于连接子句链表的前驱指针

    // 逐个读取子句
    for (int i = 0; i < clauseCount; i++) {
        // 创建新的子句节点
        SATList* clause = (SATList*)malloc(sizeof(SATList));
        clause->head = nullptr;
        clause->next = nullptr;

        SATNode* prevNode = nullptr;    // 用于连接文字链表的前驱指针
        int number;

        // 读取子句中的所有文字，直到遇到0
        while (fscanf(fp, "%d", &number) == 1 && number != 0) {
            // 创建文字节点
            SATNode* node = (SATNode*)malloc(sizeof(SATNode));
            node->data = number;
            node->next = nullptr;

            // 将文字节点加入子句链表
            if (!clause->head) {
                clause->head = node;        // 第一个文字作为子句头
            }
            else {
                prevNode->next = node;      // 连接到前一个文字
            }
            prevNode = node;
        }

        // 将子句加入CNF链表
        if (!cnf) {
            cnf = clause;                   // 第一个子句作为CNF头
        }
        else {
            prevClause->next = clause;      // 连接到前一个子句
        }
        prevClause = clause;
    }

    printf("Reading completed\n");
    fclose(fp);
    return 1;
}

/**
 * @brief 销毁CNF公式链表，释放所有内存
 * @param cnf 要销毁的CNF公式链表引用
 * @details 递归释放所有子句及其包含的文字节点，防止内存泄漏
 */
void destroyClause(SATList*& cnf)
{
    SATList* clausePtr, * nextClause;
    SATNode* literalPtr, * nextLiteral;
    
    // 遍历所有子句
    for (clausePtr = cnf; clausePtr != NULL; clausePtr = nextClause) {
        nextClause = clausePtr->next;
        
        // 释放当前子句中的所有文字节点
        for (literalPtr = clausePtr->head; literalPtr != NULL; literalPtr = nextLiteral) {
            nextLiteral = literalPtr->next;
            free(literalPtr);
        }
        
        // 释放子句节点
        free(clausePtr);
    }
    
    cnf = NULL;  // 置空指针，避免悬挂指针
}

/**
 * @brief 判断子句是否为单子句（仅包含一个文字）
 * @param cnf 待判断的子句头指针
 * @return 是单子句返回1，否则返回0
 */
int isUnitClause(SATNode* cnf)
{
    if (cnf != NULL && cnf->next == NULL)
        return 1;
    else
        return 0;
}

/**
 * @brief 寻找CNF公式中的单子句
 * @param cnf CNF公式链表
 * @return 找到单子句返回指向该文字的指针，否则返回nullptr
 * @details 遍历CNF中的所有子句，返回第一个找到的单子句中的文字
 */
SATNode* unitClause(SATList* cnf)
{
    SATList* tp = cnf;
    while (tp != nullptr) {
        if (isUnitClause(tp->head)) {
            return tp->head;
        }
        tp = tp->next;
    }
    return nullptr;
}

/**
 * @brief 在给定变量赋值下计算子句的真值
 * @param cnf 待计算的子句头指针
 * @param v 变量赋值数组，v[i]=1表示变量i为真，v[i]=0表示变量i为假
 * @return 子句为真返回1，为假返回0
 * @details 子句为真当且仅当其中至少有一个文字为真
 */
int evaluateClause(SATNode* cnf, int v[])
{
    SATNode* tp = cnf;
    while (tp != NULL) {
        // 正文字：变量为真时文字为真
        // 负文字：变量为假时文字为真
        if ((tp->data > 0 && v[tp->data] == 1) ||
            (tp->data < 0 && v[-tp->data] == 0)) {
            return 1;  // 找到一个为真的文字，子句为真
        }
        tp = tp->next;
    }
    return 0;  // 所有文字都为假，子句为假
}

/**
 * @brief 从CNF公式中删除指定子句
 * @param cnf 要删除的子句指针
 * @param root CNF公式链表的根节点指针
 * @return 删除成功返回1，失败返回0
 */
int removeClause(SATList*& cnf, SATList*& root)
{
    SATList* lp = root;
    
    if (lp == cnf) {
        // 要删除的是头节点
        root = root->next;
    }
    else {
        // 寻找要删除节点的前驱
        while (lp != NULL && lp->next != cnf) {
            lp = lp->next;
        }
        if (lp != NULL) {
            lp->next = lp->next->next;
        }
    }
    
    free(cnf);
    cnf = NULL;
    return 1;
}

/**
 * @brief 从子句中删除指定的文字节点
 * @param cnf 要删除的文字节点指针
 * @param head 子句的头节点指针
 * @return 删除成功返回1，失败返回0
 */
int removeNode(SATNode*& cnf, SATNode*& head)
{
    SATNode* lp = head;
    
    if (lp == cnf) {
        // 要删除的是头节点
        head = head->next;
    }
    else {
        // 寻找要删除节点的前驱
        while (lp != NULL && lp->next != cnf) {
            lp = lp->next;
        }
        if (lp != NULL) {
            lp->next = lp->next->next;
        }
    }
    
    free(cnf);
    cnf = NULL;
    return 1;
}

/**
 * @brief 向CNF公式中添加新子句
 * @param cnf 要添加的子句指针
 * @param root CNF公式链表的根节点指针
 * @return 添加成功返回1，失败返回0
 * @details 新子句将被插入到链表头部
 */
int addClause(SATList* cnf, SATList*& root)
{
    if (cnf != NULL) {
        cnf->next = root;
        root = cnf;
        return 1;
    }
    return 0;
}

/**
 * @brief 检查CNF公式中是否包含空子句
 * @param cnf CNF公式链表头指针
 * @return 包含空子句返回1，否则返回0
 * @details 空子句表示矛盾，整个公式不可满足
 */
int emptyClause(SATList* cnf)
{
    SATList* lp = cnf;
    while (lp != NULL) {
        if (lp->head == NULL) return 1;  // 发现空子句
        lp = lp->next;
    }
    return 0;  // 没有空子句
}

/**
 * @brief 深拷贝CNF公式链表
 * @param a 目标链表（输出参数）
 * @param b 源链表
 * @details 创建源链表的完整副本，包括所有子句和文字节点
 *          使用迭代而非递归避免栈溢出
 */
void CopyClause(SATList*& a, SATList* b) {
    // 初始化目标链表
    a = nullptr;
    if (b == nullptr) return;

    SATList* prevClause = nullptr;  // 用于连接子句链表
    SATList* currentB = b;

	// 使用迭代而不是递归来避免栈溢出
	while (currentB != nullptr) {
		// 创建新的子句节点
		SATList* newClause = (SATList*)malloc(sizeof(SATList));
		if (newClause == nullptr) {
			// 内存分配失败，清理已分配的内存
			destroyClause(a);
			a = nullptr;
			return;
		}

		newClause->head = nullptr;
		newClause->next = nullptr;

		// 复制子句中的文字
		SATNode* currentLiteral = currentB->head;
		SATNode* prevNode = nullptr;

		while (currentLiteral != nullptr) {
			SATNode* newNode = (SATNode*)malloc(sizeof(SATNode));
			if (newNode == nullptr) {
				// 内存分配失败，清理已分配的内存
				destroyClause(newClause); // 清理当前子句
				destroyClause(a);         // 清理已复制的部分
				a = nullptr;
				return;
			}

			newNode->data = currentLiteral->data;
			newNode->next = nullptr;

			if (newClause->head == nullptr) {
				newClause->head = newNode;
			}
			else {
				prevNode->next = newNode;
			}

			prevNode = newNode;
			currentLiteral = currentLiteral->next;
		}

		// 将新子句添加到目标链表
		if (a == nullptr) {
			a = newClause;
		}
		else {
			prevClause->next = newClause;
		}

		prevClause = newClause;
		currentB = currentB->next;
	}
}


/**
 * @brief DPLL算法求解SAT问题
 * @param cnf CNF公式链表（会被修改）
 * @param value 变量赋值数组（输出参数）
 * @return 有解返回1，无解返回0
 * @details 实现经典的DPLL算法：
 *          1. 单子句传播
 *          2. 纯文字消除
 *          3. 选择变量分支
 *          使用启发式选择出现频率最高的变量进行分支
 */
int DPLL(SATList*& cnf, int value[])
{
    SATList* tp = cnf, * lp = cnf, * sp;
    SATNode* dp;
    int* count;
    int i, MaxWord = 0, max = 0, re;

FIND_UNIT:
    // 单子句传播：查找并处理单子句
    while (tp != nullptr && !isUnitClause(tp->head))
        tp = tp->next;

    if (tp != nullptr) {
        // 应用单子句规则进行简化
        re = tp->head->data;
        if (re > 0) value[re] = 1;      // 正文字赋值为真
        else value[-re] = 0;            // 负文字对应变量赋值为假

		for (lp = cnf; lp != nullptr; lp = sp) {
			sp = lp->next;
			for (dp = lp->head; dp != nullptr; dp = dp->next) {
				if (dp->data == re) {
					removeClause(lp, cnf);
					break;
				}
				if (dp->data == -re) {
					removeNode(dp, lp->head);
					break;
				}
			}
		}

		// 检查极简化后结果
		if (cnf == nullptr) return 1;
		if (emptyClause(cnf)) {
			destroyClause(cnf);
			return 0;
		}
		tp = cnf;
		goto FIND_UNIT;  // 继续单子句简化
	}

	// 分配并初始化 count 数组
	count = (int*)malloc(sizeof(int) * (2 * boolCount + 2));
	for (i = 0; i <= 2 * boolCount + 1; i++) count[i] = 0;

	// 统计文字出现次数
	for (lp = cnf; lp != nullptr; lp = lp->next) {
		for (dp = lp->head; dp != nullptr; dp = dp->next) {
			if (dp->data > 0) {
				if (dp->data <= boolCount) {
					count[dp->data]++;
				}
			}
			else {
				int index = boolCount - dp->data; // dp->data < 0
				if (index <= 2 * boolCount + 1) {
					count[index]++;
				}
			}
		}
	}

	// 找出现次数最多的正文字
	max = 0; MaxWord = 0;
	for (i = 1; i <= boolCount; i++) {
		if (count[i] > max) {
			max = count[i];
			MaxWord = i;
		}
	}

	// 如果没有正文字出现，找出现次数最多的负文字
	if (max == 0) {
		for (i = 1; i <= boolCount; i++) {
			if (count[boolCount + i] > max) {
				max = count[boolCount + i];
				MaxWord = -i;
			}
		}
	}

	free(count); // 安全释放

	// 构造尝试赋值正文字分支
	SATList* branch1 = (SATList*)malloc(sizeof(SATList));
	branch1->head = (SATNode*)malloc(sizeof(SATNode));
	branch1->head->data = MaxWord;
	branch1->head->next = nullptr;
	branch1->next = nullptr;

	CopyClause(tp, cnf);
	addClause(branch1, tp);

	if (DPLL(tp, value) == 1) return 1;
	destroyClause(tp);

	// 构造尝试赋值负文字分支
	SATList* branch2 = (SATList*)malloc(sizeof(SATList));
	branch2->head = (SATNode*)malloc(sizeof(SATNode));
	branch2->head->data = -MaxWord;
	branch2->head->next = nullptr;
	branch2->next = nullptr;

	addClause(branch2, cnf);
	re = DPLL(cnf, value);
	destroyClause(cnf);
	return re;
}


/*
* 函数名称: WriteFile
* 接受参数: int,int,int[]
* 函数功能: 将运行结果保存至同名文件，文件拓展名为.res,保存成功返回1，失败返回0
* 返回值: int
*/
int WriteFile(int result, double time, int value[])
{
	FILE* fp;
	int i;
	for (i = 0; fileName[i] != '\0'; i++)
	{
		//修改拓展名
		if (fileName[i] == '.' && fileName[i + 4] == '\0')
		{
			fileName[i + 1] = 'r';
			fileName[i + 2] = 'e';
			fileName[i + 3] = 's';
			break;
        }
    }
    if ((fp = fopen(fileName, "w")) == NULL) {
        printf("File open failed!\n");
        return 0;
    }
	fprintf(fp, "s %d\nv ", result);  //求解结果
	if (result == 1)
	{
		//保存解值
		for (i = 1; i <= boolCount; i++)
		{
			if (value[i] == 1) fprintf(fp, "%d ", i);
			else fprintf(fp, "%d ", -i);
        }
    }
    fprintf(fp, "\nt %lf", time * 1000);  //运行时间/毫秒
    fclose(fp);
    return 1;
}

// ==================== 高效双核并行实现 ====================

/**
 * @brief 从传统链表结构转换为高效数组结构
 */
void FastCNF::fromSATList(SATList* cnf) {
    clauses.clear();
    
    for (SATList* clause_ptr = cnf; clause_ptr != nullptr; clause_ptr = clause_ptr->next) {
        std::vector<int> clause;
        for (SATNode* literal_ptr = clause_ptr->head; literal_ptr != nullptr; literal_ptr = literal_ptr->next) {
            clause.push_back(literal_ptr->data);
        }
        if (!clause.empty()) {
            clauses.push_back(clause);
        }
    }
    
    // 初始化状态数组
    clause_satisfied.resize(clauses.size(), false);
}

/**
 * @brief 转换回传统链表结构（兼容性）
 */
SATList* FastCNF::toSATList() const {
    if (clauses.empty()) return nullptr;
    
    SATList* cnf = nullptr;
    SATList* prev_clause = nullptr;
    
    for (const auto& clause : clauses) {
        SATList* new_clause = (SATList*)malloc(sizeof(SATList));
        new_clause->head = nullptr;
        new_clause->next = nullptr;
        
        SATNode* prev_node = nullptr;
        for (int literal : clause) {
            SATNode* new_node = (SATNode*)malloc(sizeof(SATNode));
            new_node->data = literal;
            new_node->next = nullptr;
            
            if (!new_clause->head) {
                new_clause->head = new_node;
            } else {
                prev_node->next = new_node;
            }
            prev_node = new_node;
        }
        
        if (!cnf) {
            cnf = new_clause;
        } else {
            prev_clause->next = new_clause;
        }
        prev_clause = new_clause;
    }
    
    return cnf;
}

/**
 * @brief 高效深拷贝（现代C++优化）
 */
FastCNF FastCNF::copy() const {
    FastCNF result(num_vars);
    result.clauses = clauses;                    // vector拷贝，现代C++优化
    result.assignment = assignment;
    result.trail = trail;
    result.clause_satisfied = clause_satisfied;
    result.decision_level = decision_level;
    return result;
}

/**
 * @brief 检查是否有空子句
 */
bool FastCNF::hasEmptyClause() const {
    for (size_t i = 0; i < clauses.size(); ++i) {
        if (clause_satisfied[i]) continue;
        
        bool all_false = true;
        for (int literal : clauses[i]) {
            int var = abs(literal);
            VarState val = assignment[var];
            if (val == VAR_UNASSIGNED) {
                all_false = false;
                break;
            }
            bool lit_true = (literal > 0) ? (val == VAR_TRUE) : (val == VAR_FALSE);
            if (lit_true) {
                all_false = false;
                break;
            }
        }
        if (all_false) return true;
    }
    return false;
}

/**
 * @brief 检查是否所有子句都满足
 */
bool FastCNF::allClausesSatisfied() const {
    for (bool satisfied : clause_satisfied) {
        if (!satisfied) return false;
    }
    return true;
}

/**
 * @brief 高效单子句传播
 */
bool FastCNF::unitPropagate() {
    bool changed = true;
    while (changed && !hasEmptyClause()) {
        changed = false;
        
        for (size_t i = 0; i < clauses.size(); ++i) {
            if (clause_satisfied[i]) continue;
            
            int unassigned_count = 0;
            int unassigned_literal = 0;
            
            for (int literal : clauses[i]) {
                int var = abs(literal);
                VarState val = assignment[var];
                
                if (val == VAR_UNASSIGNED) {
                    unassigned_count++;
                    unassigned_literal = literal;
                } else {
                    bool lit_true = (literal > 0) ? (val == VAR_TRUE) : (val == VAR_FALSE);
                    if (lit_true) {
                        clause_satisfied[i] = true;
                        changed = true;
                        goto next_clause;
                    }
                }
            }
            
            // 如果没有未赋值文字，且没满足 → 空子句
            if (unassigned_count == 0) {
                return false;  // 冲突
            }
            
            // 如果只有一个未赋值文字 → 单子句
            if (unassigned_count == 1) {
                int var = abs(unassigned_literal);
                VarState val = (unassigned_literal > 0) ? VAR_TRUE : VAR_FALSE;
                assign(var, val, false);  // 非决策赋值
                changed = true;
            }
            
        next_clause:;
        }
        
        // 如果传播过程中发现冲突
        if (hasEmptyClause()) return false;
    }
    
    return true;
}

/**
 * @brief 赋值变量并记录到trail
 */
void FastCNF::assign(int var, VarState value, bool is_decision) {
    if (assignment[var] != VAR_UNASSIGNED) {
        // 重复赋值！这是严重错误
        return;  // 或抛异常
    }
    
    TrailItem item;
    item.var = var;
    item.old_value = assignment[var];  // 当前是 UNASSIGNED
    item.decision_level = decision_level;
    item.is_decision = is_decision;
    
    trail.push_back(item);
    assignment[var] = value;
    
    if (is_decision) {
        decision_level++;
    }
}

/**
 * @brief 回溯到指定层级
 */
void FastCNF::backtrack(int level) {
    while (!trail.empty() && trail.back().decision_level > level) {
        TrailItem& item = trail.back();
        assignment[item.var] = item.old_value;
        trail.pop_back();
    }
    decision_level = level;
}

/**
 * @brief 选择下一个分支变量（VSIDS启发式）
 */
int FastCNF::chooseBranchVariable() const {
    std::vector<double> activity(num_vars + 1, 0.0);
    
    // 计算变量活跃度
    for (size_t i = 0; i < clauses.size(); ++i) {
        if (clause_satisfied[i]) continue;
        
        double clause_weight = 1.0 / (clauses[i].size() + 1);
        for (int literal : clauses[i]) {
            int var = abs(literal);
            if (assignment[var] == VAR_UNASSIGNED) {
                activity[var] += clause_weight;
            }
        }
    }
    
    // 选择活跃度最高的变量
    int best_var = 0;
    double best_activity = 0.0;
    for (int var = 1; var <= num_vars; ++var) {
        if (assignment[var] == VAR_UNASSIGNED && activity[var] > best_activity) {
            best_activity = activity[var];
            best_var = var;
        }
    }
    
    return best_var;
}

/**
 * @brief 高效DPLL算法实现
 */
bool FastDPLL(FastCNF& cnf) {
    if (!cnf.unitPropagate()) {
        return false;
    }
    
    if (cnf.allClausesSatisfied()) {
        return true;
    }
    
    int branch_var = cnf.chooseBranchVariable();
    if (branch_var == 0) return cnf.allClausesSatisfied();
    
    // 保存当前决策层级
    int current_level = cnf.decision_level;
    
    // 分支1：true
    cnf.assign(branch_var, VAR_TRUE, true);
    if (FastDPLL(cnf)) return true;
    
    // 回溯
    cnf.backtrack(current_level);
    
    // 分支2：false
    cnf.assign(branch_var, VAR_FALSE, true);
    bool result = FastDPLL(cnf);
    
    if (!result) {
        cnf.backtrack(current_level);
    }
    
    return result;
}

/**
 * @brief 并行DPLL线程函数
 */
void parallel_dpll_thread(FastCNF cnf_copy, int branch_var, bool branch_value, 
                         std::atomic<bool>& solution_found, 
                         std::atomic<bool>& result_ready,
                         std::vector<VarState>& global_solution,
                         std::mutex& solution_mutex) {
    
    // 检查是否已经有解
    if (solution_found.load()) return;
    
    // 设置分支赋值
    cnf_copy.assign(branch_var, branch_value ? VAR_TRUE : VAR_FALSE, true);
    
    // 运行DPLL求解
    bool found = FastDPLL(cnf_copy);
    
    // 如果找到解且是第一个找到的
    if (found && !solution_found.exchange(true)) {
        std::lock_guard<std::mutex> lock(solution_mutex);
        global_solution = cnf_copy.assignment;
        result_ready.store(true);
    }
}

/**
 * @brief 高效双核DPLL求解器主函数
 */
int DPLL_DualCore_Fast(SATList*& cnf, int value[]) {
    if (cnf == nullptr) return 0;
    
    // 转换为高效数据结构
    FastCNF fast_cnf(boolCount);
    fast_cnf.fromSATList(cnf);
    
    // 预处理：单子句传播
    if (!fast_cnf.unitPropagate()) {
        return 0;  // 发现冲突，无解
    }
    
    if (fast_cnf.allClausesSatisfied()) {
        // 已经满足，复制解
        for (int i = 1; i <= boolCount; i++) {
            value[i] = (fast_cnf.assignment[i] == VAR_TRUE) ? 1 : 0;
        }
        return 1;
    }
    
    // 选择分支变量
    int branch_var = fast_cnf.chooseBranchVariable();
    if (branch_var == 0) {
        return fast_cnf.allClausesSatisfied() ? 1 : 0;
    }
    
    // 并行控制变量
    std::atomic<bool> solution_found(false);
    std::atomic<bool> result_ready(false);
    std::vector<VarState> global_solution(boolCount + 1, VAR_UNASSIGNED);
    std::mutex solution_mutex;
    
    // 创建两个线程，分别搜索正负分支
    // 注意：这里使用现代C++的移动语义，避免深拷贝
    FastCNF cnf_copy1 = fast_cnf.copy();
    FastCNF cnf_copy2 = fast_cnf.copy();
    
    std::thread thread1(parallel_dpll_thread, std::move(cnf_copy1), branch_var, true,
                       std::ref(solution_found), std::ref(result_ready),
                       std::ref(global_solution), std::ref(solution_mutex));
    
    std::thread thread2(parallel_dpll_thread, std::move(cnf_copy2), branch_var, false,
                       std::ref(solution_found), std::ref(result_ready),
                       std::ref(global_solution), std::ref(solution_mutex));
    
    // 等待线程完成
    thread1.join();
    thread2.join();
    
    // 检查结果
    if (result_ready.load()) {
        // 复制解到输出数组
        for (int i = 1; i <= boolCount; i++) {
            value[i] = (global_solution[i] == VAR_TRUE) ? 1 : 0;
        }
        return 1;
    }
    
    return 0;
}