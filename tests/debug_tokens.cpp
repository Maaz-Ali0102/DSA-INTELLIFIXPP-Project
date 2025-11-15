#include <iostream>
#include <vector>
#include "../src/Tokenizer.h"
using namespace std;

void printTokens(const string& line) {
    Tokenizer tok;
    auto tokens = tok.tokenize(line);
    cout << "Line: \"" << line << "\"" << endl;
    cout << "Tokens (" << tokens.size() << "):" << endl;
    for (size_t i = 0; i < tokens.size(); i++) {
        cout << "  [" << i << "] type=" << (int)tokens[i].type 
             << " value=\"" << tokens[i].value << "\"" << endl;
    }
    
    // Find first and last meaningful
    int first = -1, last = -1;
    for (size_t i = 0; i < tokens.size(); i++) {
        if (tokens[i].type != TokType::WHITESPACE) {
            if (first == -1) first = i;
            last = i;
        }
    }
    if (first >= 0) {
        cout << "  FIRST: [" << first << "] type=" << (int)tokens[first].type 
             << " value=\"" << tokens[first].value << "\"" << endl;
    }
    if (last >= 0) {
        cout << "  LAST: [" << last << "] type=" << (int)tokens[last].type 
             << " value=\"" << tokens[last].value << "\"" << endl;
    }
    cout << endl;
}

int main() {
    cout << "Token types: KEYWORD=0, IDENTIFIER=1, OPERATOR=2, SEPARATOR=3," << endl;
    cout << "             STRING_LITERAL=4, COMMENT=5, WHITESPACE=6, NUMBER=7," << endl;
    cout << "             PREPROCESSOR=8, UNKNOWN=9" << endl << endl;
    
    printTokens("// This is a comment");
    printTokens("// cout < test");
    printTokens("int mian()");
    
    return 0;
}
