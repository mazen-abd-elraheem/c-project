#include "FileHandler.h"
#include <fstream>
#include <iostream>
#include <filesystem>
#include <sstream>

namespace fs = std::filesystem;

FileHandler::FileHandler(const std::string& dataDirectory) 
    : dataDir(dataDirectory),
      accountsFilePath(dataDirectory + "/accounts.txt"),
      atmConfigFilePath(dataDirectory + "/atm_config.txt") {
    ensureDataDirectoryExists();
    initializeDefaultFilesIfMissing();
}

void FileHandler::ensureDataDirectoryExists() {
    try {
        if (!fs::exists(dataDir)) {
            fs::create_directories(dataDir);
        }
    } catch (const std::exception& e) {
        std::cerr << "[ERROR] Could not create data directory: " << e.what() << "\n";
    }
}

void FileHandler::initializeDefaultFilesIfMissing() {
    if (!fs::exists(accountsFilePath)) {
        std::ofstream outFile(accountsFilePath);
        if (outFile.is_open()) {
            outFile << "john|1234|ACC1001|2500.00|0|0\n";
            outFile << "sarah|4321|ACC1002|1500.00|0|0\n";
            outFile << "ahmed|9999|ACC1003|5000.00|0|0\n";
            outFile.close();
        }

        // Add some initial transactions
        Transaction t1("2026-11-15 10:30:00", "INITIAL_DEPOSIT", 2500.00, 2500.00);
        appendTransaction("ACC1001", t1);

        Transaction t2("2026-11-15 11:15:00", "INITIAL_DEPOSIT", 1500.00, 1500.00);
        appendTransaction("ACC1002", t2);

        Transaction t3("2026-11-15 12:00:00", "INITIAL_DEPOSIT", 5000.00, 5000.00);
        appendTransaction("ACC1003", t3);
    }

    if (!fs::exists(atmConfigFilePath)) {
        std::ofstream cfgFile(atmConfigFilePath);
        if (cfgFile.is_open()) {
            cfgFile << "ATM_ID=ATM-001\n";
            cfgFile << "CASH_RESERVE=50000.00\n";
            cfgFile.close();
        }
    }
}

std::map<std::string, Account> FileHandler::loadAccounts() {
    std::map<std::string, Account> accounts;
    std::ifstream inFile(accountsFilePath);
    if (!inFile.is_open()) {
        return accounts;
    }

    std::string line;
    while (std::getline(inFile, line)) {
        if (line.empty() || line[0] == '#') continue;
        Account acc = Account::deserialize(line);
        if (!acc.getUsername().empty()) {
            std::vector<Transaction> txs = loadTransactions(acc.getAccountNumber());
            acc.setTransactions(txs);
            accounts[acc.getUsername()] = acc;
        }
    }
    inFile.close();
    return accounts;
}

bool FileHandler::saveAccounts(const std::map<std::string, Account>& accounts) {
    std::ofstream outFile(accountsFilePath, std::ios::trunc);
    if (!outFile.is_open()) {
        return false;
    }

    for (const auto& pair : accounts) {
        outFile << pair.second.serialize() << "\n";
    }
    outFile.close();
    return true;
}

std::vector<Transaction> FileHandler::loadTransactions(const std::string& accountNumber) {
    std::vector<Transaction> list;
    std::string txPath = dataDir + "/transactions_" + accountNumber + ".txt";
    std::ifstream inFile(txPath);
    if (!inFile.is_open()) {
        return list;
    }

    std::string line;
    while (std::getline(inFile, line)) {
        if (line.empty() || line[0] == '#') continue;
        Transaction tx = Transaction::deserialize(line);
        if (!tx.getType().empty()) {
            list.push_back(tx);
        }
    }
    inFile.close();
    return list;
}

bool FileHandler::appendTransaction(const std::string& accountNumber, const Transaction& tx) {
    std::string txPath = dataDir + "/transactions_" + accountNumber + ".txt";
    std::ofstream outFile(txPath, std::ios::app);
    if (!outFile.is_open()) {
        return false;
    }
    outFile << tx.serialize() << "\n";
    outFile.close();
    return true;
}

bool FileHandler::loadATMConfig(std::string& atmId, double& cashReserve) {
    std::ifstream inFile(atmConfigFilePath);
    if (!inFile.is_open()) {
        atmId = "ATM-001";
        cashReserve = 50000.00;
        return false;
    }

    atmId = "ATM-001";
    cashReserve = 50000.00;
    std::string line;
    while (std::getline(inFile, line)) {
        size_t eqPos = line.find('=');
        if (eqPos != std::string::npos) {
            std::string key = line.substr(0, eqPos);
            std::string val = line.substr(eqPos + 1);
            if (key == "ATM_ID") {
                atmId = val;
            } else if (key == "CASH_RESERVE") {
                try {
                    cashReserve = std::stod(val);
                } catch (...) {}
            }
        }
    }
    inFile.close();
    return true;
}

bool FileHandler::saveATMConfig(const std::string& atmId, double cashReserve) {
    std::ofstream outFile(atmConfigFilePath, std::ios::trunc);
    if (!outFile.is_open()) {
        return false;
    }
    outFile << "ATM_ID=" << atmId << "\n";
    outFile << "CASH_RESERVE=" << cashReserve << "\n";
    outFile.close();
    return true;
}

bool FileHandler::appendReceipt(const std::string& receiptText, const std::string& filename) {
    std::ofstream outFile(filename, std::ios::app);
    if (!outFile.is_open()) {
        return false;
    }
    outFile << receiptText << "\n";
    outFile.close();
    return true;
}
