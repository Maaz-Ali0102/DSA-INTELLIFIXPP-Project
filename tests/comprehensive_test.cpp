#include <iostream>
#include <string>
#include <vector>
#include <cassert>
#include "Utils.h"
#include "Trie.h"
#include "SymbolTable.h"
#include "Logger.h"

using namespace std;

// Global counters
int totalTests = 0;
int passedTests = 0;
int failedTests = 0;

// Helper function for line-by-line tests
bool runTest(Analyzer& analyzer, const string& input, const string& expectedOutput, const string& testName) {
    totalTests++;
    auto result = analyzer.processLine(input, totalTests);
    
    bool passed = (result.corrected == expectedOutput);
    
    if (passed) {
        cout << "[PASS] " << testName << ": " << input << endl;
        passedTests++;
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
        failedTests++;
    }
    
    return passed;
}

// Helper function for file mode tests
bool runFileTest(const vector<string>& inputLines, const vector<string>& expectedLines, const string& testName) {
    totalTests++;
    
    Trie trie;
    trie.loadDefaultDictionary();
    SymbolTable sym;
    Logger logger;
    Analyzer analyzer(trie, sym, logger);
    
    // Process each line
    vector<string> outputLines;
    for (size_t i = 0; i < inputLines.size(); ++i) {
        auto result = analyzer.processLine(inputLines[i], i + 1);
        outputLines.push_back(result.corrected);
    }
    
    // Compare results
    bool passed = true;
    if (outputLines.size() != expectedLines.size()) {
        passed = false;
    } else {
        for (size_t i = 0; i < outputLines.size(); ++i) {
            if (outputLines[i] != expectedLines[i]) {
                passed = false;
                break;
            }
        }
    }
    
    if (passed) {
        cout << "[PASS] " << testName << " (File Mode)" << endl;
        passedTests++;
    } else {
        cout << "[FAIL] " << testName << " (File Mode)" << endl;
        cout << "  Expected:" << endl;
        for (const auto& line : expectedLines) {
            cout << "    " << line << endl;
        }
        cout << "  Got:" << endl;
        for (const auto& line : outputLines) {
            cout << "    " << line << endl;
        }
        failedTests++;
    }
    
    return passed;
}

int main() {
    cout << "=== Comprehensive Analyzer Test Suite ===" << endl << endl;
    
    // Initialize core components
    Trie trie;
    trie.loadDefaultDictionary();
    SymbolTable sym;
    Logger logger;
    Analyzer analyzer(trie, sym, logger);
    
    cout << "--- Line-by-Line Mode Tests ---" << endl << endl;
    
    // Test: Preprocessor Directives
    cout << "// Test Category: Preprocessor Directives" << endl;
    runTest(analyzer, "include <iostream>", "#include <iostream>", "TC-15");
    runTest(analyzer, "#include<iostream>", "#include<iostream>", "TC-16");
    runTest(analyzer, "#include <iostream>", "#include <iostream>", "TC-17");
    runTest(analyzer, "#include <iotream>", "#include <iostream>", "TC-18");
    runTest(analyzer, "#incldue <iostream>", "#include <iostream>", "TC-19");
    runTest(analyzer, "string s = \"#include\"", "string s = \"#include\";", "TC-20");
    cout << endl;
    
    // Test: Stream Operators
    cout << "// Test Category: Stream Operators" << endl;
    runTest(analyzer, "cout < \"hi\";", "cout << \"hi\";", "TC-21");
    runTest(analyzer, "cin << var;", "cin >> var;", "TC-22");
    runTest(analyzer, "cout << \"hello\";", "cout << \"hello\";", "TC-23");
    runTest(analyzer, "cot >> \"hi\";", "cout << \"hi\";", "TC-24");
    cout << endl;
    
    // Test: For Loop Logic
    cout << "// Test Category: For Loop Logic" << endl;
    runTest(analyzer, "for(int i=0, i<5, i++)", "for(int i=0; i<5; i++)", "TC-25");
    runTest(analyzer, "for(int i=0, j=0; i<10; i++)", "for(int i=0, j=0; i<10; i++)", "TC-26");
    runTest(analyzer, "for(int i=0; i<max(a,b); i++)", "for(int i=0; i<max(a,b); i++)", "TC-27");
    cout << endl;
    
    // Test: Symbol & Keyword Corrections
    cout << "// Test Category: Symbol & Keyword Corrections" << endl;
    runTest(analyzer, "int mian()", "int main()", "TC-28");
    runTest(analyzer, "int x = 5", "int x = 5;", "TC-29");
    runTest(analyzer, "if(true)", "if(true)", "TC-30");
    runTest(analyzer, "retrun 0;", "return 0;", "TC-31a");
    runTest(analyzer, "vecotr<int> v;", "vector<int> v;", "TC-31b");
    cout << endl;
    
    // Test: Semantic (No correction needed)
    cout << "// Test Category: Semantic (No Correction Needed)" << endl;
    runTest(analyzer, "y = 10;", "y = 10;", "TC-32");
    runTest(analyzer, "int z;", "int z;", "TC-33");
    runTest(analyzer, "// This is a comment", "// This is a comment", "TC-34");
    cout << endl;
    
    // Test: Assignment in Conditionals
    cout << "// Test Category: Assignment in Conditionals" << endl;
    runTest(analyzer, "if (x = 5)", "if (x == 5)", "TC-35");
    runTest(analyzer, "while (a = b)", "while (a == b)", "TC-36");
    cout << endl;
    
    // Test: Bracket Matching
    cout << "// Test Category: Bracket Matching & Indentation" << endl;
    runTest(analyzer, "{", "{", "TC-37");
    runTest(analyzer, "int x = 10;", "    int x = 10;", "TC-38");
    runTest(analyzer, "}", "}", "TC-39");
    cout << endl;
    
    // Test: Comments and Strings Protection
    cout << "// Test Category: Comments and Strings Protection" << endl;
    runTest(analyzer, "// cout < test", "// cout < test", "TC-40");
    runTest(analyzer, "string s = \"cout < hi\";", "string s = \"cout < hi\";", "TC-41");
    cout << endl;
    
    // Test: Multiple Corrections on One Line
    cout << "// Test Category: Multiple Corrections" << endl;
    runTest(analyzer, "cout < mian();", "cout << main();", "TC-42");
    runTest(analyzer, "for(int i=0, i<10, i++) cout < i", "for(int i=0; i<10; i++) cout << i;", "TC-43");
    cout << endl;
    
    cout << "--- File Mode Tests ---" << endl << endl;
    
    // File Mode Test 1: Simple Program
    cout << "// File Test 1: Simple Program with Stream Operators" << endl;
    vector<string> fileTest1Input = {
        "int main(){",
        "cout < \"hi\";",
        "}"
    };
    vector<string> fileTest1Expected = {
        "int main(){",
        "    cout << \"hi\";",
        "}"
    };
    runFileTest(fileTest1Input, fileTest1Expected, "FT-01");
    cout << endl;
    
    // File Mode Test 2: Multiple Corrections
    cout << "// File Test 2: Multiple Corrections" << endl;
    vector<string> fileTest2Input = {
        "#include <iostream>",
        "int mian(){",
        "int x = 5",
        "cout < x;",
        "retrun 0;",
        "}"
    };
    vector<string> fileTest2Expected = {
        "#include <iostream>",
        "int main(){",
        "    int x = 5;",
        "    cout << x;",
        "    return 0;",
        "}"
    };
    runFileTest(fileTest2Input, fileTest2Expected, "FT-02");
    cout << endl;
    
    // File Mode Test 3: For Loop with Nested Calls
    cout << "// File Test 3: For Loop with Nested Calls" << endl;
    vector<string> fileTest3Input = {
        "for(int i=0, i<10, i++){",
        "cout < max(i,5);",
        "}"
    };
    vector<string> fileTest3Expected = {
        "for(int i=0; i<10; i++){",
        "    cout << max(i,5);",
        "}"
    };
    runFileTest(fileTest3Input, fileTest3Expected, "FT-03");
    cout << endl;
    
    // File Mode Test 4: Include Directives
    cout << "// File Test 4: Include Directives" << endl;
    vector<string> fileTest4Input = {
        "include <iostream>",
        "#include <vecotr>",
        "int main(){ }"
    };
    vector<string> fileTest4Expected = {
        "#include <iostream>",
        "#include <vector>",
        "int main(){ }"
    };
    runFileTest(fileTest4Input, fileTest4Expected, "FT-04");
    cout << endl;
    
    // Print Summary
    cout << "========================================" << endl;
    cout << "Test Summary:" << endl;
    cout << "  Total Tests:  " << totalTests << endl;
    cout << "  Passed:       " << passedTests << " ✓" << endl;
    cout << "  Failed:       " << failedTests << " ✗" << endl;
    cout << "  Success Rate: " << (totalTests > 0 ? (passedTests * 100 / totalTests) : 0) << "%" << endl;
    cout << "========================================" << endl;
    
    return (failedTests == 0) ? 0 : 1;
}
