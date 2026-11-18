#ifndef TRANSACTION_H
#define TRANSACTION_H

#include <string>

class Transaction {
private:
    std::string timestamp;
    std::string type;
    double amount;
    double balanceAfter;

public:
    Transaction();
    Transaction(const std::string& timestamp, const std::string& type, double amount, double balanceAfter);

    std::string getTimestamp() const;
    std::string getType() const;
    double getAmount() const;
    double getBalanceAfter() const;

    std::string serialize() const;
    static Transaction deserialize(const std::string& line);
};

#endif // TRANSACTION_H
