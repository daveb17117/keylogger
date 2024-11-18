#include <stdio.h>
// Library for implementing the sleep() function
#include <windows.h>

// Library for implementing the 
#include <time.h>

// Define the range of the waiting time in miliseconds
#define MIN_TIME 100
#define MAX_TIME 600



void waitRandTime(){

    // Random function with the range defined by MIN_TIME and MAX_TIME 
    // This is not really generating a random number since it could be calculated but for our use case it's enough to not have a constant wait time 
    srand(time(NULL));
    double result = ((rand() % (MIN_TIME - MAX_TIME)) + MIN_TIME)*100;
    // Sleep for the generated amount of time
    Sleep(result);
}


int checkIfFileExists(const char *fileName)
{
    // Initialize file
    FILE *exfilFile;
    if ((exfilFile = fopen(fileName, "r")))
    {
        fclose(exfilFile);
        return 1;
    }
    return 0;
}

void main(){

    if (checkIfFileExists("FILENAME.txt") == 1){
        printf("File exists");
    } else {
        printf("File doesn't exist");
    };


}