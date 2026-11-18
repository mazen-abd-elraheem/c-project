#ifndef FILEHANDLER_H
#define FILEHANDLER_H

#include <string>
#include <vector>
#include <map>
#include "Account.h"
#include "Transaction.h"

class FileHandler {
private:
    std::string accountsFilePath;
    std::string atmConfigFilePath;
    std::string dataDir;

public:
    FileHandler(const std::string& dataDirectory = "data");

    void ensureDataDirectoryExists();
    void initializeDefaultFilesIfMissing();

    std::map<std::string, Account> loadAccounts();
    bool saveAccounts(const std::map<std::string, Account>& accounts);

    std::vector<Transaction> loadTransactions(const std::string& accountNumber);
    bool appendTransaction(const std::string& accountNumber, const Transaction& tx);

    bool loadATMConfig(std::string& atmId, double& cashReserve);
    bool saveATMConfig(const std::string& atmId, double cashReserve);

    bool appendReceipt(const std::string& receiptText, const std::string& filename = "receipt.txt");
};

#endif // FILEHANDLER_H
