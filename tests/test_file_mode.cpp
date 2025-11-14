#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include "Trie.h"
#include "SymbolTable.h"
#include "Logger.h"
#include "Utils.h"

using namespace std;

vector<string> readAllLines(const string &path){
    ifstream in(path);
    vector<string> lines;
    string s;
    while (getline(in, s)) lines.push_back(s);
    return lines;
}

bool writeAllLines(const string &path, const vector<string> &lines){
    ofstream out(path);
    if (!out.is_open()) return false;
    for (auto &l : lines) out << l << "\n";
    return true;
}

int main(){
    cout << "=== File Upload Mode Test ===" << endl;
    
    Trie trie;
    SymbolTable sym;
    Logger logger;
    logger.init("output");
    Analyzer analyzer(trie, sym, logger);
    
    string inputFile = "test_sample.cpp";
    string outputFile = "corrected_test_sample.cpp";
    
    cout << "\nReading: " << inputFile << endl;
    auto lines = readAllLines(inputFile);
    
    cout << "Processing " << lines.size() << " lines..." << endl;
    
    vector<string> fileIssues;
    auto fixed = analyzer.processFile(lines, fileIssues);
    
    cout << "\n--- File Analysis Summary ---" << endl;
    if (!fileIssues.empty()){
        for (const auto& issue : fileIssues){
            cout << "  - " << issue << endl;
        }
    } else {
        cout << "  No issues found." << endl;
    }
    
    if (writeAllLines(outputFile, fixed)){
        cout << "\n[+] Corrected file saved: " << outputFile << endl;
    } else {
        cout << "[-] Failed to save corrected file." << endl;
    }
    
    cout << "\n--- Original vs Corrected ---" << endl;
    for (size_t i = 0; i < lines.size() && i < fixed.size(); ++i){
        if (lines[i] != fixed[i]){
            cout << "Line " << (i+1) << ":" << endl;
            cout << "  Original:  " << lines[i] << endl;
            cout << "  Corrected: " << fixed[i] << endl;
        }
    }
    
    logger.flush();
    cout << "\n[i] Detailed logs saved to: output/fixes.log" << endl;
    
    return 0;
}
