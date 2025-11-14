#include <iostream>
#include "src/Trie.h"

using namespace std;

int main() {
    // Create Trie and load the built-in C++ dictionary
    Trie trie;
    trie.loadDefaultDictionary();
    
    cout << "Trie loaded with " << trie.allWords().size() << " C++ keywords.\n\n";
    
    // Test some corrections
    string typos[] = {"includ", "iotream", "mian", "intt", "viod", "retrun"};
    
    for (const auto& typo : typos) {
        cout << "Typo: \"" << typo << "\"\n";
        auto suggestions = trie.getSuggestions(typo, 2);
        
        if (!suggestions.empty()) {
            cout << "  → Suggestion: " << suggestions[0] << "\n";
        } else {
            cout << "  → No suggestions\n";
        }
    }
    
    return 0;
}
