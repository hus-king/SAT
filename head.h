/**
 * @file head.h
 * @brief SAT求解器和百分号数独游戏的头文件
 * @author [作者姓名]
 * @date 2025-09-03
 * @version 1.0
 */

#ifndef HEAD_H
#define HEAD_H

#define _CRT_SECURE_NO_WARNINGS
#include <bits/stdc++.h>
#include <string>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <random>
#include <vector>
#include <algorithm>
using namespace std;

// ==================== 常量定义 ====================
#define N 9                     // 数独网格大小 9x9
#define MAX_VAR 10000          // SAT变量最大数量，根据实际需求调整

// ==================== 全局变量 ====================
int boolCount;                  // CNF公式中的布尔变量数量
int clauseCount;                // CNF公式中的子句数量
char fileName[100];             // 待处理的CNF文件名

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
 * @brief DPLL算法主函数，求解SAT问题
 * @param cnf CNF公式链表
 * @param value 变量赋值数组
 * @return 有解返回1，无解返回0
 */
int DPLL(SATList*& cnf, int value[]);

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

// ---------- 百分号数独相关函数 ----------

// 百分号数独特殊窗口位置定义
int upperWindow[9][2] = { {1,1},{1,2},{1,3},{2,1},{2,2},{2,3},{3,1},{3,2},{3,3} };
int lowerWindow[9][2] = { {5,5},{5,6},{5,7},{6,5},{6,6},{6,7},{7,5},{7,6},{7,7} };

/**
 * @brief 检查在数独网格指定位置放置数字是否合法
 * @param grid 数独网格
 * @param row 行号（0-8）
 * @param col 列号（0-8）
 * @param num 要放置的数字（1-9）
 * @return 合法返回true，否则返回false
 */
bool isSafe(int grid[N][N], int row, int col, int num);

/**
 * @brief 使用回溯算法填充完整的数独网格
 * @param grid 要填充的数独网格
 * @param row 当前填充的行号，默认从0开始
 * @param col 当前填充的列号，默认从0开始
 * @return 填充成功返回true，失败返回false
 */
bool fillGrid(int grid[N][N], int row = 0, int col = 0);

/**
 * @brief 从完整数独中生成谜题，确保解的唯一性
 * @param full 完整的数独解
 * @param puzzle 生成的数独谜题（输出参数）
 * @param clues 要保留的提示数字个数
 */
void generatePuzzle(int full[N][N], int puzzle[N][N], int clues);

/**
 * @brief 打印数独网格到控制台
 * @param grid 要打印的数独网格
 */
void printSudoku(int grid[N][N]);

/**
 * @brief 将数独坐标和数字转换为SAT变量索引
 * @param row 行号（0-8）
 * @param col 列号（0-8）
 * @param num 数字（1-9）
 * @return SAT变量索引
 */
int varIndex(int row, int col, int num);

/**
 * @brief 将数独谜题转换为CNF公式
 * @param puzzle 数独谜题
 * @param cnf 输出的CNF公式（输出参数）
 */
void sudokuToCNF(int puzzle[N][N], SATList*& cnf);

/**
 * @brief 检查用户输入的数独解是否正确
 * @param solved 正确的数独解
 * @param ans 用户输入的答案
 * @return 正确返回true，错误返回false
 */
bool check(int solved[9][9], int ans[9][9]);

/**
 * @brief 计算数独谜题的解的数量
 * @param puzzle 数独谜题，0表示空格
 * @param maxSolutions 最大解数限制，默认为2
 * @return 解的数量（最多计算到maxSolutions个）
 * @note 该函数使用手写栈实现的搜索算法，避免了递归调用可能导致的栈溢出
 */
int countSolutions(int puzzle[N][N], int maxSolutions = 2) {
    SATList* cnf = nullptr;
    sudokuToCNF(puzzle, cnf);

    int solutionCount = 0;

    // 手写栈实现，避免递归栈溢出
    const int MAX_STACK_SIZE = 1000;
    SATList* cnfStack[MAX_STACK_SIZE];      // CNF公式栈
    int* valueStack[MAX_STACK_SIZE];        // 变量赋值栈
    int stackTop = -1;                      // 栈顶指针

    // 初始状态入栈
    SATList* initialCNF = nullptr;
    CopyClause(initialCNF, cnf);
    int* initialValue = (int*)malloc(sizeof(int) * MAX_VAR);
    for (int i = 0; i < MAX_VAR; i++) initialValue[i] = 1;

    cnfStack[++stackTop] = initialCNF;
    valueStack[stackTop] = initialValue;

    while (stackTop >= 0 && solutionCount < maxSolutions) {
        // 从栈中取出当前状态
        SATList* currentCNF = cnfStack[stackTop];
        int* currentValue = valueStack[stackTop];
        stackTop--;

        // 使用DPLL算法求解当前CNF
        int result = DPLL(currentCNF, currentValue);

        if (result == 1) {
            // 找到一个解
            solutionCount++;

            if (solutionCount < maxSolutions) {
                // 需要继续搜索更多解，添加否定当前解的约束
                SATList* nextCNF = nullptr;
                CopyClause(nextCNF, cnf);

                // 创建否定当前解的子句
                SATList* negation = (SATList*)malloc(sizeof(SATList));
                negation->head = nullptr;
                negation->next = nullptr;

                SATNode* negHead = nullptr;
                // 遍历所有空格，构造否定解的约束
                for (int row = 0; row < N; row++) {
                    for (int col = 0; col < N; col++) {
                        if (puzzle[row][col] == 0) {
                            for (int num = 1; num <= 9; num++) {
                                if (currentValue[varIndex(row, col, num)] == 1) {
                                    SATNode* node = (SATNode*)malloc(sizeof(SATNode));
                                    node->data = -varIndex(row, col, num);
                                    node->next = negHead;
                                    negHead = node;
                                    break;
                                }
                            }
                        }
                    }
                }

                negation->head = negHead;
                addClause(negation, nextCNF);

                int* nextValue = (int*)malloc(sizeof(int) * MAX_VAR);
                for (int i = 0; i < MAX_VAR; i++) nextValue[i] = 1;

                // 检查栈容量，避免溢出
                if (stackTop < MAX_STACK_SIZE - 1) {
                    cnfStack[++stackTop] = nextCNF;
                    valueStack[stackTop] = nextValue;
                }
                else {
                    // 栈满，释放内存避免泄漏
                    destroyClause(nextCNF);
                    free(nextValue);
                }
            }
        }

        // 释放当前状态的内存
        destroyClause(currentCNF);
        free(currentValue);
    }

    // 清理栈中剩余的元素，防止内存泄漏
    while (stackTop >= 0) {
        destroyClause(cnfStack[stackTop]);
        free(valueStack[stackTop]);
        stackTop--;
    }

    // 清理原始CNF
    destroyClause(cnf);
    return solutionCount;
}

// ==================== 函数实现 ====================

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

// ---------- 百分号数独函数实现 ----------
bool isSafe(int grid[N][N], int row, int col, int num) {
	// 检查行
	for (int i = 0; i < N; i++)
		if (grid[row][i] == num) return false;

	// 检查列
	for (int i = 0; i < N; i++)
		if (grid[i][col] == num) return false;

	// 检查3x3宫
	int startRow = row - row % 3;
	int startCol = col - col % 3;
	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 3; j++)
			if (grid[startRow + i][startCol + j] == num) return false;

	// 检查主对角线
	if (row == col)
		for (int i = 0; i < N; i++)
			if (grid[i][i] == num) return false;

	// 检查副对角线
	if (row + col == N - 1)
		for (int i = 0; i < N; i++)
			if (grid[i][N - 1 - i] == num) return false;

	// 检查上窗口（左上角3x3）
	if (row >= 1 && row <= 3 && col >= 1 && col <= 3) {
		for (int i = 1; i <= 3; i++)
			for (int j = 1; j <= 3; j++)
				if (grid[i][j] == num) return false;
	}

	// 检查下窗口（右下角3x3）
	if (row >= 5 && row <= 7 && col >= 5 && col <= 7) {
		for (int i = 5; i <= 7; i++)
			for (int j = 5; j <= 7; j++)
				if (grid[i][j] == num) return false;
	}

	return true;
}

bool fillGrid(int grid[N][N], int row, int col) {
	if (row == N) return true;
	if (col == N) return fillGrid(grid, row + 1, 0);
	if (grid[row][col] != 0) return fillGrid(grid, row, col + 1);

	int nums[9] = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };
	// 随机打乱数字顺序
	for (int i = 8; i > 0; i--) {
		int j = rand() % (i + 1);
		swap(nums[i], nums[j]);
	}

	for (int i = 0; i < 9; i++) {
		if (isSafe(grid, row, col, nums[i])) {
			grid[row][col] = nums[i];
			if (fillGrid(grid, row, col + 1)) return true;
			grid[row][col] = 0;
		}
	}
	return false;
}

// 向 CNF 中添加单文字子句
void addClauseLiteral(SATList*& cnf, int literal) {
	// 创建新的子句节点
	SATList* newClause = (SATList*)malloc(sizeof(SATList));
	if (!newClause) return; // 内存分配失败
	newClause->head = nullptr;
	newClause->next = nullptr;

	// 创建文字节点
	SATNode* newNode = (SATNode*)malloc(sizeof(SATNode));
	if (!newNode) {
		free(newClause);
		return;
	}
	newNode->data = literal;
	newNode->next = nullptr;

	newClause->head = newNode;

	// 将新子句插入到 CNF 链表头
	if (cnf == nullptr) {
		cnf = newClause;
	}
	else {
		newClause->next = cnf;
		cnf = newClause;
	}
}


/*
 * 函数名称: generatePuzzle
 * 接受参数: int full[N][N] 完整数独, int puzzle[N][N] 谜题, int clues 提示数
 * 函数功能: 从完整数独中挖洞生成题目，确保有解且解唯一
 * 返回值: void
 */

void generatePuzzle(int full[N][N], int puzzle[N][N], int toRemove) {
	// 先复制完整数独
	for (int i = 0; i < N; i++)
		for (int j = 0; j < N; j++)
			puzzle[i][j] = full[i][j];

    // 构造随机顺序
    int positions[N * N][2];
    for (int i = 0; i < N * N; i++) {
        positions[i][0] = i / N;
        positions[i][1] = i % N;
    }
    
    // 使用现代C++的shuffle替代已弃用的random_shuffle
    // 创建一个随机数生成器
    random_device rd;
    mt19937 g(rd());
    
    // 将二维数组转换为可shuffle的容器
    vector<pair<int, int>> posVec;
    for (int i = 0; i < N * N; i++) {
        posVec.push_back({i / N, i % N});
    }
    shuffle(posVec.begin(), posVec.end(), g);
    
    // 复制回原数组
    for (int i = 0; i < N * N; i++) {
        positions[i][0] = posVec[i].first;
        positions[i][1] = posVec[i].second;
    }	int removed = 0;

	for (int idx = 0; idx < N * N && removed < toRemove; idx++) {
		int r = positions[idx][0];
		int c = positions[idx][1];

		if (puzzle[r][c] == 0) continue; // 已经挖空

		int backup = puzzle[r][c];
		puzzle[r][c] = 0;

		// 生成 CNF 并求解第一次
		SATList* cnf1 = nullptr;
		sudokuToCNF(puzzle, cnf1);

		int* value1 = (int*)malloc(sizeof(int) * MAX_VAR);
		for (int k = 0; k < MAX_VAR; k++) value1[k] = 1;

		int result1 = DPLL(cnf1, value1);
		destroyClause(cnf1);

		if (result1 != 1) {
			// 无解，回滚
			puzzle[r][c] = backup;
			free(value1);
			continue;
		}

		// 构造第二个 CNF 来检查是否存在第二解
		SATList* cnf2 = nullptr;
		sudokuToCNF(puzzle, cnf2);

		// 添加否定第一个解的约束
		SATList* negClause = (SATList*)malloc(sizeof(SATList));
		negClause->head = nullptr;
		negClause->next = nullptr;

		SATNode* negHead = nullptr;
		for (int row = 0; row < N; row++) {
			for (int col = 0; col < N; col++) {
				if (puzzle[row][col] == 0) {
					for (int num = 1; num <= 9; num++) {
						if (value1[varIndex(row, col, num)] == 1) {
							SATNode* node = (SATNode*)malloc(sizeof(SATNode));
							node->data = -varIndex(row, col, num);
							node->next = negHead;
							negHead = node;
							break;
						}
					}
				}
			}
		}

		if (negHead != nullptr) {
			negClause->head = negHead;
			addClause(negClause, cnf2);

			int* value2 = (int*)malloc(sizeof(int) * MAX_VAR);
			for (int k = 0; k < MAX_VAR; k++) value2[k] = 1;

			int result2 = DPLL(cnf2, value2);
			free(value2);

			if (result2 == 1) {
				// 有第二解，回滚
				puzzle[r][c] = backup;
			}
            else {
                // 唯一解，确认挖空
                removed++;
                cout << "Successfully removed position (" << r << ", " << c << "), removed " << removed << " cells" << endl;
            }
        }
        else {
            // 不需要否定的文字，解唯一
            removed++;
            cout << "Successfully removed position (" << r << ", " << c << "), removed " << removed << " cells" << endl;
        }

		destroyClause(cnf2);
		free(value1);
	}

    cout << "Final removal completed, removed " << removed << " cells, remaining hints " << (N * N - removed) << endl;
}

void printSudoku(int grid[N][N]) {
	for (int i = 0; i < N; i++) {
		if (i % 3 == 0 && i != 0) {
			cout << "------+-------+------" << endl;
		}
		for (int j = 0; j < N; j++) {
			if (j % 3 == 0 && j != 0) {
				cout << "| ";
			}
			// 用问号表示挖空的格子，数字表示已知格子
			cout << (grid[i][j] ? to_string(grid[i][j]) : "?") << " ";
		}
		cout << endl;
	}
	cout << endl;
}

int varIndex(int row, int col, int num) {
	return row * 81 + col * 9 + num; // 1~729
}

void sudokuToCNF(int puzzle[N][N], SATList*& cnf) {
	destroyClause(cnf); // 先清空
	cnf = nullptr;

	SATList* clRoot = nullptr;
	SATNode* node;

	// ---------- 格约束 ----------
	for (int i = 0; i < N; i++) {
		for (int j = 0; j < N; j++) {
			// 每格至少填一个数字
			SATList* cl = new SATList{ nullptr, nullptr };
			SATNode* head = nullptr;
			for (int num = 1; num <= 9; num++) {
				node = new SATNode{ varIndex(i, j, num), nullptr };
				node->next = head;
				head = node;
			}
			cl->head = head;
			addClause(cl, cnf);

			// 每格只能填一个数字
			for (int n1 = 1; n1 <= 8; n1++)
				for (int n2 = n1 + 1; n2 <= 9; n2++) {
					SATList* c = new SATList{ nullptr, nullptr };
					c->head = new SATNode{ -varIndex(i,j,n1), new SATNode{-varIndex(i,j,n2), nullptr} };
					addClause(c, cnf);
				}
		}
	}

	// ---------- 行约束 ----------
	for (int i = 0; i < N; i++) {
		for (int num = 1; num <= 9; num++) {
			// 每行至少有一个num
			SATList* cl = new SATList{ nullptr, nullptr };
			SATNode* head = nullptr;
			for (int j = 0; j < N; j++) {
				node = new SATNode{ varIndex(i,j,num), head };
				head = node;
			}
			cl->head = head;
			addClause(cl, cnf);

			// 每行num只出现一次
			for (int j1 = 0; j1 < N - 1; j1++)
				for (int j2 = j1 + 1; j2 < N; j2++) {
					SATList* c = new SATList{ nullptr, nullptr };
					c->head = new SATNode{ -varIndex(i,j1,num), new SATNode{-varIndex(i,j2,num), nullptr} };
					addClause(c, cnf);
				}
		}
	}

	// ---------- 列约束 ----------
	for (int j = 0; j < N; j++) {
		for (int num = 1; num <= 9; num++) {
			// 每列至少有一个num
			SATList* cl = new SATList{ nullptr, nullptr };
			SATNode* head = nullptr;
			for (int i = 0; i < N; i++) {
				node = new SATNode{ varIndex(i,j,num), head };
				head = node;
			}
			cl->head = head;
			addClause(cl, cnf);

			// 每列num只出现一次
			for (int i1 = 0; i1 < N - 1; i1++)
				for (int i2 = i1 + 1; i2 < N; i2++) {
					SATList* c = new SATList{ nullptr, nullptr };
					c->head = new SATNode{ -varIndex(i1,j,num), new SATNode{-varIndex(i2,j,num), nullptr} };
					addClause(c, cnf);
				}
		}
	}

	// ---------- 宫约束 ----------
	for (int boxRow = 0; boxRow < 3; boxRow++) {
		for (int boxCol = 0; boxCol < 3; boxCol++) {
			for (int num = 1; num <= 9; num++) {
				// 至少有一个num
				SATList* cl = new SATList{ nullptr, nullptr };
				SATNode* head = nullptr;
				for (int i = 0; i < 3; i++)
					for (int j = 0; j < 3; j++)
						head = new SATNode{ varIndex(boxRow * 3 + i, boxCol * 3 + j, num), head };
				cl->head = head;
				addClause(cl, cnf);

				// 只出现一次
				for (int i1 = 0; i1 < 3; i1++)
					for (int j1 = 0; j1 < 3; j1++)
						for (int i2 = 0; i2 < 3; i2++)
							for (int j2 = 0; j2 < 3; j2++) {
								if (i1 == i2 && j1 >= j2) continue;
								SATList* c = new SATList{ nullptr, nullptr };
								c->head = new SATNode{ -varIndex(boxRow * 3 + i1, boxCol * 3 + j1, num),
													   new SATNode{-varIndex(boxRow * 3 + i2, boxCol * 3 + j2, num), nullptr} };
								addClause(c, cnf);
							}
			}
		}
	}

	// ---------- 撇对角线约束 ----------
	for (int num = 1; num <= 9; num++) {
		// 主对角线
		for (int i1 = 0; i1 < N - 1; i1++)
			for (int i2 = i1 + 1; i2 < N; i2++) {
				SATList* c = new SATList{ nullptr, nullptr };
				c->head = new SATNode{ -varIndex(i1,i1,num), new SATNode{-varIndex(i2,i2,num), nullptr} };
				addClause(c, cnf);
			}
		// 副对角线
		/*for (int i1 = 0; i1 < N - 1; i1++)
			for (int i2 = i1 + 1; i2 < N; i2++) {
				SATList* c = new SATList{ nullptr, nullptr };
				c->head = new SATNode{ -varIndex(i1,N - 1 - i1,num), new SATNode{-varIndex(i2,N - 1 - i2,num), nullptr} };
				addClause(c, cnf);
			}*/
	}

	// ---------- 上下窗口约束 ----------
	int upper[9][2] = { {1,1},{1,2},{1,3},{2,1},{2,2},{2,3},{3,1},{3,2},{3,3} };
	int lower[9][2] = { {5,5},{5,6},{5,7},{6,5},{6,6},{6,7},{7,5},{7,6},{7,7} };
	for (int num = 1; num <= 9; num++) {
		// 上窗口
		for (int i = 0; i < 8; i++)
			for (int j = i + 1; j < 9; j++) {
				SATList* c = new SATList{ nullptr, nullptr };
				c->head = new SATNode{ -varIndex(upper[i][0], upper[i][1], num),
									   new SATNode{-varIndex(upper[j][0], upper[j][1], num), nullptr} };
				addClause(c, cnf);
			}
		// 下窗口
		for (int i = 0; i < 8; i++)
			for (int j = i + 1; j < 9; j++) {
				SATList* c = new SATList{ nullptr, nullptr };
				c->head = new SATNode{ -varIndex(lower[i][0], lower[i][1], num),
									   new SATNode{-varIndex(lower[j][0], lower[j][1], num), nullptr} };
				addClause(c, cnf);
			}
	}

	// ---------- 提示数字 ----------
	for (int i = 0; i < N; i++)
		for (int j = 0; j < N; j++)
			if (puzzle[i][j] != 0) {
				SATList* c = new SATList{ nullptr, nullptr };
				c->head = new SATNode{ varIndex(i,j,puzzle[i][j]), nullptr };
				addClause(c, cnf);
			}
}

bool check(int solved[9][9], int ans[9][9])
{
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            if (solved[i][j] != ans[i][j]) {
                return false;
            }
        }
    }
    return true;
}

#endif // HEAD_H