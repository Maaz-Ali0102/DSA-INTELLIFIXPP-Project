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
    
    string testLine = "for(int i=0, i<10, i++) cout < i";
    cout << "Testing: \"" << testLine << "\"" << endl;
    auto result = analyzer.processLine(testLine, 1);
    cout << "Result: \"" << result.corrected << "\"" << endl;
    cout << "Expected: \"for(int i=0; i<10; i++) cout << i;\"" << endl;
    cout << "Match: " << (result.corrected == "for(int i=0; i<10; i++) cout << i;" ? "YES" : "NO") << endl;
    cout << "Issues: ";
    for (const auto& issue : result.issues) {
        cout << issue << "; ";
    }
    cout << endl;
    
    return 0;
}
