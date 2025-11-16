#include "Utils.h"
#include <algorithm>
#include <cctype>
#include <sstream>
#include <cstring>
#include <iostream>

Analyzer::Analyzer(Trie &trie, SymbolTable &sym, Logger &logger)
    : trie_(trie), sym_(sym), log_(logger), autocorrect_(trie, sym, logger) {
    seedDictionary();
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

std::string Analyzer::detokenize(const std::vector<Token> &tokens) const {
    // "Dumb" detokenizer: just concatenate all token values
    // The Tokenizer is responsible for spacing, not the detokenizer
    std::string out;
    for (const auto &tk : tokens) {
        out += tk.value;
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
    if (tokens.empty()) return;
    
    // Find first meaningful (non-whitespace) token
    int firstMeaningful = -1;
    for (size_t i = 0; i < tokens.size(); ++i) {
        if (tokens[i].type != TokType::WHITESPACE) {
            firstMeaningful = i;
            break;
        }
    }
    
    if (firstMeaningful == -1) return; // Empty or whitespace-only line
    
    Token &firstToken = tokens[firstMeaningful];
    
    // Check if line starts with "include" keyword or typo of "include" (missing #)
    if ((firstToken.type == TokType::KEYWORD || firstToken.type == TokType::IDENTIFIER) && 
        (firstToken.value == "include")) {
        // Insert '#' right before the 'include' token (at firstMeaningful position)
        tokens.insert(tokens.begin() + firstMeaningful, {TokType::PREPROCESSOR, "#"});
        issues.push_back("added missing '#' before include");
        // continue to check angle brackets if present
    }
    
    // Check for typos of "include" (like "incldue", "inclde") missing #
    if (firstToken.type == TokType::IDENTIFIER && firstToken.value != "include") {
        auto suggestions = trie_.getSuggestions(firstToken.value, 2);
        if (!suggestions.empty() && suggestions[0] == "include") {
            // This is a typo of "include" - fix it and add #
            issues.push_back("identifier '" + firstToken.value + "' -> 'include'");
            firstToken.value = "include";
            firstToken.type = TokType::KEYWORD;
            // Insert '#' before it
            tokens.insert(tokens.begin() + firstMeaningful, {TokType::PREPROCESSOR, "#"});
            issues.push_back("added missing '#' before include");
            // continue to check angle brackets if present
        }
    }
    
    // Check if line starts with '#' followed by 'include' (or typo of include)
    if (firstToken.type == TokType::PREPROCESSOR && firstToken.value == "#") {
        // Find next meaningful token after '#'
        int nextMeaningful = -1;
        for (size_t i = firstMeaningful + 1; i < tokens.size(); ++i) {
            if (tokens[i].type != TokType::WHITESPACE) {
                nextMeaningful = i;
                break;
            }
        }
        
        // If next token is an IDENTIFIER, check for "include" typos
        if (nextMeaningful >= 0 && tokens[nextMeaningful].type == TokType::IDENTIFIER) {
            std::string& nextVal = tokens[nextMeaningful].value;
            
            // Check if it's a typo of "include" (e.g., "inclde", "incldue", "inlcude")
            if (nextVal != "include") {
                auto suggestions = trie_.getSuggestions(nextVal, 2);
                if (!suggestions.empty() && suggestions[0] == "include") {
                    issues.push_back("identifier '" + nextVal + "' -> 'include'");
                    nextVal = "include";
                }
            }
            // Fall-through to generic angle bracket ensure below
        }
        // Do NOT return here; continue to generic post-processing to ensure closing '>'
    }

    // Generic post-processing: if we now have an 'include' near start (with or without '#'),
    // ensure that a system-header style <...> has a closing '>'
    {
        // Find the first non-whitespace token again (it may have changed after inserts)
        int startIdx = -1; for (size_t i=0;i<tokens.size();++i){ if (tokens[i].type!=TokType::WHITESPACE){ startIdx=(int)i; break; } }
        if (startIdx >= 0) {
            int incIdx = -1;
            // If first is '#', the next meaningful should be 'include'
            if (tokens[startIdx].type == TokType::PREPROCESSOR && tokens[startIdx].value == "#"){
                for (size_t i = startIdx + 1; i < tokens.size(); ++i){ if (tokens[i].type!=TokType::WHITESPACE){ if (tokens[i].value=="include") incIdx=(int)i; break; } }
            } else if ((tokens[startIdx].type == TokType::KEYWORD || tokens[startIdx].type == TokType::IDENTIFIER) && tokens[startIdx].value=="include"){
                incIdx = startIdx;
            }
            if (incIdx >= 0){
                // Look for '<' after include
                int ltIdx = -1;
                for (size_t i = incIdx + 1; i < tokens.size(); ++i) {
                    if (tokens[i].type == TokType::WHITESPACE) continue;
                    if ((tokens[i].type == TokType::OPERATOR || tokens[i].type == TokType::SEPARATOR) && tokens[i].value == "<") { ltIdx = (int)i; break; }
                    // Stop if we encounter quote or '{' or ';' meaning it's not a system header form
                    if (tokens[i].type == TokType::STRING_LITERAL || (tokens[i].type==TokType::SEPARATOR && (tokens[i].value=="{"||tokens[i].value==";"))) break;
                }
                if (ltIdx >= 0){
                    // Find header identifier/keyword after '<'
                    int headerIdx = -1;
                    for (size_t i = ltIdx + 1; i < tokens.size(); ++i) {
                        if (tokens[i].type == TokType::WHITESPACE) continue;
                        if (tokens[i].type == TokType::IDENTIFIER || tokens[i].type == TokType::KEYWORD) { headerIdx = (int)i; break; }
                        // If we hit '>' immediately, we're done
                        if ((tokens[i].type == TokType::OPERATOR || tokens[i].type == TokType::SEPARATOR) && tokens[i].value == ">") { headerIdx = -2; break; }
                    }
                    if (headerIdx >= 0){
                        // Check token after header for '>'
                        int afterHeader = -1;
                        for (size_t i = headerIdx + 1; i < tokens.size(); ++i) { if (tokens[i].type != TokType::WHITESPACE) { afterHeader = (int)i; break; } }
                        bool hasClosing = false;
                        if (afterHeader >= 0){
                            if ((tokens[afterHeader].type == TokType::OPERATOR || tokens[afterHeader].type == TokType::SEPARATOR) && tokens[afterHeader].value == ">") hasClosing = true;
                        }
                        if (!hasClosing){
                            tokens.insert(tokens.begin() + headerIdx + 1, {TokType::OPERATOR, ">"});
                            issues.push_back("inserted missing '>' in #include<...>");
                        }
                    }
                }
            }
        }
    }

    // Delegate other include fixes to Autocorrect's pattern fixes
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
                    // Step 1: Replace commas with semicolons (for cases like: for(i=0,i<n,i++))
                    int inner_paren_level = 0;
                    for (size_t l = j + 1; l < for_loop_end; ++l) {
                        if (tokens[l].type == TokType::SEPARATOR) {
                            if (tokens[l].value == "(") {
                                inner_paren_level++;
                            } else if (tokens[l].value == ")") {
                                inner_paren_level--;
                            } else if (tokens[l].value == "," && inner_paren_level == 0) {
                                tokens[l].value = ";";
                                issues.push_back("for(...) comma -> semicolon");
                            }
                        }
                    }

                    // Step 2: Count semicolons after comma replacement
                    int semicolon_count = 0;
                    inner_paren_level = 0;
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

                    // Step 3: Handle missing semicolons
                    if (semicolon_count == 0) {
                        // Case: for(i=0 i<5 i++) - no semicolons at all
                        // Strategy: Find first comparison operator, then work backwards to find end of init
                        
                        // Step A: Find first comparison operator (<, >, <=, >=, ==, !=)
                        inner_paren_level = 0;
                        size_t first_comparison = 0;
                        for (size_t l = j + 1; l < for_loop_end; ++l) {
                            if (tokens[l].type == TokType::SEPARATOR) {
                                if (tokens[l].value == "(") inner_paren_level++;
                                else if (tokens[l].value == ")") inner_paren_level--;
                            }
                            if (inner_paren_level == 0 && tokens[l].type == TokType::OPERATOR) {
                                if (tokens[l].value == "<" || tokens[l].value == ">" || 
                                    tokens[l].value == "<=" || tokens[l].value == ">=" ||
                                    tokens[l].value == "==" || tokens[l].value == "!=") {
                                    first_comparison = l;
                                    break;
                                }
                            }
                        }

                        if (first_comparison > 0) {
                            // Step B: Find the LEFT OPERAND of comparison (e.g., 'i' in 'i<5')
                            // Go backwards from comparison operator, skip whitespace
                            size_t left_operand = first_comparison - 1;
                            while (left_operand > j + 1 && tokens[left_operand].type == TokType::WHITESPACE) {
                                left_operand--;
                            }
                            
                            // Step C: Find end of INIT expression (the token BEFORE left operand)
                            // Go backwards from left_operand to find last token of init
                            size_t init_end = left_operand - 1;
                            while (init_end > j + 1 && tokens[init_end].type == TokType::WHITESPACE) {
                                init_end--;
                            }
                            
                            // Insert first semicolon AFTER init_end (which is end of init expression)
                            tokens.insert(tokens.begin() + init_end + 1, {TokType::SEPARATOR, ";"});
                            issues.push_back("for(...) inserted first semicolon after init");
                            for_loop_end++; // Adjust end position
                            first_comparison++; // Adjust comparison position

                            // Step D: Find end of comparison expression (RIGHT OPERAND)
                            // Look for token after comparison operator
                            size_t right_operand = first_comparison + 1;
                            while (right_operand < for_loop_end && tokens[right_operand].type == TokType::WHITESPACE) {
                                right_operand++;
                            }
                            
                            // Insert second semicolon AFTER right operand
                            if (right_operand < for_loop_end && 
                                (tokens[right_operand].type == TokType::NUMBER || 
                                 tokens[right_operand].type == TokType::IDENTIFIER)) {
                                tokens.insert(tokens.begin() + right_operand + 1, {TokType::SEPARATOR, ";"});
                                issues.push_back("for(...) inserted second semicolon after condition");
                                for_loop_end++; // Adjust end position
                            }
                        }
                    } else if (semicolon_count == 1) {
                        // Case: for(i=0; i<5 i++) - only one semicolon
                        // Find the existing semicolon
                        inner_paren_level = 0;
                        size_t first_semicolon = 0;
                        for (size_t l = j + 1; l < for_loop_end; ++l) {
                            if (tokens[l].type == TokType::SEPARATOR) {
                                if (tokens[l].value == "(") inner_paren_level++;
                                else if (tokens[l].value == ")") inner_paren_level--;
                                else if (tokens[l].value == ";" && inner_paren_level == 0) {
                                    first_semicolon = l;
                                    break;
                                }
                            }
                        }

                        // Find comparison operator AFTER the first semicolon
                        inner_paren_level = 0;
                        size_t comparison_op = 0;
                        for (size_t l = first_semicolon + 1; l < for_loop_end; ++l) {
                            if (tokens[l].type == TokType::SEPARATOR) {
                                if (tokens[l].value == "(") inner_paren_level++;
                                else if (tokens[l].value == ")") inner_paren_level--;
                            }
                            if (inner_paren_level == 0 && tokens[l].type == TokType::OPERATOR) {
                                if (tokens[l].value == "<" || tokens[l].value == ">" || 
                                    tokens[l].value == "<=" || tokens[l].value == ">=" ||
                                    tokens[l].value == "==" || tokens[l].value == "!=") {
                                    comparison_op = l;
                                    break;
                                }
                            }
                        }

                        if (comparison_op > 0) {
                            // Find end of comparison expression
                            size_t comparison_end = comparison_op + 1;
                            while (comparison_end < for_loop_end && tokens[comparison_end].type == TokType::WHITESPACE) {
                                comparison_end++;
                            }
                            // Skip the comparison value
                            if (comparison_end < for_loop_end && 
                                (tokens[comparison_end].type == TokType::NUMBER || 
                                 tokens[comparison_end].type == TokType::IDENTIFIER)) {
                                comparison_end++;
                            }
                            
                            // Insert semicolon AFTER the comparison value
                            if (comparison_end < for_loop_end) {
                                tokens.insert(tokens.begin() + comparison_end, {TokType::SEPARATOR, ";"});
                                issues.push_back("for(...) inserted missing second semicolon");
                                for_loop_end++; // Adjust end position
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
    // Robust logic: Fix ANY wrong operator after cout/cin AND continue fixing chain
    for (size_t i = 0; i < tokens.size(); ++i) {
        // Skip comments and string literals
        if (tokens[i].type == TokType::COMMENT || tokens[i].type == TokType::STRING_LITERAL) {
            continue;
        }
        
        // Check for cout (or typos like cot, cut, ocout)
        if (tokens[i].type == TokType::IDENTIFIER || tokens[i].type == TokType::KEYWORD) {
            std::string lowerVal = tokens[i].value;
            std::transform(lowerVal.begin(), lowerVal.end(), lowerVal.begin(), 
                          [](unsigned char c){ return (char)std::tolower(c); });
            
            if (lowerVal == "cout" || lowerVal == "cot" || lowerVal == "cut" || 
                lowerVal == "ocout" || lowerVal == "out" || lowerVal == "ct") {
                // Fix ALL operators in the cout chain until we hit semicolon/separator
                size_t idx = i + 1;
                while (idx < tokens.size()) {
                    // Skip whitespace
                    if (tokens[idx].type == TokType::WHITESPACE) {
                        idx++;
                        continue;
                    }
                    
                    // Stop at semicolon, brace, or end of statement
                    if (tokens[idx].type == TokType::SEPARATOR && 
                        (tokens[idx].value == ";" || tokens[idx].value == "{" || 
                         tokens[idx].value == "}" || tokens[idx].value == "," ||
                         tokens[idx].value == ")" || tokens[idx].value == "(")) {
                        break;
                    }
                    
                    // Fix single-char operators or ':' to "<<"
                    if (tokens[idx].type == TokType::OPERATOR ||
                        (tokens[idx].type == TokType::SEPARATOR && tokens[idx].value == ":")) {
                        std::string op = tokens[idx].value;
                        // Preserve existing correct operator and ++/--
                        if (op == "<<" || op == "++" || op == "--") {
                            // ok
                        } else if (op.length() == 1 || op == ":") {
                            issues.push_back("stream operator '" + op + "' -> '<<' in cout chain");
                            tokens[idx].value = "<<";
                        }
                    }
                    
                    idx++;
                }
            }
            // Check for cin (or typos like cn, cinn)
            else if (lowerVal == "cin" || lowerVal == "cn" || lowerVal == "cinn") {
                // Fix ALL operators in the cin chain until we hit semicolon/separator
                size_t idx = i + 1;
                while (idx < tokens.size()) {
                    // Skip whitespace
                    if (tokens[idx].type == TokType::WHITESPACE) {
                        idx++;
                        continue;
                    }
                    
                    // Stop at semicolon, brace, or end of statement
                    if (tokens[idx].type == TokType::SEPARATOR && 
                        (tokens[idx].value == ";" || tokens[idx].value == "{" || 
                         tokens[idx].value == "}" || tokens[idx].value == "," ||
                         tokens[idx].value == ")" || tokens[idx].value == "(")) {
                        break;
                    }
                    
                    // Fix single-char operators or ':' to ">>"
                    if (tokens[idx].type == TokType::OPERATOR ||
                        (tokens[idx].type == TokType::SEPARATOR && tokens[idx].value == ":")) {
                        std::string op = tokens[idx].value;
                        // Preserve existing correct operator and ++/--
                        if (op == ">>" || op == "++" || op == "--") {
                            // ok
                        } else if (op.length() == 1 || op == ":") {
                            issues.push_back("stream operator '" + op + "' -> '>>' in cin chain");
                            tokens[idx].value = ">>";
                        }
                    }
                    
                    idx++;
                }
            }
        }
    }
}

// Fix invalid single-quoted strings: 'hello' -> "hello" (multi-char must use double quotes)
// Valid char literals like 'a' stay as 'a'
void Analyzer::fixInvalidCharLiterals(std::vector<Token> &tokens, std::vector<std::string> &issues){
    for (size_t i = 0; i < tokens.size(); ++i) {
        if (tokens[i].type == TokType::STRING_LITERAL) {
            std::string& val = tokens[i].value;
            
            // Check if it's single-quoted
            if (val.length() >= 2 && val[0] == '\'' && val.back() == '\'') {
                // Extract content between quotes
                std::string content = val.substr(1, val.length() - 2);
                
                // If content is NOT exactly 1 character, it should be double-quoted
                // Special case: empty '', or multi-char 'hello'
                if (content.length() != 1) {
                    issues.push_back("invalid char literal '" + val + "' -> \"" + content + "\" (multi-char needs double quotes)");
                    tokens[i].value = "\"" + content + "\"";
                }
            }
        }
    }
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

void Analyzer::fixIdentifiers(std::vector<Token> &tokens, std::vector<std::string> &issues){
    // **AGGRESSIVE LOGIC**: Check ALL identifiers with Trie (except in comments/strings)
    // This will catch: namspace, it, intz, cn, retun, etc.
    // Note: May cause i->if regression, but we'll fix that later with SymbolTable
    
    // First pass: Known typos (fast lookup)
    static const std::unordered_map<std::string, std::string> knownTypos = {
        {"mian", "main"}, {"mnia", "main"}, {"nitmain", "main"},
        {"cot", "cout"}, {"ocout", "cout"}, {"cut", "cout"}, {"ct", "cout"}, {"out", "cout"},
        {"cn", "cin"}, {"cinn", "cin"},
        {"retrun", "return"}, {"reutrn", "return"}, {"retun", "return"},
        {"vecotr", "vector"}, {"vcetor", "vector"},
        {"iotream", "iostream"}, {"iostraem", "iostream"},
        {"incldue", "include"}, {"inlcude", "include"}, {"inlude", "include"}, {"inclde", "include"},
        {"fi", "if"}, {"fr", "for"}, {"fo", "for"}, {"whle", "while"},  // Short keyword typos
        {"defin", "define"}, {"namspace", "namespace"}, {"it", "int"}, {"intz", "int"}  // More typos
    };
    
    for (size_t i = 0; i < tokens.size(); ++i){
        // Special case: "using namespacestd" -> "using namespace std" (missing space)
        if (tokens[i].type == TokType::KEYWORD && tokens[i].value == "using"){
            // find next meaningful token
            size_t j = i + 1;
            while (j < tokens.size() && tokens[j].type == TokType::WHITESPACE) ++j;
            if (j < tokens.size() && tokens[j].type == TokType::IDENTIFIER){
                std::string lw = tokens[j].value; std::transform(lw.begin(), lw.end(), lw.begin(), [](unsigned char c){ return (char)std::tolower(c); });
                if (lw == "namespacestd"){
                    tokens[j].value = "namespace std"; // simple split in-place
                    tokens[j].type = TokType::KEYWORD;   // treat as keyword chunk for now
                    issues.push_back("identifier 'namespacestd' -> 'namespace std'");
                }
            }
        }
        // Skip tokens in comments and string literals ONLY
        if (tokens[i].type == TokType::COMMENT || tokens[i].type == TokType::STRING_LITERAL) {
            continue;
        }
        
        // Check ALL IDENTIFIER and KEYWORD tokens
        if (tokens[i].type == TokType::IDENTIFIER || tokens[i].type == TokType::KEYWORD){
            std::string word = tokens[i].value;
            std::string lowerWord = word;
            std::transform(lowerWord.begin(), lowerWord.end(), lowerWord.begin(), 
                          [](unsigned char c){ return (char)std::tolower(c); });

            // Heuristic split: type merged with identifier (e.g., "intx" -> "int x")
            if (tokens[i].type == TokType::IDENTIFIER && lowerWord.rfind("int", 0) == 0 && word.size() > 3){
                std::string suffix = word.substr(3);
                // suffix must start with a valid identifier char
                if (!suffix.empty() && (std::isalpha((unsigned char)suffix[0]) || suffix[0]=='_')){
                    // Replace current token with 'int' keyword, insert space and suffix identifier
                    tokens[i] = {TokType::KEYWORD, "int"};
                    tokens.insert(tokens.begin() + i + 1, {TokType::WHITESPACE, " "});
                    tokens.insert(tokens.begin() + i + 2, {TokType::IDENTIFIER, suffix});
                    issues.push_back("split '" + word + "' -> 'int " + suffix + "'");
                    // Advance past inserted tokens
                    i += 2;
                    // Continue to next token
                    continue;
                }
            }
            
            // Check known typos first (fast)
            if (knownTypos.count(lowerWord)) {
                std::string correction = knownTypos.at(lowerWord);
                issues.push_back("identifier '" + word + "' -> '" + correction + "'");
                tokens[i].value = correction;
                tokens[i].type = TokType::KEYWORD;
                continue;
            }
            
            // **MODERATE TRIE CHECK**: Check identifiers length >= 4
            // Skip short words (2-3 chars) unless followed by '('
            bool checkTrie = false;
            
            if (word.length() >= 4) {
                checkTrie = true;  // Always check 4+ character words
            } else if (word.length() >= 2 && word.length() <= 3) {
                // For short words, only check if followed by '('
                for (size_t j = i + 1; j < tokens.size(); ++j) {
                    if (tokens[j].type != TokType::WHITESPACE) {
                        if (tokens[j].type == TokType::SEPARATOR && tokens[j].value == "(") {
                            checkTrie = true;
                        }
                        break;
                    }
                }
            }
            
            if (checkTrie) {
                auto suggestions = trie_.getSuggestions(word, 2);
                if (!suggestions.empty() && suggestions[0] != word) {
                    std::string correction = suggestions[0];
                    issues.push_back("identifier '" + word + "' -> '" + correction + "'");
                    tokens[i].value = correction;
                    // Mark as KEYWORD if it's a C++ keyword
                    if (correction == "for" || correction == "if" || correction == "while" || 
                        correction == "return" || correction == "int" || correction == "void" ||
                        correction == "float" || correction == "double" || correction == "char" ||
                        correction == "include" || correction == "define" || correction == "namespace") {
                        tokens[i].type = TokType::KEYWORD;
                    }
                }
            }
            // Short words like "ni", "x", "i" are left UNCHANGED (could be variables)
        }
    }
}

// Removed unused function: isControlStart (was defined but never called)

void Analyzer::addMissingSemicolon(std::vector<Token> &tokens, std::vector<std::string> &issues){
    if (tokens.empty()) return;
    
    // Step 1: Find first meaningful token (skip WHITESPACE)
    int first = -1;
    for (size_t i = 0; i < tokens.size(); ++i) {
        if (tokens[i].type != TokType::WHITESPACE) {
            first = i;
            break;
        }
    }
    
    // If no meaningful tokens, return
    if (first < 0) return;
    
    // Rule 1: PREPROCESSOR CHECK (HIGHEST PRIORITY)
    // If line starts with #, NEVER add semicolon (even if typo like "incldue")
    if (tokens[first].type == TokType::PREPROCESSOR) {
        return;
    }
    
    // Rule 2: COMMENT CHECK
    // Check if any token is a comment - EARLY EXIT
    for (const auto &tk : tokens) {
        if (tk.type == TokType::COMMENT) {
            return; // Early exit for any comment in the line
        }
    }
    
    // Step 2: Find last meaningful token (skip WHITESPACE)
    int last = -1;
    for (int i = (int)tokens.size() - 1; i >= 0; --i) {
        if (tokens[i].type != TokType::WHITESPACE) {
            last = i;
            break;
        }
    }
    
    // If no last token, return
    if (last < 0) return;
    
    TokType firstType = tokens[first].type;
    TokType lastType = tokens[last].type;
    std::string firstVal = tokens[first].value;
    std::string lastVal = tokens[last].value;
    
    // Rule 3: Check for specific separators: {, }, ;
    if (lastType == TokType::SEPARATOR && 
        (lastVal == "{" || lastVal == "}" || lastVal == ";")) {
        return;
    }

    // Rule 3.1: Unterminated string at end of line -> DO NOT add semicolon
    // If last token is STRING_LITERAL and does not end with a double quote (")
    if (lastType == TokType::STRING_LITERAL) {
        if (lastVal.empty() || lastVal.back() != '"') {
            // e.g., cout << "abdulhadi   (unterminated) -> skip semicolon
            return;
        }
    }
    
    // Rule 4: Control Statement Ignore - Fixes if, for, while, switch statements
    // Only skip if line ENDS with ) (e.g., "if(x)" or "for(;;)")
    // If line has more code after ), we should add semicolon (e.g., "for(...) cout << i")
    if (firstType == TokType::KEYWORD && lastType == TokType::SEPARATOR && lastVal == ")") {
        if (firstVal == "if" || firstVal == "for" || firstVal == "while" || firstVal == "switch") {
            return;
        }
    }
    
    // Rule 5: Function Declaration Ignore - Fixes TC-28
    // If line ends with ) and doesn't start with control statement keyword
    if (lastType == TokType::SEPARATOR && lastVal == ")") {
        // Check if firstType is NOT one of the control keywords
        bool isControlStatement = false;
        if (firstType == TokType::KEYWORD) {
            if (firstVal == "if" || firstVal == "for" || firstVal == "while" || firstVal == "switch") {
                isControlStatement = true;
            }
        }
        if (!isControlStatement) {
            // This is likely a function declaration like "int main()"
            return;
        }
    }
    
    // Rule 4: Add Semicolon - if we reach here, it's a statement that needs one
    tokens.push_back({TokType::SEPARATOR, ";"});
    issues.push_back("added missing semicolon");
    
    // Default: don't add semicolon for other cases
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

    // 2) Fix include directives first (adds missing #)
    fixInclude(tokens, res.issues);
    
    // 3) Word/identifier corrections
    fixCommonIdentifierTypos(tokens, res.issues);
    fixIdentifiers(tokens, res.issues);

    // 4) Operator & stream fixes (fixStreamOperators includes all operator fixes)
    fixStreamOperators(tokens, res.issues);
    fixInvalidCharLiterals(tokens, res.issues);
    fixForLoop(tokens, res.issues);

    // 5) Pattern fixes (semicolons) - must be after fixInclude
    addMissingSemicolon(tokens, res.issues);

    // 6) Rebuild string from tokens
    res.corrected = detokenize(tokens);

    // 7) Indent rule (BEFORE updating brace state so we use the OLD indent level)
    std::string indented = applyIndentRule(res.corrected);
    if (indented != res.corrected){
        res.issues.push_back("auto-indented");
        res.corrected = indented;
    }

    // 8) Update brace/paren state (AFTER indenting, for NEXT line)
    updateBraceState(tokens, res.issues);

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
