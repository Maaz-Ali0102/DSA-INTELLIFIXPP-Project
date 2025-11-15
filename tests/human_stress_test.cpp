// Human Stress Test
// Tests the TRUE ROBUSTNESS of the engine by combining multiple error types
// (spacing, typos, operators, missing symbols) in both Line-by-Line and File modes

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
    cout << "      HUMAN STRESS TEST SUITE" << endl;
    cout << "   Testing Combined Error Scenarios" << endl;
    cout << "========================================\n" << endl;
    
    // Initialize core components
    Trie trie;
    trie.loadDefaultDictionary();
    Logger logger;
    SymbolTable symTable;
    
    // Failure counter
    int failCount = 0;
    int passCount = 0;
    int totalTests = 0;
    
    // Helper lambda for running tests
    auto runTest = [&](Analyzer& analyzer, const string& input, const string& expected, const string& testName) {
        totalTests++;
        LineResult res = analyzer.processLine(input, totalTests);
        
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
    
    // --- Test: Line-by-Line Mode (Human Stress-Test) ---
    cout << "--- Line-by-Line Mode: Stateful Processing ---\n" << endl;
    
    // Create single Analyzer for line-by-line tests (maintains state)
    Analyzer lineAnalyzer(trie, symTable, logger);
    
    // Test 1: No-space Tokenizer + Declaration
    runTest(lineAnalyzer, "intx=5", "intx=5;", "H1 - NoSpace Decl ('intx' is valid identifier)");
    
    // Test 2: The "nitmain" context bug
    runTest(lineAnalyzer, "nitmain(){", "main(){", "H2 - 'nitmain' Fixed (known typo)");
    
    // Test 3: Combined No-Space + Typo + Missing Semicolons
    runTest(lineAnalyzer, "fr(i=0i<5i++){", "    for(i=0;i<5;i++){", "H3 - Combined 'for' Mess + Indent");
    
    // Test 4: Combined Typo + Operator + State (Indent)
    runTest(lineAnalyzer, "cot<<\"test\"", "        cout<<\"test\";", "H4 - Combined 'cout' + Double Indent");
    
    // Test 5: State (Close Brace)
    runTest(lineAnalyzer, "}", "    }", "H5 - CloseBrace (with indent)");
    
    // Test 6: English Regression Test (comment gets indented due to state)
    runTest(lineAnalyzer, "// check that main works", "    // check that main works", "H6 - Comment with State Indent");
    
    // --- Test: File Mode (The "A-to-Z" Messy File) ---
    cout << "\n--- File Mode: Complex Multi-Error Scenarios ---\n" << endl;
    
    // Create fresh Analyzer for file mode (tests state reset)
    Trie fileTrie;
    fileTrie.loadDefaultDictionary();
    Logger fileLogger;
    SymbolTable fileSymTable;
    Analyzer fileAnalyzer(fileTrie, fileSymTable, fileLogger);
    
    // Combined error scenarios on each line
    vector<string> fileInput = {
        "#incldue<iotream>",          // Typo in preprocessor + typo in header
        "intx=5",                      // No-space identifier (intx is valid)
        "int mian(){",                 // Known typo 'mian'
        "fr(i=0i<5i++){",             // Typo 'fr' + no-space + missing semicolons
        "cot<<\"test\"",              // Typo 'cot' + no-space operators
        "}"                            // Close brace
    };
    
    vector<string> expectedOutput = {
        "#include<iostream>",          // Fixed preprocessor + header
        "intx=5;",                     // Added semicolon ('intx' stays as valid identifier)
        "int main(){",                 // Fixed 'mian' -> 'main'
        "    for(i=0;i<5;i++){",      // Fixed all: typo + semicolons + indent
        "        cout<<\"test\";",     // Fixed typo + added semicolon + double indent
        "    }"                        // Closing brace with indent
    };
    
    cout << "Processing file with " << fileInput.size() << " lines...\n" << endl;
    
    // Process each line through the analyzer
    vector<LineResult> fileResults;
    for (size_t i = 0; i < fileInput.size(); i++) {
        LineResult res = fileAnalyzer.processLine(fileInput[i], i + 1);
        fileResults.push_back(res);
    }
    
    // Verify file output
    bool fileTestPass = true;
    for (size_t i = 0; i < fileResults.size(); i++) {
        totalTests++;
        if (fileResults[i].corrected != expectedOutput[i]) {
            cout << "❌ FAIL: File Line " << (i + 1) << endl;
            cout << "   Input:    \"" << fileInput[i] << "\"" << endl;
            cout << "   Output:   \"" << fileResults[i].corrected << "\"" << endl;
            cout << "   Expected: \"" << expectedOutput[i] << "\"" << endl << endl;
            fileTestPass = false;
            failCount++;
        } else {
            cout << "✅ PASS: File Line " << (i + 1) << endl;
            cout << "   Input:    \"" << fileInput[i] << "\"" << endl;
            cout << "   Output:   \"" << fileResults[i].corrected << "\"" << endl << endl;
            passCount++;
        }
    }
    
    if (fileTestPass) {
        cout << "✅ File Mode Test: ALL LINES PASSED!\n" << endl;
    } else {
        cout << "❌ File Mode Test: SOME LINES FAILED!\n" << endl;
    }
    
    // --- Test: Ultra-Messy Combined Line ---
    cout << "\n--- Ultra-Stress Test: Everything Wrong at Once ---\n" << endl;
    
    Trie ultraTrie;
    ultraTrie.loadDefaultDictionary();
    Logger ultraLogger;
    SymbolTable ultraSymTable;
    Analyzer ultraAnalyzer(ultraTrie, ultraSymTable, ultraLogger);
    
    // The most messed up line possible ('inti' stays as one identifier)
    runTest(ultraAnalyzer, 
        "fr(inti=0i<5i++)cot<<i", 
        "for(inti=0;i<5;i++)cout<<i;", 
        "ULTRA - Everything Wrong (typos + no-space + missing semicolons)");
    
    // Final Summary
    cout << "\n========================================" << endl;
    cout << "         STRESS TEST SUMMARY" << endl;
    cout << "========================================" << endl;
    cout << "Total Tests:  " << totalTests << endl;
    cout << "Passed:       " << passCount << " ✅" << endl;
    cout << "Failed:       " << failCount << " ❌" << endl;
    cout << "Success Rate: " << (totalTests > 0 ? (passCount * 100.0 / totalTests) : 0) << "%" << endl;
    
    if (failCount == 0) {
        cout << "\n🎉 ALL STRESS TESTS PASSED! ENGINE IS BULLETPROOF! 🎉\n" << endl;
    } else {
        cout << "\n⚠️  SOME STRESS TESTS FAILED - NEEDS HARDENING ⚠️\n" << endl;
    }
    
    return (failCount == 0) ? 0 : 1;
}
