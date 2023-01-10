#include <stdio.h>
// #include <stdlib.h>

int main()
{
   char str[50];

   printf("$> ");
   gets(str);

   printf("[%s]\n", str);

   return (0);
}