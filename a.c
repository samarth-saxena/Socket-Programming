#include<stdio.h>
 
int main()
{
    // Pointer to an integer
    int *p;
     
    // Pointer to an array of 5 integers
    int (*ptr)[5];
    int arr[5];
     
     arr[0] = 1;
     arr[1] = 2;
     arr[2] = 3;
     arr[3] = 4;
     arr[4] = 5;
    // Points to 0th element of the arr.
    p = arr;
     
    // Points to the whole array arr.
    ptr = &arr;
     
    printf("p = %d, ptr = %p\n", *p, ptr);
     
    p++;
    ptr++;
     
    printf("p = %d, ptr = %p\n", *p, ptr);
     
    return 0;
}