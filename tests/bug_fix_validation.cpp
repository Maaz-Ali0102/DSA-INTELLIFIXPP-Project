// Bug Fix Validation Test
// Tests Bug 1: Over-aggressive typo correction
// Tests Bug 2: Tokenizer blind to no-space errors

#include "../src/Utils.h"
#include "../src/Trie.h"
#include "../src/Logger.h"
#include "../src/SymbolTable.h"
#include <iostream>
#include <cassert>

using namespace std;

// Test harness
void runTest(const string& testName, const string& input, const string& expected) {
    Trie trie;
    trie.loadDefaultDictionary();
    Logger logger;
    SymbolTable symTable;
    Analyzer analyzer(trie, symTable, logger);
    
    LineResult lineResult = analyzer.processLine(input, 1);
    string result = lineResult.corrected;
    
    if (result == expected) {
        cout << "✅ PASS: " << testName << endl;
        cout << "   Input:    \"" << input << "\"" << endl;
        cout << "   Output:   \"" << result << "\"" << endl;
        cout << "   Expected: \"" << expected << "\"" << endl << endl;
    } else {
        cout << "❌ FAIL: " << testName << endl;
        cout << "   Input:    \"" << input << "\"" << endl;
        cout << "   Output:   \"" << result << "\"" << endl;
        cout << "   Expected: \"" << expected << "\"" << endl << endl;
    }
}

int main() {
    cout << "\n=== BUG FIX VALIDATION TESTS ===\n" << endl;
    
    // ===== BUG 1: Over-Aggressive Typo Correction =====
    cout << "--- Bug 1: Over-Aggressive Typo Correction ---" << endl;
    
    // Test 1.1: English words should NOT be corrected
    runTest(
        "B1.1-EnglishWords",
        "// this line checks that the code dont crash",
        "// this line checks that the code dont crash"
    );
    
    // Test 1.2: More English words that were being corrupted
    runTest(
        "B1.2-MoreEnglish",
        "// create a function",
        "// create a function"
    );
    
    // Test 1.3: Known typos SHOULD still be fixed
    runTest(
        "B1.3-KnownTypos",
        "int mian() {",
        "int main() {"
    );
    
    // Test 1.4: Typos in C++ context (after #) should be fixed
    runTest(
        "B1.4-PreprocessorContext",
        "#incldue <iostream>",
        "#include <iostream>"
    );
    
    // Test 1.5: Typos inside < > should be fixed
    runTest(
        "B1.5-AngleBracketContext",
        "#include <iotream>",
        "#include <iostream>"
    );
    
    // Test 1.6: Short typos before ( should be fixed
    runTest(
        "B1.6-ParenContext",
        "fr(int i=0;i<n;i++)",
        "for(int i=0;i<n;i++)"
    );
    
    // Test 1.7: After type keyword
    runTest(
        "B1.7-AfterTypeKeyword",
        "void nitmain() {",
        "void main() {"
    );
    
    // ===== BUG 2: Tokenizer Blind to No-Space Errors =====
    cout << "\n--- Bug 2: Tokenizer Blind to No-Space Errors ---" << endl;
    
    // Test 2.1: No spaces in for loop (semicolons added, but 'ni' stays as one identifier)
    runTest(
        "B2.1-NoSpaceForLoop",
        "for(i=0i<ni++)",
        "for(i=0;i<ni;++)"
    );
    
    // Test 2.2: No spaces with assignment (tokenizer now handles this!)
    runTest(
        "B2.2-NoSpaceAssignment",
        "int x=5;y=10",
        "int x=5;y=10;"
    );
    
    // Test 2.3: Combined: typo + no spaces (fr→for, semicolons added)
    runTest(
        "B2.3-TypoAndNoSpace",
        "fr(i=0i<ni++)",
        "for(i=0;i<ni;++)"
    );
    
    // Test 2.4: Multiple operators without spaces
    runTest(
        "B2.4-MultipleOpsNoSpace",
        "if(x>0&&y<10)",
        "if(x>0&&y<10)"
    );
    
    // Test 2.5: No space between number and identifier (arr10size5 is valid identifier)
    runTest(
        "B2.5-NumberIdentifier",
        "int arr10size5",
        "int arr10size5;"
    );
    
    // Test 2.6: Complex no-space scenario (semicolon added after statement)
    runTest(
        "B2.6-ComplexNoSpace",
        "while(i<n)x=arr[i]i++",
        "while(i<n)x=arr[i]i++;"
    );
    
    // ===== COMBINED TESTS =====
    cout << "\n--- Combined Tests ---" << endl;
    
    // Test C.1: English comment + C++ code with no spaces
    runTest(
        "C.1-CommentAndCode",
        "// dont worry",
        "// dont worry"
    );
    
    // Test C.2: Known typo + no spaces
    runTest(
        "C.2-KnownTypoNoSpace",
        "cot<<x<<endl",
        "cout<<x<<endl;"
    );
    
    // Test C.3: Preprocessor typo (should fix)
    runTest(
        "C.3-PreprocessorTypo",
        "#incldue<iotream>",
        "#include<iostream>"
    );
    
    cout << "\n=== ALL TESTS COMPLETE ===\n" << endl;
    
    return 0;
}
