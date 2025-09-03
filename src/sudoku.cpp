#include "sudoku.h"

// 百分号数独特殊窗口位置定义
int upperWindow[9][2] = { {1,1},{1,2},{1,3},{2,1},{2,2},{2,3},{3,1},{3,2},{3,3} };
int lowerWindow[9][2] = { {5,5},{5,6},{5,7},{6,5},{6,6},{6,7},{7,5},{7,6},{7,7} };

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
/**
 * @brief 计算数独谜题的解的数量
 * @param puzzle 数独谜题，0表示空格
 * @param maxSolutions 最大解数限制，默认为2
 * @return 解的数量（最多计算到maxSolutions个）
 * @note 该函数使用手写栈实现的搜索算法，避免了递归调用可能导致的栈溢出
 */
int countSolutions(int puzzle[N][N], int maxSolutions) {
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