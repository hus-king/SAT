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
#include <dirent.h>
#include <vector>
#include <string>
#include <algorithm>
#include <regex>

// ==================== 辅助函数 ====================

/**
 * @brief 从文件名中提取数字用于排序
 * @param filename 文件名
 * @return 提取的数字，如果没有数字则返回999999
 */
int extractNumber(const std::string& filename) {
    std::regex numberRegex(R"((\d+))");
    std::smatch match;
    
    if (std::regex_search(filename, match, numberRegex)) {
        return std::stoi(match[1].str());
    }
    return 999999; // 没有数字的文件排在最后
}

/**
 * @brief 自定义比较函数，按文件名中的数字排序
 * @param a 文件名a
 * @param b 文件名b
 * @return 如果a应该排在b前面则返回true
 */
bool compareByNumber(const std::string& a, const std::string& b) {
    int numA = extractNumber(a);
    int numB = extractNumber(b);
    
    if (numA != numB) {
        return numA < numB;
    }
    
    // 如果数字相同，按字母顺序排序
    return a < b;
}

/**
 * @brief 获取指定目录下的所有.cnf文件
 * @param directory 目录路径
 * @return 文件名列表（按数字排序）
 */
std::vector<std::string> getCnfFiles(const std::string& directory) {
    std::vector<std::string> cnfFiles;
    DIR* dir = opendir(directory.c_str());
    
    if (dir == nullptr) {
        cout << "无法打开目录: " << directory << endl;
        return cnfFiles;
    }
    
    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        std::string filename = entry->d_name;
        // 检查文件是否以.cnf结尾
        if (filename.length() > 4 && 
            filename.substr(filename.length() - 4) == ".cnf") {
            cnfFiles.push_back(filename);
        }
    }
    
    closedir(dir);
    
    // 按数字排序
    std::sort(cnfFiles.begin(), cnfFiles.end(), compareByNumber);
    
    return cnfFiles;
}

/**
 * @brief 显示文件列表并让用户选择
 * @param files 文件列表
 * @return 选择的文件索引，-1表示取消
 */
int selectCnfFile(const std::vector<std::string>& files) {
    if (files.empty()) {
        cout << "未找到.cnf文件!\n";
        return -1;
    }
    
    cout << "\n=== 可用的CNF文件 ===\n";
    for (size_t i = 0; i < files.size(); i++) {
        cout << i + 1 << ". " << files[i] << "\n";
    }
    cout << "0. 取消\n";
    cout << "请选择文件序号 [0~" << files.size() << "]: ";
    
    int choice;
    cin >> choice;
    
    if (choice == 0) {
        return -1;  // 取消
    }
    
    if (choice < 1 || choice > (int)files.size()) {
        cout << "无效选择!\n";
        return -1;
    }
    
    return choice - 1;  // 返回0基索引
}

// ==================== 主函数 ====================
int main() {
    // 初始化变量
    SATList* CNFList = nullptr;
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
        cout << "1. 读取CNF文件              2. DPLL求解并保存\n";
        cout << "3. DPLL优化求解并保存       4. DPLL双核优化\n";
        cout << "5. 生成数独                 0. 退出\n";
        cout << "-------------------------------------------------\n";
        cout << "请选择操作 [0~5]: ";
        cin >> op;

        switch (op) {
        case 1: {
            cout << "=== 自动扫描CNF文件 ===\n";
            
            // 扫描config目录下的.cnf文件
            std::string configDir = "/Users/hesiqi/SAT/cnf";
            std::vector<std::string> cnfFiles = getCnfFiles(configDir);
            
            // 让用户选择文件
            int selectedIndex = selectCnfFile(cnfFiles);
            if (selectedIndex == -1) {
                cout << "已取消文件选择\n";
                pauseProgram();
                break;
            }
            
            // 构造完整文件路径
            std::string selectedFile = cnfFiles[selectedIndex];
            std::string fullPath = configDir + "/" + selectedFile;
            
            // 复制到全局fileName变量（假设它是char数组）
            strncpy(fileName, fullPath.c_str(), sizeof(fileName) - 1);
            fileName[sizeof(fileName) - 1] = '\0';
            
            cout << "选择的文件: " << selectedFile << "\n";
            cout << "完整路径: " << fullPath << "\n";
            
            if (ReadFile(CNFList)) {
                cout << "文件加载成功!\n";
                cout << "变量数: " << boolCount << "\n";
                cout << "子句数: ";
                int clauseCount = 0;
                for (SATList* lp = CNFList; lp != nullptr; lp = lp->next) {
                    clauseCount++;
                }
                cout << clauseCount << "\n";
            } else {
                cout << "文件加载失败!\n";
            }
            pauseProgram();
        } break;

        case 2:
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

        case 3:
            if (CNFList == nullptr) {
                cout << "未加载文件!\n";
            }
            else {
                cout << "使用优化DPLL算法求解SAT问题...\n";
                cout << "优化特性: MOM启发式 + VSIDS + 纯文字消除 + 增量法高效回溯 + 哨兵监控机制 + \n";
                
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

        case 4:
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

        case 5: {
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
            cout << "无效选项! 请选择0-5.\n";
            pauseProgram();
            break;
        }
    }
    
    // 清理内存
    destroyClause(CNFList);
    return 0;
}


