#include "Utils.h"
#include <algorithm>
#include <cctype>
#include <sstream>
#include <cstring>

Analyzer::Analyzer(Trie &trie, SymbolTable &sym, Logger &logger)
    : trie_(trie), sym_(sym), log_(logger), autocorrect_(trie, sym, logger) {
    seedDictionary();
    opFix_ = {
        {"=<", "<="}, {"=>", ">="}, {"=!", "!="}, {"=~", "~="},
        {"==<", "<="}, {"==>", ">="}, {"<>", "!="}
    };
}

void Analyzer::seedDictionary(){
    // Use the Trie's built-in prioritized dictionary
    trie_.loadDefaultDictionary();
}

static std::string toLower(std::string s){
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return (char)std::tolower(c); });
    return s;
}

std::string Analyzer::trim(const std::string &s){
    size_t i=0,j=s.size();
    while (i<j && std::isspace((unsigned char)s[i])) ++i;
    while (j>i && std::isspace((unsigned char)s[j-1])) --j;
    return s.substr(i, j-i);
}

bool Analyzer::endsWith(const std::string &s, const std::string &suf){
    return s.size()>=suf.size() && s.compare(s.size()-suf.size(), suf.size(), suf)==0;
}

std::string Analyzer::detokenize(const std::vector<Token> &tokens) const {
    std::string out;
    if (tokens.empty()) return out;

    out += tokens[0].value;

    for (size_t i = 1; i < tokens.size(); ++i) {
        const Token &prev = tokens[i-1];
        const Token &curr = tokens[i];

        if (curr.type == TokType::WHITESPACE || prev.type == TokType::WHITESPACE) {
            out += curr.value;
            continue;
        }

        char prev_last_char = prev.value.empty() ? ' ' : prev.value.back();
        char curr_first_char = curr.value.empty() ? ' ' : curr.value[0];

        bool prev_is_op = prev.type == TokType::OPERATOR;
        bool curr_is_op = curr.type == TokType::OPERATOR;

        if ( (std::isalnum(prev_last_char) && std::isalnum(curr_first_char)) ||
             (std::isalnum(prev_last_char) && curr_is_op && curr.value != "++" && curr.value != "--") ||
             (prev_is_op && std::isalnum(curr_first_char) && prev.value != "++" && prev.value != "--")
           ) {
            out += " ";
        } else if (prev.value == ")" && std::isalnum(curr_first_char)) {
             out += " ";
        }


        out += curr.value;
    }
    return out;
}

int Analyzer::editDistance(const std::string &a, const std::string &b){
    const int n=a.size(), m=b.size();
    std::vector<int> dp(m+1);
    for (int j=0;j<=m;++j) dp[j]=j;
    for (int i=1;i<=n;++i){
        int prev=dp[0];
        dp[0]=i;
        for (int j=1;j<=m;++j){
            int tmp=dp[j];
            if (a[i-1]==b[j-1]) dp[j]=prev;
            else dp[j]=std::min({prev+1, dp[j]+1, dp[j-1]+1});
            prev=tmp;
        }
    }
    return dp[m];
}

std::string Analyzer::correctTokenClosest(const std::string &tok, int maxDist, std::string *chosen){
    // Use the Trie's enhanced getSuggestions method
    auto suggestions = trie_.getSuggestions(toLower(tok), maxDist);
    
    if (!suggestions.empty()){
        std::string best = suggestions[0]; // First suggestion is closest
        if (chosen) *chosen = best;
        return best;
    }
    
    // No suggestions found within maxDist
    if (chosen) *chosen = tok;
    return tok;
}

void Analyzer::fixInclude(std::vector<Token> &tokens, std::vector<std::string> &issues){
    // Delegate include fixes to Autocorrect's pattern fixes for now
    autocorrect_.fixPatterns(tokens, issues);
}

void Analyzer::fixForLoop(std::vector<Token> &tokens, std::vector<std::string> &issues){
    for (size_t i = 0; i < tokens.size(); ++i) {
        if (tokens[i].type == TokType::KEYWORD && tokens[i].value == "for") {
            size_t j = i + 1;
            while (j < tokens.size() && tokens[j].type == TokType::WHITESPACE) {
                j++;
            }

            if (j < tokens.size() && tokens[j].type == TokType::SEPARATOR && tokens[j].value == "(") {
                int paren_level = 1;
                size_t for_loop_end = 0;

                // Find the matching closing parenthesis
                for (size_t k = j + 1; k < tokens.size(); ++k) {
                    if (tokens[k].type == TokType::SEPARATOR) {
                        if (tokens[k].value == "(") {
                            paren_level++;
                        } else if (tokens[k].value == ")") {
                            paren_level--;
                        }
                    }
                    if (paren_level == 0) {
                        for_loop_end = k;
                        break;
                    }
                }

                if (for_loop_end != 0) {
                    // First, count how many semicolons are already present at depth 0
                    int semicolon_count = 0;
                    int inner_paren_level = 0;
                    for (size_t l = j + 1; l < for_loop_end; ++l) {
                        if (tokens[l].type == TokType::SEPARATOR) {
                            if (tokens[l].value == "(") {
                                inner_paren_level++;
                            } else if (tokens[l].value == ")") {
                                inner_paren_level--;
                            } else if (tokens[l].value == ";" && inner_paren_level == 0) {
                                semicolon_count++;
                            }
                        }
                    }

                    // A proper for loop needs exactly 2 semicolons
                    // If we have fewer, replace top-level commas with semicolons
                    if (semicolon_count < 2) {
                        inner_paren_level = 0;
                        int replaced = 0;
                        for (size_t l = j + 1; l < for_loop_end && semicolon_count + replaced < 2; ++l) {
                            if (tokens[l].type == TokType::SEPARATOR) {
                                if (tokens[l].value == "(") {
                                    inner_paren_level++;
                                } else if (tokens[l].value == ")") {
                                    inner_paren_level--;
                                } else if (tokens[l].value == "," && inner_paren_level == 0) {
                                    tokens[l].value = ";";
                                    issues.push_back("for(...) comma -> semicolon");
                                    replaced++;
                                }
                            }
                        }
                    }
                    i = for_loop_end;
                }
            }
        }
    }
}

// Fix common mistakes around stream operators like
// cout < "hello";  -> cout << "hello";
// cout <"hello";   -> cout << "hello";
// cin > x;         -> cin >> x;
// cin >x;          -> cin >> x;
// Works with both '"' and '\'' string literals; applies when a single '<'/'>' is present.
void Analyzer::fixStreamOperators(std::vector<Token> &tokens, std::vector<std::string> &issues){
    autocorrect_.fixOperators(tokens, issues);
}

// Fix a short list of common identifier typos prior to suggestion stage
// e.g., mian->main, mnia->main, cot->cout, cut->cout, out->cout, ct->cout, cinn->cin
void Analyzer::fixCommonIdentifierTypos(std::vector<Token> &tokens, std::vector<std::string> &issues){
    // Simple hard-coded replacements at token level (skip comments and string literals)
    for (auto &t : tokens){
        // Skip tokens in comments and string literals
        if (t.type == TokType::COMMENT || t.type == TokType::STRING_LITERAL) continue;
        
        if (t.type==TokType::IDENTIFIER){
            std::string lw = t.value; std::transform(lw.begin(), lw.end(), lw.begin(), [](unsigned char c){ return (char)std::tolower(c); });
            if (lw=="mian" || lw=="mnia") { issues.push_back("identifier '"+t.value+"' -> 'main'"); t.value = "main"; }
            else if (lw=="cot"||lw=="cut"||lw=="out"||lw=="ct") { issues.push_back("identifier '"+t.value+"' -> 'cout'"); t.value = "cout"; }
            else if (lw=="cinn") { issues.push_back("identifier '"+t.value+"' -> 'cin'"); t.value = "cin"; }
        }
    }
}

void Analyzer::fixOperators(std::vector<Token> &tokens, std::vector<std::string> &issues){
    // delegate to autocorrect rules which handle operator fixes
    autocorrect_.fixOperators(tokens, issues);
}

void Analyzer::fixIdentifiers(std::vector<Token> &tokens, std::vector<std::string> &issues){
    // Attempt to correct function-name like typos using Trie (skip comments and strings)
    for (size_t i=0;i+2<tokens.size();++i){
        // Skip tokens in comments and string literals
        if (tokens[i].type == TokType::COMMENT || tokens[i].type == TokType::STRING_LITERAL) continue;
        
        if (tokens[i].type==TokType::KEYWORD || tokens[i].type==TokType::IDENTIFIER){
            // pattern: <type> <name> (
            if (tokens[i+1].type==TokType::WHITESPACE && (tokens[i+2].type==TokType::IDENTIFIER)){
                size_t j = i+3;
                // find '(' after optional whitespace
                while (j<tokens.size() && tokens[j].type==TokType::WHITESPACE) ++j;
                if (j<tokens.size() && tokens[j].type==TokType::SEPARATOR && tokens[j].value=="("){
                    std::string name = tokens[i+2].value;
                    std::string lw = name; std::transform(lw.begin(), lw.end(), lw.begin(), [](unsigned char c){ return (char)std::tolower(c); });
                    if (lw=="mian" || lw=="mnia"){
                        issues.push_back("identifier '"+name+"' -> 'main'"); tokens[i+2].value = "main"; return; }
                    std::string fix = correctTokenClosest(name, 1);
                    if (fix != name && (fix=="main" || trie_.contains(fix))){ issues.push_back("identifier '"+name+"' -> '"+fix+"'"); tokens[i+2].value = fix; return; }
                }
            }
        }
    }
}

static bool isControlStart(const std::string &trimmed){
    return trimmed.rfind("if",0)==0 || trimmed.rfind("for",0)==0 || trimmed.rfind("while",0)==0 || trimmed.rfind("switch",0)==0 || trimmed.rfind("else",0)==0;
}

void Analyzer::addMissingSemicolon(std::vector<Token> &tokens, std::vector<std::string> &issues){
    // Find last meaningful token
    int last = -1; for (int i=(int)tokens.size()-1;i>=0;--i){ if (tokens[i].type!=TokType::WHITESPACE && tokens[i].type!=TokType::COMMENT) { last = i; break; } }
    if (last<0) return;
    // Do not add after preprocessor
    if (tokens[0].type==TokType::PREPROCESSOR) return;
        TokType ttype = tokens[last].type;
    std::string val = tokens[last].value;
    bool endsWithOk = (ttype==TokType::SEPARATOR && (val==";"||val=="{"||val=="}")) || (ttype==TokType::OPERATOR && (val==":"));
    // control start check
    bool isControl=false; for (auto &tk : tokens){ if (tk.type==TokType::KEYWORD){ std::string lw=tk.value; std::transform(lw.begin(), lw.end(), lw.begin(), [](unsigned char c){ return (char)std::tolower(c); }); if (lw=="if"||lw=="for"||lw=="while"||lw=="else"||lw=="switch") isControl=true; } }
    if (!endsWithOk && !isControl){ tokens.push_back({TokType::SEPARATOR, ";"}); issues.push_back("added missing semicolon"); }
}

std::string Analyzer::applyIndentRule(const std::string &line){
    std::string t = trim(line);
    int localIndent = indent_;
    if (!t.empty() && t[0]=='}') localIndent = std::max(0, indent_-1);
    std::string ind(localIndent*4, ' ');
    return ind + t;
}

void Analyzer::updateBraceState(const std::vector<Token> &tokens, std::vector<std::string> &issues){
    for (const auto &tk : tokens){
    if (tk.type!=TokType::SEPARATOR) continue;
        const std::string &v = tk.value;
        if (v=="{") { braceStack_.push_back('{'); ++indent_; }
        else if (v=="}") { if (!braceStack_.empty() && braceStack_.back()=='{'){ braceStack_.pop_back(); indent_ = std::max(0, indent_-1); } else issues.push_back("unmatched '}' removed or extra"); }
        else if (v=="(") { braceStack_.push_back('('); }
        else if (v==")") { if (!braceStack_.empty() && braceStack_.back()=='('){ braceStack_.pop_back(); } else issues.push_back("unmatched ')' detected"); }
        else if (v=="[") { braceStack_.push_back('['); }
        else if (v=="]") { if (!braceStack_.empty() && braceStack_.back()=='['){ braceStack_.pop_back(); } else issues.push_back("unmatched ']' detected"); }
    }
}

LineResult Analyzer::processLine(const std::string &line, size_t lineNo){
    LineResult res; res.original = line; res.corrected = line; res.changed = false;
    // Token-based pipeline:
    // 1) Tokenize
    auto tokens = tokenizer_.tokenize(line);

    // 2) Word/identifier corrections
    fixCommonIdentifierTypos(tokens, res.issues);
    fixIdentifiers(tokens, res.issues);

    // 3) Operator & stream fixes (fixStreamOperators includes all operator fixes)
    fixStreamOperators(tokens, res.issues);
    fixForLoop(tokens, res.issues);

    // 4) Pattern fixes (includes, std::, semicolons)
    addMissingSemicolon(tokens, res.issues);

    // 5) Update brace/paren state (token-only)
    updateBraceState(tokens, res.issues);

    // 6) Rebuild string from tokens
    res.corrected = detokenize(tokens);

    // 7) Indent rule
    std::string indented = applyIndentRule(res.corrected);
    if (indented != res.corrected){
        res.issues.push_back("auto-indented");
        res.corrected = indented;
    }

    res.changed = (res.corrected != res.original);

    // logging
    if (!res.issues.empty()){
        std::ostringstream oss;
        for (size_t i=0;i<res.issues.size();++i){
            oss << " - " << res.issues[i] << "\n";
        }
        if (res.changed) log_.fix(lineNo, res.original, res.corrected, "Applied corrections:\n" + oss.str());
        else log_.issue(lineNo, "Potential issues:\n" + oss.str());
    }
    return res;
}

std::vector<std::string> Analyzer::processFile(const std::vector<std::string> &lines, std::vector<std::string> &fileIssues){
    std::vector<std::string> out;
    braceStack_.clear(); indent_ = 0;
    size_t ln=1;
    for (auto &l : lines){
        auto r = processLine(l, ln++);
        out.push_back(r.corrected);
    }
    finalizeFile(out, fileIssues);
    return out;
}

void Analyzer::finalizeFile(std::vector<std::string> &corrected, std::vector<std::string> &fileIssues){
    // Only auto-insert missing '}' to preserve structure; for other unmatched symbols, log issues
    while (!braceStack_.empty()){
        char top = braceStack_.back();
        braceStack_.pop_back();
        if (top == '{'){
            corrected.push_back(std::string(std::max(0, indent_-1)*4, ' ') + "}");
            if (indent_>0) --indent_;
            fileIssues.push_back("inserted missing closing '}' at end of file");
        } else if (top == '('){
            fileIssues.push_back("missing ')' at end of file");
        } else if (top == '['){
            fileIssues.push_back("missing ']' at end of file");
        }
    }
}

std::vector<std::string> Analyzer::getUnclosedBrackets() const {
    std::vector<std::string> warnings;
    
    // Count each type of unclosed bracket
    int unclosedBraces = 0;
    int unclosedParens = 0;
    int unclosedBrackets = 0;
    
    for (char c : braceStack_){
        if (c == '{') unclosedBraces++;
        else if (c == '(') unclosedParens++;
        else if (c == '[') unclosedBrackets++;
    }
    
    if (unclosedBraces > 0){
        warnings.push_back("Missing " + std::to_string(unclosedBraces) + 
                          " closing brace" + (unclosedBraces > 1 ? "s" : "") + " '}'");
    }
    if (unclosedParens > 0){
        warnings.push_back("Missing " + std::to_string(unclosedParens) + 
                          " closing parenthes" + (unclosedParens > 1 ? "es" : "is") + " ')'");
    }
    if (unclosedBrackets > 0){
        warnings.push_back("Missing " + std::to_string(unclosedBrackets) + 
                          " closing bracket" + (unclosedBrackets > 1 ? "s" : "") + " ']'");
    }
    
    return warnings;
}
