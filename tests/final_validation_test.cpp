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
    cout << "=== Final Validation Test Suite ===" << endl;
    cout << "Testing both Line-by-Line (Stateful) and File (Stateless) modes\n" << endl;
    
    // Create core components
    Trie trie;
    trie.loadDefaultDictionary();
    SymbolTable sym;
    Logger logger;
    
    int totalFailures = 0;
    
    // Helper lambda function for line-by-line tests
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
            totalFailures++;
        }
        return passed;
    };
    
    // ========================================
    // Test: Line-by-Line Mode (Stateful)
    // ========================================
    cout << "// --- Test: Line-by-Line Mode (Stateful) ---" << endl;
    
    // Create ONE Analyzer object for all line-by-line tests
    Analyzer lineAnalyzer(trie, sym, logger);
    
    // Test preprocessor fixes
    runTest(lineAnalyzer, "include <iostream>", "#include <iostream>", "TC-15");
    runTest(lineAnalyzer, "#include <iotream>", "#include <iostream>", "TC-18");
    
    // Test stream operator fix
    runTest(lineAnalyzer, "cot >> \"hi\";", "cout << \"hi\";", "TC-24");
    
    // Test for-loop (no semicolon needed when ends with ))
    runTest(lineAnalyzer, "for(int i=0, j=0; i<10; i++)", "for(int i=0, j=0; i<10; i++)", "TC-26");
    
    // Test function declaration (no semicolon should be added)
    runTest(lineAnalyzer, "int mian()", "int main()", "TC-28");
    
    // Test comment protection (no semicolon should be added)
    runTest(lineAnalyzer, "// This is a comment", "// This is a comment", "TC-34");
    
    // Test compound statement (semicolon should be added)
    runTest(lineAnalyzer, "for(int i=0; i<10; i++) cout << i", "for(int i=0; i<10; i++) cout << i;", "TC-43");
    
    cout << "\n// Test Indentation State" << endl;
    // Test indentation state tracking
    runTest(lineAnalyzer, "{", "{", "TC-37"); // indent_ should now be 1
    runTest(lineAnalyzer, "int x = 5", "    int x = 5;", "TC-38"); // Should get 4 spaces indent
    runTest(lineAnalyzer, "}", "}", "TC-39"); // indent_ should now be 0
    
    cout << "\n// Test Semantic 'Limit' (These should PASS, no correction)" << endl;
    // Test semantic limits (should not over-correct)
    runTest(lineAnalyzer, "int x = 10;", "int x = 10;", "SEM-01");
    runTest(lineAnalyzer, "y = 10;", "y = 10;", "SEM-02"); // Should NOT find undeclared 'y' yet
    
    // ========================================
    // Test: File Mode (Stateless Reset)
    // ========================================
    cout << "\n// --- Test: File Mode (Stateless) ---" << endl;
    
    // Create a NEW Analyzer object for file mode (fresh state)
    Analyzer fileAnalyzer(trie, sym, logger);
    
    vector<string> fileInput = {
        "include <iostream>",
        "int mian(){",
        "    cot < \"hi\";",
        "    for(int i=0, i<5, i++)",
        "    int x = 5",
        "}"
    };
    
    vector<string> expectedOutput = {
        "#include <iostream>",
        "int main(){",           // No indent
        "    cout << \"hi\";",   // Indent + Fixes
        "    for(int i=0; i<5; i++)",  // Input already has 4 spaces, preserved
        "    int x = 5;",        // Input already has 4 spaces + Semicolon
        "}"                      // Dedent
    };
    
    // Process file
    vector<string> issues;
    vector<string> fileResult = fileAnalyzer.processFile(fileInput, issues);
    
    // Compare results
    bool fileTestPassed = true;
    if (fileResult.size() != expectedOutput.size()) {
        cout << "[FAIL] File Mode - Line count mismatch!" << endl;
        cout << "  Expected " << expectedOutput.size() << " lines, got " << fileResult.size() << endl;
        fileTestPassed = false;
        totalFailures++;
    } else {
        for (size_t i = 0; i < fileResult.size(); i++) {
            if (fileResult[i] != expectedOutput[i]) {
                if (fileTestPassed) {
                    cout << "[FAIL] File Mode" << endl;
                    fileTestPassed = false;
                    totalFailures++;
                }
                cout << "  Line " << (i+1) << ":" << endl;
                cout << "    Expected: \"" << expectedOutput[i] << "\"" << endl;
                cout << "    Got:      \"" << fileResult[i] << "\"" << endl;
            }
        }
        
        if (fileTestPassed) {
            cout << "[PASS] File Mode - All " << fileResult.size() << " lines correct!" << endl;
        }
    }
    
    // ========================================
    // Test: Complex File Mode (Stateful Reset & Nesting)
    // ========================================
    cout << "\n// --- Test: Complex File Mode (Stateful Reset & Nesting) ---" << endl;
    
    // Create a NEW, FRESH Analyzer object for complex file test
    Analyzer fileAnalyzer2(trie, sym, logger);
    
    vector<string> complexFileInput = {
        "// Start of file",
        "include <iotream>",      // Typo
        "",                        // Empty line
        "int mian()",              // Typo + Func decl
        "{",
        "    int x = 5",           // Missing semicolon
        "    for(int i=0, i<10, i++)", // Comma error
        "    {",
        "        cot < \"test\";", // Typo + Op error
        "    }",
        "    retrun 0;",           // Typo
        "}"
    };
    
    vector<string> complexExpectedOutput = {
        "// Start of file",
        "#include <iostream>",     // Fixed
        "",
        "int main()",              // Fixed (no indent, no semicolon)
        "{",                       // No indent
        "    int x = 5;",          // Indent + Semicolon
        "    for(int i=0; i<10; i++)", // Indent + Fixed
        "    {",                   // Indent
        "        cout << \"test\";", // Indent + Fixed
        "    }",                   // Dedent
        "    return 0;",           // Indent + Fixed
        "}"                        // Dedent
    };
    
    // Process complex file
    vector<string> complexIssues;
    vector<string> complexFileResult = fileAnalyzer2.processFile(complexFileInput, complexIssues);
    
    // Compare results
    bool complexFileTestPassed = true;
    if (complexFileResult.size() != complexExpectedOutput.size()) {
        cout << "[FAIL] Complex File Mode - Line count mismatch!" << endl;
        cout << "  Expected " << complexExpectedOutput.size() << " lines, got " << complexFileResult.size() << endl;
        complexFileTestPassed = false;
        totalFailures++;
    } else {
        for (size_t i = 0; i < complexFileResult.size(); i++) {
            if (complexFileResult[i] != complexExpectedOutput[i]) {
                if (complexFileTestPassed) {
                    cout << "[FAIL] Complex File Mode" << endl;
                    complexFileTestPassed = false;
                    totalFailures++;
                }
                cout << "  Line " << (i+1) << ":" << endl;
                cout << "    Expected: \"" << complexExpectedOutput[i] << "\"" << endl;
                cout << "    Got:      \"" << complexFileResult[i] << "\"" << endl;
            }
        }
        
        if (complexFileTestPassed) {
            cout << "[PASS] Complex File Mode - All " << complexFileResult.size() << " lines correct!" << endl;
            cout << "       (Comments, typos, operators, indentation, nesting - all validated!)" << endl;
        }
    }
    
    // ========================================
    // Final Summary
    // ========================================
    cout << "\n========================================" << endl;
    cout << "Final Validation Summary:" << endl;
    cout << "  Total Failures: " << totalFailures << endl;
    
    if (totalFailures == 0) {
        cout << "  Status: ✅ ALL TESTS PASSED! 100% SUCCESS!" << endl;
    } else {
        cout << "  Status: ❌ " << totalFailures << " test(s) failed." << endl;
    }
    cout << "========================================" << endl;
    
    return (totalFailures == 0) ? 0 : 1;
}
