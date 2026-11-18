# ATM System Simulation in C++

A complete, robust Object-Oriented C++ implementation of the ATM System Flowchart.

---

## 📌 Project Overview

This project directly translates the provided ATM System Flowchart into modern C++ (C++17) using an Object-Oriented modular design with persistent file storage.

### 🗺️ Flowchart Mapping Table

| Flowchart Node / Step | C++ Implementation |
| :--- | :--- |
| **Start: Welcome screen** | `ATM::welcomeScreen()` |
| **Enter username and password** | `ATM::authenticateUser()` |
| **Credentials valid?** | `Account::checkPin()` & map lookup |
| **3 failed attempts?** | Increments attempts count up to 3 |
| **Lock account and notify user** | `Account::lockAccount()` -> saves `isLocked=1` to file |
| **Show main menu** | `ATM::showMainMenu()` |
| **Balance Inquiry?** | `ATM::handleBalanceInquiry()` |
| **Withdrawal?** | `ATM::handleWithdrawal()` |
| **- Sufficient funds?** | Validates `balance >= withdrawalAmount` |
| **- ATM has enough cash?** | Validates `cashReserve >= withdrawalAmount` |
| **- Dispense cash and debit** | Decrements account balance & ATM cash reserve |
| **Deposit?** | `ATM::handleDeposit()` |
| **- Insert cash or cheque** | Select Cash or Cheque (+ Cheque Number) |
| **- Count, verify and confirm** | Amount validation (> 0) and user confirmation |
| **- Credit account** | Credits balance (and ATM reserve if Cash) |
| **Transfer?** | `ATM::handleTransfer()` |
| **- Enter recipient & amount** | Looks up recipient by Account Number |
| **- Recipient valid & funds sufficient?** | Validates recipient exists, not self, balance >= amount |
| **- Transfer funds** | Debits sender, credits recipient, logs both transactions |
| **Bill payment?** | `ATM::handleBillPayment()` |
| **- Select biller & amount** | Choose from Electricity, Water, Telecom, Gas |
| **- Sufficient funds? -> Pay bill** | Checks balance, debits balance, records transaction |
| **Mini statement?** | `ATM::handleMiniStatement()` (shows last 5 transactions) |
| **Change password?** | `ATM::handleChangePassword()` |
| **- Passwords match & meet rules?** | Verifies current PIN, checks match, ensures 4-digit PIN |
| **- Update password** | Updates PIN and persists to `accounts.txt` |
| **Print receipt?** | `ATM::askPrintReceipt()` -> saves to `receipt.txt` & console |
| **Another transaction?** | `ATM::askAnotherTransaction()` -> loop or logout |
| **Log out session -> End: Thank you** | `ATM::logoutSession()` |

---

## 🗂️ Project Structure

```
c++_task/
├── include/
│   ├── Account.h         # Account data and credentials logic
│   ├── ATM.h             # ATM state machine and flowchart workflows
│   ├── FileHandler.h     # File persistence and transaction logging
│   └── Transaction.h     # Transaction model and serialization
├── src/
│   ├── Account.cpp
│   ├── ATM.cpp
│   ├── FileHandler.cpp
│   ├── Transaction.cpp
│   └── main.cpp
├── data/
│   ├── accounts.txt      # User credentials, balances, and lock states
│   ├── atm_config.txt    # ATM Terminal ID and cash reserves
│   └── transactions_*.txt# Individual transaction logs
├── build.bat             # One-click Windows build script
├── receipt.txt           # Generated receipts log
└── README.md
```

---

## 👥 Default Test Accounts

| Username | PIN | Account Number | Initial Balance | Status |
| :--- | :--- | :--- | :--- | :--- |
| **john** | `1234` | `ACC1001` | $2,500.00 | Active |
| **sarah** | `4321` | `ACC1002` | $1,500.00 | Active |
| **ahmed** | `9999` | `ACC1003` | $5,000.00 | Active |

*Initial ATM Cash Reserve:* **$50,000.00**

---

## 🛠️ How to Compile and Run

### Option 1: Using the provided Batch script
Double-click `build.bat` or run in terminal:
```cmd
build.bat
```

### Option 2: Using g++ directly
```bash
g++ -std=c++17 -Iinclude src/Transaction.cpp src/Account.cpp src/FileHandler.cpp src/ATM.cpp src/main.cpp -o atm.exe
```

### Run:
```cmd
.\atm.exe
```
