#include "Trie.h"
#include <algorithm>
#include <vector>

TrieNode::~TrieNode(){
    for (auto &kv : next) delete kv.second;
}

Trie::Trie(){
    root_ = new TrieNode();
}

Trie::~Trie(){
    delete root_;
}

void Trie::insert(const std::string &word){
    // Avoid duplicates in our flat list
    if (wordSet_.find(word) == wordSet_.end()) {
        wordSet_.insert(word);
        words_.push_back(word);
    }
    TrieNode *cur = root_;
    for (char c : word){
        if (!cur->next.count(c)) cur->next[c] = new TrieNode();
        cur = cur->next[c];
    }
    cur->isWord = true;
}

bool Trie::contains(const std::string &word) const{
    const TrieNode *cur = root_;
    for (char c : word){
        auto it = cur->next.find(c);
        if (it == cur->next.end()) return false;
        cur = it->second;
    }
    return cur->isWord;
}

// Edit distance (Levenshtein) using dynamic programming
int Trie::editDistance(const std::string &a, const std::string &b){
    const int n = a.size(), m = b.size();
    std::vector<int> dp(m + 1);
    
    // Initialize base case
    for (int j = 0; j <= m; ++j) dp[j] = j;
    
    // Fill DP table row by row
    for (int i = 1; i <= n; ++i){
        int prev = dp[0];
        dp[0] = i;
        for (int j = 1; j <= m; ++j){
            int tmp = dp[j];
            if (a[i-1] == b[j-1]) {
                dp[j] = prev;
            } else {
                dp[j] = std::min({prev + 1, dp[j] + 1, dp[j-1] + 1});
            }
            prev = tmp;
        }
    }
    return dp[m];
}

// Get suggestions sorted by edit distance
std::vector<std::string> Trie::getSuggestions(const std::string& word, int maxDistance) const{
    struct Candidate {
        std::string word;
        int distance;
        int priority;
        
        bool operator<(const Candidate& other) const {
            if (distance != other.distance) return distance < other.distance;
            if (priority != other.priority) return priority < other.priority; // prioritize keywords/std ids
            return word < other.word;
        }
    };
    
    std::vector<Candidate> candidates;
    
    // Check all words in dictionary
    for (const auto& dictWord : words_){
        int dist = editDistance(word, dictWord);
        if (dist <= maxDistance){
            int pri = 2;
            auto it = priority_.find(dictWord);
            if (it != priority_.end()) pri = it->second;
            candidates.push_back({dictWord, dist, pri});
        }
    }
    
    // Sort by distance (closest first), then alphabetically
    std::sort(candidates.begin(), candidates.end());
    
    // Extract just the words
    std::vector<std::string> result;
    result.reserve(candidates.size());
    for (const auto& candidate : candidates){
        result.push_back(candidate.word);
    }
    
    return result;
}

// Load default C++ dictionary (VIP list)
void Trie::loadDefaultDictionary(){
    // Priority 0: Critical keywords, types, and common I/O
    const char* p0[] = {
        // Core Keywords
        "if", "else", "for", "while", "do", "switch", "case", "default", "break", "continue", "return",
        "class", "struct", "union", "enum", "public", "private", "protected", "namespace", "using", "typedef",
        "typename", "template", "const", "static", "extern", "volatile", "mutable", "virtual", "override",
        "final", "explicit", "inline", "new", "delete", "this", "nullptr", "true", "false",
        // Core Types
        "int", "float", "double", "char", "bool", "void", "long", "short", "auto", "size_t",
        // Critical Identifiers
        "main", "std", "cout", "cin", "cerr", "endl"
    };

    // Priority 1: Common STL Containers, Headers, and Algorithms
    const char* p1[] = {
        // Headers (without .h or brackets)
        "iostream", "vector", "string", "map", "set", "unordered_map", "unordered_set", "algorithm",
        "utility", "memory", "iomanip", "sstream", "fstream", "cmath", "cstdlib", "cstring", "limits",
        "numeric", "iterator", "queue", "stack", "deque", "list", "array", "tuple", "optional", "variant",
        // Common STL Types
        "vector", "string", "map", "set", "unordered_map", "unordered_set", "pair", "tuple",
        "queue", "stack", "deque", "list", "array", "priority_queue", "optional", "variant",
        // Common STL Functions
        "make_pair", "make_tuple", "begin", "end", "rbegin", "rend", "size", "empty", "clear",
        "push_back", "pop_back", "insert", "erase", "find", "count", "at", "front", "back",
        "sort", "reverse", "swap", "min", "max", "find_if", "count_if", "remove", "remove_if", "getline",
        // Memory
        "unique_ptr", "shared_ptr", "make_unique", "make_shared",
        // Preprocessor
        "#include", "#define", "#ifdef", "#ifndef", "#endif", "#pragma"
    };

    // Insert priority 0 then priority 1 words
    for (auto w : p0) { setPriority(w, 0); insert(w); }
    for (auto w : p1) { setPriority(w, 1); insert(w); }
}
