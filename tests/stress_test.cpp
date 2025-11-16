#include <iostream>
#include <string>
#include <vector>
#include <cassert>

#include "Trie.h"
#include "SymbolTable.h"
#include "Utils.h"
#include "Logger.h"

using namespace std;

int main(){
    // Core components
    Trie trie; SymbolTable sym; Logger logger;
    logger.init("c:/Users/iComputers/Documents/IntelliFixPP/output");
    Analyzer analyzer(trie, sym, logger);

    int failures = 0;
    auto runTest = [&](const string &input, const string &expected, const string &name){
        // Use a fresh Analyzer per test to avoid brace/indent carry-over
        Analyzer az(trie, sym, logger);
        auto res = az.processLine(input, 1);
        bool ok = (res.corrected == expected);
        cout << (ok ? "[PASS] " : "[FAIL] ") << name << "\n"
             << "  in : " << input << "\n"
             << "  out: " << res.corrected << "\n"
             << "  exp: " << expected << "\n";
        if (!ok) ++failures;
    };

    // --- Test: Unterminated Statements (The New Bugs) ---
    runTest("#inclde<iostreem", "#include<iostream>", "H1 - Unterminated Include (TC-1)");
    runTest("cout< \"abdulhadi", "cout<< \"abdulhadi", "H2 - Unterminated String (TC-9)");
    runTest("#include <vector", "#include <vector>", "H3 - Missing '>'");
    runTest("string s = \"hello", "string s = \"hello", "H4 - Missing '\"'");

    // --- Test: "Messy" Tokenizer & Typos ---
    runTest("intx=5;", "int x=5;", "H5 - No-Space Tokenizer");
    runTest("using namespacestd", "using namespace std;", "H6 - No-Space Typo (TC-2)");
    runTest("floot caravg;", "float caravg;", "H7 - Typo 'floot' (TC-3)");
    runTest("breke", "break;", "H8 - Typo 'breke' (TC-7)");

    // --- Test: "Messy" Operators ---
    runTest("cin> caravg", "cin>> caravg;", "H9 - cin > (TC-4)");
    runTest("cout<<\"abdulhadi<<caravg", "cout<<\"abdulhadi<<caravg", "H10 - String Ignore (TC-5)");
    runTest("cout: 'hello'", "cout<< \"hello\";", "H11 - cout : (TC-14)");

    // --- Test: "MessFsy" for Loop ---
    runTest("for(i =0, i< 7;i+ +){", "for(i =0; i< 7;i+ +){", "H12 - for comma (TC-6)");
    runTest("fo(i=0i<5i++)", "for(i=0;i<5;i++)", "H13 - fo + no-space + missing ;");

    // --- File-level Human-Style Test ---
    Analyzer fileAnalyzer(trie, sym, logger);
    vector<string> fileInput = {
        "// Messy test file",
        "incldue <iostreem",
        "using namespacestd",
        "int main(){",
        "floot y;",
        "fr(i=0i<5i++){",
        "cut>\"test\";",
        "cout< \"abdulhadi",
        "}"
    };

    vector<string> expectedOutput = {
        "// Messy test file",
        "#include <iostream>",
        "using namespace std;",
        "int main(){",
        "    float y;",
        "    for(i=0;i<5;i++){",
        "        cout<<\"test\";",
        "        cout<< \"abdulhadi",
        "    }",
        "}"
    };

    vector<string> fileIssues;
    auto fixed = fileAnalyzer.processFile(fileInput, fileIssues);

    size_t maxn = max(fixed.size(), expectedOutput.size());
    for (size_t i = 0; i < maxn; ++i){
        string got = (i < fixed.size() ? fixed[i] : string("<missing>"));
        string exp = (i < expectedOutput.size() ? expectedOutput[i] : string("<missing>"));
        bool ok = (got == exp);
        cout << (ok ? "[PASS] " : "[FAIL] ") << "FILE L" << (i+1) << "\n"
             << "  out: " << got << "\n"
             << "  exp: " << exp << "\n";
        if (!ok) ++failures;
    }

    cout << "\nTotal Failures: " << failures << "\n";
    // Non-zero exit on failure helps automation
    return failures == 0 ? 0 : 1;
}
