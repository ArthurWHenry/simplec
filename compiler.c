// Arthur Henry
// COP3402 - Fall 2019

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define SIZE 16

char *expr();
char *printOp(char op);
char *newTemp();
void printList();
void exitLLVM();
int contains(char identifier);

typedef struct node
{
    char var;
    char *addr;
    char *num;
    struct node *next;
} node;

typedef struct LinkedList
{
    node *head;
    node *tail;
} LinkedList;

// NOTE: Variable declarations are all happening at the beginning
//       of the simplec program.

// TODO: Storing the variable names will be a simple tail insertion
//       into a linked list.

// TODO: Loading the variable will be a simple linked list search.
// NOTE: When searching, we will be looking for the var field in the node struct.

// NOTE: These may be slow operations, but are effective for what we need them to do.

// Global file pointer so all functions have access to it.
FILE *file;

// Keeps a pointer that's in the exact position of the file we're in.
char token;

// Keeps a pointer that's ahead of every step we make.
char lookahead;

// Controls where we will be storing our temp values.
int temp = 0;

// Helps with a program that carries a single integer.
char *numG;

// The head of our linked list that will hold our variables.
// Will allow O(1) insertion at the tail and O(n) lookup.
LinkedList *variables;

// NOTE: We don't have to add anything to the number just yet,
//       because we only read declarations at the beginning.
// TODO: Make sure that the number is stored properly. (We might
//       not have to actually store it.)
node *createNode(char var, char *addr)
{
    node *newNode = calloc(1, sizeof(node));
    // printf("%c\n", var);
    newNode->var = var;
    newNode->addr = addr;
    return newNode;
}

// Allocating memory for our linked list.
LinkedList *createList()
{
    return calloc(1, sizeof(LinkedList));
}

// Inserts a new temporary variable at the end of our linked list.
void tailInsert(char var, char *addr)
{
    if (variables == NULL)
    {
        return;
    }
    else if (variables->head == NULL)
    {
        variables->head = variables->tail = createNode(var, addr);
    }
    else
    {
        variables->tail->next = createNode(var, addr);
        variables->tail = variables->tail->next;
    }
    return;
}

// Returning the address of the node.
// i.e., the number of nodes we went through the list
char *lookup(char identifier)
{
    node *temp = NULL;

    // Identifier cannot exist in empty list.
    if (variables == NULL || variables->tail == NULL)
        return 0;

    temp = variables->head;

    while (temp != NULL)
    {
        if (temp->var == identifier)
            return temp->addr;

        temp = temp->next;
    }
    return NULL;
}

int isIdentifier(char identifier)
{
    node *temp = NULL;

    // Identifier cannot exist in empty list.
    if (variables == NULL || variables->tail == NULL)
        return 0;

    temp = variables->head;

    while (temp != NULL)
    {
        if (temp->var == identifier)
            return 1;

        temp = temp->next;
    }
    return 0;
}

// Updates the lookahead char.
void peek()
{
    // Get character from the file and store in lookahead.
    lookahead = fgetc(file);
    // Going back to where the file pointer was previously.
    ungetc(lookahead, file);
}

// Updates our current file pointer and lookahead.
void consume()
{
    token = fgetc(file);
    // Everytime we update our token, update the lookahead.
    peek();
}

// Skips spaces encountered.
void spaces()
{
    while (lookahead == ' ')
        consume();
}

// Returns 1 if we have a print token in the simpelc file.
// Otherwise, returns 0.
int printCheck()
{
    peek();
    switch(lookahead)
    {
        case 'p':
            // printf("%c\n", token);
            consume();
        case 'r':
            // printf("%c\n", token);
            consume();
        case 'i':
            // printf("%c\n", token);
            consume();
        case 'n':
            // printf("%c\n", token);
            consume();
        case 't':
            // printf("%c\n", token);
            consume();
            // printf("printing!\n");
            return 1;
        default:
            return 0;
    }
}

// Generates the LLVM code for printing.
void printLLVM(char var)
{
    char *tempToPrint;

    tempToPrint = malloc(sizeof(char) * SIZE);

    if (!contains(var))
    {
        fprintf(stderr, "error: use of undeclared variable %c\n", var);
        exitLLVM();
        exit(0);
    }

    // Checks if there is one value passed into the program.
    if (temp == 0)
    {
        printf("  %%t1 = %s %s, 0\n", printOp('+'), numG);
        temp++;
    }

    // TODO: Load the value of temp onto a new variable, then print that one.
    strcpy(tempToPrint, lookup(var));

    consume();

    printf("  %s = load i32, i32* %s\n", newTemp(), tempToPrint);
    printf("  call void @print_integer(i32 %%t%d)\n\n", temp);
}

// Prints out the LLVM instruction for the operation we have to perform.
// Otherwise, if the symbol is invalid NULL is returned.
char *printOp(char op)
{
    switch(op)
    {
        case '+':
            return "add nsw i32";
        case '-':
            return "sub nsw i32";
        case '*':
            return "mul nsw i32";
        case '/':
            return "sdiv i32";
        case '%':
            return "srem i32";
        default:
            break;
    }
    return NULL;
}

// Invalid token was passed into factor().
// This is true according to project1.md specifications.
void error()
{
    printf("Error(), exiting.\n");
    exit(0);
}

// Assigns a new temporary variable to a result.
char *newTemp()
{
    char *new = malloc(sizeof(char) * (16));

    snprintf(new, 16, "%s%d", "%t", ++temp);

    return new;
}

// Computes the grammar for factor.
char *factor()
{
    int i = 0;
    char *num = NULL, *addr = NULL, *result = NULL;
    char identifier;
    num = malloc(sizeof(char) * 16);
    numG = malloc(sizeof(char) * 16);

    // spaces();

    // Nested expression encountered.
    if (lookahead == '(')
    {
        consume(); // LPAREN
        strcpy(num, expr());
        consume(); // RPAREN
    }
    else if (isdigit(lookahead) || lookahead == '-')
    {
        // printf("la: %c\n", lookahead);
        // Checks for negative numbers.
        if (lookahead == '-')
        {
            consume();
            num[i++] = token;
        }

        // Sets num to first digit encountered.
        consume();
        num[i++] = token;

        // Allows for multidigit numbers to be passed in.
        while (isdigit(lookahead))
        {
            consume();
            num[i++] = token;
        }

        num[i] = '\0';
        // printf("num in factor(): %s\n", num);

        strcpy(numG, num);
        return numG;
    }
    else if (isIdentifier(lookahead))
    {
        // printf("la: %c (supposed to be y)\n", lookahead);
        consume();
        identifier = token;
        spaces();

        // Checking to see if lookahead already exists in our linked list
        addr = lookup(identifier);

        // A variable that doesn't exist is trying to be accessed, return error
        // to avoid compiling problems.
        if (addr == NULL)
        {
            fprintf(stderr, "error: use of undeclared variable %c\n", identifier);
            exitLLVM();
            exit(0);
        }

        result = newTemp();
        printf("  %s = load i32, i32* %s\n", result, addr);
        return result;
    }
    else
    {
        fprintf(stderr, "error: use of undeclared variable %c\n", lookahead);
        exitLLVM();
        exit(0);
    }
    return num;
}

// Executes the term with a loop.
char *term()
{
    char operation;
    char *left = NULL, *right = NULL, *result = NULL;

    left = malloc(sizeof(char) * 16);
    right = malloc(sizeof(char) * 16);
    result = malloc(sizeof(char) * 16);

    spaces();
    // NOTE: I was having issues here. Just leaving this as a
    //       reference for possible future problems.
    strcpy(left, factor());
    spaces();
    // printf("lookahead: %c\n", lookahead);
    while (lookahead == '*' || lookahead == '/' || lookahead == '%' || lookahead == '=')
    {
        spaces();
        if (lookahead == '=')
            printf("%s\n", variables->head->addr);
        consume();
        operation = token;
        spaces();
        strcpy(right, factor());
        spaces();
        strcpy(result, newTemp());
        printf("  %s = %s %s, %s\n", result, printOp(operation), left, right);
        strcpy(left, result);
    }

    // printf("lookahead in term(): %c\n", lookahead);
    return left;
}

// Executes the expression with a loop.
char *expr()
{
    char operation;
    char *left = NULL, *right = NULL, *result = NULL;

    left = malloc(sizeof(char) * 16);
    right = malloc(sizeof(char) * 16);
    result = malloc(sizeof(char) * 16);

    spaces();
    strcpy(left, term());

    spaces();

    while (lookahead == '+' || lookahead == '-')
    {
        consume();
        operation = token;
        spaces();
        strcpy(right, term());
        spaces();
        strcpy(result, newTemp());
        printf("  %s = %s %s, %s\n", result, printOp(operation), left, right);
        strcpy(left, result);

    }
    return left;
}

// Opens the file if it exists. Otherwise, we exit the program.
void processInputFile(char *filename)
{
    if ((file = fopen(filename, "r")) == NULL)
    {
        fprintf(stderr, "File could not be opened in processInputFile().\n");
        exit(0);
    }
    return;
}

// Consumes semi colons or new line characters.
void semi()
{
    // if (print)
    //     printLLVM();
    while (lookahead == ';' || lookahead == '\n')
        consume();
}

// Checks if the linked list already contains the identifier passed in.
int contains(char identifier)
{
    node *temp = NULL;

    // Preventing segfault.
    if (variables == NULL || variables->tail == NULL)
        return 0;

    temp = variables->head;

    // Iterating through variable list without affecting original state.
    while (temp != NULL)
    {
        // printf("%c -> ", temp->var);
        // Identifier already exists in the list.
        if (temp->var == identifier)
            return 1;
        temp = temp->next;
    }
    // printf("\n");
    // Identifier has not been inserted into the list of variables.
    return 0;
}

// Prints the linked list to make sure all values exist within the
// data structure. (NOTE: It's for debugging purposes.)
void printList()
{
    node *temp = NULL;

    temp = variables->head;

    while (temp != NULL)
    {
        printf("%c -> ", temp->var);
        temp = temp->next;
    }

    printf("\n");
}

// Returns 1 if the beginning on the line started with 'read'.
// Otherwise, returns 0.
int readCheck()
{
    peek();
    switch (lookahead)
    {
        case 'r':
        case 'e':
        case 'a':
        case 'd':
            consume();
            consume();
            consume();
            consume();
            return 1;
        default:
            return 0;
    }
    return 0;
}

// Reads and stores integers.
void read()
{
    char identifier;
    char *addr, *result;

    addr = malloc(sizeof(char) * SIZE);
    result = malloc(sizeof(char) * SIZE);

    spaces();
    // Gets identifier.
    consume();

    identifier = token;

    spaces();

    // FIXME: segfaulting here (possible another error)
    strcpy(addr, lookup(identifier));

    if (addr == NULL)
    {
        fprintf(stderr, "error: use of undeclared variable %c\n", identifier);
        exitLLVM();
        exit(0);
    }

    strcpy(result, newTemp());
    printf("  %s = call i32 @read_integer()\n", result);
    printf("  store i32 %s, i32* %s\n\n", result, addr);
}

// Assigns values to variables that exist within our linked
// list data structure storing all temporary variables.
void assign()
{
    char identifier;
    char *addr, *result;

    addr = malloc(sizeof(char) * SIZE);
    result = malloc(sizeof(char) * SIZE);

    consume();
    identifier = token;
    spaces();

    // printf("token: %c\n", token);

    // Consumes equal sign.
    consume();
    spaces();

    // printf("514: token: %c\n", token);

    // printf("lookahead: %c\n", lookahead);

    strcpy(result, expr());

    spaces();

    // Consumes semi colon.
    semi();

    strcpy(addr, lookup(identifier));

    if (addr == NULL)
    {
        fprintf(stderr, "error: use of undeclared variable %c\n", identifier);
        exitLLVM();
        exit(0);
    }

    printf("  store i32 %s, i32* %s\n\n", result, addr);
}

// Returns 1 if there is an integer delcaration.
// Otherwise, returns 0.
int declarationCheck()
{
    switch(lookahead)
    {
        case 'i':
            consume();
        case 'n':
            consume();
        case 't':
            consume();
            return 1;
        default:
            return 0;
    }
    return 0;
}

// Declares a variable by inserting it into our linked list.
void declaration()
{
    char identifier;
    char *result = NULL;

    result = malloc(sizeof(char) * SIZE);

    spaces();

    // printf("la: %c\n", lookahead);
    if (contains(lookahead))
    {
        fprintf(stderr, "error: multiple definitions of %c\n", lookahead);
        exitLLVM();
        exit(0);
    }

    if (isalpha(lookahead))
    {
            spaces();
            // Gets identifier.
            consume();
            identifier = token;

            consume(); // consumes semi colon

            // Checking if identifier already exists in symbol table.
            if (contains(identifier))
                error();

            strcpy(result, newTemp());
            // printList();
            // printf("  ; \"int %c;\"\n", identifier);
            printf("  %s = alloca i32\n\n", result);
            tailInsert(identifier, result);
    }
    semi();
}

// Depending on the value of printCheck(), we either print the
// variable the SimpleC program is asking for, or computing an expression.
void statement()
{
    spaces();
    peek();
    if (declarationCheck())
    {
        // printf("Going into declaration()...\n");
        declaration();
    }
    else if (isIdentifier(lookahead))
    {
        // printf("Going into assign()...\n");
        assign();
    }
    else if (readCheck())
    {
        // printf("lookahead: %c\n", lookahead);
        // printf("Going into read()...\n");
        read();
    }
    else if (printCheck())
    {
        // printf("Going into print()...\n");
        spaces();
        printLLVM(lookahead);
    }
    semi();

    return;
}

// Template code that needs to be generated before we execute our
// main function.
void boilerplate()
{
    printf("target triple = \"x86_64-pc-linux-gnu\"\n"
           "declare i32 @printf(i8*, ...) #1\n"
           "@.str = private unnamed_addr constant [4 x i8] c\"%%d%c0A%c00\", align 1\n"
           "define void @print_integer(i32) #0 {\n"
           "  %%2 = alloca i32, align 4\n"
           "  store i32 %%0, i32* %%2, align 4\n"
           "  %%3 = load i32, i32* %%2, align 4\n"
           "  %%4 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str, i32 0, i32 0), i32 %%3)\n"
           "  ret void\n"
           "}\n\n", 92, 92);

    printf("%%struct._IO_FILE = type { i32, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, %%struct._IO_marker*, %%struct._IO_FILE*, i32, i32, i64, i16, i8, [1 x i8], i8*, i64, %%struct._IO_codecvt*, %%struct._IO_wide_data*, %%struct._IO_FILE*, i8*, i64, i32, [20 x i8] }\n"
           "%%struct._IO_marker = type opaque\n"
           "%%struct._IO_codecvt = type opaque\n"
           "%%struct._IO_wide_data = type opaque\n\n");

    printf("@stderr = external dso_local global %%struct._IO_FILE*, align 8\n"
           "@.str.1 = private unnamed_addr constant [25 x i8] c\"please enter an integer\\0A\\00\", align 1\n"
           "@.str.2 = private unnamed_addr constant [3 x i8] c\"%%d\\00\", align 1\n"
           "@.str.3 = private unnamed_addr constant [6 x i8] c\"scanf\\00\", align 1\n"
           "@.str.4 = private unnamed_addr constant [24 x i8] c\"no matching characters\\0A\\00\", align 1\n\n");

    printf("declare i32* @__errno_location() #2\n"
           "declare i32 @__isoc99_scanf(i8*, ...) #1\n"
           "declare void @perror(i8*) #1\n"
           "declare void @exit(i32) #3\n"
           "declare i32 @fprintf(%%struct._IO_FILE*, i8*, ...) #1\n\n");


    printf("define i32 @read_integer() #0 {\n"
           "  %%1 = alloca i32, align 4\n"
           "  %%2 = alloca i32, align 4\n"
           "  %%3 = call i32* @__errno_location() #4\n"
           "  store i32 0, i32* %%3, align 4\n"
           "  %%4 = load %%struct._IO_FILE*, %%struct._IO_FILE** @stderr, align 8\n"
           "  %%5 = call i32 (%%struct._IO_FILE*, i8*, ...) @fprintf(%%struct._IO_FILE* %%4, i8* getelementptr inbounds ([25 x i8], [25 x i8]* @.str.1, i32 0, i32 0))\n"
           "  %%6 = call i32 (i8*, ...) @__isoc99_scanf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @.str.2, i32 0, i32 0), i32* %%1)\n"
           "  store i32 %%6, i32* %%2, align 4\n"
           "  %%7 = load i32, i32* %%2, align 4\n"
           "  %%8 = icmp eq i32 %%7, 1\n"
           "  br i1 %%8, label %%9, label %%11\n"
           "\n"
           "; <label>:9:                                      ; preds = %%0\n"
           "  %%10 = load i32, i32* %%1, align 4\n"
           "  ret i32 %%10\n"
           "\n"
           "; <label>:11:                                     ; preds = %%0\n"
           "  %%12 = call i32* @__errno_location() #4\n"
           "  %%13 = load i32, i32* %%12, align 4\n"
           "  %%14 = icmp ne i32 %%13, 0\n"
           "  br i1 %%14, label %%15, label %%16\n"
           "\n"
           "; <label>:15:                                     ; preds = %%11\n"
           "  call void @perror(i8* getelementptr inbounds ([6 x i8], [6 x i8]* @.str.3, i32 0, i32 0))\n"
           "  call void @exit(i32 1) #5\n"
           "  unreachable\n"
           "\n"
           "; <label>:16:                                     ; preds = %%11\n"
           "  %%17 = load %%struct._IO_FILE*, %%struct._IO_FILE** @stderr, align 8\n"
           "  %%18 = call i32 (%%struct._IO_FILE*, i8*, ...) @fprintf(%%struct._IO_FILE* %%17, i8* getelementptr inbounds ([24 x i8], [24 x i8]* @.str.4, i32 0, i32 0))\n"
           "  call void @exit(i32 1) #5\n"
           "  unreachable\n"
           "}\n\n");

    printf("define i32 @main() #0 {\n");
}

// Prints the exit code for LLVM file.
void exitLLVM()
{
    printf("  ret i32 0\n}\n");
}

int main(int argc, char **argv)
{
    char c;
    // Making sure two arguments are passed in.
    if (argc < 2)
    {
        fprintf(stderr, "Proper usage: %s <filename>.\n", argv[0]);
        return 0;
    }

    processInputFile(argv[1]);

    variables = createList();

    boilerplate();

    while ((token = fgetc(file)) != EOF)
    {
        // Ensures we start at the very beginning of the file.
        ungetc(token, file);
        statement();
    }

    exitLLVM();
    fclose(file);

    return 0;
}
