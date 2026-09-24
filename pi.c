#include<stdio.h>

int main(int argc, char **argv)
{
     FILE *fp;
     int count[10]={0};
     int c;

     if (argc != 2) {
         printf("Kullanim: %s <dosya>\n", argv[0]);
         return 1;
     }

     fp = fopen(argv[1], "r");
     if (fp == NULL) {
         printf("Boyle bir dosya yok\n");
         return 1;
     }

     c = fgetc(fp);
     while (c != EOF) {
         if (c >= '0' && c <= '9')
             count[c - '0']++;
         c = fgetc(fp);
     }
     fclose(fp);

     for (int i = 0; i < 10; i++)
         printf(" count %d =%d \n", i, count[i]);
     return 0;
}
