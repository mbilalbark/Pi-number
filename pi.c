#include<stdio.h>

int main()
{
     FILE *fp;
     int numbers[10]={0,1,2,3,4,5,6,7,8,9};
     int count[10]={0};
     int a=0;
     int c;
     fp = fopen("pi7.txt","r");  // burada pi dosyalarını değiştirebilirsiniz. pi den başlayıp pi 7 ye kadar gider.
     if (fp == NULL) printf("Boyle bir dosya yok\n");
	 c= fgetc(fp);
	do {
       if(c==' ')
	    c= fgetc(fp);
	
	else{
	        num = atoi(&c);
	        count[numbers[num]]++;
	}    
           c= fgetc(fp);
		
	 } while (c!=EOF);
	 for(int i=0;i<10;i++)
	 printf( " count %d =%d \n",i,count[i]);
	
	
	return 0;
}
