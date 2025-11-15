#include <iostream>
#include <string>
#include "../src/Utils.h"
#include "../src/Trie.h"

using namespace std;

void check(const string& name, const string& input, const string& expected){
    Trie trie; SymbolTable sym; Logger logger; Analyzer a(trie, sym, logger);
    auto r = a.processLine(input, 1);
    bool pass = (r.corrected == expected);
    cout << (pass?"\u2713 PASS: ":"\u2717 FAIL: ") << name << "\n";
    if(!pass){
        cout << "   Input:    \"" << input << "\"\n";
        cout << "   Output:   \"" << r.corrected << "\"\n";
        cout << "   Expected: \"" << expected << "\"\n\n";
    }
}

int main(){
    cout << "\n=== COLON AFTER COUT/CIN TEST ===\n\n";

    check("cout colon with string", "cot : \"hello\"", "cout << \"hello\";");
    check("cin colon with ident",  "cn : x",         "cin >> x;");
    check("cout colon chain",      "cot:\"A\":x",  "cout<<\"A\"<<x;");
    check("cout colon with char",  "cot : 'a'",      "cout << 'a';");

    cout << "\n=== DONE ===\n";
}
