#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>



#define MAX_INSTRUCTIONS 100
#define INT_MAX 100000

// The process structure
// Type enumeration: 1 for plat, 2 for gold, 3 for silver
typedef struct {
    int id;
    int priority;
    int arrival_time;
    int remaining_time;
    int burst_time;
    int type;
    int completed;  // Flag to indicate whether the process is completed
    int instructions[MAX_INSTRUCTIONS];
} Process;

// Instruction structure from instruction.txt
typedef struct {
    int id;
    int burst_time;
} Instruction;

// Prototypes
int readInstructionsFromFile(const char *filename, Instruction instructions[]);
int readInstructionsForProcessesFromFile(const char *filename, int instructionIds[]);
int readProcessesFromFile(const char *filename, Process processes[]);
void preemptivePriorityScheduling(Process allProcesses[], Process processes[], int num_processes);
void calculateAverages(int turnaroundTimes[], int waitingTimes[], int num_processes);
Instruction* getInstructionById(int targetId, Instruction instructions[], int numInstructions);


// Function to get an instruction by ID
Instruction* getInstructionById(int targetId, Instruction instructions[], int numInstructions) {
    for (int i = 0; i < numInstructions; i++) {
        if (instructions[i].id == targetId) {
            // Return a pointer to the matching instruction
            return &instructions[i];
        }
    }

    // If no matching ID is found, return NULL
    return NULL;
}

// Function to read instructions from file and create Instruction structures
int readInstructionsFromFile(const char *filename, Instruction instructions[]) {
    FILE *file = fopen(filename, "r");

    if (file == NULL) {
        perror("Error opening file");
        return -1; // Return -1 on failure
    }

    int count = 0;
    while (count < MAX_INSTRUCTIONS && fscanf(file, "instr%d %d\n", &instructions[count].id, &instructions[count].burst_time) == 2) {
        count++;
    }

    // Check for "exit" instruction
    if (fscanf(file, "exit %d\n", &instructions[count].burst_time) == 1) {
        instructions[count].id = -1; // Use a special id (e.g., -1) to represent "exit"
        count++;
    }

    fclose(file);
    return count; // Return the number of instructions read
}

// Function to read instructions from file for each process
int readInstructionsForProcessesFromFile(const char *filename, int instructionIds[]) {
    FILE *file = fopen(filename, "r");

    if (file == NULL) {
        perror("Error opening file");
        return -1; // Return -1 on failure
    }

    int count = 0;
    while (fscanf(file, "instr%d\n", &instructionIds[count]) == 1 && count < MAX_INSTRUCTIONS) {
        count++;
    }

    // Check for "exit" instruction
    if (fscanf(file, "exit\n") == 1) {
        instructionIds[count] = -1; // Use a special id (e.g., -1) to represent "exit"
        count++;
    }

    fclose(file);
    return count;
}

// Function to read incoming processes from file and create Process structures
int readProcessesFromFile(const char *filename, Process processes[]) {
    FILE *file = fopen(filename, "r");

    if (file == NULL) {
        perror("Error opening file");
        return -1; // Return -1 on failure
    }

    int count = 0;

    // Assuming that the file format is: ID PRIORITY ARRIVAL_TIME TYPE
    while (fscanf(file, "P%d %d %d", &processes[count].id, &processes[count].priority, &processes[count].arrival_time) == 3) {
        // Extract the last part (type) separately
        char type_str[20];
        if (fscanf(file, " %s", type_str) != 1) {
            fprintf(stderr, "Error reading process type.\n");
            fclose(file);
            return -1;
        }

        // Convert type string to type enumeration
        if (strcmp(type_str, "PLATINUM") == 0) {
            processes[count].type = 1;
        } else if (strcmp(type_str, "GOLD") == 0) {
            processes[count].type = 2;
        } else if (strcmp(type_str, "SILVER") == 0) {
            processes[count].type = 3;
        } else {
            fprintf(stderr, "Error: Unknown type '%s'.\n", type_str);
            fclose(file);
            return -1;
        }

        //set the remaining time to original full time at first

        // Print or manipulate the Process structure as needed
        printf("ID: %d, Priority: %d, Arrival Time: %d, Type: %d\n",
               processes[count].id, processes[count].priority, processes[count].arrival_time, processes[count].type);

        int c;
        // Read the remaining part of the line to move to the next line
        while ((c = fgetc(file)) != '\n' && c != EOF) {}

        if (c == EOF) {
            // End of file reached
            break;
        }

        count++;
    }

    fclose(file);
    return count; // Return the number of processes read
}

// Function to calculate average turnaround time and average waiting time
void calculateAverages(int turnaroundTimes[], int waitingTimes[], int num_processes) {
    double avgTurnaround = 0.0;
    double avgWaiting = 0.0;

    for (int i = 0; i < num_processes; i++) {
        avgTurnaround += turnaroundTimes[i];
        avgWaiting += waitingTimes[i];
    }

    avgTurnaround /= num_processes;
    avgWaiting /= num_processes;

    printf("Average Turnaround Time: %.2f ms\n", avgTurnaround);
    printf("Average Waiting Time: %.2f ms\n", avgWaiting);

    // Write averages to the output file
    char fileName[20];  // Adjust the size accordingly
    sprintf(fileName, "basakout%d.txt", 3);  // Create the output file name

    FILE* outputFile = fopen(fileName, "w");  // Open file for writing

    if (outputFile != NULL) {
        if (fmod(avgWaiting, 1.0) == 0.0) {
            fprintf(outputFile, "%.0f\n", avgWaiting);
        }
        else {
            fprintf(outputFile, "%.2f\n", avgWaiting);
        }
        if (fmod(avgTurnaround, 1.0) == 0.0) {
            fprintf(outputFile, "%.0f\n", avgTurnaround);
        }
        else {
            fprintf(outputFile, "%.2f\n", avgTurnaround);
        }

        fclose(outputFile);  // Close the file
    } else {
        printf("Error opening the output file.\n");
    }
}

// Function to perform preemptive priority scheduling
/*
     * notes
     * use round robin for equal priorities
     * do a comparison of priorities at arrival times
     * line number of the interrupted process must be recorded
     * each context switch takes 10 ms to do.
     * platinums are not preempted and executed at arrival (acc to priority if they arrive at the same time)
     *  arrive at the same time, same type, same priority: according to name. P1 beforE p2
     * Silver: 80ms Gold & Plat: 120ms
     * İf silver gets 3 quantums of execution, it becomes gold
     * if gold gets 5 quantums of execution, it become platinum
     * instructions are atomic. dont cut them in t
     * Reminder:
     * Turnaround time = Time of Completion - Time Of Arrival
     * Waiting Time = Turnaround Time - Burst Time
     *
     */

void preemptivePriorityScheduling(Process allProcesses[], Process processes[], int num_processes) {

    //always a context switch at start
    int currentTime = 0;
    currentTime +=10;

    printf("REMAINING TIME IS %d",allProcesses[0].remaining_time);

    for(int i = 0;i<num_processes;i++){
        //fill in missing details in the subset list.

        //priority,type,arrival time, and id is from definition.txt

        printf("PROCESS ID IS %d\n",processes[i].id);
        printf("PROCESS PRIORITY IS %d\n",processes[i].priority);
        printf("PROCESS TYPE IS %d\n",processes[i].type);
        printf("PROCESS ARRIVAL TIME IS %d\n",processes[i].arrival_time);

        //rem time, burst time from all processes (PX.txt definitions)

        processes[i].remaining_time = allProcesses[processes[i].id-1].remaining_time;
        processes[i].burst_time = allProcesses[processes[i].id-1].burst_time;
        processes[i].completed = 0;  // Not completed

    }
    int silverQuantum = 80;
    int goldQuantum = 120;

    int silverToGoldThreshold = 3*80;
    int goldToPlatinumThreshold = 5*120;


    int turnaroundTimes[num_processes];
    int waitingTimes[num_processes];

    // Initialize arrays
    for (int i = 0; i < num_processes; i++) {
        turnaroundTimes[i] = 0;
        waitingTimes[i] = 0;
    }


    // Main scheduling loop
    //this should be while 1
    //while(1){
    for (int i = 0; i < 20; i++) {
        int foundPlatProcess = 0;
        int selectedProcess = -1;

        // Check for platinum processes arriving
        for (int i = 0; i < num_processes; i++) {
            if (processes[i].arrival_time <= currentTime && processes[i].type == 1 && processes[i].completed  == 0) {
                printf("Time %d: Executing Platinum Process P%d (ID: %d)\n", currentTime, processes[i].id, processes[i].id);
                currentTime += processes[i].burst_time;
                processes[i].remaining_time = 0;
                printf("\n!!!!TURNAROUND IS %d",currentTime - processes[i].arrival_time);
                processes[i].completed = 1;
                foundPlatProcess = 1;
                printf("\n!!!!TURNAROUND IS %d",currentTime - processes[i].arrival_time);
                printf("\n!!!!WAIT IS IS %d",turnaroundTimes[i] - processes[i].burst_time);
                turnaroundTimes[i] = currentTime - processes[i].arrival_time;
                waitingTimes[i] = turnaroundTimes[i] - processes[i].burst_time;
                break;
            }
        }

        //context switch

        if (foundPlatProcess) {
            // Wait for context switch
            currentTime += 10;
            continue;
        }


        // Check for other processes
        // Check for other processes
        int highestPriority = -1;
        int lowestPriority = INT_MAX; // Initialize with a large value for rr
        selectedProcess = -1;


        for (int i = 0; i < num_processes; i++) {
            if (processes[i].completed) {
                // Skip or delete the process as needed
                continue;
            }
            //pick the one with the highest priority
            if (processes[i].arrival_time <= currentTime && processes[i].remaining_time > 0) {
                // Update the lowest priority among active processes
                if (processes[i].priority < lowestPriority) {
                    lowestPriority = processes[i].priority;
                }
                if (processes[i].priority > highestPriority) {
                    highestPriority = processes[i].priority;
                    selectedProcess = i;
                }
            }
        }

        if (selectedProcess != -1 && !processes[selectedProcess].completed) {

            // Check for platinum processes arriving
            for (int i = 0; i < num_processes; i++) {
                if (processes[i].arrival_time <= currentTime && processes[i].type == 1 && processes[i].completed  == 0) {
                    printf("Time %d: Executing Platinum Process P%d (ID: %d)\n", currentTime, processes[i].id, processes[i].id);
                    currentTime += processes[i].burst_time;
                    processes[i].remaining_time = 0;
                    printf("\n!!!!TURNAROUND IS %d",currentTime - processes[i].arrival_time);
                    processes[i].completed = 1;
                    foundPlatProcess = 1;
                    printf("\n!!!!TURNAROUND IS %d",currentTime - processes[i].arrival_time);
                    printf("\n!!!!WAIT IS IS %d",turnaroundTimes[i] - processes[i].burst_time);
                    turnaroundTimes[i] = currentTime - processes[i].arrival_time;
                    waitingTimes[i] = turnaroundTimes[i] - processes[i].burst_time;
                    break;
                }
            }

            printf("Time %d: Executing Process P%d (ID: %d)\n", currentTime, processes[selectedProcess].id, processes[selectedProcess].id);

            if (processes[selectedProcess].type == 3) {
                // Silver process
                if (processes[selectedProcess].remaining_time <= silverQuantum) {
                    //if there wont be any platinum newcomers
                    currentTime += processes[selectedProcess].remaining_time;
                    processes[selectedProcess].remaining_time = 0;
                    processes[selectedProcess].completed = 1;
                } else {
                    // Execute one quantum for silver
                    currentTime += silverQuantum;
                    processes[selectedProcess].remaining_time -= silverQuantum;
                }
            } else if (processes[selectedProcess].type == 2) {
                // Gold process
                if (processes[selectedProcess].remaining_time <= goldQuantum) {
                    // Execute the whole burst for gold
                    currentTime += processes[selectedProcess].remaining_time;
                    processes[selectedProcess].remaining_time = 0;
                    processes[selectedProcess].completed = 1;
                } else {
                    // Execute one quantum for gold
                    currentTime += goldQuantum;
                    processes[selectedProcess].remaining_time -= goldQuantum;
                }
            }

            /*printf("\n!!!!TURNAROUND IS %d",currentTime - processes[selectedProcess].arrival_time);
            printf("\n!!!!WAIT IS IS %d",turnaroundTimes[selectedProcess] - processes[selectedProcess].burst_time);
            turnaroundTimes[selectedProcess] = currentTime - processes[selectedProcess].arrival_time;
            waitingTimes[selectedProcess] = turnaroundTimes[selectedProcess] - processes[selectedProcess].burst_time;
            */

            printf("Time %d: Successfully Executed Process P%d (ID: %d), remaining time %d\n", currentTime, processes[selectedProcess].id, processes[selectedProcess].id, processes[selectedProcess].remaining_time);

            //set the priority to the one less of the least priorty for rr
            processes[selectedProcess].priority = lowestPriority - 1;

        } else {
            // No process is ready, idle the CPU
            printf("Time %d: CPU Idle\n", currentTime);
            currentTime++;
        }


        //check thresholds for aging
        for (int i = 0; i < num_processes; i++) {
            //burst time - remaining = execution time
            if (processes[i].burst_time - processes[i].remaining_time >= silverToGoldThreshold) {
                if(processes[i].type == 3){
                    //promote silver to gold
                    processes[i].type = 2;
                }
            }

            if (processes[i].burst_time - processes[i].remaining_time >= goldToPlatinumThreshold) {
                if(processes[i].type == 2){
                    //promote gold to platinum
                    processes[i].type = 1;
                }
            }

        }

        //calculate ongoing processes
        int ongoing = num_processes;
        for (int i = 0; i < num_processes; i++) {
            if (processes[i].remaining_time <= 0) {
                ongoing--;
            }
        }

        /*
         * son process terminate etmiyor
         */

        //only 1 left in the system
        if (ongoing == 1) {
            for (int i = 0; i < num_processes; i++) {
                if (processes[i].remaining_time > 0) {
                    //execute it and terminate
                    currentTime += processes[i].remaining_time;
                    processes[i].remaining_time = 0;
                    ongoing--;
                    break;
                }
            }
        }


        // Check if all processes are completed
        int allCompleted = 1;
        for (int i = 0; i < num_processes; i++) {
            if (processes[i].remaining_time > 0) {
                allCompleted = 0;
                break;
            }
        }

        if (allCompleted) {
            break;
        }
    }

    printf("TURNAROUND IS %d",turnaroundTimes[0]);
    printf("WAIT IS IS %d",waitingTimes[0]);
    // Print average turnaround time and average waiting time
    calculateAverages(turnaroundTimes, waitingTimes, num_processes);

}

int main() {
    printf("prg start\n");

    Instruction instructionLengths[20];

    // Always fixed - Read instructions from file
    // Instruction structure: id - burst time

    printf("gonna read instr\n");
    int numInstructions = readInstructionsFromFile("instructions.txt", instructionLengths);
    printf("read all instructions\n");
    if (numInstructions == -1) {
        printf("Error reading instructions from file.\n");
        return 1;
    }

    // Print burst times
    /*for (int i = 0; i < numInstructions; i++) {
        printf("Instruction %d: Burst Time %d\n", instructionLengths[i].id, instructionLengths[i].burst_time);
    }*/


    printf("allocatingg");
    // Create 10 processes
    Process processes[10];

    printf("here");
    // Always fixed, read 10 process contents / instructions
    for (int i = 0; i < 10; i++) {
        int *instructionIds = malloc(MAX_INSTRUCTIONS * sizeof(int));
        processes[i].id = i;

        printf("allocation done\n");

        // Formulate the filename based on the index i+1 (1-based index)
        char filename[10];
        snprintf(filename, sizeof(filename), "P%d.txt", i + 1);

        // Read instructions

        printf("start reading process instruc\n");
        int numProcessInstructions = readInstructionsForProcessesFromFile(filename, instructionIds);
        if (numProcessInstructions == -1) {
            printf("Error reading process instructions from file %s.\n", filename);
            return 1;
        }
        printf("done reading process instruc\n");

        // Record instructions
        // Copy instructions individually
        for (int j = 0; j < numProcessInstructions; j++) {
            processes[i].instructions[j] = instructionIds[j];
            printf("instructions set");
        }

        printf("copying done\n");

        //take the total of the instruction bursts to determine process burst time.
        //int processInstructionCount = sizeof(instructionIds);
        int processBurstTime = 0;
        printf("burst time calculation\n");

        for(int k = 0; k<numProcessInstructions; k++){
            if(instructionIds[k] >= 1 && instructionIds[k] <= 20) {
                Instruction *currentInstruction = getInstructionById(instructionIds[k], instructionLengths, numInstructions);
                processBurstTime += currentInstruction->burst_time;
            }
        }

        //add 10 more for the exit instruction
        processBurstTime += 10;


        printf("burst time calculation done it is %d \n", processBurstTime);

        processes[i].burst_time = processBurstTime;
        processes[i].remaining_time = processBurstTime;
        printf("and we assign it to process i %d\n", processes[i].burst_time);

        free(instructionIds);

        printf("Process %d Instructions: ", i);
        for (int j = 0; j < numProcessInstructions; j++) {
            printf("%d ", processes[i].instructions[j]);
        }
        printf("\n");
    }

    Process processesToSchedule[10];
    int num_processes = readProcessesFromFile("def3.txt", processesToSchedule);


    // Perform preemptive priority scheduling
    preemptivePriorityScheduling(processes, processesToSchedule, num_processes);

    printf("prg end\n");

    return 0;
}
