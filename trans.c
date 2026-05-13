#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_RECORDS 100
#define DATA_FILE "credit.dat"
#define LOG_FILE "transaction_history.txt"
#define EXPORT_FILE "accounts.txt"

struct clientData {
    unsigned int acctNum;
    char lastName[15];
    char firstName[10];
    double balance;
};

// Prototypes
void initializeFile(FILE *fPtr);
void logTransaction(const char *msg);
void exportAccounts(FILE *fPtr);   // Option 1
void updateRecord(FILE *fPtr);     // Option 2
void newRecord(FILE *fPtr);        // Option 3
void deleteRecord(FILE *fPtr);     // Option 4
void listAccounts(FILE *fPtr);     // Option 5
void transferMoney(FILE *fPtr);    // Option 6
void searchAccounts(FILE *fPtr);   // Option 7
void showSummary(FILE *fPtr);      // Option 8
void viewHistory();                // Option 9

int main() {
    FILE *cfPtr;
    int choice;

    // Open for update, create if missing
    if ((cfPtr = fopen(DATA_FILE, "rb+")) == NULL) {
        printf("File not found. Creating new %s file...\n", DATA_FILE);
        cfPtr = fopen(DATA_FILE, "wb+");
        if (cfPtr == NULL) {
            printf("Error: Could not create file.\n");
            return 1;
        }
        initializeFile(cfPtr);
        printf("File created successfully.\n");
    }

    const char *menu =
        "\nEnter your choice\n"
        "1  - store a formatted text file of accounts\n"
        "     called \"accounts.txt\" for printing\n"
        "2  - update an account\n"
        "3  - add a new account\n"
        "4  - delete an account\n"
        "5  - list all accounts\n"
        "6  - transfer money between accounts\n"
        "7  - search accounts\n"
        "8  - show account summary\n"
        "9  - view transaction history\n"
        "10 - end program\n? ";

    while (printf("%s", menu) && scanf("%d", &choice) == 1 && choice != 10) {
        switch (choice) {
            case 1:  exportAccounts(cfPtr); break;
            case 2:  updateRecord(cfPtr);   break;
            case 3:  newRecord(cfPtr);      break;
            case 4:  deleteRecord(cfPtr);   break;
            case 5:  listAccounts(cfPtr);   break;
            case 6:  transferMoney(cfPtr);  break;
            case 7:  searchAccounts(cfPtr); break;
            case 8:  showSummary(cfPtr);    break;
            case 9:  viewHistory();         break;
            default: puts("Invalid choice. Please enter 1-10.");
        }
    }

    printf("\nProgram ended.\n");
    fclose(cfPtr);
    return 0;
}

// Pre-fill file with MAX_RECORDS blank structures
void initializeFile(FILE *fPtr) {
    struct clientData blank = {0, "", "", 0.0};
    for (int i = 0; i < MAX_RECORDS; i++)
        fwrite(&blank, sizeof(struct clientData), 1, fPtr);
    logTransaction("System Initialized: Created credit.dat");
}

// Timestamped logging
void logTransaction(const char *msg) {
    FILE *lPtr = fopen(LOG_FILE, "a");
    if (!lPtr) return;
    time_t now = time(NULL);
    char *ts = ctime(&now);
    ts[strlen(ts) - 1] = '\0'; // Remove newline
    fprintf(lPtr, "[%s] %s\n", ts, msg);
    fclose(lPtr);
}

// Option 1: Export all active accounts to accounts.txt
void exportAccounts(FILE *fPtr) {
    struct clientData c;
    FILE *outPtr = fopen(EXPORT_FILE, "w");
    if (!outPtr) {
        printf("Error: Could not create %s\n", EXPORT_FILE);
        return;
    }

    fprintf(outPtr, "%-6s %-15s %-10s %10s\n", "Acct", "Last Name", "First Name", "Balance");
    fprintf(outPtr, "----------------------------------------------\n");

    rewind(fPtr);
    int count = 0;
    while (fread(&c, sizeof(struct clientData), 1, fPtr)) {
        if (c.acctNum != 0) {
            fprintf(outPtr, "%-6u %-15s %-10s %10.2f\n",
                    c.acctNum, c.lastName, c.firstName, c.balance);
            count++;
        }
    }

    fclose(outPtr);
    if (count == 0)
        printf("No active accounts to export.\n");
    else
        printf("Exported %d account(s) to \"%s\".\n", count, EXPORT_FILE);
    logTransaction("Exported accounts to accounts.txt");
}

// Option 2: Update (adjust balance of) an account
void updateRecord(FILE *fPtr) {
    struct clientData c;
    unsigned int acc;
    double move;

    printf("Enter account number to update (1-%d): ", MAX_RECORDS);
    scanf("%u", &acc);

    if (acc < 1 || acc > MAX_RECORDS) {
        printf("Invalid account number.\n");
        return;
    }

    fseek(fPtr, (acc - 1) * sizeof(struct clientData), SEEK_SET);
    fread(&c, sizeof(struct clientData), 1, fPtr);

    if (c.acctNum == 0) {
        printf("Account #%u does not exist.\n", acc);
    } else {
        printf("Account: %-6u %-15s %-10s Balance: %.2f\n",
               c.acctNum, c.lastName, c.firstName, c.balance);
        printf("Enter adjustment amount (+credit / -debit): ");
        scanf("%lf", &move);
        c.balance += move;
        fseek(fPtr, (acc - 1) * sizeof(struct clientData), SEEK_SET);
        fwrite(&c, sizeof(struct clientData), 1, fPtr);
        printf("New balance for account #%u: %.2f\n", acc, c.balance);

        char buf[120];
        sprintf(buf, "Updated account #%u (%s): adjustment %.2f, new balance %.2f",
                acc, c.lastName, move, c.balance);
        logTransaction(buf);
    }
}

// Option 3: Add a new account
void newRecord(FILE *fPtr) {
    struct clientData client = {0, "", "", 0.0};
    unsigned int acc;

    printf("Enter new account number (1-%d): ", MAX_RECORDS);
    scanf("%u", &acc);

    if (acc < 1 || acc > MAX_RECORDS) {
        printf("Invalid account number.\n");
        return;
    }

    fseek(fPtr, (acc - 1) * sizeof(struct clientData), SEEK_SET);
    fread(&client, sizeof(struct clientData), 1, fPtr);

    if (client.acctNum != 0) {
        printf("Error: Account #%u already exists.\n", acc);
    } else {
        client.acctNum = acc;
        printf("Enter Last Name: ");  scanf("%14s", client.lastName);
        printf("Enter First Name: "); scanf("%9s", client.firstName);
        printf("Enter Balance: ");    scanf("%lf", &client.balance);

        fseek(fPtr, (acc - 1) * sizeof(struct clientData), SEEK_SET);
        fwrite(&client, sizeof(struct clientData), 1, fPtr);

        printf("Account #%u created for %s %s.\n", acc, client.firstName, client.lastName);

        char buf[120];
        sprintf(buf, "Created account #%u for %s %s with balance %.2f",
                acc, client.firstName, client.lastName, client.balance);
        logTransaction(buf);
    }
}

// Option 4: Delete an account
void deleteRecord(FILE *fPtr) {
    struct clientData c, blank = {0, "", "", 0.0};
    unsigned int acc;

    printf("Enter account number to delete (1-%d): ", MAX_RECORDS);
    scanf("%u", &acc);

    if (acc < 1 || acc > MAX_RECORDS) {
        printf("Invalid account number.\n");
        return;
    }

    fseek(fPtr, (acc - 1) * sizeof(struct clientData), SEEK_SET);
    fread(&c, sizeof(struct clientData), 1, fPtr);

    if (c.acctNum == 0) {
        printf("Account #%u does not exist.\n", acc);
    } else {
        printf("Deleting account: %-6u %-15s %-10s Balance: %.2f\n",
               c.acctNum, c.lastName, c.firstName, c.balance);
        fseek(fPtr, (acc - 1) * sizeof(struct clientData), SEEK_SET);
        fwrite(&blank, sizeof(struct clientData), 1, fPtr);
        printf("Account #%u deleted.\n", acc);

        char buf[120];
        sprintf(buf, "Deleted account #%u (%s %s)", acc, c.firstName, c.lastName);
        logTransaction(buf);
    }
}

// Option 5: List all active accounts
void listAccounts(FILE *fPtr) {
    struct clientData c;
    int found = 0;

    rewind(fPtr);
    printf("\n%-6s %-15s %-10s %10s\n", "Acct", "Last Name", "First Name", "Balance");
    printf("----------------------------------------------\n");

    while (fread(&c, sizeof(struct clientData), 1, fPtr)) {
        if (c.acctNum != 0) {
            printf("%-6u %-15s %-10s %10.2f\n",
                   c.acctNum, c.lastName, c.firstName, c.balance);
            found++;
        }
    }

    if (!found)
        printf("No active accounts found.\n");
    else
        printf("----------------------------------------------\n");
}

// Option 6: Transfer money between two accounts
void transferMoney(FILE *fPtr) {
    struct clientData s = {0}, r = {0};
    unsigned int from, to;
    double amt;

    printf("From Account (1-%d): ", MAX_RECORDS); scanf("%u", &from);
    printf("To Account   (1-%d): ", MAX_RECORDS); scanf("%u", &to);
    printf("Amount to Transfer: ");               scanf("%lf", &amt);

    if (from < 1 || from > MAX_RECORDS || to < 1 || to > MAX_RECORDS || from == to) {
        printf("Invalid account numbers.\n");
        return;
    }
    if (amt <= 0) {
        printf("Transfer amount must be positive.\n");
        return;
    }

    fseek(fPtr, (from - 1) * sizeof(struct clientData), SEEK_SET);
    fread(&s, sizeof(struct clientData), 1, fPtr);
    fseek(fPtr, (to - 1) * sizeof(struct clientData), SEEK_SET);
    fread(&r, sizeof(struct clientData), 1, fPtr);

    if (!s.acctNum || !r.acctNum) {
        printf("Transfer Failed: One or both accounts do not exist.\n");
    } else if (s.balance < amt) {
        printf("Transfer Failed: Insufficient balance in account #%u (Balance: %.2f).\n",
               from, s.balance);
    } else {
        s.balance -= amt;
        r.balance += amt;

        fseek(fPtr, (from - 1) * sizeof(struct clientData), SEEK_SET);
        fwrite(&s, sizeof(struct clientData), 1, fPtr);
        fseek(fPtr, (to - 1) * sizeof(struct clientData), SEEK_SET);
        fwrite(&r, sizeof(struct clientData), 1, fPtr);

        printf("Transfer Complete: $%.2f from account #%u to account #%u.\n", amt, from, to);
        printf("  Account #%u new balance: %.2f\n", from, s.balance);
        printf("  Account #%u new balance: %.2f\n", to, r.balance);

        char buf[150];
        sprintf(buf, "Transferred %.2f from #%u (%s) to #%u (%s)",
                amt, from, s.lastName, to, r.lastName);
        logTransaction(buf);
    }
}

// Option 7: Search accounts by number, last name, or first name
void searchAccounts(FILE *fPtr) {
    struct clientData c;
    int searchChoice;
    int found = 0;

    printf("\nSearch by:\n");
    printf("  1 - Account Number\n");
    printf("  2 - Last Name\n");
    printf("  3 - First Name\n");
    printf("? ");
    scanf("%d", &searchChoice);

    if (searchChoice == 1) {
        unsigned int acc;
        printf("Enter account number: ");
        scanf("%u", &acc);

        if (acc >= 1 && acc <= MAX_RECORDS) {
            fseek(fPtr, (acc - 1) * sizeof(struct clientData), SEEK_SET);
            fread(&c, sizeof(struct clientData), 1, fPtr);
            if (c.acctNum != 0) {
                printf("\n%-6s %-15s %-10s %10s\n", "Acct", "Last Name", "First Name", "Balance");
                printf("----------------------------------------------\n");
                printf("%-6u %-15s %-10s %10.2f\n",
                       c.acctNum, c.lastName, c.firstName, c.balance);
                found = 1;
            }
        }
    } else if (searchChoice == 2 || searchChoice == 3) {
        char searchTerm[15];
        printf("Enter %s name: ", searchChoice == 2 ? "last" : "first");
        scanf("%14s", searchTerm);

        rewind(fPtr);
        printf("\n%-6s %-15s %-10s %10s\n", "Acct", "Last Name", "First Name", "Balance");
        printf("----------------------------------------------\n");

        while (fread(&c, sizeof(struct clientData), 1, fPtr)) {
            if (c.acctNum != 0) {
                int match = (searchChoice == 2)
                    ? (strcasecmp(c.lastName, searchTerm) == 0)
                    : (strcasecmp(c.firstName, searchTerm) == 0);
                if (match) {
                    printf("%-6u %-15s %-10s %10.2f\n",
                           c.acctNum, c.lastName, c.firstName, c.balance);
                    found++;
                }
            }
        }
    } else {
        printf("Invalid search option.\n");
        return;
    }

    if (!found)
        printf("No matching accounts found.\n");
}

// Option 8: Show account summary
void showSummary(FILE *fPtr) {
    struct clientData c;
    double total = 0.0, highest = 0.0, lowest = 1e18;
    int count = 0;

    rewind(fPtr);
    while (fread(&c, sizeof(struct clientData), 1, fPtr)) {
        if (c.acctNum != 0) {
            total += c.balance;
            count++;
            if (c.balance > highest) highest = c.balance;
            if (c.balance < lowest)  lowest  = c.balance;
        }
    }

    printf("\n========== ACCOUNT SUMMARY ==========\n");
    if (count == 0) {
        printf("  No active accounts.\n");
    } else {
        printf("  Total Accounts : %d\n", count);
        printf("  Total Balance  : $%.2f\n", total);
        printf("  Average Balance: $%.2f\n", total / count);
        printf("  Highest Balance: $%.2f\n", highest);
        printf("  Lowest Balance : $%.2f\n", lowest);
    }
    printf("=====================================\n");
}

// Option 9: View transaction history
void viewHistory() {
    char line[200];
    FILE *l = fopen(LOG_FILE, "r");

    printf("\n========== TRANSACTION HISTORY ==========\n");
    if (!l) {
        printf("  No transaction history found.\n");
    } else {
        int lines = 0;
        while (fgets(line, sizeof(line), l)) {
            printf("%s", line);
            lines++;
        }
        fclose(l);
        if (lines == 0)
            printf("  History file is empty.\n");
    }
    printf("=========================================\n");
}