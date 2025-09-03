#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <cstdlib>

using namespace std;

struct Clause {
    vector<int> literals;
};

struct CNF {
    int numVars;
    int numClauses;
    vector<Clause> clauses;
};

struct Solution {
    bool satisfiable;
    vector<int> assignment; // assignment[i] = value of variable i (1-indexed)
    double time;
};

// 解析CNF文件
bool parseCNF(const string& filename, CNF& cnf) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "错误：无法打开CNF文件 " << filename << endl;
        return false;
    }
    
    string line;
    bool headerFound = false;
    
    while (getline(file, line)) {
        // 跳过注释行
        if (line.empty() || line[0] == 'c') {
            continue;
        }
        
        // 解析头部行
        if (line[0] == 'p') {
            istringstream iss(line);
            string p, cnf_str;
            iss >> p >> cnf_str >> cnf.numVars >> cnf.numClauses;
            headerFound = true;
            continue;
        }
        
        if (!headerFound) {
            continue;
        }
        
        // 解析子句
        istringstream iss(line);
        Clause clause;
        int literal;
        while (iss >> literal && literal != 0) {
            clause.literals.push_back(literal);
        }
        
        if (!clause.literals.empty()) {
            cnf.clauses.push_back(clause);
        }
    }
    
    file.close();
    return true;
}

// 解析RES文件
bool parseRES(const string& filename, Solution& solution) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "错误：无法打开RES文件 " << filename << endl;
        return false;
    }
    
    string line;
    solution.satisfiable = false;
    
    while (getline(file, line)) {
        if (line.empty()) continue;
        
        istringstream iss(line);
        char prefix;
        iss >> prefix;
        
        if (prefix == 's') {
            int result;
            iss >> result;
            solution.satisfiable = (result == 1);
        }
        else if (prefix == 'v' && solution.satisfiable) {
            // 解析变量赋值
            int var;
            while (iss >> var && var != 0) {
                int varIndex = abs(var);
                if (varIndex < solution.assignment.size()) {
                    solution.assignment[varIndex] = (var > 0) ? 1 : -1;
                }
            }
        }
        else if (prefix == 't') {
            iss >> solution.time;
        }
    }
    
    file.close();
    return true;
}

// 验证解是否满足CNF
bool verifySolution(const CNF& cnf, const Solution& solution) {
    if (!solution.satisfiable) {
        cout << "解标记为不可满足，无需验证赋值。" << endl;
        return true; // 我们无法验证不可满足的情况，只能相信求解器
    }
    
    // 检查赋值数组大小
    if (solution.assignment.size() <= cnf.numVars) {
        cerr << "错误：赋值数组大小不足" << endl;
        return false;
    }
    
    cout << "开始验证解..." << endl;
    cout << "变量数：" << cnf.numVars << endl;
    cout << "子句数：" << cnf.numClauses << endl;
    
    int satisfiedClauses = 0;
    vector<int> unsatisfiedClauses;
    
    // 检查每个子句
    for (int i = 0; i < cnf.clauses.size(); ++i) {
        const Clause& clause = cnf.clauses[i];
        bool clauseSatisfied = false;
        
        // 检查子句中的每个文字
        for (int literal : clause.literals) {
            int var = abs(literal);
            int value = solution.assignment[var];
            
            // 检查文字是否满足
            if ((literal > 0 && value == 1) || (literal < 0 && value == -1)) {
                clauseSatisfied = true;
                break;
            }
        }
        
        if (clauseSatisfied) {
            satisfiedClauses++;
        } else {
            unsatisfiedClauses.push_back(i);
        }
    }
    
    cout << "满足的子句数：" << satisfiedClauses << "/" << cnf.clauses.size() << endl;
    
    if (unsatisfiedClauses.empty()) {
        cout << "✓ 验证成功！所有子句都被满足。" << endl;
        return true;
    } else {
        cout << "✗ 验证失败！以下子句未被满足：" << endl;
        for (int i = 0; i < min(10, (int)unsatisfiedClauses.size()); ++i) {
            int clauseIndex = unsatisfiedClauses[i];
            cout << "子句 " << clauseIndex + 1 << ": ";
            for (int literal : cnf.clauses[clauseIndex].literals) {
                cout << literal << " ";
            }
            cout << "0" << endl;
            
            // 显示该子句中各文字的赋值情况
            cout << "  文字赋值：";
            for (int literal : cnf.clauses[clauseIndex].literals) {
                int var = abs(literal);
                int value = solution.assignment[var];
                cout << literal << "(" << value << ") ";
            }
            cout << endl;
        }
        
        if (unsatisfiedClauses.size() > 10) {
            cout << "... 还有 " << unsatisfiedClauses.size() - 10 << " 个子句未满足" << endl;
        }
        
        return false;
    }
}

// 显示部分赋值信息
void showAssignmentSample(const Solution& solution, int maxShow = 20) {
    if (!solution.satisfiable) return;
    
    cout << "变量赋值示例（前" << maxShow << "个变量）：" << endl;
    for (int i = 1; i <= min(maxShow, (int)solution.assignment.size() - 1); ++i) {
        if (solution.assignment[i] != 0) {
            cout << "x" << i << "=" << (solution.assignment[i] == 1 ? "true" : "false") << " ";
            if (i % 10 == 0) cout << endl;
        }
    }
    cout << endl;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        cout << "使用方法: " << argv[0] << " <cnf文件> <res文件>" << endl;
        cout << "例如: " << argv[0] << " problem.cnf problem.res" << endl;
        return 1;
    }
    
    string cnfFile = argv[1];
    string resFile = argv[2];
    
    cout << "=== SAT求解结果验证工具 ===" << endl;
    cout << "CNF文件: " << cnfFile << endl;
    cout << "RES文件: " << resFile << endl;
    cout << endl;
    
    // 解析CNF文件
    CNF cnf;
    if (!parseCNF(cnfFile, cnf)) {
        return 1;
    }
    
    // 解析RES文件
    Solution solution;
    solution.assignment.resize(cnf.numVars + 1, 0);
    if (!parseRES(resFile, solution)) {
        return 1;
    }
    
    cout << "求解结果: " << (solution.satisfiable ? "可满足" : "不可满足") << endl;
    cout << "求解时间: " << solution.time << "ms" << endl;
    cout << endl;
    
    // 显示赋值示例
    if (solution.satisfiable) {
        showAssignmentSample(solution);
    }
    
    // 验证解
    bool isValid = verifySolution(cnf, solution);
    
    cout << endl;
    cout << "=== 验证结果 ===" << endl;
    if (isValid) {
        cout << "✓ 解是正确的！" << endl;
        return 0;
    } else {
        cout << "✗ 解不正确或不完整！" << endl;
        return 1;
    }
}
