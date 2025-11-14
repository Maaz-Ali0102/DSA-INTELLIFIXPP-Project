#include <iostream>
#include <iomanip>
#include "src/Trie.h"

using namespace std;

void printSeparator() {
    cout << string(60, '-') << "\n";
}

void testCorrection(Trie& trie, const string& word, int maxDist) {
    cout << "Input: \"" << word << "\" (max distance: " << maxDist << ")\n";
    
    auto suggestions = trie.getSuggestions(word, maxDist);
    
    if (suggestions.empty()) {
        cout << "  ✗ No corrections found\n";
    } else {
        cout << "  ✓ Found " << suggestions.size() << " suggestion(s):\n";
        for (size_t i = 0; i < suggestions.size() && i < 5; ++i) {
            cout << "    " << (i+1) << ". " << suggestions[i] << "\n";
        }
    }
    cout << "\n";
}

int main() {
    Trie trie;
    
    // Build comprehensive C++ dictionary
    const char* keywords[] = {
        // Directives
        "#include", "include", "define", "pragma",
        
        // Keywords
        "if", "else", "for", "while", "do", "switch", "case", "break", "continue", "return",
        "class", "struct", "public", "private", "protected", "namespace", "using",
        "int", "float", "double", "char", "void", "bool", "auto", "main",
        "const", "static", "virtual", "override", "template", "typename",
        "new", "delete", "nullptr", "true", "false",
        
        // Standard library
        "iostream", "<iostream>", "vector", "<vector>", "string", "<string>",
        "cout", "cin", "endl", "std"
    };
    
    for (auto word : keywords) {
        trie.insert(word);
    }
    
    cout << "\n";
    printSeparator();
    cout << "  INTELLIFIX++ TRIE AUTO-CORRECTION DEMO\n";
    printSeparator();
    cout << "\nDictionary loaded with " << trie.allWords().size() << " C++ keywords and identifiers.\n\n";
    
    // Demo common typos
    cout << "=== COMMON TYPOS ===\n\n";
    
    testCorrection(trie, "includ", 2);       // Missing 'e'
    testCorrection(trie, "iotream", 2);      // Transposed 'os' -> 'io'
    testCorrection(trie, "mian", 2);         // Transposed 'ai' -> 'ia'
    testCorrection(trie, "intt", 1);         // Extra 't'
    testCorrection(trie, "retur", 1);        // Missing 'n'
    testCorrection(trie, "viod", 1);         // Typo 'oi' -> 'io'
    
    cout << "=== EXACT MATCHES ===\n\n";
    testCorrection(trie, "main", 0);         // Exact match
    testCorrection(trie, "cout", 0);         // Exact match
    
    cout << "=== NO MATCHES ===\n\n";
    testCorrection(trie, "xyz", 1);          // Too different
    testCorrection(trie, "qwerty", 2);       // Not in dictionary
    
    printSeparator();
    
    // Interactive mode
    cout << "\n>>> Interactive Mode (type 'quit' to exit) <<<\n\n";
    
    string input;
    while (true) {
        cout << "Enter a word to correct: ";
        if (!getline(cin, input) || input == "quit" || input.empty()) {
            break;
        }
        
        cout << "\n";
        testCorrection(trie, input, 2);
    }
    
    cout << "\nGoodbye!\n";
    return 0;
}
