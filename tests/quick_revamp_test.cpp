// Quick test for revamp
#include "../src/Utils.h"
#include "../src/Trie.h"
#include "../src/Logger.h"
#include "../src/SymbolTable.h"
#include <iostream>

using namespace std;

int main() {
    Trie trie;
    trie.loadDefaultDictionary();
    Logger logger;
    SymbolTable symTable;
    Analyzer analyzer(trie, symTable, logger);
    
    cout << "\n=== REVAMP QUICK TEST ===\n" << endl;
    
    // Test 1: fo(...) typo
    auto r1 = analyzer.processLine("fo(i=0;i<5;i++)", 1);
    cout << "Test 1 - fo(...) typo:" << endl;
    cout << "  Input:  fo(i=0;i<5;i++)" << endl;
    cout << "  Output: " << r1.corrected << endl;
    cout << "  Expected: for(i=0;i<5;i++)" << endl << endl;
    
    // Test 2: retun typo
    auto r2 = analyzer.processLine("retun 0", 2);
    cout << "Test 2 - retun typo:" << endl;
    cout << "  Input:  retun 0" << endl;
    cout << "  Output: " << r2.corrected << endl;
    cout << "  Expected: return 0;" << endl << endl;
    
    // Test 3: namspace typo
    auto r3 = analyzer.processLine("namspace std", 3);
    cout << "Test 3 - namspace typo:" << endl;
    cout << "  Input:  namspace std" << endl;
    cout << "  Output: " << r3.corrected << endl;
    cout << "  Expected: namespace std;" << endl << endl;
    
    // Test 4: ni should NOT change (short variable)
    auto r4 = analyzer.processLine("int ni=5", 4);
    cout << "Test 4 - ni stays ni:" << endl;
    cout << "  Input:  int ni=5" << endl;
    cout << "  Output: " << r4.corrected << endl;
    cout << "  Expected: int ni=5;" << endl << endl;
    
    return 0;
}
