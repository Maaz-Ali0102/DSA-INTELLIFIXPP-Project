| Test ID | Test Case (Description) | Input (Line-by-Line) | Expected (Corrected) Output | Actual (Program) Output | Status (Pass/Fail) |
| :--- | :--- | :--- | :--- | :--- | :--- |
| TC-001 | cout Typo (`<` instead of `<<`) | `cout < "test";` | `cout << "test";` | `cout << "test";` | ✅ PASS |
| TC-002 | cout Logic Error (`>>` instead of `<<`) | `cout >> "test";` | `cout << "test";` | `cout << "test";` | ✅ PASS |
| TC-003 | cin Typo (`>` instead of `>>`) | `cin > myVar;` | `cin >> myVar;` | `cin >> myVar;` | ✅ PASS |
| TC-004 | cin Logic Error (`<<` instead of `>>`) | `cin << myVar;` | `cin >> myVar;` | `cin >> myVar;` | ✅ PASS |
| TC-005 | Comment Ignore | `// cout < "ignore";` | `// cout < "ignore";` (no change) | `// cout < "ignore";` | ✅ PASS |
| TC-006 | String Literal Ignore | `string s = "cout <";` | `string s = "cout <";` (no change) | `string s = "cout <";` | ✅ PASS |
| TC-007 | for Loop Comma Error | `for(int i=0, i<5, i++)` | `for(int i = 0; i < 5; i++)` | `for(int i = 0; i < 5; i++)` | ✅ PASS |
| TC-008 | for Loop Nested Function | `for(int i=0; i<max(a, b); i++)` | `for(int i = 0; i < max(a, b); i++)` (no change) | `for(int i = 0; i < max(a, b); i++)` | ✅ PASS |
| TC-009 | for Loop Valid Comma | `for(int i=0, j=0; i<10; i++)` | `for(int i = 0, j = 0; i < 10; i++)` (no change) | `for(int i = 0, j = 0; i < 10; i++)` | ✅ PASS |
| TC-010 | Missing Semicolon | `int x = 5` | `int x = 5;` | `int x = 5;` | ✅ PASS |
| TC-011 | Control Statement (No Semicolon) | `if(x > 0)` | `if(x > 0)` (no change) | `if(x > 0)` | ✅ PASS |
| TC-012 | Simple Brace (Auto-indent) | `int main(){` | `    int main(){` (indented) | `    int main(){` | ✅ PASS |
| TC-013 | Unmatched Brace (Auto-indent) | `void func(){ {` | `            void func(){ {` (indented) | `            void func(){ {` | ✅ PASS |
| TC-014 | Keyword Typo (`mian` -> `main`) | `int mian()` | `            int main();` (corrected + semicolon + indent) | `            int main();` | ✅ PASS |

## Test Summary
- **Total Tests:** 14
- **Passed:** 14 (100%)
- **Failed:** 0 (0%)
- **Test Date:** November 14, 2025
- **Build Version:** Phase 1 Complete (Token-based Pipeline)
