#include <iostream>
#include <vector>
#include <string>
#include "Trie.h"
#include "SymbolTable.h"
#include "Logger.h"
#include "Utils.h"

void run_test(Analyzer& analyzer, const std::string& test_case, int case_num) {
    std::cout << "--- Test Case " << case_num << " ---" << std::endl;
    auto res = analyzer.processLine(test_case, 1);
    std::cout << "Original:  " << test_case << std::endl;
    std::cout << "Corrected: " << res.corrected << std::endl;
    if (!res.issues.empty()) {
        std::cout << "Issues:" << std::endl;
        for (const auto& iss : res.issues) {
            std::cout << "  - " << iss << std::endl;
        }
    }
    std::cout << std::endl;
}

int main() {
    Trie trie;
    SymbolTable sym;
    Logger logger;
    logger.init("output");
    Analyzer analyzer(trie, sym, logger);

    std::vector<std::string> tests = {
        "cout < \"test\";",
        "cout >> \"test\";",
        "cin > myVar;",
        "cin << myVar;",
        "// cout < \"ignore\";",
        "string s = \"cout <\";",
        "for(int i=0, i<5, i++)",
        "for(int i=0; i<max(a, b); i++)",
        "for(int i=0, j=0; i<10; i++)",
        "int x = 5",
        "if(x > 0)",
        "int main(){",
        "void func(){ {",
        "int mian()"
    };

    for (size_t i = 0; i < tests.size(); ++i) {
        run_test(analyzer, tests[i], i + 1);
    }

    return 0;
}
