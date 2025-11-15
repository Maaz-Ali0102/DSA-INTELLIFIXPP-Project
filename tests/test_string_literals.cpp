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
    cout << "   STRING & CHAR LITERAL BUG TEST\n";
    cout << "========================================\n\n";

    cout << "--- Issue: cot/cut before string/char literals ---\n\n";

    // Bug reported by user
    test("Bug 1: cot before string", 
         "cot > \"hello\"",
         "cout << \"hello\";");

    test("Bug 2: cut before string", 
         "cut > \"hello\"",
         "cout << \"hello\";");

    test("Bug 3: cut before char", 
         "cut > 'h'",
         "cout << 'h';");

    test("Bug 4: cin with string", 
         "cn > \"test\"",
         "cin >> \"test\";");

    cout << "--- Related: Multiple outputs ---\n\n";

    test("Multiple strings", 
         "cot>\"hello\">\"world\"",
         "cout<<\"hello\"<<\"world\";");

    test("String with variable", 
         "cot>\"Value:\">x",
         "cout<<\"Value:\"<<x;");

    cout << "========================================\n";
    cout << "         TEST COMPLETE\n";
    cout << "========================================\n";

    return 0;
}
