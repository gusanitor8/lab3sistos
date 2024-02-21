#include <sys/syscall.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <pthread.h>

#define ROWS 9
#define COLUMNS 9

int sudoku[ROWS][COLUMNS]; //sudoku 

typedef struct {
    void* mapped_data;
    off_t file_size;
} FileData;


/**
 * @brief Reads the file and returns a FileData struct
 * 
 * @param file_path the path of the file to read
 * @return FileData struct containing the mapped data and the file size
 */
FileData read_file(char* file_path){
    int file_descriptor = open(file_path, O_RDONLY);
    off_t file_size = lseek(file_descriptor, 0, SEEK_END);

    void *mapped_data = mmap(
        NULL, // Let the kernel choose the address
        file_size,  // The size of the file
        PROT_READ, // indicates that the mapped memory should be readable but not writable.
        MAP_PRIVATE, //  Changes made to the mapped data won't affect the original file, and each process that maps the file gets its own private copy
        file_descriptor, // The file descriptor of the file to map
        0 // offset
    );

    FileData file_data = {mapped_data, file_size};
    return file_data;
}


/**
 * @brief Copies the sudoku from the mapped data into the sudoku array
 * 
 * @param mapped_data mapped data of the file
 * @param file_size size of the file in bytes
 * @return int 
 */
int copy_sudoku(void* mapped_data, off_t file_size){
    // Copy the file into the sudoku array
    for (off_t i = 0; i < file_size; i++){
        char number_str = ((char*)mapped_data)[i];
        int number = number_str - '0';

        sudoku[i/9][i%9] = number;
    }

    return 0;
}


/**
 * @brief prints teh sudoku array
 * 
 * @return int 
 */
int print_sudoku(){
    for(int i = 0; i < ROWS; i++) {
        for(int j = 0; j < COLUMNS; j++) {
            printf("%d ", sudoku[i][j]);
        }        
        printf("\n"); 
    }
    printf("\n"); 

    return 0;
}

bool check_three_by_three(int index){
    int validation[] = {0, 0, 0, 0, 0, 0, 0, 0, 0}; 

    for (int row = 0; row < 3; row++){
        for (int col = 0; col < 3; col++){
            int number = sudoku[row + (index / 3) * 3][col + (index % 3) * 3];
            
            if (validation[number - 1] == 0){
                validation[number - 1] = 1;
            }else{
                return false;
            }            
        }
    }

    return true;
}


/**
 * @brief Validates the sudoku in either a row or a column
 * 
 * @param index the index of the row or column
 * @param axis the orientation of the validation (0 for row, 1 for column)
 * @return true the sudoku is valid
 * @return false the sudoku is invalid
 */
bool check_line(int index, int axis){
    int validation[] = {0, 0, 0, 0, 0, 0, 0, 0, 0};     

    if (axis == 0){
        for (int columns = 0; columns < COLUMNS; columns++){
            int number = sudoku[index][columns];
            
            if (validation[number - 1] == 0){
                validation[number - 1] = 1;
            }else{
                return false;
            }
        }
    }else{
        for (int rows = 0; rows < ROWS; rows++){
            int number = sudoku[rows][index];
            
            if (validation[number - 1] == 0){
                validation[number - 1] = 1;
            }else{
                return false;
            }
        }
    }

    return true;
}

/**
 * @brief main function
 * 
 * @param argc 
 * @param argv file path for the sudoku file
 * @return int 
 */
int main(int argc, char *argv[]){

    FileData file_data = read_file(argv[1]);

    void* mapped_data = file_data.mapped_data;
    off_t file_size = file_data.file_size;
    
    copy_sudoku(mapped_data, file_size);    
    print_sudoku();

    pid_t pid = getpid();
    // printf("PID: %d\n", pid);
    
    pid_t child = fork();

    if (child == 0){
        // child process        
        char pid_str[20];
        sprintf(pid_str, "%d", pid);

        execlp("ps", "ps", "-p", pid_str, "-lLf", NULL);
    }else{
        // parent process

        wait(NULL);
    }

    return 0;
}