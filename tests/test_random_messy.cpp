#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "../src/Utils.h"
#include "../src/Trie.h"

using namespace std;

int main() {
    cout << "\n╔════════════════════════════════════════╗\n";
    cout << "║   RANDOM MESSY CODE TEST               ║\n";
    cout << "║   Testing with UGLY student code       ║\n";
    cout << "╚════════════════════════════════════════╝\n\n";
    
    // Read messy file
    ifstream inFile("tests/random_messy_code.cpp");
    vector<string> lines;
    string line;
    while (getline(inFile, line)) {
        lines.push_back(line);
    }
    inFile.close();
    
    cout << "✓ Read " << lines.size() << " lines of messy code\n\n";
    
    // Process
    Trie trie;
    SymbolTable sym;
    Logger logger;
    Analyzer analyzer(trie, sym, logger);
    
    vector<string> fileIssues;
    vector<string> corrected = analyzer.processFile(lines, fileIssues);
    
    // Write corrected
    ofstream outFile("output/random_messy_code_CORRECTED.cpp");
    for (const auto& correctedLine : corrected) {
        outFile << correctedLine << "\n";
    }
    outFile.close();
    
    cout << "✓ Fixed " << fileIssues.size() << " issues\n";
    cout << "✓ Output written to: output/random_messy_code_CORRECTED.cpp\n\n";
    
    // Show side-by-side comparison for first 20 lines
    cout << "========================================\n";
    cout << "   SIDE-BY-SIDE COMPARISON (First 20)\n";
    cout << "========================================\n\n";
    
    for (size_t i = 0; i < min((size_t)20, lines.size()); i++) {
        cout << "Line " << (i+1) << ":\n";
        cout << "  BEFORE: \"" << lines[i] << "\"\n";
        cout << "  AFTER:  \"" << corrected[i] << "\"\n\n";
    }
    
    cout << "╔════════════════════════════════════════╗\n";
    cout << "║   TEST COMPLETE!                       ║\n";
    cout << "╚════════════════════════════════════════╝\n\n";
    
    return 0;
}
