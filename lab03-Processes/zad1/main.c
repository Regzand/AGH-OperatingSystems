#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/wait.h>
#include <fcntl.h>

int       arg_operator;   // '<': -1, '=': 0, '>': 1
struct tm arg_date;

int compare_dates(struct tm* date1, struct tm* date2) {
    if( (date1 -> tm_year) == (date2 -> tm_year) && (date1 -> tm_yday) == (date2 -> tm_yday) )
        return 0;
    if( (date1 -> tm_year) >  (date1 -> tm_year) || ( (date1 -> tm_year) == (date2 -> tm_year) && (date1 -> tm_yday) > (date2 -> tm_yday) ) )
        return 1;
    return -1;
}

void handle_file(const char* path, const struct stat* stats){

    // get file modification date
    struct tm* date = localtime(&(stats -> st_mtime));

    // check if date satisfy program arguments
    if(compare_dates(date, &arg_date) != arg_operator)
        return;

    // display file path
    printf("%s\n", path);

    // display file size
    printf("\t%ld bytes\n", stats -> st_size);

    // display permissions
    printf("\t");
    printf( (stats -> st_mode & S_IRUSR) ? "r" : "-" );
    printf( (stats -> st_mode & S_IWUSR) ? "w" : "-" );
    printf( (stats -> st_mode & S_IXUSR) ? "x" : "-" );
    printf( (stats -> st_mode & S_IRGRP) ? "r" : "-" );
    printf( (stats -> st_mode & S_IWGRP) ? "w" : "-" );
    printf( (stats -> st_mode & S_IXGRP) ? "x" : "-" );
    printf( (stats -> st_mode & S_IROTH) ? "r" : "-" );
    printf( (stats -> st_mode & S_IWOTH) ? "w" : "-" );
    printf( (stats -> st_mode & S_IXOTH) ? "x" : "-" );
    printf("\n");

    // display last modification date
    char string_date[11];
    strftime(string_date, 11, "%d.%m.%Y", date);
    printf("\t%s\n", string_date);
}


void custom_traverse(char* path, int* path_size) {

    // adds '/' at the end of path
    strcat(path, "/");

    // gets path length
    int path_length = strlen(path);

    // adds NULL at the end of path
    path[path_length] = '\0';

    // ensures that there is enough space in buffer, if not resizes it twice
    if(2 * path_length > (*path_size)) {
        path = realloc(path, 2 * (*path_size) * sizeof(char));
        *path_size *= 2;
    }

    // opens directory
    DIR* directory = opendir(path);
    if (directory == NULL) {
        fprintf(stderr, "En error occurred while opening directory");
        exit(10);
    }

    // reads all entries in directory
    struct dirent* entry;
    while((entry = readdir(directory)) != NULL) {

        // skips '..' and '.'
        if(strcmp(entry -> d_name, "..") == 0 || strcmp(entry -> d_name, ".") == 0)
            continue;

        // prepares path to the entry
        strcat(path, entry -> d_name);

        // gets entry stats
        struct stat stats;
        if(lstat(path, &stats) < 0) {
            fprintf(stderr, "En error occurred while accessing entry stats");
            exit(10);
        }

        // handle directories (recursive) - using fork
        if(S_ISDIR(stats.st_mode)){
            //custom_traverse(path, path_size);

            // create new process
            pid_t process_id = fork();
            if(process_id < 0){
                fprintf(stderr, "En error occurred while creating process with fork()");
                exit(100);
            }

            // traverse in child process
            if(process_id == 0){
                custom_traverse(path, path_size);
                return;
            }

            // wait for child process to end
            int process_status;
            if(wait(&process_status) < 0){
                fprintf(stderr, "En error occurred while waiting for child precess to end");
                exit(101);
            }

            // check child process return status
            if(!WIFEXITED(process_status)){
                fprintf(stderr, "Child process ended with unexpected status %d", process_status);
                exit(102);
            }

        // handle normal files
        }else if(S_ISREG(stats.st_mode)){
            handle_file(path, &stats);

        }
        // we don't care about other entry types...

        // trim path back to original by adding NULL
        path[path_length] = '\0';
    }

    // closes directory
    if (closedir(directory) != 0) {
        fprintf(stderr, "En error occurred while closing directory");
        exit(12);
    }
}

int main(int argc, char** args){

    // check arguments count
    if(argc < 4){
        fprintf(stderr, "Missing arguments!\nUsage: <path> <operator> <date>\n");
        exit(1);
    }

    // stores path info, used for custom traversing
    int* path_size = malloc(sizeof(int));
    *path_size = 1000;
    char* path = malloc(*path_size * sizeof(char));

    // prepare path
    if(args[1][0] == '/'){
        strcpy(path, args[1]);
    }else{
        getcwd(path, 1000);
        strcat(path, "/");
        strcat(path, args[1]);
    }

    // prepare operator
    if(strcmp(args[2], "<") == 0) {
        arg_operator = -1;
    }else if (strcmp(args[2], "=") == 0){
        arg_operator = 0;
    }else if(strcmp(args[2], ">") == 0){
        arg_operator = 1;
    }else{
        fprintf(stderr, "Wrong argument!\nOperator can only be '<', '=' or '>'\n");
        exit(2);
    }

    // prepare date
    strptime(args[3], "%d.%m.%Y", &arg_date);

    // traverse
    custom_traverse(path, path_size);

    return 0;
}
