# IntelliFix++ Quick Reference Guide

## Compilation

```powershell
cd c:\Users\iComputers\Documents\IntelliFixPP
g++ -std=c++17 -Wall -Wextra -I. src/main.cpp src/Utils.cpp src/Trie.cpp src/Logger.cpp src/SymbolTable.cpp -o output/intellifix.exe
```

## Running

```powershell
.\output\intellifix.exe
```

## Interactive Commands

| Command | Description |
|---------|-------------|
| `:quit` | Exit with bracket validation warning |
| `:save <filename>` | Save code with bracket validation |
| `:clear` | Clear the screen |
| `:show` | Display current code buffer |
| `:show logs` | Display internal correction logs |

## Features

### ✅ Syntax Corrections
- Header typos: `#includ` → `#include`
- Keyword typos: `mian` → `main`, `cinn` → `cin`, `cot`/`cut`/`out`/`ct` → `cout`
- Stream operators: `cout <` → `cout <<`, `cin >` → `cin >>`
- Missing semicolons
- For-loop fixes: commas → semicolons
- Auto-indentation

### ✅ Bracket Tracking (Stack-based)
- Tracks `{}`, `()`, `[]` in real-time
- Warns immediately on mismatch
- Pre-save validation
- Pre-quit validation
- Suggests missing brackets

### ✅ Clean UI
- **Console:** Only shows issues, suggestions, and decisions
- **Internal Log:** Full tracking of all corrections
- **`:show logs`:** View complete correction history on demand

## Example Session

```
line 1: #includ <iostream>

📋 Issues detected:
  💡 Corrected header directive: '#includ' → '#include'
✓ Suggested correction: #include <iostream>

Apply correction? (Y/N): y

line 2: int main( {

📋 Issues detected:
  ⚠️  unmatched '{' detected
✓ Suggested correction: int main() {

Apply correction? (Y/N): y

line 3: :show logs

╔════════════════════════════════════════════════════════╗
║              INTERNAL CORRECTION LOGS                  ║
╚════════════════════════════════════════════════════════╝

--- Line 1 ---
Original:  #includ <iostream>
Corrected: #include <iostream>
Issues:
  - Corrected header directive: '#includ' → '#include'
Decision: Applied correction

--- Line 2 ---
Original:  int main( {
Corrected: int main() {
Issues:
  - unmatched '{' detected
Decision: Applied correction
```

## File Modes

### Mode 1: Interactive (Line-by-Line)
- Type code interactively
- Get real-time feedback
- Decide to apply each correction
- View logs with `:show logs`

### Mode 2: Batch File Processing
- Upload existing file
- Automatic corrections
- Generates `corrected_<filename>.cpp`
- Summary displayed at end

## Logging

### Console Output (Clean)
- Issues detected
- Suggested corrections
- Bracket warnings

### Internal Memory Log
- All original lines
- All corrected lines
- All detected issues
- All user decisions
- Access with `:show logs`

### Persistent File Log
- Location: `output/analysis.txt`
- Format: `[line N] Original / Corrected / Issues`
- Appends across sessions
- Can be viewed in separate window

## Error Icons

| Icon | Meaning |
|------|---------|
| 💡 | Suggestion or correction |
| ⚠️  | Bracket error or warning |
| ✓ | Success or OK |
| ✅ | Completed action |
| ❌ | Failed action |
| 📋 | Issues detected |
| 📄 | Code buffer |

## Tips

1. **Check brackets frequently:** Use `:show logs` to see bracket state
2. **Save often:** Use `:save temp.cpp` to backup your work
3. **Clear for focus:** Use `:clear` for a clean workspace
4. **Review before quit:** Always check bracket warnings
5. **View buffer:** Use `:show` to see your complete code

## Troubleshooting

### "Permission denied" when compiling
- Close any running `main.exe` or `intellifix.exe`
- Compile to different name: `-o output/intellifix2.exe`

### Brackets not matching
- Use `:show logs` to see which line has the issue
- Type the missing bracket on a new line
- System will auto-close `}` at EOF if needed

### Corrections not appearing
- Make sure to type Y/y when prompted
- Check `:show logs` to see if correction was logged
- Check `output/analysis.txt` for persistent log
