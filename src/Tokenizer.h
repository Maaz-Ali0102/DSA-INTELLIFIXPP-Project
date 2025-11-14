#pragma once
#include <string>
#include <vector>
#include <unordered_set>

// Token types for the tokenizer and downstream analysis
enum class TokType {
    KEYWORD,
    IDENTIFIER,
    OPERATOR,
    SEPARATOR,
    STRING_LITERAL,
    COMMENT,
    WHITESPACE,
    NUMBER,
    PREPROCESSOR,
    UNKNOWN
};

struct Token {
    TokType type;
    std::string value;
};

// Lightweight tokenizer implemented as a small FSM
class Tokenizer {
public:
    Tokenizer();

    // Tokenize a single line of C++ source into tokens
    std::vector<Token> tokenize(const std::string &line) const;

private:
    std::unordered_set<std::string> keywords_;
    void initKeywords();
};
