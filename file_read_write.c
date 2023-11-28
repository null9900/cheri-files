#include <sys/param.h>
#include <sys/stat.h>
#include <err.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define PHYSPAGES_THRESHOLD (32 * 1024)
#define bufferSIZE_MAX (2 * 1024 * 1024)
#define bufferSIZE_SMALL (MAXPHYS)
const char* PATH = "/root/compiled_program/text.txt";

static uintptr_t get_write_offset(uintptr_t offset, char* buffer){
    return (offset + (uintptr_t)buffer);
}

static void write_input_to_output(int fd_to_read, int fd_to_write, char* buffer, size_t buffer_size){
    uintptr_t offset =0;
    ssize_t number_of_reads=0;
    ssize_t number_of_writes=0;
    while ((number_of_reads = read(fd_to_read, buffer, buffer_size)) > 0){
        for (offset = 0; number_of_reads; number_of_reads -= number_of_writes, offset += number_of_writes){
            number_of_writes = write(fd_to_write, (const void *)get_write_offset(offset,buffer), (size_t)number_of_reads);
            if (number_of_writes < 0){
                err(1, "write(2) failed");
            }
        }
    }
}

static void get_fstat_data(int file, struct stat* bufferfer){
    if (fstat(file, bufferfer))
        err(1, "stdout");   
}

static size_t get_bufferfer_size_from_ram(){
    return sysconf(_SC_PHYS_PAGES) > PHYSPAGES_THRESHOLD? MIN(bufferSIZE_MAX, MAXPHYS * 8): bufferSIZE_SMALL;
}

static void output_file_content(long file){
    printf("%s\n", "Printing file content");
    int fd_to_read, fd_to_write;
    static size_t buffer_size;
    static char *buffer = NULL;
    struct stat sbuffer;
    fd_to_read = (int)file;
    fd_to_write = fileno(stdout);
    get_fstat_data(fd_to_write,&sbuffer);
    buffer_size = get_bufferfer_size_from_ram();
    if ((buffer = malloc(buffer_size)) == NULL)
        err(1, "malloc() failure of IO bufferfer");
    write_input_to_output(fd_to_read, fd_to_write, buffer, buffer_size);
}

static void write_to_file(long file){
    printf("%s\n","Writting \"hello\" to file");
    FILE *fp = (FILE *)file;
    char str[] = "hello\n";
    for (int i = 0; str[i] != '\0'; i++) {
        fputc(str[i], fp);
    }
    fclose(fp);
    printf("%s\n", "Executed successfully!");
}


static void do_tasks(long file, int write_flag){
    write_flag? write_to_file(file) : output_file_content(file);
}

static void process_file(char *argv[], int write_flag){
    int fd =-1;
    FILE *fp;
    fd = open(PATH, O_RDWR);
    if (write_flag) {
        fp = fdopen(fd, "r+");
        do_tasks((long)fp, write_flag);
    } else {
        do_tasks(fd, write_flag);
    }
    close(fd);
}

int main(int argc, char *argv[]){
    process_file(argv, 0);
    process_file(argv, 1);
    exit(1);
}