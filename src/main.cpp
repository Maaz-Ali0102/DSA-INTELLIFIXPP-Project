#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <iomanip>
#include <filesystem>
#ifdef _WIN32
#include <windows.h>
#endif

#include "Trie.h"
#include "SymbolTable.h"
#include "Utils.h"
#include "Logger.h"

using namespace std;

static void openAnalysisWindow(const std::string &analysisPath){
#ifdef _WIN32
    // Spawn a persistent console that tails the analysis file
    std::string cmd = "cmd /c start \"IntelliFix++ Log\" powershell -NoExit -Command \"Get-Content -Path '" + analysisPath + "' -Wait -Tail 200\"";
    system(cmd.c_str());
#else
    (void)analysisPath;
#endif
}

static vector<string> readAllLines(const string &path){
    ifstream in(path);
    vector<string> lines;
    string s;
    while (std::getline(in, s)) lines.push_back(s);
    return lines;
}

static bool writeAllLines(const string &path, const vector<string> &lines){
    ofstream out(path);
    if (!out.is_open()) return false;
    for (auto &l : lines) out << l << "\n";
    return true;
}

int main(){
    namespace fs = std::filesystem;

    cout << "IntelliFix++ — C++ autocorrect and suggestions" << endl;
    cout << "-------------------------------------------" << endl;

    // Prepare core services
    Trie trie; SymbolTable sym; Logger logger;
    std::string outDir = (fs::path("c:/Users/iComputers/Documents/IntelliFixPP/output")).string();
    logger.init(outDir);
    Analyzer analyzer(trie, sym, logger);

    // Launch a persistent analysis/log window once
    openAnalysisWindow(logger.analysisPath());

    while (true){
        cout << "\nSelect mode:\n";
        cout << "  1) Interactive (line-by-line)\n";
        cout << "  2) File upload / batch fix\n";
        cout << "  0) Exit\n> ";
        string choice; if (!std::getline(cin, choice)) break;
        if (choice == "0") break;
        else if (choice == "1"){
            cout << "\n+============================================================+" << endl;
            cout << "|          IntelliFix++ Interactive Mode                     |" << endl;
            cout << "+============================================================+" << endl;
            cout << "\nCommands:" << endl;
            cout << "  :quit        - Exit (with bracket validation)" << endl;
            cout << "  :save <file> - Save code (with bracket validation)" << endl;
            cout << "  :clear       - Clear screen" << endl;
            cout << "  :show        - Show current code buffer" << endl;
            cout << "  :show logs   - Display internal correction logs" << endl;
            cout << "\nStart typing your C++ code:\n" << endl;
            
            size_t lineNo = 1;
            vector<string> buffer;
            vector<string> internalLog; // Store internal log for :show logs
            
            while (true){
                cout << "line " << lineNo << ": ";
                string line; if (!std::getline(cin, line)) break;
                if (!line.empty() && line[0] == ':'){
                    // Handle interactive commands
                    std::istringstream iss(line.substr(1));
                    std::string cmd; iss >> cmd;
                    
                    if (cmd == "quit"){
                        // Check for unmatched brackets before quitting
                        auto unclosedBrackets = analyzer.getUnclosedBrackets();
                        if (!unclosedBrackets.empty()){
                            cout << "\n[!] WARNING: Unmatched brackets detected before exit!" << endl;
                            for (const auto& warning : unclosedBrackets){
                                cout << "  - " << warning << endl;
                            }
                            cout << "\nAre you sure you want to quit? (y/n): ";
                            string confirm;
                            if (std::getline(cin, confirm) && (confirm == "y" || confirm == "Y")){
                                logger.writeAnalysis({"[session] quit with bracket warnings"});
                                logger.flush();
                                return 0;
                            } else {
                                cout << "Exit cancelled. Continue editing.\n" << endl;
                                continue;
                            }
                        }
                        logger.writeAnalysis({"[session] quit"});
                        logger.flush();
                        return 0;
                        
                    } else if (cmd == "save"){
                        // Check for unmatched brackets before saving
                        auto unclosedBrackets = analyzer.getUnclosedBrackets();
                        if (!unclosedBrackets.empty()){
                            cout << "\n[!] WARNING: Unmatched brackets detected before save!" << endl;
                            for (const auto& warning : unclosedBrackets){
                                cout << "  - " << warning << endl;
                            }
                            cout << "\nDo you want to save anyway? (y/n): ";
                            string confirm;
                            if (!std::getline(cin, confirm) || (confirm != "y" && confirm != "Y")){
                                cout << "Save cancelled.\n" << endl;
                                continue;
                            }
                        }
                        
                        std::string path; iss >> std::ws; std::getline(iss, path);
                        if (path.empty()){
                            cout << "Usage: :save <file>\n" << endl;
                        } else if (writeAllLines(path, buffer)){
                            cout << "[+] Saved to: " << path << "\n" << endl;
                            logger.writeAnalysis({"[session] saved to " + path});
                            logger.flush();
                        } else {
                            cout << "[-] Failed to save: " << path << "\n" << endl;
                        }
                        continue;
                        
                    } else if (cmd == "clear"){
                        #ifdef _WIN32
                        system("cls");
                        #else
                        system("clear");
                        #endif
                        cout << "Screen cleared.\n" << endl;
                        continue;
                        
                    } else if (cmd == "show"){
                        std::string subcmd;
                        iss >> subcmd;
                        
                        if (subcmd == "logs"){
                            // Display internal correction logs
                            if (internalLog.empty()){
                                cout << "\n[i] No logs yet.\n" << endl;
                            } else {
                                cout << "\n+============================================================+" << endl;
                                cout << "|              INTERNAL CORRECTION LOGS                      |" << endl;
                                cout << "+============================================================+" << endl;
                                for (const auto& logEntry : internalLog){
                                    cout << logEntry << endl;
                                }
                                cout << "\n" << endl;
                            }
                        } else {
                            // Show current code buffer
                            if (buffer.empty()){
                                cout << "\n[i] Buffer is empty.\n" << endl;
                            } else {
                                cout << "\n+============================================================+" << endl;
                                cout << "|                  CURRENT CODE BUFFER                       |" << endl;
                                cout << "+============================================================+" << endl;
                                for (size_t i=0; i<buffer.size(); ++i){
                                    cout << setw(3) << (i+1) << " | " << buffer[i] << endl;
                                }
                                cout << "\n" << endl;
                            }
                        }
                        continue;
                        
                    } else {
                        cout << "[-] Unknown command: :" << cmd << endl;
                        cout << "Available: :quit, :save <file>, :clear, :show, :show logs\n" << endl;
                        continue;
                    }
                }
                
                // Process the line
                auto res = analyzer.processLine(line, lineNo);

                // ========== MAIN CONSOLE - SILENT (No suggestions shown here) ==========
                // Main console only shows OK/ERROR status, no detailed suggestions
                
                // ========== INTERNAL LOGGING (Hidden from console) ==========
                // Build internal log entry
                string logEntry = "\n--- Line " + to_string(lineNo) + " ---";
                internalLog.push_back(logEntry);
                internalLog.push_back("Original:  " + res.original);
                
                if (res.changed){
                    internalLog.push_back("Corrected: " + res.corrected);
                }
                
                if (!res.issues.empty()){
                    internalLog.push_back("Issues:");
                    for (const auto& issue : res.issues){
                        internalLog.push_back("  - " + issue);
                    }
                } else {
                    internalLog.push_back("Status: No issues detected");
                }

                // ========== LOG WINDOW OUTPUT (analysis.txt - shown in separate window) ==========
                vector<string> fileLog;
                fileLog.push_back("\n+============================================================+");
                fileLog.push_back("|  Line " + to_string(lineNo) + " Analysis");
                fileLog.push_back("+============================================================+");
                fileLog.push_back("Original:  " + res.original);
                
                if (res.changed){
                    fileLog.push_back("Corrected: " + res.corrected);
                    fileLog.push_back("");
                }
                
                if (!res.issues.empty()){
                    fileLog.push_back("Issues detected:");
                    for (auto &msg : res.issues){
                        if (msg.find("unmatched") != string::npos || 
                            msg.find("missing") != string::npos){
                            fileLog.push_back("  [!] " + msg);
                        } else {
                            fileLog.push_back("  [-] " + msg);
                        }
                    }
                    fileLog.push_back("");
                }
                
                if (res.changed){
                    fileLog.push_back("Suggested correction: " + res.corrected);
                } else {
                    fileLog.push_back("Status: No changes needed");
                }
                fileLog.push_back("");
                
                logger.writeAnalysis(fileLog);
                logger.flush();

                // ========== MAIN CONSOLE - User decision prompt ==========
                if (res.changed){
                    cout << "Do you want to apply these changes? (Y/N): ";
                } else {
                    cout << "Continue? (Y/N): ";
                }
                
                string ans; 
                std::getline(cin, ans);
                
                if (!ans.empty() && (ans[0]=='Y' || ans[0]=='y')){
                    buffer.push_back(res.corrected);
                    internalLog.push_back("Decision: Applied correction");
                    logger.writeAnalysis({"[Decision] User applied correction"});
                } else {
                    buffer.push_back(res.original);
                    internalLog.push_back("Decision: Kept original");
                    logger.writeAnalysis({"[Decision] User kept original"});
                }
                
                logger.flush();
                ++lineNo;
            }
        } else if (choice == "2"){
            cout << "Enter input filename (absolute or relative): ";
            string path; if (!std::getline(cin, path)) continue;
            if (!fs::exists(path)){
                cout << "File not found: " << path << endl;
                continue;
            }
            auto lines = readAllLines(path);
            vector<string> fileIssues;
            auto fixed = analyzer.processFile(lines, fileIssues);

            // Display file issues including bracket warnings
            if (!fileIssues.empty()){
                cout << "\n[i] File Analysis Summary:" << endl;
                for (const auto& issue : fileIssues){
                    if (issue.find("missing") != string::npos || 
                        issue.find("inserted") != string::npos){
                        cout << "  [!] " << issue << endl;
                    } else {
                        cout << "  [-] " << issue << endl;
                    }
                }
            }

            // Prepare output path
            fs::path inPath(path);
            string outName = "corrected_" + inPath.filename().string();
            fs::path outPath = inPath.parent_path() / outName;

            if (writeAllLines(outPath.string(), fixed)){
                cout << "\n[+] Wrote corrected file: " << outPath.string() << endl;
            } else {
                cout << "Failed to write corrected file: " << outPath.string() << endl;
            }

            vector<string> analysis;
            analysis.push_back("Batch analysis summary:");
            for (auto &msg : fileIssues) analysis.push_back(" - " + msg);
            logger.writeAnalysis(analysis);
            logger.flush();

            cout << "Fix log: " << logger.fixesPath() << endl;
        } else {
            cout << "Unknown choice: " << choice << endl;
        }
    }

    cout << "Goodbye." << endl;
    return 0;
}
