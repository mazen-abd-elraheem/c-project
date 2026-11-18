#ifndef ATM_H
#define ATM_H

#include <string>
#include <map>
#include "Account.h"
#include "FileHandler.h"

class ATM {
private:
    std::string atmId;
    double cashReserve;
    std::map<std::string, Account> accounts;
    Account* currentAccount;
    FileHandler fileHandler;

    // Helper functions
    std::string getCurrentDateTime() const;
    Account* findAccountByNumber(const std::string& accountNumber);
    void saveState();
    void showHeader(const std::string& title) const;
    void printReceipt(const std::string& txType, double amount, double balance, const std::string& details = "");
    bool askPrintReceipt(const std::string& txType, double amount, double balance, const std::string& details = "");
    bool askAnotherTransaction();
    void showErrorMessage(const std::string& message) const;

    // Flowchart operations
    void welcomeScreen();
    bool authenticateUser();
    void showMainMenu();
    void handleBalanceInquiry();
    void handleWithdrawal();
    void handleDeposit();
    void handleTransfer();
    void handleBillPayment();
    void handleMiniStatement();
    void handleChangePassword();
    void logoutSession();

public:
    ATM();
    void run();
};

#endif // ATM_H
