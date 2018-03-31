#include <stdio.h>

int main(int argc, char* argv[]){
    int val[100];
    for (int i = 0; i < 100; ++i){
	val[i] = i;
}	
   // int *p = val;
    printf("%d\n", *(val+(byte)4));
	

for(int i = 0; i< 100; ++i){

	printf("%p\n", &val[i]);
}

return 0;
}
