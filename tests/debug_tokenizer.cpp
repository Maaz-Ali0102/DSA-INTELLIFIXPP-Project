#include "../src/Tokenizer.h"
#include <iostream>

using namespace std;

int main() {
    Tokenizer tokenizer;
    
    string test1 = "for(i=0i<ni++)";
    cout << "Input: \"" << test1 << "\"" << endl;
    auto tokens1 = tokenizer.tokenize(test1);
    cout << "Tokens (" << tokens1.size() << "): ";
    for (size_t i = 0; i < tokens1.size(); i++) {
        cout << "[" << i << ":" << tokens1[i].value << "] ";
    }
    cout << endl << endl;
    
    string test2 = "arr10size5";
    cout << "Input: \"" << test2 << "\"" << endl;
    auto tokens2 = tokenizer.tokenize(test2);
    cout << "Tokens: ";
    for (const auto& tok : tokens2) {
        cout << "[" << tok.value << "] ";
    }
    cout << endl << endl;
    
    string test3 = "x=5y=10";
    cout << "Input: \"" << test3 << "\"" << endl;
    auto tokens3 = tokenizer.tokenize(test3);
    cout << "Tokens: ";
    for (const auto& tok : tokens3) {
        cout << "[" << tok.value << "] ";
    }
    cout << endl;
    
    return 0;
}
