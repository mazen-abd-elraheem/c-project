#include "ATM.h"
#include <iostream>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <limits>
#include <algorithm>

ATM::ATM() : currentAccount(nullptr) {
    accounts = fileHandler.loadAccounts();
    fileHandler.loadATMConfig(atmId, cashReserve);
}

std::string ATM::getCurrentDateTime() const {
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    std::tm ltm{};
#if defined(_WIN32) || defined(_WIN64)
    localtime_s(&ltm, &now_time);
#else
    localtime_r(&now_time, &ltm);
#endif
    char buffer[64];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &ltm);
    return std::string(buffer);
}

Account* ATM::findAccountByNumber(const std::string& accountNumber) {
    for (auto& pair : accounts) {
        if (pair.second.getAccountNumber() == accountNumber) {
            return &(pair.second);
        }
    }
    return nullptr;
}

void ATM::saveState() {
    fileHandler.saveAccounts(accounts);
    fileHandler.saveATMConfig(atmId, cashReserve);
}

void ATM::showHeader(const std::string& title) const {
    std::cout << "\n========================================\n";
    std::cout << "         " << title << "\n";
    std::cout << "========================================\n";
}

void ATM::showErrorMessage(const std::string& message) const {
    std::cout << "\n----------------------------------------\n";
    std::cout << " [ERROR] " << message << "\n";
    std::cout << "----------------------------------------\n";
}

void ATM::printReceipt(const std::string& txType, double amount, double balance, const std::string& details) {
    std::string timestamp = getCurrentDateTime();
    std::ostringstream oss;
    oss << "----------------------------------------\n";
    oss << "             ATM RECEIPT                \n";
    oss << " Terminal ID: " << atmId << "\n";
    oss << " Date/Time  : " << timestamp << "\n";
    if (currentAccount) {
        oss << " Account No : " << currentAccount->getAccountNumber() << "\n";
        oss << " Customer   : " << currentAccount->getUsername() << "\n";
    }
    oss << " Transaction: " << txType << "\n";
    if (!details.empty()) {
        oss << " Details    : " << details << "\n";
    }
    oss << " Amount     : $" << std::fixed << std::setprecision(2) << amount << "\n";
    oss << " Balance    : $" << std::fixed << std::setprecision(2) << balance << "\n";
    oss << " Status     : COMPLETED\n";
    oss << "----------------------------------------\n";

    std::cout << "\n" << oss.str();
    fileHandler.appendReceipt(oss.str(), "receipt.txt");
    std::cout << "[INFO] Receipt saved to 'receipt.txt'.\n";
}

bool ATM::askPrintReceipt(const std::string& txType, double amount, double balance, const std::string& details) {
    std::cout << "\nPrint receipt? (Y/N): ";
    char choice;
    if (std::cin >> choice) {
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        if (choice == 'Y' || choice == 'y') {
            printReceipt(txType, amount, balance, details);
            return true;
        }
    }
    return false;
}

bool ATM::askAnotherTransaction() {
    std::cout << "\nAnother transaction? (Y/N): ";
    char choice;
    if (std::cin >> choice) {
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        if (choice == 'Y' || choice == 'y') {
            return true;
        }
    }
    return false;
}

void ATM::welcomeScreen() {
    std::cout << "\n****************************************\n";
    std::cout << "*         WELCOME TO SECURE ATM        *\n";
    std::cout << "*              " << atmId << "                *\n";
    std::cout << "****************************************\n";
}

bool ATM::authenticateUser() {
    int attempts = 0;
    while (attempts < 3) {
        std::cout << "\n[Step] Enter username and password:\n";
        std::cout << "Username: ";
        std::string username;
        if (!(std::cin >> username)) return false;

        std::cout << "PIN (4 digits): ";
        std::string pin;
        if (!(std::cin >> pin)) return false;

        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        auto it = accounts.find(username);
        if (it != accounts.end()) {
            Account& acc = it->second;

            if (acc.getIsLocked()) {
                showErrorMessage("This account is LOCKED. Please contact your branch.");
                return false;
            }

            if (acc.checkPin(pin)) {
                // Credentials valid
                acc.resetFailedAttempts();
                currentAccount = &acc;
                saveState();
                std::cout << "\n[SUCCESS] Authentication successful! Welcome, " << username << ".\n";
                return true;
            } else {
                acc.incrementFailedAttempts();
                saveState();
                attempts = acc.getFailedAttempts();
                if (attempts >= 3) {
                    showErrorMessage("3 failed attempts reached!");
                    std::cout << "\n[CRITICAL] Lock account and notify user!\n";
                    std::cout << "Account " << acc.getAccountNumber() << " has been locked.\n";
                    return false;
                } else {
                    showErrorMessage("Invalid credentials! Attempts remaining: " + std::to_string(3 - attempts));
                }
            }
        } else {
            attempts++;
            if (attempts >= 3) {
                showErrorMessage("3 failed attempts reached! Access blocked.");
                return false;
            } else {
                showErrorMessage("Invalid credentials! Attempts remaining: " + std::to_string(3 - attempts));
            }
        }
    }
    return false;
}

void ATM::handleBalanceInquiry() {
    showHeader("BALANCE INQUIRY");
    std::cout << " Account Holder: " << currentAccount->getUsername() << "\n";
    std::cout << " Account Number: " << currentAccount->getAccountNumber() << "\n";
    std::cout << " Current Balance: $" << std::fixed << std::setprecision(2) << currentAccount->getBalance() << "\n";

    askPrintReceipt("BALANCE INQUIRY", 0.0, currentAccount->getBalance(), "Balance viewed");
}

void ATM::handleWithdrawal() {
    showHeader("CASH WITHDRAWAL");
    std::cout << "Current Balance: $" << std::fixed << std::setprecision(2) << currentAccount->getBalance() << "\n";
    std::cout << "Enter withdrawal amount: $";
    double amount;
    if (!(std::cin >> amount) || amount <= 0) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        showErrorMessage("Invalid amount entered.");
        return;
    }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    // Sufficient funds check
    if (currentAccount->getBalance() < amount) {
        showErrorMessage("Insufficient funds in your account!");
        return;
    }

    // ATM cash check
    if (cashReserve < amount) {
        showErrorMessage("ATM has insufficient cash to dispense this amount.");
        return;
    }

    // Dispense cash and debit account
    currentAccount->debit(amount);
    cashReserve -= amount;

    std::string timestamp = getCurrentDateTime();
    Transaction tx(timestamp, "WITHDRAWAL", -amount, currentAccount->getBalance());
    currentAccount->addTransaction(tx);
    fileHandler.appendTransaction(currentAccount->getAccountNumber(), tx);
    saveState();

    std::cout << "\n[SUCCESS] Please take your cash: $" << std::fixed << std::setprecision(2) << amount << "\n";
    std::cout << "Remaining Balance: $" << std::fixed << std::setprecision(2) << currentAccount->getBalance() << "\n";

    askPrintReceipt("CASH WITHDRAWAL", amount, currentAccount->getBalance());
}

void ATM::handleDeposit() {
    showHeader("DEPOSIT");
    std::cout << "1. Cash Deposit\n";
    std::cout << "2. Cheque Deposit\n";
    std::cout << "Select deposit type (1-2): ";
    int typeChoice;
    if (!(std::cin >> typeChoice) || (typeChoice != 1 && typeChoice != 2)) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        showErrorMessage("Invalid selection.");
        return;
    }

    std::string chequeNumber = "";
    if (typeChoice == 2) {
        std::cout << "Enter Cheque Number: ";
        std::cin >> chequeNumber;
    }

    std::cout << "Enter deposit amount: $";
    double amount;
    if (!(std::cin >> amount) || amount <= 0) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        showErrorMessage("Invalid amount. Deposit rejected.");
        return;
    }

    // Count, verify and confirm amount
    std::cout << "\n[Verifying...] Amount counted: $" << std::fixed << std::setprecision(2) << amount;
    if (typeChoice == 2) {
        std::cout << " (Cheque #" << chequeNumber << ")";
    }
    std::cout << "\nConfirm deposit? (Y/N): ";
    char confirm;
    std::cin >> confirm;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    if (confirm != 'Y' && confirm != 'y') {
        std::cout << "[INFO] Deposit cancelled by user.\n";
        return;
    }

    // Credit account
    currentAccount->credit(amount);
    if (typeChoice == 1) {
        cashReserve += amount; // Cash increases ATM reserve
    }

    std::string typeName = (typeChoice == 1) ? "DEPOSIT_CASH" : "DEPOSIT_CHEQUE";
    std::string details = (typeChoice == 2) ? ("Cheque #" + chequeNumber) : "Cash Deposit";
    std::string timestamp = getCurrentDateTime();

    Transaction tx(timestamp, typeName, amount, currentAccount->getBalance());
    currentAccount->addTransaction(tx);
    fileHandler.appendTransaction(currentAccount->getAccountNumber(), tx);
    saveState();

    std::cout << "\n[SUCCESS] Account credited with: $" << std::fixed << std::setprecision(2) << amount << "\n";
    std::cout << "New Balance: $" << std::fixed << std::setprecision(2) << currentAccount->getBalance() << "\n";

    askPrintReceipt(typeName, amount, currentAccount->getBalance(), details);
}

void ATM::handleTransfer() {
    showHeader("FUNDS TRANSFER");
    std::cout << "Enter recipient account number (e.g. ACC1002): ";
    std::string targetAccNo;
    std::cin >> targetAccNo;

    std::cout << "Enter transfer amount: $";
    double amount;
    if (!(std::cin >> amount) || amount <= 0) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        showErrorMessage("Invalid transfer amount.");
        return;
    }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    // Recipient valid and funds sufficient?
    Account* recipient = findAccountByNumber(targetAccNo);
    if (!recipient) {
        showErrorMessage("Recipient account not found!");
        return;
    }

    if (recipient->getAccountNumber() == currentAccount->getAccountNumber()) {
        showErrorMessage("Cannot transfer funds to your own account!");
        return;
    }

    if (currentAccount->getBalance() < amount) {
        showErrorMessage("Insufficient funds for this transfer!");
        return;
    }

    // Transfer funds
    currentAccount->debit(amount);
    recipient->credit(amount);

    std::string timestamp = getCurrentDateTime();
    std::string senderType = "TRANSFER_TO " + recipient->getAccountNumber();
    std::string recipientType = "TRANSFER_FROM " + currentAccount->getAccountNumber();

    Transaction txSender(timestamp, senderType, -amount, currentAccount->getBalance());
    currentAccount->addTransaction(txSender);
    fileHandler.appendTransaction(currentAccount->getAccountNumber(), txSender);

    Transaction txRecipient(timestamp, recipientType, amount, recipient->getBalance());
    recipient->addTransaction(txRecipient);
    fileHandler.appendTransaction(recipient->getAccountNumber(), txRecipient);

    saveState();

    std::cout << "\n[SUCCESS] Successfully transferred $" << std::fixed << std::setprecision(2) 
              << amount << " to account " << targetAccNo << " (" << recipient->getUsername() << ").\n";
    std::cout << "New Balance: $" << std::fixed << std::setprecision(2) << currentAccount->getBalance() << "\n";

    askPrintReceipt("TRANSFER", amount, currentAccount->getBalance(), "To: " + targetAccNo);
}

void ATM::handleBillPayment() {
    showHeader("BILL PAYMENT");
    std::cout << "1. Electricity Company\n";
    std::cout << "2. Water Utility\n";
    std::cout << "3. Internet / Telecom\n";
    std::cout << "4. Gas Utility\n";
    std::cout << "Select biller (1-4): ";
    int billerChoice;
    if (!(std::cin >> billerChoice) || billerChoice < 1 || billerChoice > 4) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        showErrorMessage("Invalid biller choice.");
        return;
    }

    std::string billerName = "Unknown";
    switch (billerChoice) {
        case 1: billerName = "Electricity Company"; break;
        case 2: billerName = "Water Utility"; break;
        case 3: billerName = "Internet / Telecom"; break;
        case 4: billerName = "Gas Utility"; break;
    }

    std::cout << "Enter Bill / Subscriber Reference Number: ";
    std::string refNumber;
    std::cin >> refNumber;

    std::cout << "Enter bill amount to pay: $";
    double amount;
    if (!(std::cin >> amount) || amount <= 0) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        showErrorMessage("Invalid bill amount.");
        return;
    }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    // Sufficient funds?
    if (currentAccount->getBalance() < amount) {
        showErrorMessage("Insufficient funds to pay this bill!");
        return;
    }

    // Pay bill
    currentAccount->debit(amount);
    std::string timestamp = getCurrentDateTime();
    std::string txType = "BILL_PAY (" + billerName + ")";

    Transaction tx(timestamp, txType, -amount, currentAccount->getBalance());
    currentAccount->addTransaction(tx);
    fileHandler.appendTransaction(currentAccount->getAccountNumber(), tx);
    saveState();

    std::cout << "\n[SUCCESS] Bill paid successfully to " << billerName << "!\n";
    std::cout << "Reference: " << refNumber << "\n";
    std::cout << "Amount Paid: $" << std::fixed << std::setprecision(2) << amount << "\n";
    std::cout << "New Balance: $" << std::fixed << std::setprecision(2) << currentAccount->getBalance() << "\n";

    askPrintReceipt("BILL_PAYMENT", amount, currentAccount->getBalance(), billerName + " Ref: " + refNumber);
}

void ATM::handleMiniStatement() {
    showHeader("MINI STATEMENT");
    const auto& txs = currentAccount->getTransactions();

    if (txs.empty()) {
        std::cout << "No past transactions found for this account.\n";
    } else {
        std::cout << std::left 
                  << std::setw(22) << "Date & Time" 
                  << std::setw(26) << "Type" 
                  << std::setw(12) << "Amount" 
                  << std::setw(12) << "Balance" << "\n";
        std::cout << "--------------------------------------------------------------------\n";

        // Show last up to 5 transactions
        int total = static_cast<int>(txs.size());
        int start = std::max(0, total - 5);
        for (int i = start; i < total; ++i) {
            std::cout << std::left 
                      << std::setw(22) << txs[i].getTimestamp()
                      << std::setw(26) << txs[i].getType()
                      << "$" << std::setw(11) << std::fixed << std::setprecision(2) << txs[i].getAmount()
                      << "$" << std::setw(11) << std::fixed << std::setprecision(2) << txs[i].getBalanceAfter()
                      << "\n";
        }
        std::cout << "--------------------------------------------------------------------\n";
    }

    askPrintReceipt("MINI_STATEMENT", 0.0, currentAccount->getBalance(), "Last transactions summary");
}

void ATM::handleChangePassword() {
    showHeader("CHANGE PASSWORD / PIN");
    std::cout << "Enter current password (PIN): ";
    std::string oldPin;
    std::cin >> oldPin;

    if (!currentAccount->checkPin(oldPin)) {
        showErrorMessage("Current password does not match!");
        return;
    }

    std::cout << "Enter new 4-digit PIN: ";
    std::string newPin;
    std::cin >> newPin;

    std::cout << "Confirm new 4-digit PIN: ";
    std::string confirmPin;
    std::cin >> confirmPin;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    // Passwords match and meet rules? (Must match and be exactly 4 digits)
    bool isFourDigits = (newPin.length() == 4 && 
                         std::all_of(newPin.begin(), newPin.end(), ::isdigit));

    if (newPin != confirmPin) {
        showErrorMessage("New passwords do not match!");
        return;
    }

    if (!isFourDigits) {
        showErrorMessage("New password must be exactly 4 digits (0-9)!");
        return;
    }

    // Update password
    currentAccount->setPin(newPin);
    saveState();
    std::cout << "\n[SUCCESS] Password updated successfully.\n";
}

void ATM::logoutSession() {
    std::cout << "\nLogging out session...\n";
    currentAccount = nullptr;
    std::cout << "****************************************\n";
    std::cout << "*     End: Thank you for using ATM!    *\n";
    std::cout << "****************************************\n\n";
}

void ATM::showMainMenu() {
    bool inSession = true;
    while (inSession) {
        showHeader("MAIN MENU");
        std::cout << "1. Balance Inquiry\n";
        std::cout << "2. Cash Withdrawal\n";
        std::cout << "3. Deposit (Cash/Cheque)\n";
        std::cout << "4. Funds Transfer\n";
        std::cout << "5. Bill Payment\n";
        std::cout << "6. Mini Statement\n";
        std::cout << "7. Change Password\n";
        std::cout << "8. Logout\n";
        std::cout << "----------------------------------------\n";
        std::cout << "Select an option (1-8): ";

        int choice;
        if (!(std::cin >> choice)) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            showErrorMessage("Invalid input. Please enter a number from 1 to 8.");
            continue;
        }
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        switch (choice) {
            case 1:
                handleBalanceInquiry();
                break;
            case 2:
                handleWithdrawal();
                break;
            case 3:
                handleDeposit();
                break;
            case 4:
                handleTransfer();
                break;
            case 5:
                handleBillPayment();
                break;
            case 6:
                handleMiniStatement();
                break;
            case 7:
                handleChangePassword();
                break;
            case 8:
                logoutSession();
                return;
            default:
                showErrorMessage("Invalid selection. Choose between 1 and 8.");
                break;
        }

        // According to flowchart: After transaction (or error) -> Another transaction?
        if (!askAnotherTransaction()) {
            logoutSession();
            inSession = false;
        }
    }
}

void ATM::run() {
    bool systemRunning = true;
    while (systemRunning) {
        welcomeScreen();
        std::cout << "1. Insert Card / Log in\n";
        std::cout << "2. Shutdown ATM\n";
        std::cout << "Choice: ";
        int choice;
        if (!(std::cin >> choice)) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            break;
        }
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        if (choice == 1) {
            if (authenticateUser()) {
                showMainMenu();
            } else {
                std::cout << "Session terminated.\n";
            }
        } else if (choice == 2) {
            std::cout << "ATM shutting down. Have a nice day!\n";
            systemRunning = false;
        } else {
            std::cout << "Invalid option.\n";
        }
    }
}
