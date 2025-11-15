// ULTIMATE EDGE CASE TEST - Find ALL remaining bugs
#include "../src/Utils.h"
#include "../src/Trie.h"
#include "../src/Logger.h"
#include "../src/SymbolTable.h"
#include <iostream>

using namespace std;

int main() {
    cout << "\n========================================" << endl;
    cout << "   ULTIMATE EDGE CASE TEST" << endl;
    cout << "   Finding ALL Remaining Bugs" << endl;
    cout << "========================================\n" << endl;
    
    Trie trie;
    trie.loadDefaultDictionary();
    Logger logger;
    SymbolTable symTable;
    
    int failCount = 0;
    int passCount = 0;
    
    auto runTest = [&](const string& input, const string& expected, const string& testName) {
        Analyzer analyzer(trie, symTable, logger);
        LineResult res = analyzer.processLine(input, 1);
        
        if (res.corrected == expected) {
            cout << "✅ PASS: " << testName << endl;
            passCount++;
        } else {
            cout << "❌ FAIL: " << testName << endl;
            cout << "   Input:    \"" << input << "\"" << endl;
            cout << "   Output:   \"" << res.corrected << "\"" << endl;
            cout << "   Expected: \"" << expected << "\"" << endl;
            failCount++;
        }
        cout << endl;
    };
    
    // === EDGE CASE 1: Variable Names (Symbol Table) ===
    cout << "--- Edge Case 1: Variable Name Preservation ---\n" << endl;
    runTest("int i=0", "int i=0;", "E1.1 - 'i' variable (must NOT become 'if')");
    runTest("int ni=5", "int ni=5;", "E1.2 - 'ni' variable (must NOT become 'cin')");
    runTest("int fo=10", "int fo=10;", "E1.3 - 'fo' variable (must NOT become 'for')");
    
    // === EDGE CASE 2: Strings & Comments ===
    cout << "\n--- Edge Case 2: String & Comment Preservation ---\n" << endl;
    runTest("cout<<\"incldue\"", "cout<<\"incldue\";", "E2.1 - Typo in string (must NOT fix)");
    runTest("// fo mian retun", "// fo mian retun", "E2.2 - Typos in comment (must NOT fix)");
    runTest("string s=\"fo(i=0)\"", "string s=\"fo(i=0)\";", "E2.3 - Code in string (must NOT fix)");
    
    // === EDGE CASE 3: Operators in Context ===
    cout << "\n--- Edge Case 3: Operator Context Detection ---\n" << endl;
    runTest("int x>y", "int x>y;", "E3.1 - Comparison operator (must NOT change)");
    runTest("cout>x", "cout<<x;", "E3.2 - Stream operator (must change > to <<)");
    runTest("if(x>5&&y<10)", "if(x>5&&y<10)", "E3.3 - Multiple operators (must NOT change)");
    
    // === EDGE CASE 4: Templates ===
    cout << "\n--- Edge Case 4: Template Syntax ---\n" << endl;
    runTest("vector<int>v", "vector<int>v;", "E4.1 - Template (must NOT confuse < >)");
    runTest("map<string,int>m", "map<string,int>m;", "E4.2 - Nested template");
    
    // === EDGE CASE 5: Pointers & References ===
    cout << "\n--- Edge Case 5: Pointers & References ---\n" << endl;
    runTest("int*ptr", "int*ptr;", "E5.1 - Pointer (must NOT confuse *)");
    runTest("int&ref=x", "int&ref=x;", "E5.2 - Reference (must NOT confuse &)");
    
    // === EDGE CASE 6: Nested Structures ===
    cout << "\n--- Edge Case 6: Nested Structures ---\n" << endl;
    runTest("fo(inti=0;i<5;i++)", "for(int i=0;i<5;i++)", "E6.1 - Nested typos in for-loop");
    runTest("if(x>0){cot<<x}", "if(x>0){cout<<x;}", "E6.2 - Typo inside if-block");
    
    // === EDGE CASE 7: Multiple Statements ===
    cout << "\n--- Edge Case 7: Multiple Statements per Line ---\n" << endl;
    runTest("intx=5;inty=10", "int x=5;int y=10;", "E7.1 - Two declarations");
    runTest("x=5;y=10;z=15", "x=5;y=10;z=15;", "E7.2 - Three assignments");
    
    // === EDGE CASE 8: Unusual Spacing ===
    cout << "\n--- Edge Case 8: Unusual Spacing ---\n" << endl;
    runTest("fr  (  i=0  )", "for  (  i=0  )", "E8.1 - Extra spaces");
    runTest("cout<<x<<y<<z", "cout<<x<<y<<z;", "E8.2 - No spaces in chain");
    
    // === EDGE CASE 9: Numbers & Literals ===
    cout << "\n--- Edge Case 9: Numbers & Literals ---\n" << endl;
    runTest("int x=0x10", "int x=0x10;", "E9.1 - Hex literal (must NOT break)");
    runTest("float f=3.14", "float f=3.14;", "E9.2 - Decimal literal");
    
    // === EDGE CASE 10: Preprocessor ===
    cout << "\n--- Edge Case 10: Preprocessor Directives ---\n" << endl;
    runTest("defin MAX 100", "#define MAX 100", "E10.1 - Missing # in define");
    runTest("#inclde<iostream>", "#include<iostream>", "E10.2 - Typo in include");
    
    // Final Summary
    cout << "\n========================================" << endl;
    cout << "         EDGE CASE SUMMARY" << endl;
    cout << "========================================" << endl;
    cout << "Total Tests:  " << (passCount + failCount) << endl;
    cout << "Passed:       " << passCount << " ✅" << endl;
    cout << "Failed:       " << failCount << " ❌" << endl;
    cout << "Success Rate: " << ((passCount * 100.0) / (passCount + failCount)) << "%" << endl;
    
    if (failCount == 0) {
        cout << "\n🎉 100% PRODUCTION READY! 🎉\n" << endl;
    } else {
        cout << "\n⚠️  " << failCount << " EDGE CASES NEED FIXING ⚠️\n" << endl;
    }
    
    return (failCount == 0) ? 0 : 1;
}
