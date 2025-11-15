#include <iostream>
#include <string>
#include "../src/Utils.h"
#include "../src/Trie.h"

using namespace std;

void test(const string& name, const string& input, const string& expected) {
    Trie trie;
    SymbolTable sym;
    Logger logger;
    Analyzer analyzer(trie, sym, logger);
    
    LineResult result = analyzer.processLine(input, 1);
    bool pass = (result.corrected == expected);
    cout << (pass ? "✓ PASS: " : "✗ FAIL: ") << name << "\n";
    if (!pass) {
        cout << "   Input:    \"" << input << "\"\n";
        cout << "   Output:   \"" << result.corrected << "\"\n";
        cout << "   Expected: \"" << expected << "\"\n";
    }
    cout << "\n";
}

int main() {
    cout << "\n========================================\n";
    cout << "   QUOTE CONVERSION TEST\n";
    cout << "========================================\n\n";

    cout << "--- Single quotes with multi-char should become double quotes ---\n\n";

    test("Multi-char in single quotes", 
         "cut > 'hello'",
         "cout << \"hello\";");

    test("Multi-char with cout", 
         "cot > 'world'",
         "cout << \"world\";");

    test("Multi-char with cin", 
         "cn > 'test'",
         "cin >> \"test\";");

    cout << "--- Single char should STAY in single quotes ---\n\n";

    test("Single char (valid)", 
         "cut > 'h'",
         "cout << 'h';");

    test("Single char with cout", 
         "cot > 'a'",
         "cout << 'a';");

    cout << "--- Edge cases ---\n\n";

    test("Empty single quotes", 
         "cut > ''",
         "cout << \"\";");

    test("Chain with mixed quotes", 
         "cot>'hello'>'a'>x",
         "cout<<\"hello\"<<'a'<<x;");

    cout << "========================================\n";
    cout << "         TEST COMPLETE\n";
    cout << "========================================\n";

    return 0;
}
