#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"jrb.h"
#include"btree.h"
void themphienam(char *word,char *mean)
{
  	int i,j,k=0,check=0;
  	for(i=0;i<strlen(word);i++)
  	{
    		if(word[i]=='/'){
      			mean[0]='\0';
      			check=1;
      			break;
    		}
  	}
  	if(check==0) {
    		word[strlen(word)-1]='\0';
    		return;
  		}
  	for(j=i;j<=strlen(word);j++){
    		mean[k++]=word[j];
  		}
  	word[i-1]='\0';
}
void xu_li_name(char *a){
  	int i;
  	for(i=1;i<=strlen(a);i++)
    	{
      		a[i-1]=a[i];
    	}
}
int main(int argc, char* argv[]) {
	BTA *data;
	FILE* f;
	int line = 0;
  	data = btcrt(argv[2], 0, 0);
	int index,i=0,k,count=0;
  	char name[50],mean[100000],tmp[100];
	if (argc < 2) {
    		printf("Error\n");
   		exit(EXIT_FAILURE);
  	}
  	if ((f = fopen(argv[1], "r")) == NULL) {
    		printf("Error\n");
    		exit(EXIT_FAILURE);
  	}
  	
  	while(!feof(f)){
    		fgets(tmp,100,f);
    		while(tmp[0]!='@'){
      			strcat(mean,tmp);
      			if(feof(f)) break;
      			fgets(tmp,100,f);
      					
    		}
    		//printf("nghia:%s",mean);
    		int k=btins(data,name,mean,500*sizeof(char));
    		if(k==0) count++;
    		if(tmp[0]=='@'){
      			strcpy(name,tmp);
      			mean[0]='\0';
      			themphienam(name,mean);
      			xu_li_name(name);
      			printf("tu:%s\n",name);
    			}
    		}
  		printf("da them %d tu vao tu dien\n",count);
  		fclose(f);
  		btcls(data);
}
 	