#ifndef ACCOUNT_H
#define ACCOUNT_H

#include <string>
#include <vector>
#include "Transaction.h"

class Account {
private:
    std::string username;
    std::string pin;
    std::string accountNumber;
    double balance;
    bool isLocked;
    int failedAttempts;
    std::vector<Transaction> transactions;

public:
    Account();
    Account(const std::string& username, const std::string& pin, const std::string& accountNumber,
            double balance, bool isLocked = false, int failedAttempts = 0);

    std::string getUsername() const;
    std::string getPin() const;
    std::string getAccountNumber() const;
    double getBalance() const;
    bool getIsLocked() const;
    int getFailedAttempts() const;
    const std::vector<Transaction>& getTransactions() const;

    void setBalance(double newBalance);
    void setPin(const std::string& newPin);
    void setIsLocked(bool locked);
    void setFailedAttempts(int attempts);

    bool checkPin(const std::string& inputPin) const;
    void incrementFailedAttempts();
    void resetFailedAttempts();
    void lockAccount();

    bool debit(double amount);
    void credit(double amount);

    void addTransaction(const Transaction& tx);
    void setTransactions(const std::vector<Transaction>& txs);

    std::string serialize() const;
    static Account deserialize(const std::string& line);
};

#endif // ACCOUNT_H
