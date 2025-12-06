/* main.c  - SRMS: multi-word names, formatted table, unique roll & name checks */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define STUDENT_FILE      "student.txt"
#define CREDENTIALS_FILE  "credentials.txt"
#define LINEBUF_SIZE      512
#define NAME_MAX          200

char currentRole[10];
char currentUser[50];

/* prototypes */
int  loginSystem(void);
void mainMenu(void);
void adminMenu(void);
void staffMenu(void);
void guestMenu(void);

void addStudent(void);
void displayStudents(void);
void searchStudent(void);
void updateStudent(void);
void deleteStudent(void);

/* helpers */
void readLine(const char *prompt, char *out, size_t outSize);
int  readInt(const char *prompt);
float readFloat(const char *prompt);
void trimNewline(char *s);
void sanitizePipe(char *s);
int  rollExists(int roll);
int  nameExists(const char *name);
int  nameExistsExceptRoll(const char *name, int roll);

int main(void) {
    if (loginSystem()) {
        mainMenu();
    } else {
        printf("\nLogin Failed. Exiting...\n");
    }
    return 0;
}

/* ---------- login (same behavior) ---------- */
int loginSystem(void) {
    char username[50], password[50];
    char fileUser[50], filePass[50], fileRole[20];
    int attempts = 3;

    while (attempts > 0) {
        printf("========= LOGIN SCREEN =========\n");
        printf("Attempts remaining: %d\n", attempts);
        readLine("Username: ", username, sizeof(username));
        readLine("Password: ", password, sizeof(password));

        FILE *fp = fopen(CREDENTIALS_FILE, "r");
        if (!fp) {
            /* If credentials file missing, allow ADMIN fallback for testing */
            strcpy(currentRole, "ADMIN");
            strcpy(currentUser, "localadmin");
            return 1;
        }

        int found = 0;
        while (fscanf(fp, "%49s %49s %19s", fileUser, filePass, fileRole) == 3) {
            if (strcmp(username, fileUser) == 0 && strcmp(password, filePass) == 0) {
                strncpy(currentRole, fileRole, sizeof(currentRole)-1);
                currentRole[sizeof(currentRole)-1] = '\0';
                strncpy(currentUser, fileUser, sizeof(currentUser)-1);
                currentUser[sizeof(currentUser)-1] = '\0';
                fclose(fp);
                printf("Login successful!\n");
                return 1;
            }
        }
        fclose(fp);

        attempts--;
        if (attempts > 0) {
            printf("Invalid credentials. Try again.\n\n");
        } else {
            printf("Login failed. Maximum attempts exceeded.\n");
            return 0;
        }
    }
    return 0;
}

/* ---------- menus ---------- */
void mainMenu(void) {
    if (strcmp(currentRole, "ADMIN") == 0)
        adminMenu();
    else if (strcmp(currentRole, "STAFF") == 0)
        staffMenu();
    else
        guestMenu();
}

void adminMenu(void) {
    int choice;
    do {
        printf("\n====== ADMIN MENU ======\n");
        printf("1. Add Student\n");
        printf("2. Display Students\n");
        printf("3. Search Student\n");
        printf("4. Update Student\n");
        printf("5. Delete Student\n");
        printf("6. Logout\n");
        choice = readInt("Enter choice: ");

        switch (choice) {
            case 1: addStudent();      break;
            case 2: displayStudents(); break;
            case 3: searchStudent();   break;
            case 4: updateStudent();   break;
            case 5: deleteStudent();   break;
            case 6: printf("Logging out...\n"); break;
            default: printf("Invalid Choice!\n");
        }
    } while (choice != 6);
}

void staffMenu(void) {
    int choice;
    do {
        printf("\n====== STAFF MENU ======\n");
        printf("1. Add Student\n");
        printf("2. Display All Records\n");
        printf("3. Search Student\n");
        printf("4. Logout\n");
        choice = readInt("Enter choice: ");

        switch (choice) {
            case 1: addStudent();      break;
            case 2: displayStudents(); break;
            case 3: searchStudent();   break;
            case 4: printf("Logging out...\n"); break;
            default: printf("Invalid Choice!\n");
        }
    } while (choice != 4);
}

void guestMenu(void) {
    int choice;
    do {
        printf("\n====== GUEST MENU ======\n");
        printf("1. Display All Records\n");
        printf("2. Search Student\n");
        printf("3. Logout\n");
        choice = readInt("Enter choice: ");

        switch (choice) {
            case 1: displayStudents(); break;
            case 2: searchStudent();   break;
            case 3: printf("Logging out...\n"); break;
            default: printf("Invalid Choice!\n");
        }
    } while (choice != 3);
}

/* ---------- add student (multi-word name allowed, unique roll & name) ---------- */
void addStudent(void) {
    int roll = readInt("\nEnter Roll: ");

    if (rollExists(roll)) {
        printf("A student with roll %d already exists. Choose another roll.\n", roll);
        return;
    }

    char namebuf[NAME_MAX];
    readLine("Enter Name (can include spaces): ", namebuf, sizeof(namebuf));
    sanitizePipe(namebuf); /* protect against '|' in names */

    if (nameExists(namebuf)) {
        printf("A student with the name \"%s\" already exists. Choose a different name.\n", namebuf);
        return;
    }

    float marks = readFloat("Enter Marks: ");

    FILE *fp = fopen(STUDENT_FILE, "a");
    if (!fp) {
        printf("Error opening student file for append.\n");
        return;
    }
    /* store with delimiter '|' to allow spaces in name */
    fprintf(fp, "%d|%s|%.2f\n", roll, namebuf, marks);
    fclose(fp);
    printf("Student Added Successfully!\n");
}

/* ---------- display students (parses '|') ---------- */
void displayStudents(void) {
    FILE *fp = fopen(STUDENT_FILE, "r");
    char line[LINEBUF_SIZE];
    if (!fp) {
        printf("No student records found (file missing).\n");
        return;
    }

    /* header with aligned columns */
    printf("\n%-6s %-30s %8s\n", "Roll", "Name", "Marks");
    printf("---------------------------------------------------------------\n");

    while (fgets(line, sizeof(line), fp)) {
        trimNewline(line);
        if (line[0] == '\0') continue;

        char copy[LINEBUF_SIZE];
        strncpy(copy, line, sizeof(copy)-1);
        copy[sizeof(copy)-1] = '\0';

        /* parse roll|name|marks */
        char *p = strtok(copy, "|");
        if (!p) continue;
        int roll = atoi(p);

        char *name = strtok(NULL, "|");
        if (!name) continue;

        char *marksStr = strtok(NULL, "|");
        if (!marksStr) continue;
        float marks = (float) atof(marksStr);

        printf("%-6d %-30s %8.2f\n", roll, name, marks);
    }

    fclose(fp);
}

/* ---------- search (by roll or by name substring) ---------- */
void searchStudent(void) {
    printf("\nSearch by:\n1. Roll\n2. Name (partial, case-insensitive)\n");
    int mode = readInt("Enter choice: ");

    if (mode == 1) {
        int key = readInt("Enter Roll to search: ");
        FILE *fp = fopen(STUDENT_FILE, "r");
        char line[LINEBUF_SIZE];
        int found = 0;
        if (!fp) { printf("No student records.\n"); return; }
        while (fgets(line, sizeof(line), fp)) {
            trimNewline(line);
            if (line[0]=='\0') continue;
            char copy[LINEBUF_SIZE];
            strncpy(copy, line, sizeof(copy)-1); copy[sizeof(copy)-1]=0;
            char *p = strtok(copy, "|");
            if (!p) continue;
            int roll = atoi(p);
            char *name = strtok(NULL, "|");
            char *marksStr = strtok(NULL, "|");
            if (!name || !marksStr) continue;
            if (roll == key) {
                float marks = (float)atof(marksStr);
                printf("\nRecord Found:\nRoll: %d\nName: %s\nMarks: %.2f\n", roll, name, marks);
                found = 1; break;
            }
        }
        fclose(fp);
        if (!found) printf("Record not found.\n");
    }
    else if (mode == 2) {
        char query[NAME_MAX];
        readLine("Enter name or part of name to search: ", query, sizeof(query));
        /* case-insensitive substring search */
        for (char *p = query; *p; ++p) *p = (char) tolower((unsigned char)*p);

        FILE *fp = fopen(STUDENT_FILE, "r");
        char line[LINEBUF_SIZE];
        int found = 0;
        if (!fp) { printf("No student records.\n"); return; }

        printf("\nMatches:\n%-6s %-30s %8s\n", "Roll", "Name", "Marks");
        printf("---------------------------------------------------------------\n");

        while (fgets(line, sizeof(line), fp)) {
            trimNewline(line);
            if (line[0] == '\0') continue;
            char copy[LINEBUF_SIZE];
            strncpy(copy, line, sizeof(copy)-1); copy[sizeof(copy)-1]=0;

            char *p = strtok(copy, "|");
            if (!p) continue;
            int roll = atoi(p);
            char *name = strtok(NULL, "|");
            char *marksStr = strtok(NULL, "|");
            if (!name || !marksStr) continue;

            char lowerName[NAME_MAX];
            strncpy(lowerName, name, sizeof(lowerName)-1);
            lowerName[sizeof(lowerName)-1] = '\0';
            for (char *q = lowerName; *q; ++q) *q = (char) tolower((unsigned char)*q);

            if (strstr(lowerName, query) != NULL) {
                float marks = (float) atof(marksStr);
                printf("%-6d %-30s %8.2f\n", roll, name, marks);
                found = 1;
            }
        }
        fclose(fp);
        if (!found) printf("No matching records found.\n");
    } else {
        printf("Invalid choice.\n");
    }
}

/* ---------- update student (prevents name duplicates) ---------- */
void updateStudent(void) {
    int key = readInt("Enter Roll to update: ");
    FILE *fp = fopen(STUDENT_FILE, "r");
    FILE *temp = fopen("temp.txt", "w");
    char line[LINEBUF_SIZE];
    int found = 0;
    if (!fp || !temp) { if (fp) fclose(fp); if (temp) fclose(temp); printf("Error opening files.\n"); return; }

    while (fgets(line, sizeof(line), fp)) {
        trimNewline(line);
        if (line[0] == '\0') continue;
        char copy[LINEBUF_SIZE];
        strncpy(copy, line, sizeof(copy)-1); copy[sizeof(copy)-1]=0;

        char *p = strtok(copy, "|");
        if (!p) continue;
        int roll = atoi(p);
        char *name = strtok(NULL, "|");
        char *marksStr = strtok(NULL, "|");
        if (!name || !marksStr) continue;
        float marks = (float) atof(marksStr);

        if (roll == key) {
            printf("Current -> Roll:%d Name:%s Marks:%.2f\n", roll, name, marks);
            char newName[NAME_MAX];
            readLine("Enter new Name (can include spaces): ", newName, sizeof(newName));
            sanitizePipe(newName);
            /* allow keeping same name; otherwise block if duplicate */
            if ( (strcasecmp(newName, name) != 0) && nameExistsExceptRoll(newName, roll) ) {
                printf("A student with the name \"%s\" already exists. Update cancelled.\n", newName);
                fprintf(temp, "%s\n", line); /* keep original */
            } else {
                float newMarks = readFloat("Enter new Marks: ");
                fprintf(temp, "%d|%s|%.2f\n", roll, newName, newMarks);
                found = 1;
            }
        } else {
            fprintf(temp, "%s\n", line);
        }
    }

    fclose(fp);
    fclose(temp);

    remove(STUDENT_FILE);
    rename("temp.txt", STUDENT_FILE);

    if (found) printf("Record updated.\n"); else printf("Record not found or update cancelled.\n");
}

/* ---------- delete student ---------- */
void deleteStudent(void) {
    int key = readInt("Enter Roll to delete: ");
    FILE *fp = fopen(STUDENT_FILE, "r");
    FILE *temp = fopen("temp.txt", "w");
    char line[LINEBUF_SIZE];
    int found = 0;
    if (!fp || !temp) { if (fp) fclose(fp); if (temp) fclose(temp); printf("Error opening files.\n"); return; }

    while (fgets(line, sizeof(line), fp)) {
        trimNewline(line);
        if (line[0] == '\0') continue;
        char copy[LINEBUF_SIZE];
        strncpy(copy, line, sizeof(copy)-1); copy[sizeof(copy)-1]=0;

        char *p = strtok(copy, "|");
        if (!p) continue;
        int roll = atoi(p);
        if (roll == key) {
            found = 1;
            continue; /* skip */
        }
        fprintf(temp, "%s\n", line);
    }

    fclose(fp);
    fclose(temp);

    remove(STUDENT_FILE);
    rename("temp.txt", STUDENT_FILE);

    if (found) printf("Record deleted.\n"); else printf("Record not found.\n");
}

/* ---------- helpers ---------- */

void readLine(const char *prompt, char *out, size_t outSize) {
    if (prompt) printf("%s", prompt);
    if (!fgets(out, (int) outSize, stdin)) { out[0] = '\0'; return; }
    trimNewline(out);
}

void trimNewline(char *s) {
    size_t n = strlen(s);
    while (n > 0 && (s[n-1] == '\n' || s[n-1] == '\r')) {
        s[n-1] = '\0'; n--;
    }
}

/* read integer with validation */
int readInt(const char *prompt) {
    char buf[LINEBUF_SIZE];
    int val;
    while (1) {
        readLine(prompt, buf, sizeof(buf));
        if (sscanf(buf, "%d", &val) == 1) return val;
        printf("Invalid integer — try again.\n");
    }
}

/* read float with validation */
float readFloat(const char *prompt) {
    char buf[LINEBUF_SIZE];
    float val;
    while (1) {
        readLine(prompt, buf, sizeof(buf));
        if (sscanf(buf, "%f", &val) == 1) return val;
        printf("Invalid number — try again.\n");
    }
}

/* remove '|' characters from name (replace with space) */
void sanitizePipe(char *s) {
    for (; *s; ++s) if (*s == '|') *s = ' ';
}

/* check if roll already exists in STUDENT_FILE */
int rollExists(int roll) {
    FILE *fp = fopen(STUDENT_FILE, "r");
    char line[LINEBUF_SIZE];
    if (!fp) return 0; /* file doesn't exist -> not found */

    while (fgets(line, sizeof(line), fp)) {
        trimNewline(line);
        if (line[0] == '\0') continue;
        char copy[LINEBUF_SIZE];
        strncpy(copy, line, sizeof(copy)-1); copy[sizeof(copy)-1]=0;
        char *p = strtok(copy, "|");
        if (!p) continue;
        int r = atoi(p);
        if (r == roll) {
            fclose(fp);
            return 1;
        }
    }
    fclose(fp);
    return 0;
}

/* check if name already exists in STUDENT_FILE (case-insensitive) */
int nameExists(const char *name) {
    FILE *fp = fopen(STUDENT_FILE, "r");
    char line[LINEBUF_SIZE];
    if (!fp) return 0;

    char loweredName[NAME_MAX];
    strncpy(loweredName, name, sizeof(loweredName)-1);
    loweredName[sizeof(loweredName)-1] = '\0';
    for (char *p = loweredName; *p; ++p)
        *p = (char) tolower((unsigned char)*p);

    while (fgets(line, sizeof(line), fp)) {
        trimNewline(line);
        if (line[0] == '\0') continue;

        char copy[LINEBUF_SIZE];
        strncpy(copy, line, sizeof(copy)-1);
        copy[sizeof(copy)-1] = '\0';

        char *p = strtok(copy, "|");   /* roll */
        char *n = strtok(NULL, "|");   /* name */
        if (!n) continue;

        char loweredExisting[NAME_MAX];
        strncpy(loweredExisting, n, sizeof(loweredExisting)-1);
        loweredExisting[sizeof(loweredExisting)-1] = '\0';
        for (char *q = loweredExisting; *q; ++q)
            *q = (char) tolower((unsigned char)*q);

        if (strcmp(loweredName, loweredExisting) == 0) {
            fclose(fp);
            return 1;  /* duplicate name found */
        }
    }

    fclose(fp);
    return 0;
}

/* check if name exists for a different roll (used when updating) */
int nameExistsExceptRoll(const char *name, int roll) {
    FILE *fp = fopen(STUDENT_FILE, "r");
    char line[LINEBUF_SIZE];
    if (!fp) return 0;

    char loweredName[NAME_MAX];
    strncpy(loweredName, name, sizeof(loweredName)-1);
    loweredName[sizeof(loweredName)-1] = '\0';
    for (char *p = loweredName; *p; ++p)
        *p = (char) tolower((unsigned char)*p);

    while (fgets(line, sizeof(line), fp)) {
        trimNewline(line);
        if (line[0] == '\0') continue;

        char copy[LINEBUF_SIZE];
        strncpy(copy, line, sizeof(copy)-1);
        copy[sizeof(copy)-1] = '\0';

        char *p = strtok(copy, "|");   /* roll */
        char *n = strtok(NULL, "|");   /* name */
        if (!p || !n) continue;
        int r = atoi(p);

        if (r == roll) continue; /* ignore same roll */

        char loweredExisting[NAME_MAX];
        strncpy(loweredExisting, n, sizeof(loweredExisting)-1);
        loweredExisting[sizeof(loweredExisting)-1] = '\0';
        for (char *q = loweredExisting; *q; ++q)
            *q = (char) tolower((unsigned char)*q);

        if (strcmp(loweredName, loweredExisting) == 0) {
            fclose(fp);
            return 1;  /* duplicate name found on different roll */
        }
    }

    fclose(fp);
    return 0;
}
// this code can be pushed into github by double clicking on push.bat file