#include "Tokenizer.h"
#include <cctype>
#include <algorithm>
#include <cstring>

using namespace std;

void Tokenizer::initKeywords(){
    static const char *kws[] = {
        "if","else","for","while","return","class","struct","namespace","using","include",
        "int","float","double","char","void","auto","switch","case","break","continue","true","false",
        "template","typename","public","private","protected","std","cout","cin","cerr","main","string","vector","map","unordered_map","push_back","sort","pair","queue","stack"
    };
    for (auto k : kws) keywords_.insert(k);
}

Tokenizer::Tokenizer(){ initKeywords(); }

static bool isIdentStart(char c){ return std::isalpha((unsigned char)c) || c=='_'; }
static bool isIdentChar(char c){ return std::isalnum((unsigned char)c) || c=='_'; }

std::vector<Token> Tokenizer::tokenize(const std::string &line) const {
    std::vector<Token> tokens;
    size_t i=0, n=line.size();
    while (i<n){
        unsigned char c = line[i];
        if (std::isspace(c)){
            size_t j=i; while (j<n && std::isspace((unsigned char)line[j])) ++j;
            tokens.push_back({TokType::WHITESPACE, line.substr(i, j-i)});
            i=j; continue;
        }
        // Comments
        if (c=='/' && i+1<n && line[i+1]=='/'){
            tokens.push_back({TokType::COMMENT, line.substr(i)});
            break;
        }
        // Preprocessor
        if (c=='#'){
            tokens.push_back({TokType::PREPROCESSOR, line.substr(i)});
            break;
        }
        // String or char literal
        if (c=='"' || c=='\''){
            char q = (char)c; size_t j=i+1;
            while (j<n){ if (line[j]=='\\') { j+=2; } else if (line[j]==q) { ++j; break; } else ++j; }
            tokens.push_back({TokType::STRING_LITERAL, line.substr(i, j-i)});
            i=j; continue;
        }
        // Two-char operators
        if (i+1<n){
            std::string two = line.substr(i,2);
            static const std::unordered_set<std::string> twos = {"<<", ">>", "<=", ">=", "==", "!=", "&&", "||", "+=", "-=", "++", "--", "->", "::"};
            if (twos.count(two)){
                tokens.push_back({TokType::OPERATOR, two}); i+=2; continue;
            }
        }
        // Separators and single char punctuation
        if (strchr("(){}[];,:.", c)){
            std::string s(1,(char)c);
            tokens.push_back({TokType::SEPARATOR, s}); ++i; continue;
        }
        if (strchr("+-=*/%<>!&|^~?:", c)){
            tokens.push_back({TokType::OPERATOR, std::string(1,(char)c)}); ++i; continue;
        }
        // Identifier
        if (isIdentStart(c)){
            size_t j=i; while (j<n && isIdentChar(line[j])) ++j;
            std::string w = line.substr(i, j-i);
            std::string lw = w; std::transform(lw.begin(), lw.end(), lw.begin(), [](unsigned char ch){ return (char)std::tolower(ch); });
            if (keywords_.count(lw)) tokens.push_back({TokType::KEYWORD, w});
            else tokens.push_back({TokType::IDENTIFIER, w});
            i=j; continue;
        }
        // Number
        if (std::isdigit(c)){
            size_t j=i; while (j<n && (std::isdigit((unsigned char)line[j]) || line[j]=='.')) ++j;
            tokens.push_back({TokType::NUMBER, line.substr(i, j-i)}); i=j; continue;
        }
        // Fallback
    tokens.push_back({TokType::UNKNOWN, std::string(1,(char)c)}); ++i;
    }
    return tokens;
}
