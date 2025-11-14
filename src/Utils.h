#pragma once
#include <string>
#include <vector>
#include <stack>
#include <unordered_map>
#include "Trie.h"
#include "SymbolTable.h"
#include "Logger.h"
#include "Autocorrect.h"
#include "Tokenizer.h"

struct LineResult {
    std::string original;
    std::string corrected;
    std::vector<std::string> issues;
    bool changed = false;
};

class Analyzer {
public:
    Analyzer(Trie &trie, SymbolTable &sym, Logger &logger);

    // Process a single line (interactive mode)
    LineResult processLine(const std::string &line, size_t lineNo);

    // Process an entire file worth of lines; returns corrected lines
    std::vector<std::string> processFile(const std::vector<std::string> &lines, std::vector<std::string> &fileIssues);

    // Close any remaining braces at EOF
    void finalizeFile(std::vector<std::string> &corrected, std::vector<std::string> &fileIssues);

    // Get warnings about unclosed brackets (for pre-save/pre-quit checks)
    std::vector<std::string> getUnclosedBrackets() const;

    static int editDistance(const std::string &a, const std::string &b);

private:
    Trie &trie_;
    SymbolTable &sym_;
    Logger &log_;
    Autocorrect autocorrect_;
    Tokenizer tokenizer_;

    std::vector<char> braceStack_;
    int indent_ = 0;

    std::unordered_map<std::string,std::string> opFix_;

    void seedDictionary();

    std::string trim(const std::string &s);
    bool endsWith(const std::string &s, const std::string &suf);

    // Token-based fix functions (operate on token streams)
    void fixInclude(std::vector<Token> &tokens, std::vector<std::string> &issues);
    void fixForLoop(std::vector<Token> &tokens, std::vector<std::string> &issues);
    void fixOperators(std::vector<Token> &tokens, std::vector<std::string> &issues);
    // Normalize common stream operator and identifier typos before suggestions
    void fixStreamOperators(std::vector<Token> &tokens, std::vector<std::string> &issues);
    void fixCommonIdentifierTypos(std::vector<Token> &tokens, std::vector<std::string> &issues);
    void fixIdentifiers(std::vector<Token> &tokens, std::vector<std::string> &issues);
    void addMissingSemicolon(std::vector<Token> &tokens, std::vector<std::string> &issues);
    std::string applyIndentRule(const std::string &line);

    // Update brace/paren state from tokens only
    void updateBraceState(const std::vector<Token> &tokens, std::vector<std::string> &issues);

    // Convert tokens back to a single line string
    std::string detokenize(const std::vector<Token> &tokens) const;
    std::string correctTokenClosest(const std::string &tok, int maxDist, std::string *chosen = nullptr);
};
