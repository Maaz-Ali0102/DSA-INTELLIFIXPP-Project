#include <iostream>
#include "Utils.h"
#include "Trie.h"
#include "Logger.h"
#include "SymbolTable.h"

using namespace std;

void testCase(const string& testName, const string& input, const string& expected) {
    Trie trie;
    trie.loadDefaultDictionary();
    SymbolTable sym;
    Logger logger;
    Analyzer analyzer(trie, sym, logger);
    
    auto result = analyzer.processLine(input, 1);
    
    cout << "Test: " << testName << endl;
    cout << "Input:    " << input << endl;
    cout << "Expected: " << expected << endl;
    cout << "Got:      " << result.corrected << endl;
    
    if (result.corrected == expected) {
        cout << "✅ PASS" << endl;
    } else {
        cout << "❌ FAIL" << endl;
    }
    cout << "Issues: ";
    for (const auto& issue : result.issues) {
        cout << issue << "; ";
    }
    cout << endl << endl;
}

int main() {
    cout << "=== Testing Include and Semicolon Fixes ===" << endl << endl;
    
    // Test 1: Normal #include should not get semicolon
    testCase("Normal #include",
             "#include <iostream>",
             "#include <iostream>");
    
    // Test 2: Missing # before include
    testCase("Missing # before include",
             "include <vector>",
             "#include <vector>");
    
    // Test 3: #include with extra spaces
    testCase("#include with spacing",
             "#  include <string>",
             "#  include <string>");
    
    // Test 4: Regular statement should get semicolon
    testCase("Regular statement needs semicolon",
             "int x = 5",
             "int x = 5;");
    
    // Test 5: #define should not get semicolon
    testCase("#define should not get semicolon",
             "#define MAX 100",
             "#define MAX 100");
    
    // Test 6: Missing # and space
    testCase("Missing # needs space",
             "include<map>",
             "#include<map>");
    
    return 0;
}
