#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <limits.h>

int expr();

// Global file pointer so all functions have access to it.
FILE *file;

// Keeps a pointer that's in the exact position of the file we're in.
char token;

// Keeps a pointer that's ahead of every step we make.
char lookahead;

// Controls where we will be storing our temp values.
int temp = 0;
int counter = 1;

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
int printT()
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
            return 1;
        default:
            return 0;
    }
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
int newTemp()
{
    return ++temp;
}

// Computes the grammar for factor.
int factor()
{
    int num = 0, neg = 0;

    // Nested expression encountered.
    if (lookahead == '(')
    {
        consume(); // LPAREN
        num = expr();
        consume(); // RPAREN
    }
    else if (isdigit(lookahead) || lookahead == '-')
    {
        // Checks for negative numbers.
        if (lookahead == '-')
        {
            neg = 1;
            consume();
        }

        // Sets num to first digit encountered.
        consume();
        num = token - '1' + 1;

        // Allows for multidigit numbers to be passed in.
        while (isdigit(lookahead))
        {
            consume();
            num *= 10;
            num += token - '1' + 1;
        }

        if (neg && num != 0)
            num *= -1;   
    }
    else
    {
        error(); // invalid character
    }
    return num;
}

// Executes term prime grammar according to project1.md specifications.
int termPrime(int left)
{
    int right, result;
    char operation;
    spaces();

    // Term fits the order of => *FT` | /FT` | %FT`.
    if (lookahead == '*' || lookahead == '/' || lookahead == '%')
    {
        consume(); // consumes operation token
        operation = token;
        spaces();
        right = factor();
        result = newTemp();
        printf("  %%t%d = %s %d, %d\n", result, printOp(operation), left, right);
        return termPrime(result);
    }
    else
    {
        return left; // epsilon, do nothing
    }
}

// Executes term grammar according to project1.md specifications.
int term()
{
    int left = 0, result = 0;
    spaces();
    left = factor();
    result = termPrime(left);
    return result;
}

int exprPrime(int left)
{
    int right = 0, result = 0, num = 0;
    char operation;
    spaces();

    // Term fits the order of => +TE` | -TE`.
    if (lookahead == '+' || lookahead == '-')
    {
        consume(); // consumes operation token
        operation = token;
        right = term();
        result = newTemp();
        // if (left > right && temp > 1 && counter++ != temp)
        printf("  %%t%d = %s %d, %d\n", result, printOp(operation), left, right);
        return exprPrime(result);
    }
    else
    {
        return left; // epsilon, do nothing
    }
}

// Executes the grammar for expression prime according to
// project1.md specifications.
int expr()
{
    int left = 0, result = 0;
    left = term();
    result = exprPrime(left);
    return result;
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

// A statement is considering to be each line of code we read up until
// a semi-colon.
void statement()
{
    int print = printT();

    expr();

    if (lookahead == ';')
    {
        consume();
        if (print && temp > 1)
            // printf("  call void @print_integer(i32 %%t%d)\n"
            //        "  ret i32 0\n"
            //        "}\n", temp);
        exit(0);
    }
    return;
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

    
    // printf("target triple = \"x86_64-pc-linux-gnu\"\n"
    //        "declare i32 @printf(i8*, ...) #1\n"
    //        "@.str = private unnamed_addr constant [4 x i8] c\"%%d%c0A%c00\", align 1\n"
    //        "define void @print_integer(i32) #0 {\n"
    //        "  %%2 = alloca i32, align 4\n"
    //        "  store i32 %%0, i32* %%2, align 4\n"
    //        "  %%3 = load i32, i32* %%2, align 4\n"
    //        "  %%4 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str, i32 0, i32 0), i32 %%3)\n"
    //        "  ret void\n"
    //        "}\n"
    //        "\n"
    //        "define i32 @main() #0 {\n", 92, 92);

    while ((token = fgetc(file)) != EOF)
    {
        // Ensures we start at the very beginning of the file.
        ungetc(token, file);
        statement();
    }

    // printf("  ret i32 0\n"
    //        "}\n");

    return 0;
}
