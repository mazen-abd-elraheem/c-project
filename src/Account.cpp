#include "Account.h"
#include <sstream>
#include <iomanip>

Account::Account() 
    : username(""), pin(""), accountNumber(""), balance(0.0), isLocked(false), failedAttempts(0) {}

Account::Account(const std::string& username, const std::string& pin, const std::string& accountNumber,
                 double balance, bool isLocked, int failedAttempts)
    : username(username), pin(pin), accountNumber(accountNumber),
      balance(balance), isLocked(isLocked), failedAttempts(failedAttempts) {}

std::string Account::getUsername() const {
    return username;
}

std::string Account::getPin() const {
    return pin;
}

std::string Account::getAccountNumber() const {
    return accountNumber;
}

double Account::getBalance() const {
    return balance;
}

bool Account::getIsLocked() const {
    return isLocked;
}

int Account::getFailedAttempts() const {
    return failedAttempts;
}

const std::vector<Transaction>& Account::getTransactions() const {
    return transactions;
}

void Account::setBalance(double newBalance) {
    balance = newBalance;
}

void Account::setPin(const std::string& newPin) {
    pin = newPin;
}

void Account::setIsLocked(bool locked) {
    isLocked = locked;
}

void Account::setFailedAttempts(int attempts) {
    failedAttempts = attempts;
}

bool Account::checkPin(const std::string& inputPin) const {
    return pin == inputPin;
}

void Account::incrementFailedAttempts() {
    failedAttempts++;
    if (failedAttempts >= 3) {
        isLocked = true;
    }
}

void Account::resetFailedAttempts() {
    failedAttempts = 0;
}

void Account::lockAccount() {
    isLocked = true;
}

bool Account::debit(double amount) {
    if (amount > 0 && balance >= amount) {
        balance -= amount;
        return true;
    }
    return false;
}

void Account::credit(double amount) {
    if (amount > 0) {
        balance += amount;
    }
}

void Account::addTransaction(const Transaction& tx) {
    transactions.push_back(tx);
}

void Account::setTransactions(const std::vector<Transaction>& txs) {
    transactions = txs;
}

std::string Account::serialize() const {
    std::ostringstream oss;
    oss << username << "|"
        << pin << "|"
        << accountNumber << "|"
        << std::fixed << std::setprecision(2) << balance << "|"
        << (isLocked ? 1 : 0) << "|"
        << failedAttempts;
    return oss.str();
}

Account Account::deserialize(const std::string& line) {
    std::istringstream iss(line);
    std::string user, p, accNo, balStr, lockedStr, attemptsStr;
    if (std::getline(iss, user, '|') &&
        std::getline(iss, p, '|') &&
        std::getline(iss, accNo, '|') &&
        std::getline(iss, balStr, '|') &&
        std::getline(iss, lockedStr, '|') &&
        std::getline(iss, attemptsStr, '|')) {
        try {
            double bal = std::stod(balStr);
            bool locked = (lockedStr == "1" || lockedStr == "true");
            int attempts = std::stoi(attemptsStr);
            return Account(user, p, accNo, bal, locked, attempts);
        } catch (...) {
            return Account();
        }
    }
    return Account();
}
