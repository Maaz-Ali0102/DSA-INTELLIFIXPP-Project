#include "Autocorrect.h"
#include <algorithm>
#include <cctype>
#include <sstream>

using namespace std;

Autocorrect::Autocorrect(Trie &trie, SymbolTable &sym, Logger &logger)
    : trie_(trie), sym_(sym), log_(logger) {
    // seed frequency table for common C++ keywords / STL identifiers
    const char *high[] = {"int","float","double","if","else","for","while","return","class","struct","void","auto","std","cout","cin","cerr","string","vector","map","unordered_map","push_back","sort","pair","queue","stack","include","main","iostream"};
    for (auto w : high) freq_[w] = 1000;
}

int Autocorrect::editDistance(const std::string &a, const std::string &b){
    const int n = (int)a.size();
    const int m = (int)b.size();
    vector<int> dp(m+1);
    for (int j=0;j<=m;++j) dp[j]=j;
    for (int i=1;i<=n;++i){
        int prev = dp[0];
        dp[0]=i;
        for (int j=1;j<=m;++j){
            int tmp = dp[j];
            if (a[i-1]==b[j-1]) dp[j]=prev;
            else dp[j]=min({prev+1, dp[j]+1, dp[j-1]+1});
            prev = tmp;
        }
    }
    return dp[m];
}

string Autocorrect::chooseBest(const string &orig, const vector<string> &cands) const {
    if (cands.empty()) return orig;
    double bestScore = -1e9;
    string best = orig;
    for (const auto &cand : cands){
        int dist = editDistance(orig, cand);
        int f = 1;
        auto it = freq_.find(cand);
        if (it!=freq_.end()) f = it->second;
        double score = -dist * 1000 + f;
        if (score > bestScore || (score==bestScore && cand < best)){
            bestScore = score; best = cand;
        }
    }
    return best;
}

// Use the shared Tokenizer for tokenization
// (Tokenizer is defined in Tokenizer.h and is constructed where needed)

// Correct single word using trie suggestions and internal frequency table
string Autocorrect::correctWord(const string &w) const {
    if (w.empty()) return w;
    if (trie_.contains(w) || trie_.contains(string(w))) return w;
    string lw = w; transform(lw.begin(), lw.end(), lw.begin(), [](unsigned char c){ return (char)tolower(c); });
    auto suggestions = trie_.getSuggestions(lw, 2);
    if (suggestions.empty()) return w;
    string chosen = chooseBest(lw, suggestions);
    return chosen;
}

void Autocorrect::fixOperators(vector<Token> &tokens, vector<string> &issues) const {
    for (size_t i=0;i+1<tokens.size();++i){
        if (tokens[i].type==TokType::IDENTIFIER){
            string id = tokens[i].value;
            if ((id=="cout" || id=="cerr") && tokens[i+1].type==TokType::OPERATOR && tokens[i+1].value=="<"){
                tokens[i+1].value = "<<"; issues.push_back("stream operator: '"+id+" <' -> '"+id+" <<'");
            }
            if (id=="cin" && tokens[i+1].type==TokType::OPERATOR && tokens[i+1].value==">"){
                tokens[i+1].value = ">>"; issues.push_back("stream operator: 'cin >' -> 'cin >>'");
            }
        }
    }

    for (size_t i=0;i<tokens.size();++i){
        if (tokens[i].type==TokType::KEYWORD){
            string kw = tokens[i].value;
            string lw = kw; transform(lw.begin(), lw.end(), lw.begin(), [](unsigned char c){ return (char)tolower(c); });
            if (lw=="if" || lw=="while"){
                size_t j=i+1; while (j<tokens.size() && tokens[j].type==TokType::WHITESPACE) ++j;
                if (j<tokens.size() && tokens[j].type==TokType::SEPARATOR && tokens[j].value=="("){
                    int depth=0; for (size_t k=j;k<tokens.size();++k){
                        if (tokens[k].type==TokType::SEPARATOR){ if (tokens[k].value=="(") ++depth; else if (tokens[k].value==")") { --depth; if (depth==0) break; } }
                        if (tokens[k].type==TokType::OPERATOR && tokens[k].value=="="){
                            if (!(k+1<tokens.size() && tokens[k+1].type==TokType::OPERATOR && tokens[k+1].value=="=")){
                                tokens[k].value = "=="; issues.push_back("used '=' in condition -> '=='");
                            }
                        }
                    }
                }
            }
            if (lw=="for"){
                size_t j=i+1; while (j<tokens.size() && tokens[j].type==TokType::WHITESPACE) ++j;
                if (j<tokens.size() && tokens[j].type==TokType::SEPARATOR && tokens[j].value=="("){
                    int depth=0; for (size_t k=j;k<tokens.size();++k){
                        if (tokens[k].type==TokType::SEPARATOR){ if (tokens[k].value=="(") ++depth; else if (tokens[k].value==")") { --depth; if (depth==0) break; } }
                        if (tokens[k].type==TokType::SEPARATOR && tokens[k].value==","){
                            tokens[k].value = ";"; tokens[k].type = TokType::SEPARATOR; issues.push_back("for(...) comma -> semicolon");
                        }
                    }
                }
            }
        }
    }
}

void Autocorrect::fixPatterns(vector<Token> &tokens, vector<string> &issues) const {
    if (!tokens.empty() && tokens[0].type==TokType::PREPROCESSOR){
        string t = tokens[0].value;
        if (t.find("include")!=string::npos){
            if (t.find('<')==string::npos && t.find('>')==string::npos){
                size_t pos = t.find("iostream");
                if (pos!=string::npos){
                    tokens[0].value = "#include <iostream>";
                    issues.push_back("fixed include -> '#include <iostream>'");
                }
            }
        }
        return;
    }

    ssize_t last = -1; for (ssize_t i=(ssize_t)tokens.size()-1;i>=0;--i){ if (tokens[i].type!=TokType::WHITESPACE) { last=i; break; } }
    if (last>=0){
        auto &tk = tokens[last];
        bool endsWithSemicolon = (tk.type==TokType::SEPARATOR && tk.value==";") || (tk.type==TokType::SEPARATOR && (tk.value=="}" || tk.value=="{"));
        bool isControl = false;
        for (size_t i=0;i<tokens.size();++i){ if (tokens[i].type==TokType::KEYWORD){ string lw=tokens[i].value; transform(lw.begin(), lw.end(), lw.begin(), [](unsigned char c){ return (char)tolower(c); }); if (lw=="if"||lw=="for"||lw=="while"||lw=="else"||lw=="switch") isControl=true; } }
        if (!endsWithSemicolon && !isControl && tokens[0].type!=TokType::PREPROCESSOR){
            tokens.push_back({TokType::SEPARATOR, ";"}); issues.push_back("added missing semicolon");
        }
    }
    static const vector<string> stltypes = {"vector","string","map","unordered_map","queue","stack","pair"};
    for (size_t i=0;i<tokens.size();++i){
        if (tokens[i].type==TokType::IDENTIFIER){
            for (auto &typ : stltypes){
                if (tokens[i].value==typ){
                    bool hasStd=false;
                    if (i>=2 && tokens[i-1].value=="::" && tokens[i-2].value=="std") hasStd=true;
                    if (!hasStd){ tokens[i].value = string("std::") + tokens[i].value; issues.push_back(string("added 'std::' to ")+typ); }
                }
            }
        }
    }
}

static string rebuildFromTokens(const vector<Token> &tokens){
    string out;
    for (size_t i=0;i<tokens.size();++i){
        const auto &t = tokens[i];
        if (t.type==TokType::WHITESPACE) { out += t.value; continue; }
        if (!out.empty()){
            char prev = out.back();
            bool needSpace = false;
            if ((t.type==TokType::IDENTIFIER || t.type==TokType::NUMBER || t.type==TokType::KEYWORD) && (prev!= ' ' && prev!='(' && prev!='[' && prev=='\0')){
                needSpace = true;
            }
            if (!needSpace){
                if (!out.empty() && isalnum((unsigned char)out.back()) && isalnum((unsigned char)t.value[0])) needSpace=true;
            }
            if (needSpace) out.push_back(' ');
        }
        out += t.value;
    }
    string finalOut;
    for (size_t i=0;i<out.size();++i){ char c = out[i];
        if ((c=='+'||c=='-'||c=='='||c=='<'||c=='>'||c=='*'||c=='/'||c=='%') && i+1<out.size() && out[i+1]!=' ' && out[i-1]!=' '){
            finalOut.push_back(' '); finalOut.push_back(c); finalOut.push_back(' ');
        } else finalOut.push_back(c);
    }
    return finalOut;
}

string Autocorrect::autocorrectLine(const string &line, vector<string> &issues, size_t /*lineNo*/){
    Tokenizer tk;
    auto tokens = tk.tokenize(line);
    for (auto &t : tokens){
        if (t.type==TokType::IDENTIFIER || t.type==TokType::KEYWORD){
            string corrected = correctWord(t.value);
            if (corrected != t.value){ issues.push_back("word: '"+t.value+"' -> '"+corrected+"'"); t.value = corrected; }
        }
    }
    fixOperators(tokens, issues);
    fixPatterns(tokens, issues);
    string out = rebuildFromTokens(tokens);
    return out;
}
