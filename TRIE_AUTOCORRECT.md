# Trie Auto-Correction Enhancement

## Overview
The Trie class now supports intelligent auto-correction using edit distance (Levenshtein distance) to suggest corrections for misspelled C++ keywords, identifiers, and standard library names.

## New API

### `getSuggestions()`
```cpp
std::vector<std::string> getSuggestions(const std::string& word, int maxDistance = 2) const;
```

**Parameters:**
- `word`: The potentially misspelled word to correct
- `maxDistance`: Maximum edit distance to consider (default: 2)

**Returns:**
- Vector of suggested corrections, sorted by edit distance (closest first)
- Empty vector if no suggestions found within maxDistance

**Edit Distance:**
The Levenshtein distance counts the minimum number of single-character edits (insertions, deletions, or substitutions) needed to transform one string into another.

## Dictionary Contents

The dictionary is pre-seeded with:

### Preprocessor Directives
- `#include`, `include`, `define`, `pragma`, `ifdef`, `ifndef`, `endif`, `undef`

### C++ Keywords
- Control flow: `if`, `else`, `for`, `while`, `do`, `switch`, `case`, `default`, `break`, `continue`, `return`
- Types: `int`, `float`, `double`, `char`, `long`, `short`, `void`, `bool`, `auto`
- Classes: `class`, `struct`, `union`, `enum`, `public`, `private`, `protected`
- Namespaces: `namespace`, `using`, `typedef`, `typename`, `template`
- Modifiers: `const`, `static`, `extern`, `register`, `volatile`, `mutable`, `virtual`, `override`, `final`, `explicit`, `inline`
- Memory: `new`, `delete`, `this`, `nullptr`
- Literals: `true`, `false`
- Exceptions: `try`, `catch`, `throw`
- Operators: `sizeof`, `typeid`, `const_cast`, `dynamic_cast`, `reinterpret_cast`, `static_cast`

### Standard Library Headers
- `iostream`, `<iostream>`, `vector`, `<vector>`, `string`, `<string>`
- `map`, `unordered_map`, `set`, `unordered_set`
- `algorithm`, `utility`, `memory`, `iomanip`, `sstream`, `fstream`
- `cstdio`, `cstdlib`, `cstring`, `cmath`, `limits`
- `bitset`, `deque`, `queue`, `stack`, `list`, `array`
- `iterator`, `tuple`, `optional`, `variant`, `any`

### Common std Identifiers
- I/O: `std`, `cout`, `cin`, `cerr`, `endl`, `main`
- Types: `size_t`, `ptrdiff_t`, `nullptr_t`
- Container operations: `begin`, `end`, `size`, `empty`, `clear`, `push_back`, `pop_back`, `insert`, `erase`, `find`, `count`, `at`, `front`, `back`
- Utilities: `make_pair`, `make_unique`, `make_shared`, `sort`, `reverse`, `swap`, `min`, `max`

## Usage Examples

### Example 1: Basic Correction
```cpp
Trie trie;
trie.insert("include");
trie.insert("iostream");
trie.insert("main");

// Get suggestions for typo with maxDistance=2
auto suggestions = trie.getSuggestions("includ", 2);
// Returns: ["include"]

suggestions = trie.getSuggestions("iotream", 2);
// Returns: ["iostream"]

suggestions = trie.getSuggections("mian", 2);
// Returns: ["main"]
```

### Example 2: Multiple Suggestions
```cpp
Trie trie;
trie.insert("int");
trie.insert("in");
trie.insert("inline");

auto suggestions = trie.getSuggestions("in", 1);
// Returns: ["in", "int"] (sorted by distance, then alphabetically)
```

### Example 3: No Suggestions
```cpp
auto suggestions = trie.getSuggestions("xyz123", 2);
// Returns: [] (empty - no words within distance 2)
```

### Example 4: Integration with Analyzer
The `Analyzer` class automatically uses `getSuggestions()` internally:

```cpp
Trie trie;
SymbolTable sym;
Logger logger;
logger.init("output");

Analyzer analyzer(trie, sym, logger);

// Process a line with typos
auto result = analyzer.processLine("#includ <iotream>", 1);
// Automatically corrects to: "#include <iostream>"
```

## Performance Characteristics

- **Time Complexity:** O(n * m) per word comparison, where n and m are word lengths
- **Space Complexity:** O(k * m) where k is dictionary size, m is average word length
- **Optimization:** Results are sorted once, so getting top suggestions is efficient

## Testing

Run the included test program:
```bash
g++ -std=c++17 -Wall -Wextra -I. test_trie.cpp src/Trie.cpp -o output/test_trie.exe
./output/test_trie.exe
```

## Common Typo Corrections

| Typo | Suggestion | Distance |
|------|------------|----------|
| `includ` | `include` | 1 |
| `iotream` | `iostream` | 2 |
| `mian` | `main` | 2 |
| `intt` | `int` | 1 |
| `retur` | `return` | 1 |
| `vectro` | `vector` | 2 |
| `strig` | `string` | 2 |
| `virtuall` | `virtual` | 2 |
| `privat` | `private` | 1 |
| `namepsace` | `namespace` | 2 |
| `fales` | `false` | 2 |

## Future Enhancements

Potential improvements:
- Cache edit distance calculations for frequently queried words
- Use BK-tree or similar structure for faster distance-based lookups
- Add context-aware suggestions (e.g., suggest headers after `#include`)
- Support for user-defined dictionaries
- Phonetic similarity matching (Soundex, Metaphone)
