#include <stdio.h> 
#include <stdlib.h> 
#include <string.h>

  int main(int argc, char * argv[]) {
    if (argc == 1) {
      printf("my-zip: file1 [file2 ...]\n");
	    exit(1);
    }
      char thischar;
      char lastchar;
      int count, totalCount = 0;
      lastchar = EOF;
    for (int i = 1; i < argc; i++) {
      FILE * fp = fopen(argv[i], "r");
      if (fp == NULL) {
        printf("my-zip: cannot open file\n");
        exit(1);
      }

      while ((thischar = fgetc(fp)) != EOF) {
          totalCount++;
          if(totalCount == 1){
            lastchar = thischar;
          }
          if(thischar == EOF && thischar != lastchar && totalCount > 1){
            fwrite(&count, 4, 1, stdout);
            printf("%c", lastchar);
          }
          if(thischar == lastchar){
            count++;
          }
          else{
            fwrite(&count, 4,1,stdout);
            printf("%c", lastchar);
            lastchar = thischar;
            count = 1;
          }
          }
      if(thischar == EOF && i == argc -1){
        fwrite(&count, 4,1,stdout);
        printf("%c", lastchar);
      }
      fclose(fp);
      }
    return 0;
  }
