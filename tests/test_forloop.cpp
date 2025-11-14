#include <iostream>
#include "Trie.h"
#include "SymbolTable.h"
#include "Logger.h"
#include "Utils.h"

int main(){
    Trie trie; SymbolTable sym; Logger logger;
    logger.init("output");
    Analyzer analyzer(trie, sym, logger);

    std::vector<std::string> tests = {
        "for(int i=0, i<5, i++)",
        "for(int i=0; i < max(a, b); i++)",
        "for (int i = 0, j = 0, i < 10, ++i)"
    };

    for (auto &t : tests){
        auto res = analyzer.processLine(t, 1);
        std::cout << "Original: " << t << std::endl;
        std::cout << "Corrected: " << res.corrected << std::endl;
        if (!res.issues.empty()){
            std::cout << "Issues:" << std::endl;
            for (auto &iss : res.issues) std::cout << "  - " << iss << std::endl;
        }
        std::cout << "---------------------------" << std::endl;
    }

    return 0;
}
