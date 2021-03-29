#include <stdio.h>
#define INPUT_LENGTH 128
int main(){

    /* Saves server text to a new file.*/
    FILE *file;
    int numSimilar;
    numSimilar = 0;
    char * file_name = (char *) malloc(INPUT_LENGTH);
    while (1){
        strcpy(file_name, "chat_history.txt");
        if (numSimilar != 0){
            char * numEncoder = (char *) malloc(INPUT_LENGTH);
            sprintf(numEncoder, "%d", numSimilar);
            strcat(file_name, "_");
            strcat(file_name, numEncoder);
            free(numEncoder);
        }
        file = fopen(file_name, "r");
        if (file == NULL) break;
        else numSimilar++;
    }

    // We found a file_name that we are looking for
    fwrite("Yo what's up", 1, sizeof("Yo what's up"), file);
}