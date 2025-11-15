# IntelliFix++ (Phase 1.5)
### A Robust C++ Syntactic Analyzer & Auto-Corrector

Project by: Syed Maaz Ali (Roll: CT-232, Section: E)

---

## 1) Phase 1.5 Changelog — What’s New

This release focuses on reliability and real-world “human testing” fixes. The engine is now far better at handling messy, no-space code and context-aware typos without over-correcting valid variables.

- Robust Tokenizer (Tokenizer.h/.cpp)
  - Upgraded FSM to split at type boundaries, so no-space code is parsed correctly:
    - `intx=5;` → tokens: `int`, `x`, `=`, `5`, `;`
    - `i=0i<n` → tokens: `i`, `=`, `0`, `i`, `<`, `n`
- Context-Aware Typos (Utils.cpp → Analyzer::fixIdentifiers)
  - Smart correction using Trie + rules:
    - 4+ character words are checked by default
    - 2–3 character words checked only in function/keyword contexts (e.g., `fr(` → `for(`)
    - Short variables like `i`, `x`, `ni` are preserved
- Smart for(...) Loop Repair (Utils.cpp → Analyzer::fixForLoop)
  - Now inserts missing semicolons even when there are none:
    - `for(i=0 i<n i++)` → `for(i=0; i<n; i++)`
    - `for(i=0i<ni++)` → `for(i=0; i<ni; ++i)` (based on tokens)
- Robust Stream Operators (Utils.cpp → Analyzer::fixStreamOperators)
  - Catches any wrong single-char operator or colon after cout/cin and fixes the whole chain:
    - `cot > "hello"` → `cout << "hello";`
    - `cn < x` → `cin >> x;`
    - `cot:"A":x` → `cout<<"A"<<x;`
    - Preserves `++`/`--` and avoids touching comparisons like `x > y`
- Invalid Char Literal Repair (Utils.cpp → Analyzer::fixInvalidCharLiterals)
  - Converts invalid single-quoted strings to proper double-quoted strings:
    - `'hello'` → `"hello"` (multi-char becomes string)
    - `'a'` stays `'a'`

---

## 2) Project Overview — What It Is

IntelliFix++ is a C++ syntactic auto-corrector. It reads C++ code and automatically fixes grammar-level mistakes (operators, keywords, simple punctuation, typos) while preserving valid code.

Modes of operation:
- Interactive Mode (line-by-line)
- File Mode (batch fix a .cpp file)

---

## 3) How It Works — The Token-Based Engine

Pipeline:
`Input String` → 1) Tokenizer → 2) Analyzer (Fixers) → 3) Detokenizer → `Output String`

- Tokenizer (Tokenizer.h/.cpp)
  - FSM-based lexer producing token types like KEYWORD, IDENTIFIER, OPERATOR, SEPARATOR, STRING_LITERAL, COMMENT, WHITESPACE, NUMBER, PREPROCESSOR
- Analyzer (Utils.h/.cpp → class Analyzer)
  - Runs ordered fixers over the token stream:
    - `fixInclude` (adds missing `#`, fixes include typos)
    - `fixCommonIdentifierTypos` + `fixIdentifiers` (Trie-backed typo recovery)
    - `fixStreamOperators` (normalizes operators after cout/cin, supports `:`)
    - `fixInvalidCharLiterals` ("'hello'" → "\"hello\"")
    - `fixForLoop` (inserts missing `;` in for headers)
    - `addMissingSemicolon` (adds `;` for plain statements)
  - `updateBraceState` and `applyIndentRule` handle scope/indentation
- Detokenizer (Utils.cpp)
  - Rebuilds corrected tokens back to a clean line

Key sources: `src/Tokenizer.*`, `src/Utils.*`, `src/Trie.*`, `src/SymbolTable.*`, `src/Logger.*`

---

## 4) DSA & Algorithms Used (VIP Logic)

- Hashing
  - Tokenizer keyword tables (O(1) lookup)
  - SymbolTable uses `std::unordered_set`/maps for O(1) declared-name checks
- Stack
  - `Analyzer::braceStack_` tracks `{}`, `()`, `[]` for indentation and balance
  - `SymbolTable::scopes_` (stack of scope sets) for local vs global
- Trie (Prefix Tree) — `src/Trie.*`
  - Stores curated C++ dictionary (~140+ words) and priorities
  - `Trie::getSuggestions` returns closest matches
- Dynamic Programming (DP)
  - `Trie::editDistance` (Levenshtein) used to score typo distance
- Sorting (rank best suggestions)
  - `std::sort` in `Trie::getSuggestions` to prioritize keywords (`cout` over `const`)
- FSM (Finite State Machine)
  - `Tokenizer::tokenize` is an FSM that transitions across states (idle/string/comment/etc.)

Code snippets (excerpts):

```cpp
// Trie::editDistance (DP) — src/Trie.cpp
int Trie::editDistance(const std::string &a, const std::string &b){
    const int n = a.size(), m = b.size();
    std::vector<int> dp(m + 1);
    for (int j = 0; j <= m; ++j) dp[j] = j;
    for (int i = 1; i <= n; ++i){
        int prev = dp[0]; dp[0] = i;
        for (int j = 1; j <= m; ++j){
            int tmp = dp[j];
            dp[j] = (a[i-1]==b[j-1]) ? prev : std::min({prev+1, dp[j]+1, dp[j-1]+1});
            prev = tmp;
        }
    }
    return dp[m];
}
```

---

## 5) Features & Edge Cases Handled

What it fixes:
- `cout > x` → `cout << x` (and chains)
- `cn < x` → `cin >> x`
- `for(i=0 i<n i++)` → `for(i=0; i<n; i++)`
- `incldue <iostream>` → `#include <iostream>`
- `fr(a,b)` → `for(a,b)` (when in keyword context)
- `nitmain()` → `main()`
- `'hello'` → `"hello"` (multi-char char literal → string)

What it ignores:
- `// comments` and string literals `"inside quotes"`
- Valid commas like `for(int i=0, j=0; ...)`
- Short variables like `i`, `x`, `ni` (won’t become `if`/`cin`)

---

## 6) How to Use (Build & Run)

Prerequisite: a C++17 compiler (g++/clang++)

Build (Windows PowerShell):

```powershell
g++ -std=c++17 -Wall -Wextra -I src src/main.cpp src/Utils.cpp src/Trie.cpp src/Logger.cpp src/SymbolTable.cpp src/Autocorrect.cpp src/Tokenizer.cpp -o IntelliFixPP.exe
```

Run:

```powershell
.\n+IntelliFixPP.exe
```

Modes:
- Select 1 for Interactive (type lines, see fixes)
- Select 2 for File Mode and provide a .cpp path

---

## 7) Known Limitations (To Watch Out For)

- Semantic analysis not active yet: 
  - Using undeclared variables won’t be flagged (e.g., `y = 5;` passes)
- Line-by-line engine: 
  - Complex multi-line constructs (e.g., split statements) may lose context
- English comments:
  - Spell-checker intentionally ignores natural language inside comments
- Some rare variable names that resemble keywords (e.g., `fo`) may be over-corrected
- Preprocessor `define` edge cases: inserting `#` and avoiding `;` is planned refinement

---

## 8) Next Steps — Phase 2 Plan

- Activate and fully integrate `SymbolTable` for semantic checks
  - “Use of undeclared identifier”, duplicate declarations, scope-aware protections
- Smarter space-preservation around split identifiers (e.g., `inti` → `int i`)
- Improve nested statement semicolon inference inside `{}` blocks
- Expand dictionary and prioritization for STL and headers

---

## 9) Accessibility Impact

By automatically fixing common syntax errors (like missing `;`, wrong stream operators, and typos), IntelliFix++ reduces friction and cognitive load for learners. This is especially helpful for users who experience motor or cognitive challenges and benefit from tools that reduce repetitive correction tasks.

---

## 10) Project Structure

```
IntelliFixPP/
├── src/
│   ├── main.cpp
│   ├── Utils.h/.cpp          # Analyzer (token-based pipeline, fixers)
│   ├── Tokenizer.h/.cpp      # FSM tokenizer
│   ├── Trie.h/.cpp           # Dictionary + edit distance + ranking
│   ├── Autocorrect.h/.cpp    # Operator rules and helpers
│   ├── Logger.h/.cpp         # Logging (fixes, analysis)
│   └── SymbolTable.h/.cpp    # Scopes (planned Phase 2 integration)
├── tests/                    # Test drivers and scenarios
└── output/                   # Logs and corrected outputs
```

---

## 11) Quick Examples

```text
Input : cot > 'hello'
Output: cout << "hello";

Input : for(i=0 i<10 i++)
Output: for(i=0; i<10; i++)

Input : incldue <iostream>
Output: #include <iostream>
```

---

© 2025 Syed Maaz Ali — DSA-INTELLIFIXPP-Project

---

## 📄 License

This project is licensed under the MIT License - see the LICENSE file for details.

---

## 👥 Authors

- **Maaz Ali** - *Initial work & Phase 1 implementation* - [GitHub Profile](https://github.com/Maaz-Ali0102)

---

## 🙏 Acknowledgments

- Inspired by classic compiler design techniques (Dragon Book)
- Trie implementation based on standard prefix tree algorithms
- Levenshtein distance algorithm from dynamic programming literature
- Thanks to the C++ community for STL and modern language features

---

## 📧 Contact

For questions, issues, or feature requests:
- **GitHub Issues**: [Create an issue](https://github.com/Maaz-Ali0102/DSA-INTELLIFIXPP-Project/issues)
- **GitHub**: [@Maaz-Ali0102](https://github.com/Maaz-Ali0102)
- **Email**: maazali0102@gmail.com
- **LinkedIn**: [Maazali0102](https://www.linkedin.com/in/maazali0102/)

---

**⭐ If you find this project helpful, please star the repository!**
