#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>

#define INFECTED_DURATION 5
#define IMMUNE_DURATION 1


int MAX_X_COORDINATE;
int MAX_Y_COORDINATE;
int POPULATION_SIZE;


enum DiseaseState {
    Infected,     // 0
    Susceptible,  // 1
    Immune        // 2
};
enum MovementDirection{
    North, // 0
    South, // 1
    East,  // 2
    West   // 3
};

typedef struct Person{
    int PersonID;
    int coordX;
    int coordY;
    enum DiseaseState state;
    enum MovementDirection direction;
    int patternAmplitude;
    int time;
    int infectionCount;
}Person;


pthread_barrier_t barrier;
pthread_mutex_t infectionMutex;

typedef struct {
    int startIndex;
    int endIndex;
    int simulationTime;
    Person *population;
    int threadNumber;
} ThreadData;

const char* getStateName(enum DiseaseState state) {
    switch (state) {
        case Infected: return "Infected";
        case Susceptible: return "Susceptible";
        case Immune: return "Immune";
        default: return "Unknown";
    }
}

const char* getDirectionName(enum MovementDirection direction) {
    switch (direction) {
        case North: return "North";
        case South: return "South";
        case East: return "East";
        case West: return "West";
        default: return "Unknown";
    }
}

Person readPerson(FILE *fp){
    Person p;
    int state;
    int direction;
    p.infectionCount = 0;
    fscanf(fp, "%d %d %d %d %d %d", &p.PersonID, &p.coordX, &p.coordY, &state, &direction, &p.patternAmplitude);
    if (state == 0){
        p.time = INFECTED_DURATION;
        p.infectionCount = 1;
    }
    else if (state == 2){
        p.time = IMMUNE_DURATION;
    }
    else{
        p.time = 0;
    }
    switch (state)
    {
    case 0:
        p.state = Infected;
        break;
    case 1:
        p.state = Susceptible;
        break;
    case 2:
        p.state = Immune;
        break;
    default:printf("Invalid state\n");
        break;
    }
    switch (direction)
    {
    case 0:
        p.direction = North;
        break;
    case 1:
        p.direction = South;
        break;
    case 2:
        p.direction = East;
        break;
    case 3:
        p.direction = West;
        break;
    default:printf("Invalid direction\n");
        break;
    }
    return p;
}

void readFile (char* filename, Person** population){
    FILE * fp = fopen(filename, "r");
    if (fp == NULL) {
        printf("Error: File '%s' not found\n", filename);
        perror("Error: ");
    }
    printf("File '%s' opened successfully!\n", filename);
    fscanf(fp, "%d %d", &MAX_X_COORDINATE, &MAX_Y_COORDINATE);
    fscanf(fp, "%d", &POPULATION_SIZE);
    *population = (Person*)malloc(POPULATION_SIZE * sizeof(Person));
    printf ("population size:%d\n", POPULATION_SIZE);
    for (int i = 0; i < POPULATION_SIZE; i++){
        Person p = readPerson(fp);
        (*population)[i] = p;
        //printf("PersonID: %d, coordX: %d, coordY: %d, state: %s, direction: %s, patternAmplitude: %d, time:%d\n", p.PersonID, p.coordX, p.coordY, getStateName(p.state), getDirectionName(p.direction), p.patternAmplitude, p.time);
    }
    fclose(fp);
}

void printPersons(Person* population){
    for (int i = 0; i < POPULATION_SIZE; i++){
        printf("PersonID: %d, coordX: %d, coordY: %d, state: %s, direction: %s, patternAmplitude: %d, time:%d, infection couter:%d\n", population[i].PersonID, population[i].coordX, population[i].coordY, getStateName(population[i].state), getDirectionName(population[i].direction), population[i].patternAmplitude, population[i].time, population[i].infectionCount);
    }
    printf("\n");
}

void duplicatePopulation (Person *population, Person *newPopulation){
    for (int i = 0; i < POPULATION_SIZE; i++){
        newPopulation[i] = population[i];
    }
}

void updateLocation(Person *p) {
    switch (p->direction) {
        case North:
            if (p->coordY - p->patternAmplitude < 0) {  
                p->direction = South;  
                p->coordY = - (p->coordY - p->patternAmplitude); 
            } else {
                p->coordY -= p->patternAmplitude;
            }
            break;

        case South:
            if (p->coordY + p->patternAmplitude >= MAX_Y_COORDINATE) { 
                p->direction = North;  
                p->coordY = MAX_Y_COORDINATE - 1 - (p->coordY + p->patternAmplitude - (MAX_Y_COORDINATE - 1)); 
            } else {
                p->coordY += p->patternAmplitude;
            }
            break;

        case East:
            if (p->coordX + p->patternAmplitude >= MAX_X_COORDINATE) { 
                p->direction = West;  
                p->coordX = MAX_X_COORDINATE - 1 - (p->coordX + p->patternAmplitude - (MAX_X_COORDINATE - 1)); 
            } else {
                p->coordX += p->patternAmplitude;
            }
            break;

        case West:
            if (p->coordX - p->patternAmplitude < 0) { 
                p->direction = East;  
                p->coordX = - (p->coordX - p->patternAmplitude); 
            } else {
                p->coordX -= p->patternAmplitude;
            }
            break;

        default:
            printf("Invalid direction\n");
            break;
    }
}

void updateStatus (Person *p){
        if (p->state == Infected){
            if (p->time > 0)
                p->time--;
            if (p->time == 0){
                p->state = Immune;
                p->time = IMMUNE_DURATION;
            }
        }
        else if (p->state == Immune){
            if (p->time > 0)
                p->time--;
            if (p->time == 0){
                p->state = Susceptible;
            }
        }
}

void executeSerialSimulation(int TOTAL_SIMULATION_TIME, Person* population){
    while (TOTAL_SIMULATION_TIME > 0){
        for (int i = 0; i < POPULATION_SIZE; i++){
            updateLocation(&population[i]);
            updateStatus(&population[i]);
        }
        for (int i = 0; i < POPULATION_SIZE; i++){
            if (population[i].state == Infected){
                for (int j = 0; j < POPULATION_SIZE; j++){
                    if (population[j].coordX == population[i].coordX && population[j].coordY == population[i].coordY && population[j].state == Susceptible){
                        population[j].state = Infected;
                        population[j].infectionCount++;
                        population[j].time = INFECTED_DURATION;
                    }
                }
            }
        }
        TOTAL_SIMULATION_TIME--;
    }
}

void executeSerialSimulationDEBUG(int TOTAL_SIMULATION_TIME, Person* population){
    while (TOTAL_SIMULATION_TIME > 0){
        for (int i = 0; i < POPULATION_SIZE; i++){
            updateLocation(&population[i]);
            updateStatus(&population[i]);
        }
        for (int i = 0; i < POPULATION_SIZE; i++){
            if (population[i].state == Infected){
                for (int j = 0; j < POPULATION_SIZE; j++){
                    if (population[j].coordX == population[i].coordX && population[j].coordY == population[i].coordY && population[j].state == Susceptible){
                        population[j].state = Infected;
                        population[j].infectionCount++;
                        population[j].time = INFECTED_DURATION;
                    }
                }
            }
        }
        TOTAL_SIMULATION_TIME--;
        printPersons(population);
    }
}

void* parallelSimulationThread(void* args) {
    ThreadData* data = (ThreadData*) args; 
    int start = data->startIndex; 
    int end = data->endIndex; 
    int TOTAL_SIMULATION_TIME = data->simulationTime;
    Person* population = data->population;
    //printPersons(population);
    //printf("Thread number:%d, start:%d, end:%d\n", data->threadNumber, start, end);
    while (TOTAL_SIMULATION_TIME > 0) {
        for (int i = start; i < end; i++) {
            updateLocation(&population[i]);
            updateStatus(&population[i]);
        }

        pthread_barrier_wait(&barrier);

        for (int i = start; i < end; i++) {
            if (population[i].state == Infected) {
                for (int j = 0; j < POPULATION_SIZE; j++) {
                    if (population[i].coordX == population[j].coordX && population[i].coordY == population[j].coordY && population[j].state == Susceptible) { 
                        pthread_mutex_lock(&infectionMutex);
                        population[j].state = Infected;
                        population[j].infectionCount++;
                        population[j].time = INFECTED_DURATION;
                        pthread_mutex_unlock(&infectionMutex);
                    }
                }
            }
        }

        pthread_barrier_wait(&barrier);
        TOTAL_SIMULATION_TIME--;
    }

    return NULL;
}

void executeParallelSimulation(int TOTAL_SIMULATION_TIME, Person* population, int threadCount) {
    
    pthread_barrier_init(&barrier, NULL, threadCount);
    pthread_mutex_init(&infectionMutex, NULL);

    pthread_t threads[threadCount];
    ThreadData threadArgs[threadCount];

    int chunkSize = POPULATION_SIZE / threadCount;

    for (int i = 0; i < threadCount; i++) {
        threadArgs[i].population = population;
        threadArgs[i].startIndex = i * chunkSize;
        threadArgs[i].endIndex = (i == threadCount - 1) ? POPULATION_SIZE : (i + 1) * chunkSize;
        threadArgs[i].simulationTime = TOTAL_SIMULATION_TIME;
        threadArgs[i].threadNumber = i;
        pthread_create(&threads[i], NULL, parallelSimulationThread, &threadArgs[i]);
    }

    for (int i = 0; i < threadCount; i++) {
        pthread_join(threads[i], NULL);
    }

    pthread_barrier_destroy(&barrier);
    pthread_mutex_destroy(&infectionMutex);
}

void* parallelSimulationThreadDEBUG(void* args) {
    ThreadData* data = (ThreadData*) args; 
    int start = data->startIndex; 
    int end = data->endIndex; 
    int TOTAL_SIMULATION_TIME = data->simulationTime;
    Person* population = data->population;
    //printPersons(population);
    //printf("Thread number:%d, start:%d, end:%d\n", data->threadNumber, start, end);
    while (TOTAL_SIMULATION_TIME > 0) {
        for (int i = start; i < end; i++) {
            updateLocation(&population[i]);
            updateStatus(&population[i]);
        }

        pthread_barrier_wait(&barrier);

        for (int i = start; i < end; i++) {
            if (population[i].state == Infected) {
                for (int j = 0; j < POPULATION_SIZE; j++) {
                    if (population[i].coordX == population[j].coordX && population[i].coordY == population[j].coordY && population[j].state == Susceptible) {
                        pthread_mutex_lock(&infectionMutex);
                        population[j].state = Infected;
                        population[j].infectionCount++;
                        population[j].time = INFECTED_DURATION;
                        pthread_mutex_unlock(&infectionMutex);
                    }
                }
            }
        }

        pthread_barrier_wait(&barrier);
        TOTAL_SIMULATION_TIME--;
        if (data->threadNumber == 0)
            printPersons(population);
    }

    return NULL;
}

void executeParallelSimulationDEBUG(int TOTAL_SIMULATION_TIME, Person* population, int threadCount) {
    
    pthread_barrier_init(&barrier, NULL, threadCount);
    pthread_mutex_init(&infectionMutex, NULL);

    pthread_t threads[threadCount];
    ThreadData threadArgs[threadCount];

    int chunkSize = POPULATION_SIZE / threadCount;

    for (int i = 0; i < threadCount; i++) {
        threadArgs[i].population = population;
        threadArgs[i].startIndex = i * chunkSize;
        threadArgs[i].endIndex = (i == threadCount - 1) ? POPULATION_SIZE : (i + 1) * chunkSize;
        threadArgs[i].simulationTime = TOTAL_SIMULATION_TIME;
        threadArgs[i].threadNumber = i;
        pthread_create(&threads[i], NULL, parallelSimulationThreadDEBUG, &threadArgs[i]);
    }

    for (int i = 0; i < threadCount; i++) {
        pthread_join(threads[i], NULL);
    }

    pthread_barrier_destroy(&barrier);
    pthread_mutex_destroy(&infectionMutex);
}

void printSerialOutputInFile (Person *population, char *filename){
    char outputFileName[100];
    strcpy(outputFileName, filename);
    strcat(outputFileName, "_serial_out.txt");
    FILE *fp = fopen(outputFileName, "w");
    if (fp == NULL) {
        printf("Error: File '%s' not found\n", outputFileName);
        perror("Error: ");
    }
    //for i in popoulation print:position, state, infection count
    for (int i = 0; i < POPULATION_SIZE; i++){
        fprintf(fp, "Coord:%d %d, counter:%d, status:%s\n", population[i].coordX, population[i].coordY, population[i].infectionCount, getStateName(population[i].state));
    }
    fclose(fp);
}

void printParallelOutputInFile (Person *population, char *filename){
    char outputFileName[100];
    strcpy(outputFileName, filename);
    strcat(outputFileName, "_parallel_out.txt");
    FILE *fp = fopen(outputFileName, "w");
    if (fp == NULL) {
        printf("Error: File '%s' not found\n", outputFileName);
        perror("Error: ");
    }
    //for i in popoulation print:position, state, infection count
    for (int i = 0; i < POPULATION_SIZE; i++){
        fprintf(fp, "Coord:%d %d, counter:%d, status:%s\n", population[i].coordX, population[i].coordY, population[i].infectionCount, getStateName(population[i].state));
    }
    fclose(fp);
}

void comparePopulation (Person *population, Person *newPopulation){
    for (int i = 0; i < POPULATION_SIZE; i++){
        if (population[i].coordX != newPopulation[i].coordX || population[i].coordY != newPopulation[i].coordY || population[i].state != newPopulation[i].state || population[i].direction != newPopulation[i].direction || population[i].patternAmplitude != newPopulation[i].patternAmplitude || population[i].time != newPopulation[i].time || population[i].infectionCount != newPopulation[i].infectionCount){
            printf("Error: Parallel and Serial outputs are not the same\n");
            return;
        }
    }
    printf("Parallel and Serial outputs are the same\n");
}

void totalInfectionCount (Person *population){
    int count = 0;
    for (int i = 0; i < POPULATION_SIZE; i++)
        count+=population[i].infectionCount;
    printf("Total infection count:%d\n", count);
    }
//personId, coordX, coordY, state, direction, patternAmplitude
//verfiy is serial and parallel outputs are the same
//2 modes of running:active(for time measurements) and debug (that prints all the info after each time jump)
//measure serial and parallel time and compute speedup
//test for:
//population size: 10K, 20k, 50k, 100k, 500k
//simulation size: 50, 100, 150, 200, 500

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Usage: %s TOTAL_SIMULATION_TIME InputFileName ThreadNumber\n", argv[0]);
        return 1;
    }

    int TOTAL_SIMULATION_TIME = atoi(argv[1]);  
    char *InputFileName = argv[2];               
    int ThreadNumber = atoi(argv[3]);            

    printf("Total Simulation Time: %d\n", TOTAL_SIMULATION_TIME);
    printf("Input File: %s\n", InputFileName);
    printf("Thread Number: %d\n", ThreadNumber);

    Person *population;

    readFile(InputFileName, &population);
    Person *newPopulation = (Person*)malloc(POPULATION_SIZE * sizeof(Person));
    duplicatePopulation(population, newPopulation);
    printf ("Max X: %d, Max Y: %d, Population Size: %d\n\n", MAX_X_COORDINATE, MAX_Y_COORDINATE, POPULATION_SIZE);

    int mode;
    printf("Enter 1 for normal mode and 2 for debug mode\n");
    scanf("%d", &mode);
    if (mode == 1){
        //serial execution and time measurement
        clock_t serial_start = clock();               
        executeSerialSimulation(TOTAL_SIMULATION_TIME, population);
        clock_t serial_end = clock();                 
        double serial_time = (double)(serial_end - serial_start) / CLOCKS_PER_SEC;  
        printf("Serial Execution Time: %f seconds\n", serial_time);
        printSerialOutputInFile(population, InputFileName);
        //parallel execution and time measurement
        clock_t parallel_start = clock();             
        executeParallelSimulation(TOTAL_SIMULATION_TIME, newPopulation, ThreadNumber);
        clock_t parallel_end = clock();               
        double parallel_time = (double)(parallel_end - parallel_start) / CLOCKS_PER_SEC;  
        printf("Parallel Execution Time: %f seconds\n", parallel_time);
        printf("Speedup: %f\n", serial_time / parallel_time);
        printParallelOutputInFile(newPopulation, InputFileName);

        comparePopulation(population, newPopulation);
        totalInfectionCount(population);
    }
    else if (mode == 2){
        printf("Debug mode:\nExecuting serial simulation...\n");
        printPersons(population);
        executeSerialSimulationDEBUG(TOTAL_SIMULATION_TIME, population);

        printf("Executing parallel simulation...\n");
        printPersons(newPopulation);
        executeParallelSimulationDEBUG(TOTAL_SIMULATION_TIME, newPopulation, ThreadNumber);
        comparePopulation(population, newPopulation);
    }
    else{
        printf("Invalid mode\n");
    }

    free(population);
    free(newPopulation);
    return 0;
}
