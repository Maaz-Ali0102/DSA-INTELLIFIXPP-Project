#include <iostream>
#include "Tokenizer.h"

using namespace std;

int main() {
    Tokenizer tok;
    string testComment = "// This is a comment";
    
    cout << "Tokenizing: " << testComment << endl;
    auto tokens = tok.tokenize(testComment);
    
    cout << "Number of tokens: " << tokens.size() << endl;
    for (size_t i = 0; i < tokens.size(); ++i) {
        cout << "Token " << i << ": type=" << (int)tokens[i].type 
             << " value=\"" << tokens[i].value << "\"" << endl;
    }
    
    return 0;
}
