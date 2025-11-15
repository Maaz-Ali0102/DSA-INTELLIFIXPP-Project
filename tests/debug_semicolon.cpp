#include <iostream>
#include <vector>
#include "../src/Utils.h"
#include "../src/Trie.h"
#include "../src/SymbolTable.h"
#include "../src/Logger.h"
using namespace std;

int main() {
    Trie trie;
    trie.loadDefaultDictionary();
    SymbolTable symTable;
    Logger logger;
    Analyzer analyzer(trie, symTable, logger);
    
    vector<string> testLines = {
        "// This is a comment",
        "// cout < test",
        "int mian()"
    };
    
    for (const auto& line : testLines) {
        cout << "\n========================================" << endl;
        cout << "Testing: \"" << line << "\"" << endl;
        auto result = analyzer.processLine(line, 1);
        cout << "Result: \"" << result.corrected << "\"" << endl;
        cout << "Changed: " << (result.changed ? "YES" : "NO") << endl;
        cout << "Issues: ";
        if (result.issues.empty()) {
            cout << "(none)";
        } else {
            for (const auto& issue : result.issues) {
                cout << issue << "; ";
            }
        }
        cout << endl;
    }
    
    return 0;
}
