#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/sysinfo.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include<pthread.h>
#define handle_error(msg) \
    do { perror(msg); exit(EXIT_FAILURE); } while (0)


typedef struct {
   int s_count;
   char c;
   //int merged;
} __attribute__((__packed__)) subs;

typedef struct {
   int c_idx;
   int done;
} chunk_info;


pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;    
int CMAX;
int work_idx = 0;
char *fileptr;
int chunksize, c_number;
off_t c_offset, c_mod;
subs ** out_buffer;
int *outchunk_sz;
chunk_info *chunkqueue;

void *consumer(void *arg){
	while(1){
		pthread_mutex_lock(&lock); 
		int temp_index = work_idx;
		//printf(" worker: %d ", *arg);
		//printf(" work_idx: %d", work_idx);
		if(work_idx >= c_number){
			//printf("finished!\n");
			pthread_mutex_unlock(&lock);		
			break;
		}
		if(chunkqueue[temp_index].c_idx != temp_index){
			//printf("index not match!\n");
			pthread_mutex_unlock(&lock);
			continue;
		}
		work_idx++;
		if(chunkqueue[temp_index].done == 1){
			//printf("job already done!\n");
			pthread_mutex_unlock(&lock);
			continue;
		}
		pthread_mutex_unlock(&lock);
		chunk_info curr_chunk = chunkqueue[temp_index];
		chunkqueue[temp_index].done = 1;

		int end_offset;
		if(c_mod != 0 && curr_chunk.c_idx == c_number - 1){
			end_offset = c_mod;
		}
		else{
			end_offset = chunksize;
		}
		//printf(" temp_index: %d , end_offset: %d", temp_index, end_offset);

		

		// zip
		char lastchar = 0;
		int count = 0;
		int subindex = 0;
		for(int i = temp_index * chunksize; i < temp_index * chunksize + end_offset; i++){
			char c = fileptr[i];
			if(count == 0){
				lastchar = c;
				count++;
				continue;
			}
			if(c == lastchar){
				count++;
				continue;
			}
			else{
			  	subs sb = {count, lastchar};
			  	out_buffer[temp_index][subindex] = sb;
			  	subindex++;
				count = 1;
				lastchar = c;
			}
		}

		// add last group of characters
		subs sb = {count, lastchar};
		out_buffer[temp_index][subindex] = sb;

		//pthread_mutex_lock(&lock); 
		outchunk_sz[temp_index] = subindex;
		//pthread_mutex_unlock(&lock); 		
		//printf(" subindex: %d \n", subindex);
	}
	return NULL;
}

int
main(int argc, char *argv[])
{
  CMAX = get_nprocs();
	int fd;
	struct stat sb;
	size_t filesz;
	chunksize = 256 * sysconf(_SC_PAGE_SIZE);
	subs last = {-1,0};
  int init_merge_flag = 1;
  	
	for(int i = 1; i < argc; i++){
		// open file
		fd = open(argv[i], O_RDONLY);
		if (fd == -1)
		handle_error("open");

		// obtain file size
		if (fstat(fd, &sb) == -1)
		handle_error("fstat");
		filesz = sb.st_size;
    
    //if it's empty file, skip
    if(filesz == 0){
      continue;
    }
   
		c_offset = filesz & ~(chunksize - 1);
		c_mod = filesz & (chunksize - 1);
		c_number = (c_offset / chunksize) + 1;
   		if(c_mod == 0){
   			c_number -= 1;
   		}
   		
    	//allocate memory for out_buffer
		out_buffer = malloc(c_number * sizeof(subs *));
		for(int j = 0; j < c_number; j++){
		  out_buffer[j] = malloc(5 * chunksize);
		}
		outchunk_sz = malloc(c_number * sizeof(int));
	   
   
		// align with page size
		//pa_offset = filesz & ~(sysconf(_SC_PAGE_SIZE) - 1);

		//map file
		fileptr = mmap(NULL, filesz, PROT_READ,
				  MAP_PRIVATE, fd, 0);
				  
		if (fileptr == MAP_FAILED)
		handle_error("mmap");
		work_idx = 0;
		
		//create queue
		chunkqueue = malloc(c_number * sizeof(chunk_info*));
		for(int j = 0; j < c_number; j++){
		    chunk_info ck_info = {j, 0};
			chunkqueue[j] = ck_info;
		}
		
		pthread_t cid[CMAX];
		for(int j = 0; j < CMAX; j ++){
			pthread_create(&cid[j], NULL, consumer, NULL);
		}
		for(int j = 0; j < CMAX; j ++){
			pthread_join(cid[j], NULL);
		}

		//printf("print stdout");
		//write to stdout
		//merge outbuffer
		int stitchflag = 0;
		int stitchsum = 0;
		subs * merge_buffer = malloc(chunksize * c_number * 5);

		int merge_buf_idx = 0;
		for(int j = 0; j < c_number; j++){
		    for(int k = 0; k <= outchunk_sz[j]; k++){
		    // if k is last one of curr chunk and there is a next chunk
		    if(k == outchunk_sz[j] && j != c_number - 1){
		    	char chr1 = out_buffer[j][k].c;
		    	char chr2 = out_buffer[j+1][0].c;
		    	int count1 = out_buffer[j][k].s_count;
		    	int count2 = out_buffer[j+1][0].s_count;
		    	if(chr1 == chr2){
		    		//out_buffer[j][k].merged = 1;
		    		stitchflag++;
		    		if(stitchflag == 1){
		    			stitchsum += count1 + count2;
		    		}
		    		else if(stitchflag > 1){
		    			stitchsum += count2;
		    		}
		    		continue;
		    	}
		    }
		    
		    if(stitchflag > 0){
		    	stitchflag = 0;
		    	out_buffer[j][k].s_count = stitchsum;
		    	merge_buffer[merge_buf_idx] = out_buffer[j][k];
		    	merge_buf_idx++;
		    	//fwrite(&stitchsum, sizeof(int), 1, stdout);
		    	//printf("%c", out_buffer[j][k].c);
		    	//printf("count: %d char: %c\n",stitchsum, out_buffer[j][k].c);
		    	stitchsum = 0;
		    	continue;
		    }  
			merge_buffer[merge_buf_idx] = out_buffer[j][k];
			merge_buf_idx++;
		    //fwrite(&(out_buffer[j][k].s_count), sizeof(int), 1, stdout);
		    //printf("%c", out_buffer[j][k].c);
		    //printf("here\n");
		    //printf("count: %d char: %c\n",out_buffer[j][k].s_count, out_buffer[j][k].c);
		  }
		}
		
		//printf("here: %c\n", merge_buffer[merge_buf_idx-2].c);
		merge_buf_idx = merge_buf_idx -1;
		if(init_merge_flag == 1){
			last = merge_buffer[merge_buf_idx];
      init_merge_flag = 0;
		}
		else{
			if(last.c == merge_buffer[0].c){
				merge_buffer[0].s_count += last.s_count;
			}
			else{
				//printf("%d%c",last.s_count, last.c);
				fwrite(&last, sizeof(subs), 1, stdout);
			}
			last = merge_buffer[merge_buf_idx];
		}
		fwrite(merge_buffer, sizeof(subs), merge_buf_idx, stdout);
		if(i == argc-1){
			fwrite(merge_buffer + merge_buf_idx, sizeof(subs), 1, stdout);
		}

		//free memory
		munmap(fileptr, filesz);
		close(fd);
		for(int j = 0; j < c_number; j++){
		  free(out_buffer[j]);
		}
		free(out_buffer);
    free(merge_buffer);
	}
	exit(EXIT_SUCCESS);
    
}
