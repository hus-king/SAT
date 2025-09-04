/**
 * @file main.cpp
 * @brief SAT求解器和百分号数独游戏的主程序
 * @author [Hus-King]
 * @date 2025-09-03
 * @version 1.0
 */

#include "common.h"
#include "cnf.h"
#include "sudoku.h"
#include "optimize_cnf.h"
#include "dualcore_cnf.h"

// ==================== 主函数 ====================
int main() {
    // 初始化变量
    SATList* CNFList = nullptr;
    SATList* lp;
    SATNode* tp;
    srand((unsigned)time(0));
    clock_t start, finish;
    double timeElapsed;
    int op = 1, i, result;
    int* value;

    while (op) {
        clearScreen();
        
        // 显示主菜单
        cout << "\n\n";
        cout << "              SAT求解器与数独游戏\n";
        cout << "-------------------------------------------------\n";
        cout << "1. 读取CNF文件              2. 显示子句\n";
        cout << "3. DPLL求解并保存           4. DPLL优化求解并保存\n";
        cout << "5. DPLL双核优化             6. 生成数独\n";
        cout << "0. 退出\n";
        cout << "-------------------------------------------------\n";
        cout << "请选择操作 [0~6]: ";
        cin >> op;

        switch (op) {
        case 1:
            cout << "请输入CNF文件名: ";
            cin >> fileName;
            if (ReadFile(CNFList)) {
                cout << "文件加载成功!\n";
            }
            pauseProgram();
            break;

        case 2:
            if (CNFList == nullptr) {
                cout << "未加载文件!\n";
            }
            else {
                cout << "CNF子句:\n";
                int clauseNum = 1;
                for (lp = CNFList; lp != nullptr; lp = lp->next) {
                    cout << "子句 " << clauseNum++ << ": ";
                    for (tp = lp->head; tp != nullptr; tp = tp->next) {
                        cout << tp->data << " ";
                    }
                    cout << "0\n";  // CNF格式以0结尾
                }
            }
            pauseProgram();
            break;

        case 3:
            if (CNFList == nullptr) {
                cout << "未加载文件!\n";
            }
            else {
                cout << "使用DPLL算法求解SAT问题...\n";
                
                // 分配内存并初始化变量赋值
                value = (int*)malloc(sizeof(int) * (boolCount + 1));
                if (value == nullptr) {
                    cout << "内存分配失败!\n";
                    break;
                }
                
                for (i = 1; i <= boolCount; i++) value[i] = 1;

                // 计时求解
                start = clock();
                result = DPLL(CNFList, value);
                finish = clock();
                
                timeElapsed = (double)(finish - start) / CLOCKS_PER_SEC;
                
                // 显示结果
                cout << "\n=== DPLL求解结果 ===\n";
                cout << "结果: " << (result ? "SAT" : "UNSAT") << endl;
                
                if (result == 1) {
                    cout << "变量赋值: ";
                    for (i = 1; i <= boolCount; i++) {
                        cout << (value[i] == 1 ? i : -i) << " ";
                    }
                    cout << "\n";
                }
                
                cout << "求解时间: " << timeElapsed * 1000 << " ms\n";

                // 保存结果
                if (WriteFile(result, timeElapsed, value) == 1) {
                    cout << "结果已保存到.res文件\n";
                }
                else {
                    cout << "保存结果失败\n";
                }
                
                free(value);
            }
            pauseProgram();
            break;

        case 4:
            if (CNFList == nullptr) {
                cout << "未加载文件!\n";
            }
            else {
                cout << "使用优化DPLL算法求解SAT问题...\n";
                cout << "优化特性: MOM启发式 + Jeroslow-Wang + 纯文字消除 + 高效传播\n";
                
                // 分配内存并初始化变量赋值
                value = (int*)malloc(sizeof(int) * (boolCount + 1));
                if (value == nullptr) {
                    cout << "内存分配失败!\n";
                    break;
                }
                
                for (i = 1; i <= boolCount; i++) value[i] = 1;

                // 计时求解
                start = clock();
                result = DPLL_Optimized(CNFList, value);
                finish = clock();
                
                timeElapsed = (double)(finish - start) / CLOCKS_PER_SEC;
                
                // 显示结果
                cout << "\n=== 优化DPLL求解结果 ===\n";
                cout << "结果: " << (result ? "SAT" : "UNSAT") << endl;
                
                if (result == 1) {
                    cout << "变量赋值: ";
                    for (i = 1; i <= boolCount; i++) {
                        cout << (value[i] == 1 ? i : -i) << " ";
                    }
                    cout << "\n";
                }
                
                cout << "优化求解时间: " << timeElapsed * 1000 << " ms\n";

                // 保存结果
                if (WriteFile(result, timeElapsed, value) == 1) {
                    cout << "结果已保存到.res文件\n";
                }
                else {
                    cout << "保存结果失败\n";
                }
                
                free(value);
            }
            pauseProgram();
            break;

        case 5:
            if (CNFList == nullptr) {
                cout << "未加载文件!\n";
            }
            else {
                cout << "使用双核并行DPLL算法求解SAT问题...\n";
                cout << "优化特性: 双核并行 + MOM启发式 + Jeroslow-Wang + 纯文字消除 + 高效传播\n";
                
                // 分配内存并初始化变量赋值
                value = (int*)malloc(sizeof(int) * (boolCount + 1));
                if (value == nullptr) {
                    cout << "内存分配失败!\n";
                    break;
                }
                
                for (i = 1; i <= boolCount; i++) value[i] = 1;

                // 计时求解
                start = clock();
                result = DPLL_DualCore(CNFList, value);
                finish = clock();
                
                timeElapsed = (double)(finish - start) / CLOCKS_PER_SEC;
                
                // 显示结果
                cout << "\n=== 双核并行DPLL求解结果 ===\n";
                cout << "结果: " << (result ? "SAT" : "UNSAT") << endl;
                
                if (result == 1) {
                    cout << "变量赋值: ";
                    for (i = 1; i <= boolCount; i++) {
                        cout << (value[i] == 1 ? i : -i) << " ";
                    }
                    cout << "\n";
                }
                
                cout << "双核并行求解时间: " << timeElapsed * 1000 << " ms\n";

                // 保存结果
                if (WriteFile(result, timeElapsed, value) == 1) {
                    cout << "结果已保存到.res文件\n";
                }
                else {
                    cout << "保存结果失败\n";
                }
                
                free(value);
            }
            pauseProgram();
            break;

        case 6: {
            cout << "=== 数独游戏生成器 ===\n";
            
            int full[N][N] = {};
            int puzzle[N][N] = {};

            // 生成完整数独
            cout << "正在生成完整数独...\n";
            if (!fillGrid(full)) {
                cout << "生成完整数独失败!\n";
                pauseProgram();
                break;
            }
            cout << "完整数独生成成功!\n";

            // 挖洞生成题目
            cout << "正在挖空生成谜题...\n";
            generatePuzzle(full, puzzle, 35);

            // 验证题目是否有解
            destroyClause(CNFList);
            sudokuToCNF(puzzle, CNFList);

            int* sudokuValue = (int*)malloc(sizeof(int) * MAX_VAR);
            if (sudokuValue == nullptr) {
                cout << "内存分配失败!\n";
                pauseProgram();
                break;
            }
            
            for (int i = 0; i < MAX_VAR; i++) sudokuValue[i] = 1;

            clock_t sudokuStart = clock();
            int sudokuResult = DPLL(CNFList, sudokuValue);
            clock_t sudokuFinish = clock();
            double sudokuTime = (double)(sudokuFinish - sudokuStart) / CLOCKS_PER_SEC;
            
            cout << "DPLL求解时间: " << sudokuTime * 1000 << " ms\n\n";

            if (sudokuResult) {
                cout << "生成的数独题目:\n";
                printSudoku(puzzle);

                // 构造解答
                int solved[N][N] = {};
                for (int row = 0; row < N; row++) {
                    for (int col = 0; col < N; col++) {
                        if (puzzle[row][col] != 0) {
                            solved[row][col] = puzzle[row][col];
                        }
                        else {
                            for (int num = 1; num <= 9; num++) {
                                if (sudokuValue[varIndex(row, col, num)] == 1) {
                                    solved[row][col] = num;
                                    break;
                                }
                            }
                        }
                    }
                }
                
                cout << "是否直接查看答案? (y/n): ";
                char choice;
                cin >> choice;
                
                if (choice == 'n' || choice == 'N') {
                    cout << "请输入你的解答 (9x9个数字):\n";
                    int ans[9][9];
                    for (int i = 0; i < 9; i++) {
                        for (int j = 0; j < 9; j++) {
                            cin >> ans[i][j];
                        }
                    }
                    
                    if (check(solved, ans)) {
                        cout << "答案正确! 棒极了! (*´◡`*)\n";
                    }
                    else {
                        cout << "答案错误! (┬┬﹏┬┬)\n";
                    }
                }
                
                cout << "\n数独答案:\n";
                printSudoku(solved);
            }
            else {
                cout << "错误: 生成的题目无解!\n";
            }

            free(sudokuValue);
            pauseProgram();
        } break;


        case 0:
            cout << "感谢使用SAT求解器与数独游戏!\n";
            break;
            
        default:
            cout << "无效选项! 请选择0-6.\n";
            pauseProgram();
            break;
        }
    }
    
    // 清理内存
    destroyClause(CNFList);
    return 0;
}


