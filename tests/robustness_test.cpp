#include <iostream>
#include <string>
#include <vector>
#include <cassert>
#include "../src/Utils.h"
#include "../src/Trie.h"
#include "../src/SymbolTable.h"
#include "../src/Logger.h"

using namespace std;

int main() {
    cout << "=== Robustness Test Suite ===" << endl;
    cout << "Testing #include typos, priority fixes, and short keyword corrections\n" << endl;
    
    // Create core components
    Trie trie;
    trie.loadDefaultDictionary();
    SymbolTable sym;
    Logger logger;
    
    // Create ONE Analyzer object for all tests (line-by-line mode)
    Analyzer analyzer(trie, sym, logger);
    
    int totalFailures = 0;
    
    // Helper lambda function for tests
    auto runTest = [&totalFailures](Analyzer& analyzer, const string& input, const string& expectedOutput, const string& testName) -> bool {
        auto result = analyzer.processLine(input, 1);
        bool passed = (result.corrected == expectedOutput);
        
        if (passed) {
            cout << "[PASS] " << testName << ": " << input << endl;
        } else {
            cout << "[FAIL] " << testName << endl;
            cout << "  Input:    " << input << endl;
            cout << "  Expected: " << expectedOutput << endl;
            cout << "  Got:      " << result.corrected << endl;
            if (!result.issues.empty()) {
                cout << "  Issues:   ";
                for (const auto& issue : result.issues) {
                    cout << issue << "; ";
                }
                cout << endl;
            }
            totalFailures++;
        }
        return passed;
    };
    
    // ========================================
    // Test: #include Typo, Priority, Semicolon
    // ========================================
    cout << "// --- Test: #include Typo, Priority, Semicolon ---" << endl;
    
    // Test missing # with typo in "include" and typo in "iostream"
    // Should fix: add #, fix "incldue" -> "include", fix "istream" -> "iostream"
    // Should NOT add semicolon (preprocessor line)
    // Note: No space in original, so output preserves that
    runTest(analyzer, "incldue<istream>", "#include<iostream>", "TC-2.1");
    
    // Test # present but typo in "include" and "iostream"
    // Should fix: "inclde" -> "include", "iosteam" -> "iostream"
    // Should NOT add semicolon
    runTest(analyzer, "#inclde <iosteam>", "#include <iostream>", "TC-2.2");
    
    // Test # with space, typo in "include" and using "stream" (not iostream)
    // Should fix: "incld" -> "include"
    // Should NOT change "stream" (it's valid)
    // Should NOT add semicolon
    runTest(analyzer, "# incld <stream>", "# include <stream>", "TC-2.3");
    
    cout << endl;
    
    // ========================================
    // Test: 'for' Loop Short Typo
    // ========================================
    cout << "// --- Test: 'for' Loop Short Typo ---" << endl;
    
    // Test short keyword typo "fr" followed by "("
    // Should fix: "fr" -> "for"
    // Should NOT add semicolon (ends with ))
    runTest(analyzer, "fr(i=0; i<5; i++)", "for(i=0; i<5; i++)", "TC-2.4");
    
    // Test short keyword typo "fr" with space before "("
    // Should fix: "fr" -> "for"
    // Should NOT add semicolon (ends with ))
    runTest(analyzer, "fr (i=0; i<5; i++)", "for (i=0; i<5; i++)", "TC-2.5");
    
    cout << endl;
    
    // ========================================
    // Test: Regression (Short Identifier)
    // ========================================
    cout << "// --- Test: Regression (Short Identifier) ---" << endl;
    
    // Test that short variable names like "i" are NOT changed to "if"
    // Should NOT change "i" (it's a variable, not followed by "(")
    // SHOULD add semicolon (statement)
    runTest(analyzer, "int i = 0", "int i = 0;", "TC-2.6");
    
    // Test short variable in assignment (no parenthesis)
    // Should NOT change "i" 
    runTest(analyzer, "i = 5", "i = 5;", "TC-2.7");
    
    // Test short variable in expression
    // Should NOT change "x" or "y"
    runTest(analyzer, "x = y + 1", "x = y + 1;", "TC-2.8");
    
    cout << endl;
    
    // ========================================
    // Test: Additional Edge Cases
    // ========================================
    cout << "// --- Test: Additional Edge Cases ---" << endl;
    
    // Test "if" typo (2 chars) followed by "("
    // Should fix: "fi" -> "if" (if Trie suggests it)
    runTest(analyzer, "fi(x > 0)", "if(x > 0)", "TC-2.9");
    
    // Test "while" typo (should work with longer word)
    // Should fix: "whle" -> "while"
    runTest(analyzer, "whle(true)", "while(true)", "TC-2.10");
    
    // Test preprocessor should never get semicolon, even after fixes
    runTest(analyzer, "#defin MAX 100", "#define MAX 100", "TC-2.11");
    
    cout << endl;
    
    // ========================================
    // Final Summary
    // ========================================
    cout << "========================================" << endl;
    cout << "Robustness Test Summary:" << endl;
    cout << "  Total Failures: " << totalFailures << endl;
    
    if (totalFailures == 0) {
        cout << "  Status: ✅ ALL ROBUSTNESS TESTS PASSED!" << endl;
    } else {
        cout << "  Status: ❌ " << totalFailures << " test(s) failed." << endl;
    }
    cout << "========================================" << endl;
    
    return (totalFailures == 0) ? 0 : 1;
}
