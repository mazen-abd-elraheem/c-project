#include "Transaction.h"
#include <sstream>
#include <iomanip>

Transaction::Transaction() : timestamp(""), type(""), amount(0.0), balanceAfter(0.0) {}

Transaction::Transaction(const std::string& timestamp, const std::string& type, double amount, double balanceAfter)
    : timestamp(timestamp), type(type), amount(amount), balanceAfter(balanceAfter) {}

std::string Transaction::getTimestamp() const {
    return timestamp;
}

std::string Transaction::getType() const {
    return type;
}

double Transaction::getAmount() const {
    return amount;
}

double Transaction::getBalanceAfter() const {
    return balanceAfter;
}

std::string Transaction::serialize() const {
    std::ostringstream oss;
    oss << timestamp << "|" << type << "|" << std::fixed << std::setprecision(2) << amount << "|" << balanceAfter;
    return oss.str();
}

Transaction Transaction::deserialize(const std::string& line) {
    std::istringstream iss(line);
    std::string timeStr, typeStr, amtStr, balStr;
    if (std::getline(iss, timeStr, '|') &&
        std::getline(iss, typeStr, '|') &&
        std::getline(iss, amtStr, '|') &&
        std::getline(iss, balStr, '|')) {
        try {
            double amt = std::stod(amtStr);
            double bal = std::stod(balStr);
            return Transaction(timeStr, typeStr, amt, bal);
        } catch (...) {
            return Transaction();
        }
    }
    return Transaction();
}
