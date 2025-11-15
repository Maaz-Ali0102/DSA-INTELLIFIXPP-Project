#include <iostream>
#include "Tokenizer.h"
#include <vector>

using namespace std;

int main() {
    Tokenizer tok;
    
    string test1 = "#include <iostream>";
    string test2 = "#include<iostream>";
    string test3 = "include <vector>";
    
    cout << "Test 1: " << test1 << endl;
    auto tokens1 = tok.tokenize(test1);
    for (const auto& t : tokens1) {
        cout << "[" << (int)t.type << "]\"" << t.value << "\" ";
    }
    cout << endl << endl;
    
    cout << "Test 2: " << test2 << endl;
    auto tokens2 = tok.tokenize(test2);
    for (const auto& t : tokens2) {
        cout << "[" << (int)t.type << "]\"" << t.value << "\" ";
    }
    cout << endl << endl;
    
    cout << "Test 3: " << test3 << endl;
    auto tokens3 = tok.tokenize(test3);
    for (const auto& t : tokens3) {
        cout << "[" << (int)t.type << "]\"" << t.value << "\" ";
    }
    cout << endl;
    
    return 0;
}
