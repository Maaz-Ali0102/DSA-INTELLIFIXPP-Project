#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <unordered_set>

struct TrieNode {
    std::unordered_map<char, TrieNode*> next;
    bool isWord = false;
    ~TrieNode();
};

class Trie {
public:
    Trie();
    ~Trie();

    void insert(const std::string &word);
    bool contains(const std::string &word) const;

    // Load default C++ dictionary (keywords, headers, common identifiers)
    void loadDefaultDictionary();

    // Return list of words in dictionary (for edit-distance search)
    const std::vector<std::string>& allWords() const { return words_; }

    // Get suggestions for a misspelled word based on edit distance
    std::vector<std::string> getSuggestions(const std::string& word, int maxDistance = 2) const;

private:
    TrieNode *root_;
    std::vector<std::string> words_;
    std::unordered_set<std::string> wordSet_;
    std::unordered_map<std::string,int> priority_; // lower is higher priority

    // Helper for edit distance calculation
    static int editDistance(const std::string &a, const std::string &b);
    
    // Set priority bucket for a word (0 = highest priority)
    void setPriority(const std::string &word, int p) { priority_[word] = p; }
};
