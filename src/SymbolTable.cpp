#include "SymbolTable.h"

SymbolTable::SymbolTable(){
    enterScope();
}

void SymbolTable::enterScope(){
    scopes_.push_back({});
}

void SymbolTable::exitScope(){
    if (!scopes_.empty()) scopes_.pop_back();
}

void SymbolTable::declare(const std::string &name){
    if (scopes_.empty()) enterScope();
    scopes_.back().insert(name);
}

bool SymbolTable::isDeclared(const std::string &name) const{
    for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it){
        if (it->count(name)) return true;
    }
    return false;
}
