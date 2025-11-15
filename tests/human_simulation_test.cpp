// Human Simulation Test
// Tests Bug 1 (Context-Aware Typo Correction) and Bug 2 (Smart Tokenizer & For-Loop)
// Also checks for regressions

#include "../src/Utils.h"
#include "../src/Trie.h"
#include "../src/Logger.h"
#include "../src/SymbolTable.h"
#include <iostream>
#include <string>
#include <vector>
#include <cassert>

using namespace std;

int main() {
    cout << "\n========================================" << endl;
    cout << "   HUMAN SIMULATION TEST SUITE" << endl;
    cout << "========================================\n" << endl;
    
    // Initialize core components
    Trie trie;
    trie.loadDefaultDictionary();
    Logger logger;
    SymbolTable symTable;
    
    // Create single Analyzer for line-by-line tests
    Analyzer analyzer(trie, symTable, logger);
    
    // Failure counter
    int failCount = 0;
    int passCount = 0;
    int totalTests = 0;
    
    // Helper lambda for running tests
    auto runTest = [&](Analyzer& a, const string& input, const string& expected, const string& testName) {
        totalTests++;
        LineResult res = a.processLine(input, totalTests);
        
        if (res.corrected == expected) {
            cout << "✅ PASS: " << testName << endl;
            cout << "   Input:    \"" << input << "\"" << endl;
            cout << "   Output:   \"" << res.corrected << "\"" << endl;
            cout << "   Expected: \"" << expected << "\"" << endl << endl;
            passCount++;
        } else {
            cout << "❌ FAIL: " << testName << endl;
            cout << "   Input:    \"" << input << "\"" << endl;
            cout << "   Output:   \"" << res.corrected << "\"" << endl;
            cout << "   Expected: \"" << expected << "\"" << endl << endl;
            failCount++;
        }
    };
    
    // --- Test: Bug 1 (Context Typos) & Regressions ---
    cout << "--- Bug 1: Context-Aware Typo Correction ---\n" << endl;
    
    runTest(analyzer, "fr(i=0;)", "for(i=0;)", "H1 - 'fr' typo (PASS)");
    runTest(analyzer, "int nitmain()", "int main()", "H2 - 'nitmain' typo (SHOULD BE FIXED)");
    runTest(analyzer, "whle(true)", "while(true)", "H3 - 'whle' typo (KNOWN TYPO)");
    runTest(analyzer, "// check that main works", "// check that main works", "H4 - 'that' REGRESSION (Must NOT be 'char')");
    runTest(analyzer, "int i = 0", "int i = 0;", "H5 - 'i' REGRESSION (Must NOT be 'if')");
    runTest(analyzer, "notman", "notman;", "H6 - 'notman' REGRESSION (Must NOT be 'main')");
    
    // --- Test: Bug 2 (Tokenizer & Smart For-Loop) ---
    cout << "\n--- Bug 2: Smart Tokenizer & For-Loop ---\n" << endl;
    
    runTest(analyzer, "for(i=0i<ni++)", "for(i=0;i<ni;++)", "H7 - NoSpace 'for' (SEMICOLONS INSERTED)");
    runTest(analyzer, "for(i=0;i<ni++)", "for(i=0;i<ni;++)", "H8 - Missing one ';' (ONE SEMICOLON INSERTED)");
    runTest(analyzer, "intx=5", "intx=5;", "H9 - NoSpace Tokenizer ('intx' is valid identifier)");
    runTest(analyzer, "cout<<\"hi\"<<\"world\"", "cout<<\"hi\"<<\"world\";", "H10 - NoSpace Operators (WORKING)");
    runTest(analyzer, "if(x==0&&x>10)", "if(x==0&&x>10)", "H11 - NoSpace 'if' (WORKING)");
    
    // --- Test: File Mode (Multi-Line with State) ---
    cout << "\n--- File Mode: Multi-Line Processing ---\n" << endl;
    
    // Create fresh Analyzer for file mode
    Trie fileTrie;
    fileTrie.loadDefaultDictionary();
    Logger fileLogger;
    SymbolTable fileSymTable;
    Analyzer fileAnalyzer(fileTrie, fileSymTable, fileLogger);
    
    vector<string> fileInput = {
        "int x=5",
        "int mian(){",
        "fr(i=0i<5i++){",
        "cout<<\"test\"",
        "}"
    };
    
    vector<string> expectedOutput = {
        "int x=5;",
        "int main(){",
        "    for(i=0;i<5;i++){",
        "        cout<<\"test\";",
        "    }"
    };
    
    cout << "Processing file with " << fileInput.size() << " lines...\n" << endl;
    
    vector<LineResult> fileResults;
    for (size_t i = 0; i < fileInput.size(); i++) {
        LineResult res = fileAnalyzer.processLine(fileInput[i], i + 1);
        fileResults.push_back(res);
    }
    
    // Verify file output
    bool fileTestPass = true;
    for (size_t i = 0; i < fileResults.size(); i++) {
        if (fileResults[i].corrected != expectedOutput[i]) {
            cout << "❌ FAIL: Line " << (i + 1) << endl;
            cout << "   Input:    \"" << fileInput[i] << "\"" << endl;
            cout << "   Output:   \"" << fileResults[i].corrected << "\"" << endl;
            cout << "   Expected: \"" << expectedOutput[i] << "\"" << endl << endl;
            fileTestPass = false;
            failCount++;
        } else {
            cout << "✅ PASS: Line " << (i + 1) << endl;
            cout << "   Input:    \"" << fileInput[i] << "\"" << endl;
            cout << "   Output:   \"" << fileResults[i].corrected << "\"" << endl << endl;
            passCount++;
        }
    }
    
    totalTests += fileInput.size();
    
    if (fileTestPass) {
        cout << "✅ File Mode Test: ALL LINES PASSED!\n" << endl;
    } else {
        cout << "❌ File Mode Test: SOME LINES FAILED!\n" << endl;
    }
    
    // Final Summary
    cout << "\n========================================" << endl;
    cout << "         TEST SUMMARY" << endl;
    cout << "========================================" << endl;
    cout << "Total Tests:  " << totalTests << endl;
    cout << "Passed:       " << passCount << " ✅" << endl;
    cout << "Failed:       " << failCount << " ❌" << endl;
    cout << "Success Rate: " << (passCount * 100.0 / totalTests) << "%" << endl;
    
    if (failCount == 0) {
        cout << "\n🎉 ALL TESTS PASSED! PROJECT IS ROBUST! 🎉\n" << endl;
    } else {
        cout << "\n⚠️  SOME TESTS FAILED - REVIEW NEEDED ⚠️\n" << endl;
    }
    
    return (failCount == 0) ? 0 : 1;
}
