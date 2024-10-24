// Copyright (c) 2024 German Aerospace Center (DLR), Institute of Software Technology, Germany
// SPDX-FileContributor: Melven Roehrig-Zoellner <Melven.Roehrig-Zoellner@DLR.de>
//
// SPDX-License-Identifier: BSD-3-Clause

// required includes
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <limits.h>
#include <stdint.h>

// the maximal available memory (in double numbers)
#ifndef MAX_MEMORY
#define MAX_MEMORY 450
#endif

// must be called in the beginning, to make my_malloc work
void init_my_malloc();

// return pointer to memory of size n
void* my_malloc(long n);

// set printing to pointer type instead of double
void set_print_as_pointer(double** ptr, long n);

// free pointer to memory
void my_free(void* ptr);

// print the whole memory usage (row-wise)
void print_memory_used();

// print the value that is stored in memory (row-wise)
void print_memory_value();

// ---------------------------------------------------------------
// example usage
// ---------------------------------------------------------------
/*
int main()
{
    init_my_malloc();

    // allocate an array of length 7
    //int n = 17;
    //double* x = (double*)my_malloc(sizeof(double)*n);

    // allocate two pointers to double arrays
    //double** y = (double**)my_malloc(sizeof(double*)*2);
    // make printing of pointers nicer
    //set_print_as_pointer(y, 2);
    // let the array of two pointer point to something
    //y[0] = (double*)my_malloc(sizeof(double)*2);
    //y[1] = (double*)my_malloc(sizeof(double)*3);

    print_memory_used();
    print_memory_value();

    // free all memory
    //my_free(x);
    //my_free(y[0]);
    //my_free(y[1]);
    //my_free(y);
}
*/

// ---------------------------------------------------------------
// internal implementation
// ---------------------------------------------------------------

static double MEMORY_BUFFER[MAX_MEMORY];
static bool MEMORY_USED[MAX_MEMORY];
static bool MEMORY_IS_POINTER[MAX_MEMORY];

double double_rand()
{
    _Static_assert(RAND_MAX > UCHAR_MAX, "We need at least randomness for one unsigned char...");
    _Static_assert(sizeof(double) == 8*sizeof(unsigned char), "check sizes of datatypes");

    // the standard rand function in C doesn't generate doubles...
    double resultBuff;
    unsigned char* pBuff = (unsigned char*)&resultBuff;
    for(int i = 0; i < 8; i++)
        pBuff[i] = rand() % UCHAR_MAX;

    return resultBuff;
}

void init_my_malloc()
{
    srand(time(NULL));
    for(long i = 0; i < MAX_MEMORY; i++)
    {
        MEMORY_USED[i] = false;
        MEMORY_BUFFER[i] = double_rand();
    }
}

void* my_malloc(long n)
{
    // we are using double, so divide n by 8
    n = (n-1) / 8 + 1;
    const long search_start_address = rand() % MAX_MEMORY;

    // find free space big enough to hold n+2 bytes
    long offset = 0;
    while(offset < MAX_MEMORY)
    {
        const long start_address = (search_start_address + offset) % MAX_MEMORY;
        if( MEMORY_USED[start_address] )
        {
            offset++;
            continue;
        }

        bool found_free_space = true;
        for(long i = 0; i < n+2; i++)
        {
            const long end_address = start_address + i;
            if( end_address >= MAX_MEMORY || MEMORY_USED[end_address] )
            {
                offset += i+1;
                found_free_space = false;
                break;
            }
        }

        if( found_free_space )
        {
            for(int i = 1; i < n+1; i++)
            {
                MEMORY_USED[start_address+i] = true;
                MEMORY_IS_POINTER[start_address+i] = false;
            }
            return &MEMORY_BUFFER[start_address+1];
        }
    }

    printf("Out of memory!\n");
    exit(1);
}

void set_print_as_pointer(double **ptr, long n)
{
    intptr_t offset = (double*)ptr - MEMORY_BUFFER;
    for(int i = 0; i < n; i++)
        MEMORY_IS_POINTER[offset+i] = true;
}

void my_free(void* ptr)
{
    long address = ((double*)ptr - &MEMORY_BUFFER[0]);
    while( address < MAX_MEMORY && MEMORY_USED[address] )
    {
        MEMORY_USED[address] = false;
        address++;
    }
}

void print_memory_used()
{
    printf("Current memory usage:");
    const int cols = 25;
    for(int i = 0; i < MAX_MEMORY; i++)
    {
        if( i % cols == 0 )
            printf("\n%6d: ",i);
        if( MEMORY_USED[i] )
        {
            if( MEMORY_IS_POINTER[i] )
                printf("p");
            else
                printf("x");
        }
        else
            printf(".");
    }
    printf("\n");
}

void print_memory_value()
{
    printf("Current memory values:");
    const int cols = 25;
    for(int i = 0; i < MAX_MEMORY; i++)
    {
        if( i % cols == 0 )
            printf("\n%6d:",i);
        if( MEMORY_USED[i] )
        {
            if( MEMORY_IS_POINTER[i] )
            {
                double *pMemD = &MEMORY_BUFFER[i];
                double** ppMemD = (double**)pMemD;
                printf(" %5ld", *ppMemD - &MEMORY_BUFFER[0]);
            }
            else
                printf(" %5.2g", MEMORY_BUFFER[i]);
        }
        else
            printf("      ");
    }
    printf("\n");
}
