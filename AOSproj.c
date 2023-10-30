#include<stdio.h>
#include<fcntl.h>
#include<stdlib.h>
#include<sys/stat.h>
#include<errno.h>
#include<string.h>
#include<time.h>
#include<unistd.h>
#define CHECK(X) ({int __val = (X); (__val ==-1 ? \
		({char __buf[256]; \
		snprintf(__buf, 256, "ERROR (" __FILE__ ":%d) --%s\n",__LINE__,strerror(errno)); \
		write(STDERR_FILENO, __buf, strlen(__buf)); \
		exit(-1);}) : __val); })


char* formatdate(char* str, time_t val)
{
        strftime(str, 36, "%d.%m.%Y %H:%M:%S", localtime(&val));
        return str;
}


int readfile(const char*,char *,int,int);
int writefile(const char*,char*,int,int);
void help();
void file_stat(const char*);
int copyfile(const char* src,const char* dst);
int createfile(const char*,mode_t);


int main(int argc,char *argv[]){
    /* Format to call this program
    program_name file_name r/cr/w/s/c 
    Ex1 ./AOSproj cr a.txt  permissions
    Ex2 ./AOSproj r  a.txt  0(optional) 10(optional)
    Ex3 ./AOSproj w  a.txt  0 "data_to_write"
    Ex4 ./AOSproj s  a.txt 
    Ex5 ./AOSproj c  a.txt  b.txt
    Ex6 ./AOSproj h*/
    
    if(!strcmp(argv[1],"h")){ 
        help(); 
        return 0;
        }
    if(argc < 3){
        printf("please provide correct arguments!");
        exit(1);
    }
    if(!strcmp(argv[1],"r")){
        if(argc < 3){
            printf("please provide correct  arguments!\nWanna see help? (y/n)");
            char c = getchar();
            if(c == 'y' || c == 'Y') help();
            exit(1);
        }
        else if(argc == 3) {
            readfile(argv[2],NULL,0,-1);
            return 0;}
        else if(argc == 4) {
            int offset;
            int result = sscanf(argv[3], "%d", &offset); 
        // convert the string to an integer using sscanf()
        if (result == EOF)  offset = 0;
        readfile(argv[2],NULL,offset,-1);
        return 0;
        }
        unsigned long long offset,size ;
        int result = sscanf(argv[3], "%llu", &offset); 
        int result1 = sscanf(argv[4], "%llu", &size); 
        // convert the string to an integer using sscanf()
        if (result == EOF)  offset = 0;
        if (result1 == EOF)  size = -1;
        int read_bytes = readfile(argv[2],NULL,offset,size);
        return 0;
    }
    else if(!strcmp(argv[1],"w")) {
        if(argc < 5){
            printf("please provide correct  arguments!\nWanna see help? (y/n)");
            char c = getchar();
            if(c == 'y' || c == 'Y') help();
            exit(1);
        }
        unsigned long long int offset ;
        int result = sscanf(argv[3], "%llu", &offset); 
        if(result == EOF) offset = 0;
        unsigned long long int size = strlen(argv[4]);
        int bytes_write = writefile(argv[2],argv[4],offset,size);
        printf("%d bytes were written to the file\n",bytes_write);
    }
    else if(!strcmp(argv[1],"c")){
        if(argc != 4){
            printf("please provide correct number of arguments!");
            exit(1);
        }
        copyfile(argv[2],argv[3]);
    }
    else if(!strcmp(argv[1],"s"))   file_stat(argv[2]);
    else if(!strcmp(argv[1],"cr")){
        mode_t permissions = strtol(argv[3], NULL, 8);
        createfile(argv[2],permissions);
    }
    else{
        printf("Invalid arguments!\nWanna see help? (y/n)");
        char c = getchar();
        if(c == 'y' || c == 'Y') help();
    }
    return 0;
}

void help(){
    printf("Command Name : AOSproj.out");
    printf("Usage: AOSProj flag fileName ...\n");
    printf("To read from a file       : AOSproj r <filename> <offset(optional)> <size(optional)>\n");
    printf("To Write to a file        : AOSproj w <filename> <data_to_write>\n");
    printf("To make a copy of a file  : AOSproj c <sourceFile> <destinationFile>\n");
    printf("To create a file          : AOSproj cr <filename> <permissions(in octal form)>\n");
    printf("To display the file stats : AOSproj s <filename>\n");
    printf("To display help           : AOSproj h \n");
    printf("In case of read operation,offset is by default 0.If the size is either not provied ");
    printf("or is invalid, then whole file content will be displayed.");
}

int readfile(const char* f_name,char *buf,int offset,int size){
    int fd = open(f_name,O_RDONLY);
    CHECK(fd);
    if(buf == NULL){ 
        char c;
        int bytes_read;
        if(offset > 0)  lseek(fd,offset,SEEK_SET);
        if(size <= -1){
            while(read(fd,&c,1) != 0){ 
                bytes_read++;
                printf("%c",c);}}
        else{ 
            while ((read(fd,&c,1) != 0) && bytes_read < size)
            {
                bytes_read++;
                printf("%c",c);
            }
            if(bytes_read < size)
                printf("\nOnly %d bytes were available.",bytes_read);
        }
        close(fd);
        return bytes_read;
    }
    int bytes_read = read(fd,buf,size);
    close(fd);
    return bytes_read;

}

int writefile(const char *f_name,char* buf,int offset,int size){
    int fd = open(f_name,O_CREAT|O_WRONLY|O_APPEND,0777);
    CHECK(fd);
    if(offset > 0)  lseek(fd,offset,SEEK_SET);
    int bytes_write = write(fd,buf,size);
    close(fd);
    return bytes_write;
}

void file_stat(const char* f_name){
    struct stat* f_stat = malloc(sizeof(struct stat));
    int fd = stat(f_name,f_stat);
    CHECK(fd);
    close(fd);
    // Displaying the file statistics
    char date[36];
    printf("\nFile name: %s\n", f_name);
    printf("Size: %ld bytes\n", f_stat -> st_size);
    printf("Permissions: %o\n", f_stat -> st_mode & 0777);
    printf("User ID of owner: %d\n", f_stat -> st_uid);
    printf("Group ID of owner: %d\n", f_stat -> st_gid);
    printf("Last accessed: %s\n", formatdate(date,f_stat -> st_atime));
    printf("Last modified: %s\n", formatdate(date,f_stat -> st_mtime));
    printf("Last status change: %s\n", formatdate(date,f_stat -> st_ctime));
}

int copyfile(const char* src,const char* dst){
    int fd2 = open(dst,O_CREAT|O_EXCL|O_WRONLY);
    if(fd2 == -1 && errno == EEXIST){ 
        printf("The  %s file already exists!\n",dst);
        printf("DO you wanna append to the file?(y for appending /n for truncating/any other character to exit)");
        char ans = getchar();
        if(ans == 'y' || ans == 'Y')
            fd2 = open(dst,O_APPEND|O_WRONLY);
        else if(ans == 'n' || ans == 'N')
            fd2 = open(dst,O_WRONLY|O_TRUNC);
        else exit(0);
        CHECK(fd2);
        close(fd2);
    }
    else CHECK(fd2);
    char c;
    int offset = 0;
    while(readfile(src,&c,offset,1) != 0){offset++;
        writefile(dst,&c,offset,1);}
    printf("copied the content of the file to %s successfully.",dst);
}

int createfile(const char* f_name,mode_t mode){
    if(access(f_name,F_OK) == 0) {
        printf("file already exist!\n");
        printf("wanna truncate the file? (y/n)   ");
        char ans = getchar();
        if(ans == 'y' || ans == 'Y') {
            int fd = creat(f_name,mode);
            CHECK(fd);
            printf("%s created successfully\n",f_name);
            close(fd);
        }
    }
    else{
        int fd = creat(f_name,mode); 
        CHECK(fd);
        close(fd);
    }
    
    return 0;
}



