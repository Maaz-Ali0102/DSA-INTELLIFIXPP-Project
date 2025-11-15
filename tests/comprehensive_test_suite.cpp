// COMPREHENSIVE TEST SUITE - Testing IntelliFixPP with various code scenarios
// This file contains: happy cases, typos, edge cases, messy real-world code

// ========== TEST CATEGORY 1: HAPPY CODE (Should work perfectly) ==========

#include <iostream>
using namespace std;

int main() {
    cout << "Hello World" << endl;
    int x = 10;
    for(int i = 0; i < 5; i++) {
        cout << i << endl;
    }
    return 0;
}

// ========== TEST CATEGORY 2: COMMON TYPOS (Should fix automatically) ==========

// Test 2.1: Main typos
int mian() {
    cot << "Testing" << endl;
}

// Test 2.2: Stream operator typos
void test_streams() {
    cut > "Hello";
    cn < x;
    cot > 'a';
}

// Test 2.3: Keyword typos
void test_keywords() {
    retrun 0;
    fi(x > 0) {
        whle(true) {
            break;
        }
    }
}

// Test 2.4: Include typos
incldue <iostream>
#inlcude <vector>
inclue <string>

// ========== TEST CATEGORY 3: FOR-LOOP ERRORS (Should fix) ==========

// Test 3.1: Missing semicolons
void test_forloop_missing_semicolons() {
    for(int i=0 i<10 i++) { }
    for(i=0i<ni++) { }
    fo(inti=0;i<5;i++) { }
}

// Test 3.2: Comma instead of semicolon
void test_forloop_commas() {
    for(int i=0, i<10, i++) { }
    for(i=0,i<n,i++) { }
}

// Test 3.3: No spaces
void test_forloop_nospace() {
    for(i=0i<ni++) { }
    for(intj=0j<10j++) { }
}

// ========== TEST CATEGORY 4: MESSY REAL-WORLD CODE ==========

// Test 4.1: Student code with multiple typos
int mian(){
    int n,sum=0;
    cot>"Enter n:";
    cn>n;
    fo(inti=1i<=ni++){
        sum+=i;
    }
    cut<<"Sum="<sum<endl;
    retrun 0;
}

// Test 4.2: Nested structures with typos
void messy_nested(){
    fi(x>0){
        whle(y<10){
            fo(inti=0i<5i++){
                cot>i>endl;
            }
        }
    }
}

// Test 4.3: Multiple statements per line (tricky)
int test_multiline(){intx=5;inty=10;cot>x>y;retrun x+y;}

// ========== TEST CATEGORY 5: EDGE CASES ==========

// Test 5.1: Empty statements
void test_empty() {
    ;;;
    {}
    for(;;) { break; }
}

// Test 5.2: Comments with typos (should NOT fix)
void test_comments() {
    // mian should stay as mian in comments
    /* cot should stay as cot */
    cout << "hello"; // retrun in comment stays
}

// Test 5.3: Strings with typos (should NOT fix)
void test_strings() {
    cout << "mian is not main here";
    string s = "cot cut cn retrun";
    char c = 'x';
}

// Test 5.4: Templates (tricky)
void test_templates() {
    vector<int> v;
    map<string,int> m;
    pair<int,int> p;
}

// Test 5.5: Pointers and references
void test_pointers() {
    int* ptr = nullptr;
    int& ref = x;
    int** doublePtr;
}

// ========== TEST CATEGORY 6: RANDOM MESSY CODE (Stress test) ==========

// Test 6.1: Everything wrong at once
int mian(){
    incldue<iostream>
    usng namspace std;
    intx=5,y=10;
    fo(inti=0i<ni++)
    {
        fi(i%2==0)
            cot>i>endl
        else
            cut>'Odd:'>i>endl;
    }
    retrun0
}

// Test 6.2: No spaces anywhere
void nospace(){intx=5;inty=10;fo(inti=0i<10i++){cot>i>endl;}}

// Test 6.3: Mixed quote types
void test_quotes() {
    cot>'hello';  // should become "hello"
    cut>'world';  // should become "world"
    cn>'test';    // should become "test"
    cout<<'a';    // should stay 'a'
}

// Test 6.4: Chain operators
void test_chains() {
    cot>'a'>'b'>'c';
    cut>"x">"y">"z";
    cot>x>y>z>endl;
}

// ========== TEST CATEGORY 7: VARIABLE NAMES (Should NOT corrupt) ==========

// Test 7.1: Short variable names
void test_short_vars() {
    int i = 0;
    int ni = 10;
    int fo = 5;
    int fi = 3;
    int cn = 7;
    for(i=0;i<ni;i++) {
        cout << fo << fi << cn;
    }
}

// Test 7.2: One-letter variables
void test_one_letter() {
    int a, b, c, d, e, f, g, h, i, j, k;
    int x, y, z;
    int n, m;
}

// ========== TEST CATEGORY 8: COMPLEX EXPRESSIONS ==========

// Test 8.1: Math expressions with operators
void test_math() {
    int result = (a+b)*(c-d)/e%f;
    bool flag = x>y && a<b || c==d;
    int bitwise = x&y|z^w;
}

// Test 8.2: Increment/decrement (should NOT change)
void test_increment() {
    i++;
    ++i;
    i--;
    --i;
    ni++;
    x--;
}

// Test 8.3: Array/pointer operations
void test_arrays() {
    int arr[10];
    arr[i] = 5;
    int* p = &arr[0];
    *p = 10;
    p[i] = 20;
}

// ========== TEST CATEGORY 9: PREPROCESSOR ==========

// Test 9.1: Define without #
defin MAX 100
define MIN 0

// Test 9.2: Include variations
incldue <vector>
#inlcude <map>
include <set>

// Test 9.3: Ifdef/ifndef
#idef DEBUG
#infdef RELEASE

// ========== TEST CATEGORY 10: UNUSUAL BUT VALID C++ ==========

// Test 10.1: Lambda expressions
void test_lambda() {
    auto f = [](int x) { return x*2; };
    auto g = [&]() { cout << x; };
}

// Test 10.2: Auto keyword
void test_auto() {
    auto x = 5;
    auto y = "hello";
    auto z = 3.14;
}

// Test 10.3: Range-based for
void test_range_for() {
    for(auto x : vec) {
        cout << x;
    }
}
