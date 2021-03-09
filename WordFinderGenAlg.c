#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<malloc.h>
#include<math.h>

#define MAXCHAR 84
#define POPNUM 20
#define MUTATIONRATE 0.1                                //mutation rate per allele

//1: roulette wheel, 2: rank selection, 3: Boltzmann selection
#define SELECTION 3

typedef struct {
    char *word;
    float fitness;
} ind_t;

typedef struct {
    ind_t *actualGen;
    ind_t *newGen;
    float best;
    int gen;
} generation_t;

char availableChar(char random) {                       //from 0 to 84

    if(random < 32) {                                   //symbols, numbers
        random = random + 32;
    } else if(random > 31 && random < 58) {             //LETTERS
        random = random + 65 - 32;
    } else if(random > 57 && random < 85) {             //''', letters
        random = random + 96 - 58;
    }
    return random;
}

generation_t *allocatePopulations(int wordLength) {
    generation_t *gen = malloc(sizeof(generation_t));
    gen->actualGen = malloc((POPNUM + 1) * sizeof(ind_t));
    gen->newGen = malloc((POPNUM + 1) * sizeof(ind_t));

    for(int i = 0; i < POPNUM; i++) {
        gen->actualGen[i].word = malloc((wordLength + 1) * sizeof(char));
        gen->newGen[i].word = malloc((wordLength + 1) * sizeof(char));
    }
    return gen;
}

void createPopulation(ind_t *population, int wordLength) {
    for(int i = 0; i < POPNUM; i++) {
        for(int j = 0; j < wordLength; j++) {
            population[i].word[j] = availableChar(rand() % (MAXCHAR + 1));
        }
        population[i].word[wordLength] = '\0';
    }
}

void calculateFitness(ind_t *ind, char *toGuess) {
    int wordlength = strlen(toGuess);
    float fitness = 0;

    ind->fitness = 0;
    for(int i = 0; i < wordlength; i++) {
        if(ind->word[i] == toGuess[i]) {
            fitness++;
        }
    }
    ind->fitness = fitness;
}

void mutate(ind_t *ind) {
    int wordLen = strlen(ind->word), pos = rand() % wordLen;
    
    ind->word[pos] = availableChar(rand() % (MAXCHAR + 1));
}

void crossover(ind_t *son, ind_t *father, ind_t *mother) {
    int wordLength = strlen(father->word), crossoverPoint = rand() % (wordLength + 1), index;
    float random;

    for(index = 0; index < crossoverPoint; index++) {
        son->word[index] = father->word[index];
    }
    for(; index < wordLength + 1; index++) {
        son->word[index] = mother->word[index];
    }
}

int cmpFunction(const void *a, const void *b) {
    return ((ind_t*) a) -> fitness - ((ind_t*) b) -> fitness;
}

ind_t *select(generation_t *gen) {
    int index1, index2;
    float fitnessSum = 0, partialSum, random;

    for(int i = 0; i < POPNUM; i++) {
        fitnessSum += gen -> actualGen[i].fitness;
    }

    if(fitnessSum != 0) {
        for(int i = 1; i < POPNUM; i++) {
            random = (float) rand() / (float) (RAND_MAX) * fitnessSum;
            index1 = 0;
            partialSum = gen -> actualGen[0].fitness;
            while(partialSum < random) {
                index1++;
                partialSum += gen -> actualGen[index1].fitness;
            }
            random = (float) rand() / (float) (RAND_MAX) * fitnessSum;
            index2 = 0;
            partialSum = gen -> actualGen[0].fitness;
            while(partialSum < random) {
                index2++;
                partialSum += gen -> actualGen[index2].fitness;
            }
            crossover(&gen->newGen[i], &gen->actualGen[index1], &gen->actualGen[index2]);
        }
    } else {
        for(int i = 1; i < POPNUM; i++) {
            index1 = rand() % POPNUM;
            index2 = rand() % POPNUM;
            crossover(&gen->newGen[i], &gen->actualGen[index1], &gen->actualGen[index2]);
        }
    }
}

/**
 * rank selection 
*/
void rankSelection(generation_t *gen) {
    qsort(gen->actualGen, POPNUM, sizeof(ind_t), cmpFunction);
    for(int i = 0; i < POPNUM; i++) {
        gen->actualGen[i].fitness = (float) 1 / (i + 2);
    }
    select(gen);
}

/*
 * Boltzmann selection
*/
void boltzmannSelection(generation_t *gen) {
    float t, a;

    for(int i = 0; i < POPNUM; i++) {
        t = (float) rand() / (float) (RAND_MAX) * 95 + 5;   //[5, 100]
        a = (float) rand() / (float) (RAND_MAX);            //[0, 1]
        gen->actualGen[i].fitness = exp(- (gen->best - gen->actualGen[i].fitness) / (t * (1 - a)));
    }
    select(gen);
}

/**
 * roulette wheel
*/
void rouletteWheel(generation_t *gen) {
    select(gen);
}

void terminate(char *string, generation_t *gen) {
    for(int i = 0; i < POPNUM; i++) {
        free(gen->actualGen[i].word);
        free(gen->newGen[i].word);
    }
    free(gen->actualGen);
    free(gen->newGen);
    free(gen);
    free(string);
}

void geneticAlgorithm(char *toGuess) {
    void (*selection_ptr)(generation_t *gen);
    int wordLength = strlen(toGuess);
    float random;
    ind_t *temp;
    char *best;

    switch (SELECTION) {
    case 1:
        selection_ptr = &rouletteWheel;
        break;
    case 2:
        selection_ptr = &rankSelection;
        break;
    case 3:
        selection_ptr = &boltzmannSelection;
        break;
    default:
        printf("Error\n");
        return;
    }
    
    generation_t *gen = allocatePopulations(wordLength);
    gen->best = 0;
    gen->gen = 1;

    createPopulation(gen->actualGen, wordLength);
    best = malloc((wordLength + 1) * sizeof(char));
    
    while(gen->best < wordLength) {
        for(int i = 0; i < POPNUM; i++) {
            calculateFitness(&gen->actualGen[i], toGuess);
            //printf("%d: %s %.0f\n", generation, gen->actualGen[i].word, gen->actualGen[i].fitness);
            if(gen->actualGen[i].fitness > gen->best) {
                printf("\n%d: %s %.0f", gen->gen, gen->actualGen[i].word, gen->actualGen[i].fitness);
                gen->best = gen->actualGen[i].fitness;
                strcpy(best, gen->actualGen[i].word);
                if(gen->best == wordLength) {
                    printf("\nFounded in generation %d\n%s\n", gen->gen, gen->actualGen[i].word);
                    terminate(toGuess, gen);
                    return;
                }
            }
        }
        //printf("%d: %s %.0f %d\n", generation, best, bestFitness, wordLength);
        gen->gen++;
        strcpy(gen->newGen[0].word, best);
        gen->newGen[0].fitness = gen->best;

        (selection_ptr)(gen);

        temp = gen->actualGen;
        gen->actualGen = gen->newGen;
        gen->newGen = temp;

        for(int i = 0; i < POPNUM; i++) {
            random = (float) (rand() % 101) / 100;
            if(MUTATIONRATE > random) {
                mutate(&gen->actualGen[i]);
            }
        }
    }
}

int main() {
    int lenMax = 10, currentSize = lenMax;
    char *toGuess = malloc(lenMax * sizeof(char)), dummy;

    printf("Enter the string: ");
    int c;
	unsigned int i = 0;
	
    while ((c = getchar()) != '\n' && c != EOF) {
		toGuess[i] = (char) c;
        i++;
		if(i == currentSize) {
            currentSize = i + lenMax;
			toGuess = realloc(toGuess, currentSize);
		}
	}
	toGuess[i] = '\0';
    
    geneticAlgorithm(toGuess);

    scanf("%c", &dummy);
}