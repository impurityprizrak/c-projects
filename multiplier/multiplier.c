#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define MAX_BITS 17
#define MAX_COMPUTABLE 511

// Logic Gates

int and(int a, int b) 
{
    return a && b;
}

int or(int a, int b) 
{
    return a || b;
}

int not(int a) 
{
    return !a;
}

int xor(int a, int b) 
{
    return and(or(a, b), not(and(a, b)));
}

// Adder and Subtracter Bit-To-Bit Implementation

int adder(int a, int b, int *carry)
{
    int result, new_carry;
    result = xor(xor(a, b), *carry);
    new_carry = and(a, b) || (and(xor(a, b), *carry));
    
    *carry = new_carry;

    return result;
}

int subtracter(int a, int b, int *borrow)
{
    int result, new_borrow;
    result = xor(xor(a, b), *borrow);
    new_borrow = or(and(not(a), b), and(xor(not(a), b), *borrow));
    
    *borrow = new_borrow;

    return result;
}

// Utils to convert decimal to binary and vice versa

int *decimalToBin(int n) 
{
    int *binary = malloc(MAX_BITS * sizeof(int));
    memset(binary, 0, MAX_BITS * sizeof(int));
    
    int counter = 0;
    int curr = n;
    
    while (curr > 0) {
        binary[MAX_BITS - 1 - counter] = curr % 2;
        curr /= 2;
        counter++;
    }

    return binary;
}

int binToDecimal(int *bin) 
{
    int result = 0;

    for (int i = 0; i < MAX_BITS; i++) {
        if (bin[i] == 1) {
            result += pow(2, MAX_BITS - 1 - i);
        }
    }

    return result;
}

// Bytearray manipulation

void swapArrays(int *a, int *b)
{
    int temp;

    for (int i = 0; i < MAX_BITS; i++) {
        temp = a[i];
        a[i] = b[i];
        b[i] = temp;
    }
}

int compareArrays(int *a, const int *b, int size) 
{
    for (int i = 0; i < size; i++) {
        if (a[i] != b[i]) return 0;
    }
    return 1;
}

int compareBinary(int *a, int *b)
{
    for (int i = 0; i < MAX_BITS; i++) {
        if (a[i] > b[i]) {
            return 1;
        } else if (a[i] < b[i]) {
            return -1;
        }
    }

    return 0;
}

// Computation of pre-calculated math arrays

void computeSquare(int **square_array, int **odds_array)
{
    int *current_square = decimalToBin(0);
    int carry = 0;

    for (int c = 0; c < MAX_COMPUTABLE; c++) {
        square_array[c] = malloc(MAX_BITS * sizeof(int));
        memcpy(square_array[c], current_square, MAX_BITS * sizeof(int));

        int *next_square = malloc(MAX_BITS * sizeof(int));
        memset(next_square, 0, MAX_BITS * sizeof(int));

        for (int i = MAX_BITS - 1; i >= 0; i--) {
            next_square[i] = adder(current_square[i], odds_array[c][i], &carry);   
        }

        free(current_square);
        current_square = next_square;
    }
}

void computeOdds(int **odds_array)
{
    const int *two = decimalToBin(2);
    const int *max_num = decimalToBin(MAX_COMPUTABLE-1);
    int *current = decimalToBin(1);
    int carry = 0;

    for (int c = 0; c < MAX_COMPUTABLE; c++) {
        odds_array[c] = malloc(MAX_BITS * sizeof(int));
        memcpy(odds_array[c], current, MAX_BITS * sizeof(int));

        int *next = malloc(MAX_BITS * sizeof(int));
        for (int i = MAX_BITS - 1; i >= 0; i--) {
            next[i] = adder(current[i], two[i], &carry);
        }

        free(current);
        current = next;
    }

    free(current);
}

void computeRange(int **range) 
{
    const int *unit = decimalToBin(1);
    const int *max_num = decimalToBin(MAX_COMPUTABLE-1);
    int *current = decimalToBin(0);
    int counter = 0;
    int carry = 0;

    range[counter] = malloc(MAX_BITS * sizeof(int));
    memcpy(range[counter], current, MAX_BITS * sizeof(int));
    counter += 1;

    while (!compareArrays(current, max_num, MAX_BITS)) {
        int *result = malloc(MAX_BITS * sizeof(int));

        for (int i = MAX_BITS - 1; i >= 0; i--) {
            int a = current[i];
            int b = unit[i];
            result[i] = adder(a, b, &carry);
        }

        range[counter] = result;
        memcpy(current, result, MAX_BITS * sizeof(int));
        counter++;
    }
}

// Components of the math formula of multiplication

int *sum(int *n1, int *n2) 
{
    int *result = malloc(MAX_BITS * sizeof(int));
    int carry = 0;

    for (int i = MAX_BITS - 1; i >= 0; i--) {
        int a = n1[i];
        int b = n2[i];
        int r = adder(a, b, &carry);
        result[i] = r;
    }

    return result;
}

int *sub(int *n1, int *n2)
{
    int *result = malloc(MAX_BITS * sizeof(int));
    int borrow = 0;

    for (int i = MAX_BITS - 1; i >= 0; i--) {
        int a = n1[i];
        int b = n2[i];
        int r = subtracter(a, b, &borrow);
        result[i] = r;
    }

    return result;
}

int *shiftRight(int *number, int places)
{
    int *result = malloc(MAX_BITS * sizeof(int));
    memset(result, 0, MAX_BITS);

    for (int i = MAX_BITS - 1; i >= places; i--) {
        result[i] = number[i - places];
    }

    return result;
}

int indexLoc(int **range, int *n)
{
    for (int i = 0; i < MAX_COMPUTABLE; i++) {
        int *src = range[i];
        
        if (compareArrays(src, n, MAX_BITS)) {
            return i;
        }
    }

    return 0;
}

int main(void) {
    int **range_array = (int **)malloc(MAX_COMPUTABLE * sizeof(int *));
    computeRange(range_array);

    int **odds_array = (int **)malloc(MAX_COMPUTABLE * sizeof(int *));
    computeOdds(odds_array);

    int **square_array = (int **)malloc(MAX_COMPUTABLE * sizeof(int *));
    computeSquare(square_array, odds_array);

    char bits1[MAX_BITS+1];
    char bits2[MAX_BITS+1];

    int *n1 = malloc(MAX_BITS * sizeof(int));
    int *n2 = malloc(MAX_BITS * sizeof(int));
    memset(n1, 0, MAX_BITS * sizeof(int));
    memset(n2, 0, MAX_BITS * sizeof(int));

    printf("Enter the first number (max. %d bits): ", MAX_BITS/2);
    fgets(bits1, MAX_BITS + 1, stdin);

    printf("Enter the second number (max. %d bits): ", MAX_BITS/2);
    fgets(bits2, MAX_BITS + 1, stdin);

    for (int i = MAX_BITS-1, j = strlen(bits1)-2; j >= 0; i--, j--) {
        n1[i] = (bits1[j] == '1') ? 1 : 0;
    }

    for (int i = MAX_BITS-1, j = strlen(bits2)-2; j >= 0; i--, j--) {
        n2[i] = (bits2[j] == '1') ? 1 : 0;
    }

    if (compareBinary(n1, n2) < 0) swapArrays(n1, n2);

    int *sum_result = sum(n1, n2);
    int sum_idx = indexLoc(range_array, sum_result);
    int *sum_square = square_array[sum_idx];
    int *sum_square_shf = shiftRight(sum_square, 2);

    int *sub_result = sub(n1, n2);
    int sub_idx = indexLoc(range_array, sub_result);
    int *sub_square = square_array[sub_idx];
    int *sub_square_shf = shiftRight(sub_square, 2);

    int *result = sub(sum_square_shf, sub_square_shf);

    printf("\nThe result in base 2 is: ");
    for (int i = 0; i < MAX_BITS; i++) {
        printf("%d", result[i]);
    }

    printf("\nThe result in base 10 is: %d\n", binToDecimal(result));
    
    return 0;
}