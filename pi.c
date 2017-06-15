#include<stdio.h>

int main()
{
	FILE *fp;
     int count[10]={0,0,0,0,0,0,0,0,0};
     int a=0;
      int c;
     fp = fopen("pi7.txt","r");
     if (fp == NULL) printf("Boyle bir dosya yok\n");
	do {
       c = getc(fp);    /* Bir karakter oku  */
       
	     if(c=='0')
	      count[0]++;    
		 else if(c=='1')
		 count[1]++;
		 else if(c=='2')
		 count[2]++;  
         else if(c=='3')
		 count[3]++; 
		 else if(c=='4')
		  count[4]++; 
         else if(c=='5')
		  count[5]++; 
		 else if(c=='6')
		  count[6]++;   
         else if(c=='7')
		  count[7]++; 
		 else if(c=='8')
		  count[8]++; 
		 else if(c=='9')
		  count[9]++; 
		 
		
	 } while (c!=EOF);
	 for(int i=0;i<10;i++)
	 printf( " count%d =%d \n",i,count[i]);
	
	
	return 0;
}
