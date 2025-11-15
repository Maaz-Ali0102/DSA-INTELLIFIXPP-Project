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
    cout << "=== FINAL ROBUSTNESS SUITE ===" << endl;
    cout << "Testing human-style errors, spacing variations, combined errors, and edge cases\n" << endl;
    
    // Create core components
    Trie trie;
    trie.loadDefaultDictionary();
    SymbolTable sym;
    Logger logger;
    
    int totalFailures = 0;
    
    // Helper lambda function for line-by-line tests
    auto runTest = [](Analyzer& analyzer, const string& input, const string& expectedOutput, const string& testName) -> int {
        auto result = analyzer.processLine(input, 1);
        bool passed = (result.corrected == expectedOutput);
        
        if (passed) {
            cout << "[PASS] " << testName << endl;
            return 0;
        } else {
            cout << "[FAIL] " << testName << endl;
            cout << "  Input:    \"" << input << "\"" << endl;
            cout << "  Expected: \"" << expectedOutput << "\"" << endl;
            cout << "  Got:      \"" << result.corrected << "\"" << endl;
            if (!result.issues.empty()) {
                cout << "  Issues:   ";
                for (const auto& issue : result.issues) {
                    cout << issue << "; ";
                }
                cout << endl;
            }
            return 1;
        }
    };
    
    // ========================================
    // Test: Line-by-Line Mode (Stateful, Human-Style)
    // ========================================
    cout << "// --- Test: Line-by-Line Mode (Stateful, Human-Style) ---" << endl;
    
    // Create ONE Analyzer object for all line-by-line tests (stateful)
    Analyzer lineAnalyzer(trie, sym, logger);
    
    int failures = 0;
    
    // Test 1: Typos, Missing #, No Space, Semicolon Bug
    failures += runTest(lineAnalyzer, "incldue<istream>", "#include<iostream>", "H1-IncludeComplexFix");
    
    // Test 2: Spacing variation, Header typo
    failures += runTest(lineAnalyzer, "#inclde   <iotream>", "#include   <iostream>", "H2-IncludeSpacingFix");
    
    // Test 3: Short Keyword Typo (fr) + Spacing
    failures += runTest(lineAnalyzer, "fr  (i=0; i<5; i++)", "for  (i=0; i<5; i++)", "H3-ShortTypo 'fr'");
    
    // Test 4: Short Keyword Typo (fi)
    failures += runTest(lineAnalyzer, "fi(x>0)", "if(x>0)", "H4-ShortTypo 'fi'");
    
    // Test 5: State - Open Brace (Indent state = 0)
    failures += runTest(lineAnalyzer, "int main() {", "int main() {", "H5-OpenBrace"); // indent_ = 1
    
    // Test 6: Combined Error (Typo + Operator) + State (Indent = 1)
    failures += runTest(lineAnalyzer, "  cot < mian();", "    cout << main();", "H6-CombinedErrorIndent");
    
    // Test 7: Semicolon on Compound Line + State (Indent = 1)
    failures += runTest(lineAnalyzer, "for(i=0;;) cout << i", "    for(i=0;;) cout << i;", "H7-CompoundSemicolon");
    
    // Test 8: Short Identifier Regression (Must NOT fix 'i')
    failures += runTest(lineAnalyzer, "int i = 0;", "    int i = 0;", "H8-IdentifierRegression");
    
    // Test 9: Whitespace/Empty Lines (Must not add ';')
    // Note: Empty line gets auto-indented to match current indent state (indent_ = 1 = 4 spaces)
    failures += runTest(lineAnalyzer, "", "    ", "H9-EmptyLine");
    failures += runTest(lineAnalyzer, "    ", "    ", "H10-WhitespaceLine");
    
    // Test 11: State - Close Brace (Indent state = 1)
    failures += runTest(lineAnalyzer, "}", "}", "H11-CloseBrace"); // indent_ = 0
    
    totalFailures += failures;
    
    cout << "\nLine-by-Line Mode: " << (11 - failures) << "/11 tests passed" << endl;
    
    // ========================================
    // Test: Complex File Mode (Stateful Reset & Nesting)
    // ========================================
    cout << "\n// --- Test: Complex File Mode (Stateful Reset & Nesting) ---" << endl;
    
    // Create a NEW, FRESH Analyzer object for file mode (tests state reset)
    Analyzer fileAnalyzer(trie, sym, logger);
    
    vector<string> complexFileInput = {
        "// Human Test File",
        "incldue <iotream>",      // Typo, missing #, header typo
        "",                        // Empty line
        "int mian()",              // Typo, func decl (no ;)
        "{",
        "    fr(int i=0, j=0; i<10; i++)",  // Typo, valid comma
        "    {",
        "        cot < \"test\";", // Typo x2, spacing
        "    }",
        "    // Semicolon test below",  // Comment (no ;)
        "    retrun 0",            // Typo, missing ;
        "}"
    };
    
    // Define the EXACT expected output
    vector<string> complexExpectedOutput = {
        "// Human Test File",
        "#include <iostream>",     // Fixed
        "",
        "int main()",              // Fixed (no indent, no ;)
        "{",                       // No indent
        "    for(int i=0, j=0; i<10; i++)",  // Indent + Fixed
        "    {",                   // Indent
        "        cout << \"test\";", // Indent + Fixed
        "    }",                   // Dedent
        "    // Semicolon test below",  // Indent + Ignored
        "    return 0;",           // Indent + Fixed
        "}"                        // Dedent
    };
    
    // Process complex file
    vector<string> complexIssues;
    vector<string> complexFileResult = fileAnalyzer.processFile(complexFileInput, complexIssues);
    
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
            cout << "       (Typos, operators, indentation, nesting, comments - all validated!)" << endl;
        }
    }
    
    // ========================================
    // Additional Edge Case Tests
    // ========================================
    cout << "\n// --- Test: Additional Edge Cases ---" << endl;
    
    // Create fresh analyzer for edge cases
    Analyzer edgeAnalyzer(trie, sym, logger);
    
    int edgeFailures = 0;
    
    // Test multiple spaces between tokens
    edgeFailures += runTest(edgeAnalyzer, "int     x     =     5", "int     x     =     5;", "E1-MultipleSpaces");
    
    // Test tabs (tabs get normalized to spaces during indentation)
    edgeFailures += runTest(edgeAnalyzer, "\tint x = 5", "int x = 5;", "E2-TabIndent");
    
    // Test comment with typo (should NOT fix typo in comment)
    edgeFailures += runTest(edgeAnalyzer, "// mian function", "// mian function", "E3-CommentTypo");
    
    // Test string with typo (should NOT fix typo in string)
    edgeFailures += runTest(edgeAnalyzer, "string s = \"mian\";", "string s = \"mian\";", "E4-StringTypo");
    
    // Test mixed spacing around operators
    edgeFailures += runTest(edgeAnalyzer, "x=y+1", "x=y+1;", "E5-NoSpacing");
    
    // Test preprocessor with define
    edgeFailures += runTest(edgeAnalyzer, "#defin MAX 100", "#define MAX 100", "E6-DefineTypo");
    
    totalFailures += edgeFailures;
    
    cout << "\nEdge Cases: " << (6 - edgeFailures) << "/6 tests passed" << endl;
    
    // ========================================
    // Final Summary
    // ========================================
    cout << "\n========================================" << endl;
    cout << "FINAL ROBUSTNESS SUITE SUMMARY:" << endl;
    cout << "  Line-by-Line Tests: 11" << endl;
    cout << "  File Mode Tests:    1 (12 lines)" << endl;
    cout << "  Edge Case Tests:    6" << endl;
    cout << "  Total Test Groups:  18" << endl;
    cout << "  Total Failures:     " << totalFailures << endl;
    
    if (totalFailures == 0) {
        cout << "\n  Status: ✅ ALL ROBUSTNESS TESTS PASSED!" << endl;
        cout << "  🎉 Project is PRODUCTION READY! 🎉" << endl;
    } else {
        cout << "\n  Status: ❌ " << totalFailures << " test(s) failed." << endl;
    }
    cout << "========================================" << endl;
    
    return (totalFailures == 0) ? 0 : 1;
}
