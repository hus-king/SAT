/**
 * @file main.cpp
 * @brief SAT求解器和百分号数独游戏的主程序
 * @author [Hus-King]
 * @date 2025-0            if (files.empty()) {
        cout << "❌ 未找到.cnf文件!\n";
        return -1;
    }
    
    cout << "\n╔══════════════════════════════════════════╗\n";
    cout << "║              📋 可用的CNF文件             ║\n";
    cout << "╠══════════════════════════════════════════╣\n"; * @version 1.0
 */

#include "common.h"
#include "cnf.h"
#include "sudoku.h"
#include "optimize_cnf.h"
#include "dualcore_cnf.h"
#include <dirent.h>
#include <iomanip>
#include <vector>
#include <string>
#include <algorithm>
#include <regex>
#include <map>

// ==================== 全局变量 ====================

// 用于存储每个文件的DPLL求解时间
std::map<std::string, double> dpllTimeHistory;

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
/**
 * @brief 计算字符串的显示宽度（考虑中文字符）
 * @param str 输入字符串
 * @return 显示宽度
 */
int getDisplayWidth(const std::string& str) {
    int width = 0;
    for (size_t i = 0; i < str.length(); i++) {
        unsigned char c = str[i];
        if (c < 128) {
            // ASCII字符，宽度为1
            width += 1;
        } else {
            // 多字节字符（如中文），宽度为2
            width += 2;
            // 跳过UTF-8的后续字节
            while (i + 1 < str.length() && (str[i + 1] & 0xC0) == 0x80) {
                i++;
            }
        }
    }
    return width;
}

int selectCnfFile(const std::vector<std::string>& files) {
    if (files.empty()) {
        cout << "❌ 未找到.cnf文件!\n";
        return -1;
    }
    
    cout << "\n╔══════════════════════════════════════════════════╗\n";
    cout << "║                📋 可用的CNF文件                  ║\n";
    cout << "╠══════════════════════════════════════════════════╣\n";
    for (size_t i = 0; i < files.size(); i++) {
        // 计算实际显示宽度
        int displayWidth = getDisplayWidth(files[i]);
        // 总宽度为48，减去边框、序号和点号占用的空间
        int paddingNeeded = 44 - displayWidth;
        
        cout << "║ " << std::setw(2) << std::right << (i + 1) << ". " 
             << files[i] << std::string(paddingNeeded, ' ') << " ║\n";
    }
    int cancelPadding = 44 - getDisplayWidth("取消");
    cout << "║ " << std::setw(2) << std::right << "0" << ". " 
         << "取消" << std::string(cancelPadding, ' ') << " ║\n";
    cout << "╚══════════════════════════════════════════════════╝\n";
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
        cout << "╔═══════════════════════════════════════════════════════════╗\n";
        cout << "║                  🧠 SAT求解器与数独游戏 🎯                ║\n";
        cout << "╠═══════════════════════════════════════════════════════════╣\n";
        cout << "║  📁 1. 读取CNF文件              🔢 2. 遍历输出读到的语句  ║\n";
        cout << "║  🔧 3. DPLL求解并保存           ⚡ 4. DPLL优化求解并保存  ║\n";
        cout << "║  🚀 5. DPLL双核优化             🎮 6. 生成数独            ║\n";
        cout << "║  🚪 0. 退出                                               ║\n";
        cout << "╚═══════════════════════════════════════════════════════════╝\n";
        cout << "请选择操作 [0~6]: ";
        cin >> op;

        switch (op) {
        case 1: {
            cout << "\n╔═══════════════════════════════════════════╗\n";
            cout << "║           📂 自动扫描CNF文件              ║\n";
            cout << "╚═══════════════════════════════════════════╝\n";
            
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
            
            cout << "\n╔═══════════════════════════════════════════╗\n";
            cout << "║             📖 正在读取文件               ║\n";
            cout << "╚═══════════════════════════════════════════╝\n";
            cout << "选择的文件: " << selectedFile << "\n";
            cout << "完整路径: " << fullPath << "\n";
            cout << "🔄 正在解析CNF格式...\n";
            
            if (ReadFile(CNFList)) {
                // 计算实际子句数
                int clauseCount = 0;
                for (SATList* lp = CNFList; lp != nullptr; lp = lp->next) {
                    clauseCount++;
                }
                
                cout << "\n╔═══════════════════════════════════════════╗\n";
                cout << "║             📊 文件解析结果               ║\n";
                cout << "╠═══════════════════════════════════════════╣\n";
                
                // 变量数行
                std::string varText = "变量数: " + std::to_string(boolCount);
                int varPadding = 44 - varText.length();
                cout << "║ " << varText << std::string(max(0, varPadding), ' ') << " ║\n";
                
                // 子句数行
                std::string clauseText = "子句数: " + std::to_string(clauseCount);
                int clausePadding = 44 - clauseText.length();
                cout << "║ " << clauseText << std::string(max(0, clausePadding), ' ') << " ║\n";
                
                cout << "╠═══════════════════════════════════════════╣\n";
                
                // CNF合法性检查
                bool isValid = true;
                std::string validationResult = "";
                
                if (boolCount <= 0) {
                    isValid = false;
                    validationResult = "❌ 错误: 变量数必须大于0";
                } else if (clauseCount <= 0) {
                    isValid = false;
                    validationResult = "❌ 错误: 子句数必须大于0";
                } else if (CNFList == nullptr) {
                    isValid = false;
                    validationResult = "❌ 错误: CNF子句列表为空";
                } else {
                    // 检查子句是否有效（至少包含一个文字）
                    bool hasEmptyClause = false;
                    for (SATList* lp = CNFList; lp != nullptr; lp = lp->next) {
                        if (lp->head == nullptr) {
                            hasEmptyClause = true;
                            break;
                        }
                    }
                    
                    if (hasEmptyClause) {
                        isValid = false;
                        validationResult = "❌ 错误: 发现空子句";
                    } else {
                        validationResult = "✅ CNF格式验证通过";
                    }
                }
                
                // 显示验证结果
                int validationPadding = 48 - validationResult.length() + (isValid ? 0 : 4); // emoji补偿
                cout << "║ " << validationResult << std::string(max(0, validationPadding), ' ') << " ║\n";
                cout << "╚═══════════════════════════════════════════╝\n";
                
                if (!isValid) {
                    cout << "\n⚠️  CNF文件格式不合法，无法进行SAT求解!\n";
                    cout << "请检查文件格式是否符合DIMACS标准。\n";
                    // 清理无效的CNF数据
                    destroyClause(CNFList);
                    CNFList = nullptr;
                }
                
            } else {
                cout << "\n╔═══════════════════════════════════════════╗\n";
                cout << "║             ❌ 文件读取失败               ║\n";
                cout << "╠═══════════════════════════════════════════╣\n";
                cout << "║ 可能的原因:                               ║\n";
                cout << "║ • 文件不存在或无法访问                    ║\n";
                cout << "║ • 文件格式不符合DIMACS标准                ║\n";
                cout << "║ • 文件内容损坏或为空                      ║\n";
                cout << "╚═══════════════════════════════════════════╝\n";
            }
            pauseProgram();
        } break;

        case 2: {
            if (CNFList == nullptr) {
                cout << "❌ 未加载文件!\n";
            }
            else {
                cout << "\n╔═══════════════════════════════════════════╗\n";
                cout << "║         📄 遍历输出CNF子句内容            ║\n";
                cout << "╚═══════════════════════════════════════════╝\n";
                
                // 计算子句数
                int clauseCount = 0;
                for (SATList* lp = CNFList; lp != nullptr; lp = lp->next) {
                    clauseCount++;
                }
                
                cout << "\n变量数: " << boolCount << "\n";
                cout << "子句数: " << clauseCount << "\n";
                cout << "\n╔═══════════════════════════════════════════╗\n";
                cout << "║               📋 子句详细内容             ║\n";
                cout << "╠═══════════════════════════════════════════╣\n";
                
                int currentClause = 1;
                for (SATList* lp = CNFList; lp != nullptr; lp = lp->next) {
                    cout << "║ 子句 " << std::setw(3) << currentClause << ": ";
                    
                    std::string clauseStr = "";
                    for (SATNode* np = lp->head; np != nullptr; np = np->next) {
                        clauseStr += std::to_string(np->data);
                        if (np->next != nullptr) {
                            clauseStr += " ";
                        }
                    }
                    clauseStr += " 0"; // DIMACS格式以0结尾
                    
                    // 控制显示长度，如果太长则截断
                    if (clauseStr.length() > 30) {
                        clauseStr = clauseStr.substr(0, 27) + "...";
                    }
                    
                    int padding = 30 - clauseStr.length();
                    cout << clauseStr << std::string(max(0, padding), ' ') << " ║\n";
                    
                    currentClause++;
                }
                
                cout << "╚═══════════════════════════════════════════╝\n";
                cout << "\n✅ 子句遍历完成! 共显示了 " << clauseCount << " 个子句。\n";
            }
            pauseProgram();
        } break;

        case 3: {
            if (CNFList == nullptr) {
                cout << "❌ 未加载文件!\n";
            }
            else {
                cout << "\n╔═══════════════════════════════════════════╗\n";
                cout << "║           🔧 DPLL算法求解SAT问题          ║\n";
                cout << "╚═══════════════════════════════════════════╝\n";
                
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
                cout << "\n╔═══════════════════════════════════════════╗\n";
                cout << "║             📊 DPLL求解结果               ║\n";
                cout << "╠═══════════════════════════════════════════╣\n";
                
                // 结果行
                std::string resultText = result ? "✅ SAT (可满足)" : "❌ UNSAT (不可满足)";
                int resultPadding = 43 - 4 - resultText.length(); // 41总宽度 - "结果: "4字符 - 结果文本长度
                cout << "║ 结果: " << resultText << std::string(resultPadding, ' ') << " ║\n";
                
                if (result == 1) {
                    cout << "║ 变量赋值: ";
                    std::string assignment = "";
                    for (i = 1; i <= min(8, boolCount); i++) {
                        assignment += std::to_string(value[i] == 1 ? i : -i) + " ";
                    }
                    if (boolCount > 8) assignment += "...";
                    
                    // 计算需要的填充空格，总宽度41，减去"变量赋值: "的长度(11)
                    int assignmentPadding = 42 - 11 - assignment.length();
                    cout << assignment << std::string(max(0, assignmentPadding), ' ') << " ║\n";
                }
                
                // 时间行
                std::string timeStr = std::to_string(timeElapsed * 1000) + " ms";
                int timePadding = 38 - 7 - timeStr.length(); // 41总宽度 - "求解时间: "7字符 - 时间文本长度
                cout << "║ 求解时间: " << timeStr << std::string(timePadding, ' ') << " ║\n";
                cout << "╚═══════════════════════════════════════════╝\n";

                // 保存结果
                if (WriteFile(result, timeElapsed, value) == 1) {
                    cout << "结果已保存到.res文件\n";
                }
                else {
                    cout << "保存结果失败\n";
                }
                
                // 记录当前文件的DPLL求解时间
                std::string currentFile(fileName);
                dpllTimeHistory[currentFile] = timeElapsed;
                
                free(value);
            }
            pauseProgram();
        } break;

        case 4: {
            if (CNFList == nullptr) {
                cout << "❌ 未加载文件!\n";
            }
            else {
                cout << "\n╔═══════════════════════════════════════════╗\n";
                cout << "║         ⚡ 优化DPLL算法求解SAT问题        ║\n";
                cout << "╠═══════════════════════════════════════════╣\n";
                cout << "║ 🚀 MOM启发式 + VSIDS + 纯文字消除         ║\n";
                cout << "║ 🔄 增量法高效回溯 + 哨兵监控机制          ║\n";
                cout << "╚═══════════════════════════════════════════╝\n";
                
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
                cout << "\n╔═══════════════════════════════════════════╗\n";
                cout << "║           📊 优化DPLL求解结果             ║\n";
                cout << "╠═══════════════════════════════════════════╣\n";
                
                // 结果行
                std::string resultText = result ? "✅ SAT (可满足)" : "❌ UNSAT (不可满足)";
                int resultPadding = 43 - 4 - resultText.length(); // 41总宽度 - "结果: "4字符 - 结果文本长度
                cout << "║ 结果: " << resultText << std::string(resultPadding, ' ') << " ║\n";
                
                if (result == 1) {
                    cout << "║ 变量赋值: ";
                    std::string assignment = "";
                    for (i = 1; i <= min(8, boolCount); i++) {
                        assignment += std::to_string(value[i] == 1 ? i : -i) + " ";
                    }
                    if (boolCount > 8) assignment += "...";
                    
                    // 计算需要的填充空格，总宽度41，减去"变量赋值: "的长度(11)
                    int assignmentPadding = 42 - 11 - assignment.length();
                    cout << assignment << std::string(max(0, assignmentPadding), ' ') << " ║\n";
                }
                
                // 时间行
                std::string timeStr = std::to_string(timeElapsed * 1000) + " ms";
                int timePadding = 42 - 15 - timeStr.length(); // 41总宽度 - "优化求解时间: "15字符 - 时间文本长度
                cout << "║ 优化求解时间: " << timeStr << std::string(timePadding, ' ') << " ║\n";
                
                // 检查是否有之前的DPLL求解时间记录，计算优化率
                std::string currentFile(fileName);
                if (dpllTimeHistory.find(currentFile) != dpllTimeHistory.end()) {
                    double previousTime = dpllTimeHistory[currentFile];
                    double improvementRatio = ((previousTime - timeElapsed) / previousTime) * 100;
                    
                    cout << "╠═══════════════════════════════════════════╣\n";
                    std::string improvementStr;
                    if (improvementRatio > 0) {
                        improvementStr = "🚀 性能提升: " + std::to_string((int)improvementRatio) + "%";
                    } else {
                        improvementStr = "⚠️  性能变化: " + std::to_string((int)abs(improvementRatio)) + "% (较慢)";
                    }
                    int improvementPadding = 43 - improvementStr.length() + 4; // +4 for emoji width compensation
                    cout << "║ " << improvementStr << std::string(max(0, improvementPadding), ' ') << " ║\n";
                    
                    std::string comparisonStr = "vs 普通DPLL: " + std::to_string(previousTime * 1000) + " ms";
                    int comparisonPadding = 43 - comparisonStr.length();
                    cout << "║ " << comparisonStr << std::string(max(0, comparisonPadding), ' ') << " ║\n";
                }
                
                cout << "╚═══════════════════════════════════════════╝\n";

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
        } break;

        case 5: {
            if (CNFList == nullptr) {
                cout << "❌ 未加载文件!\n";
            }
            else {
                cout << "\n╔═══════════════════════════════════════════╗\n";
                cout << "║        🚀 双核并行DPLL算法求解SAT问题     ║\n";
                cout << "╠═══════════════════════════════════════════╣\n";
                cout << "║ ⚡ 双核并行 + MOM启发式 + Jeroslow-Wang   ║\n";
                cout << "║ 🔄 纯文字消除 + 高效传播                  ║\n";
                cout << "╚═══════════════════════════════════════════╝\n";
                
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
                cout << "\n╔═══════════════════════════════════════════╗\n";
                cout << "║          📊 双核并行DPLL求解结果          ║\n";
                cout << "╠═══════════════════════════════════════════╣\n";
                
                // 结果行
                std::string resultText = result ? "✅ SAT (可满足)" : "❌ UNSAT (不可满足)";
                int resultPadding = 43 - 4 - resultText.length(); // 41总宽度 - "结果: "4字符 - 结果文本长度
                cout << "║ 结果: " << resultText << std::string(resultPadding, ' ') << " ║\n";
                
                if (result == 1) {
                    cout << "║ 变量赋值: ";
                    std::string assignment = "";
                    for (i = 1; i <= min(8, boolCount); i++) {
                        assignment += std::to_string(value[i] == 1 ? i : -i) + " ";
                    }
                    if (boolCount > 8) assignment += "...";
                    
                    // 计算需要的填充空格，总宽度41，减去"变量赋值: "的长度(11)
                    int assignmentPadding = 42 - 11 - assignment.length();
                    cout << assignment << std::string(max(0, assignmentPadding), ' ') << " ║\n";
                }
                
                // 时间行
                std::string timeStr = std::to_string(timeElapsed * 1000) + " ms";
                int timePadding = 40 - 17 - timeStr.length(); // 41总宽度 - "双核并行求解时间: "17字符 - 时间文本长度
                cout << "║ 双核并行求解时间: " << timeStr << std::string(timePadding, ' ') << " ║\n";
                cout << "╚═══════════════════════════════════════════╝\n";

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
        } break;

        case 6: {
            cout << "\n╔═══════════════════════════════════════════╗\n";
            cout << "║           🎮 数独游戏生成器               ║\n";
            cout << "╚═══════════════════════════════════════════╝\n";
            
            int full[N][N] = {};
            int puzzle[N][N] = {};

            // 生成完整数独
            cout << "🔄 正在生成完整数独...\n";
            if (!fillGrid(full)) {
                cout << "❌ 生成完整数独失败!\n";
                pauseProgram();
                break;
            }
            cout << "✅ 完整数独生成成功!\n";

            // 挖洞生成题目
            cout << "🔄 正在挖空生成谜题...\n";
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
            
            cout << "⏱️  DPLL求解时间: " << sudokuTime * 1000 << " ms\n\n";

            if (sudokuResult) {
                cout << "🎯 生成的数独题目:\n";
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
                
                // 创建用户操作的副本
                int userPuzzle[N][N];
                for (int i = 0; i < N; i++) {
                    for (int j = 0; j < N; j++) {
                        userPuzzle[i][j] = puzzle[i][j];
                    }
                }
                
                cout << "\n╔═══════════════════════════════════════════╗\n";
                cout << "║              🎮 交互式数独填空            ║\n";
                cout << "╠═══════════════════════════════════════════╣\n";
                cout << "║ 📝 输入格式: x y value                    ║\n";
                cout << "║ 📍 坐标范围: x,y ∈ [1,9]                  ║\n";
                cout << "║ 🔢 数值范围: value ∈ [1,9]                ║\n";
                cout << "║ 🚪 输入 0 0 0 退出游戏                    ║\n";
                cout << "║ 💡 输入 -1 -1 -1 查看答案                 ║\n";
                cout << "╚═══════════════════════════════════════════╝\n";
                
                while (true) {
                    cout << "\n当前数独状态:\n";
                    printSudoku(userPuzzle);
                    
                    cout << "\n请输入坐标和数值 (x y value): ";
                    int x, y, value;
                    cin >> x >> y >> value;
                    
                    // 退出游戏
                    if (x == 0 && y == 0 && value == 0) {
                        cout << "🚪 游戏结束!\n";
                        break;
                    }
                    
                    // 查看答案
                    if (x == -1 && y == -1 && value == -1) {
                        cout << "\n🔍 数独答案:\n";
                        printSudoku(solved);
                        continue;
                    }
                    
                    // 验证输入
                    if (x < 1 || x > 9 || y < 1 || y > 9) {
                        cout << "❌ 坐标超出范围! 请输入1-9之间的坐标。\n";
                        continue;
                    }
                    
                    if (value < 1 || value > 9) {
                        cout << "❌ 数值超出范围! 请输入1-9之间的数字。\n";
                        continue;
                    }
                    
                    // 转换为0基索引
                    int row = x - 1;
                    int col = y - 1;
                    
                    // 检查是否是固定的数字
                    if (puzzle[row][col] != 0) {
                        cout << "❌ 位置 (" << x << "," << y << ") 是题目给定的数字，不能修改!\n";
                        continue;
                    }
                    
                    // 填入数字
                    userPuzzle[row][col] = value;
                    cout << "✅ 已在位置 (" << x << "," << y << ") 填入数字 " << value << "\n";
                    
                    // 检查是否完成
                    bool isComplete = true;
                    for (int i = 0; i < N && isComplete; i++) {
                        for (int j = 0; j < N && isComplete; j++) {
                            if (userPuzzle[i][j] == 0) {
                                isComplete = false;
                            }
                        }
                    }
                    
                    if (isComplete) {
                        cout << "\n🎉 数独填写完成! 正在验证答案...\n";
                        if (check(solved, userPuzzle)) {
                            cout << "🏆 恭喜你! 答案完全正确! 🎊\n";
                            cout << "🌟 你成功解决了这个数独谜题! 🌟\n";
                        } else {
                            cout << "💔 很遗憾，答案不正确。请检查后重试。\n";
                            cout << "💡 提示: 输入 -1 -1 -1 可以查看正确答案。\n";
                        }
                        break;
                    }
                }
            }
            else {
                cout << "错误: 生成的题目无解!\n";
            }

            free(sudokuValue);
            pauseProgram();
        } break;


        case 0:
            cout << "\n╔═══════════════════════════════════════════╗\n";
            cout << "║      👋 感谢使用SAT求解器与数独游戏!      ║\n";
            cout << "║            🎉 期待下次再见! 🎉            ║\n";
            cout << "╚═══════════════════════════════════════════╝\n";
            break;
            
        default:
            cout << "❌ 无效选项! 请选择0-6.\n";
            pauseProgram();
            break;
        }
    }
    
    // 清理内存
    destroyClause(CNFList);
    return 0;
}


