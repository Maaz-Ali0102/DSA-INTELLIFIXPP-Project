#include <iostream>
#include "src/Trie.h"

using namespace std;

int main(){
    Trie trie;
    
    // Add C++ keywords and common identifiers
    const char* words[] = {
        "#include", "include", "iostream", "<iostream>", "vector", "<vector>", 
        "string", "<string>", "main", "cout", "cin", "return",
        "int", "float", "double", "for", "while", "do",
        "if", "else", "switch", "case", "break", "continue",
        "std", "namespace", "using", "class", "public", "private", "protected",
        "void", "bool", "true", "false", "nullptr",
        "new", "delete", "const", "static", "virtual", "override",
        "template", "typename"
    };
    
    for (auto w : words) {
        trie.insert(w);
    }
    
    cout << "=== Trie Auto-Correction Test ===\n\n";
    
    // Test cases
    struct TestCase {
        string input;
        int maxDist;
    };
    
    TestCase tests[] = {
        {"includ", 2},       // should suggest "include"
        {"iotream", 2},      // should suggest "iostream"
        {"mian", 1},         // should suggest "main"
        {"intt", 1},         // should suggest "int"
        {"retur", 1},        // should suggest "return"
        {"vectro", 2},       // should suggest "vector"
        {"strig", 2},        // should suggest "string"
        {"cout", 0},         // exact match
        {"virtuall", 2},     // should suggest "virtual"
        {"privat", 1},       // should suggest "private"
        {"namepsace", 2},    // should suggest "namespace"
        {"treu", 1},         // should suggest "true"
        {"fales", 2},        // should suggest "false"
    };
    
    for (const auto& test : tests){
        cout << "Word: \"" << test.input << "\" (maxDistance=" << test.maxDist << ")\n";
        
        auto suggestions = trie.getSuggestions(test.input, test.maxDist);
        
        if (suggestions.empty()){
            cout << "  No suggestions found.\n";
        } else {
            cout << "  Suggestions (" << suggestions.size() << "):\n";
            int count = 0;
            for (const auto& sugg : suggestions){
                cout << "    " << (++count) << ". " << sugg << "\n";
                if (count >= 5) break; // Show top 5
            }
        }
        cout << "\n";
    }
    
    return 0;
}
