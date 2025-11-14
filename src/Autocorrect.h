#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include "Trie.h"
#include "SymbolTable.h"
#include "Logger.h"

// Use shared Token / TokenType from Tokenizer
#include "Tokenizer.h"

// Autocorrect engine built on Trie + DP + simple frequency scoring
class Autocorrect {
public:
    Autocorrect(Trie &trie, SymbolTable &sym, Logger &logger);

    // Correct a single word (identifier/keyword) using trie suggestions and frequency scoring
    std::string correctWord(const std::string &w) const;

    // Fix operator mistakes based on token stream (cout/cin, '=' in conditions, commas in for())
    void fixOperators(std::vector<Token> &tokens, std::vector<std::string> &issues) const;

    // Fix patterns like missing semicolons, parentheses for control statements, include angle-brackets
    void fixPatterns(std::vector<Token> &tokens, std::vector<std::string> &issues) const;

    // Full-line autocorrect pipeline: tokens -> word-corrections -> operator -> patterns -> rebuilt string
    std::string autocorrectLine(const std::string &line, std::vector<std::string> &issues, size_t lineNo);

    // Helper: set frequency for a token (used to bias suggestions)
    void setFrequency(const std::string &word, int freq) { freq_[word] = freq; }

private:
    Trie &trie_;
    SymbolTable &sym_;
    Logger &log_;
    std::unordered_map<std::string,int> freq_; // higher = more common

    // Levenshtein distance DP (classic)
    static int editDistance(const std::string &a, const std::string &b);

    // Choose best candidate from suggestions using edit distance + frequency
    std::string chooseBest(const std::string &orig, const std::vector<std::string> &cands) const;
};
