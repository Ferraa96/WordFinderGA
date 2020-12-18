#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<malloc.h>

#define MAXCHAR 84
#define POPNUM 20
#define MUTATIONRATE 0.1

typedef struct {
    char *word;
    int fitness;
} ind_t;

typedef struct {
    ind_t *actualGen;
    ind_t *newGen;
} generation_t;

/*char availableChar(char random) {                     //from 0 to 52

    if(random < 26) {                                   //LETTERS
        random += 65;
    } else if(random < 52) {                            //letters
        random += 71;
    } else {
        random = 32;                                    //space
    }
    return random;
}*/

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

/**
 * creates a random population of POPNUM random strings 
*/
void createPopulation(ind_t *population, int wordLength) {
    for(int i = 0; i < POPNUM; i++) {
        for(int j = 0; j < wordLength; j++) {
            population[i].word[j] = availableChar(rand() % (MAXCHAR + 1));
        }
        population[i].word[wordLength] = '\0';
    }
}

/**
 * fitness = number of correct character in the string
*/
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

/**
 * mutates one character in a random position
*/
void mutate(ind_t *ind) {
    int wordLen = strlen(ind->word), pos = rand() % wordLen;
    
    ind->word[pos] = availableChar(rand() % (MAXCHAR + 1));
}

/**
 * single-point crossover
*/
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

/**
 * roulette wheel
*/
ind_t *selection(int fitnessSum, generation_t *gen) {
    int index1, index2, partialSum, random;

    if(fitnessSum != 0) {
        for(int i = 1; i < POPNUM; i++) {
            random = rand() % (fitnessSum + 1);
            index1 = 0;
            partialSum = gen -> actualGen[0].fitness;
            while(random > partialSum) {
                index1++;
                partialSum += gen -> actualGen[index1].fitness;
            }
            random = rand() % (fitnessSum + 1);
            index2 = 0;
            partialSum = gen -> actualGen[0].fitness;
            while(random > partialSum) {
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

/**
 * runs the genetic algorithm until the input word is found
*/
void geneticAlgorithm(char *toGuess) {
    int generation = 1, totalFit, wordLength = strlen(toGuess);
    int bestFitness = 0;
    float random;
    ind_t *temp;
    char *best;
    
    generation_t *gen = allocatePopulations(wordLength);
    createPopulation(gen->actualGen, wordLength);
    best = malloc((wordLength + 1) * sizeof(char));
    
    while(bestFitness < wordLength) {
        totalFit = 0;
        for(int i = 0; i < POPNUM; i++) {
            calculateFitness(&gen->actualGen[i], toGuess);
            totalFit += gen->actualGen[i].fitness;
            if(gen->actualGen[i].fitness > bestFitness) {
                printf("\n%d: %s %d/%d", generation, gen->actualGen[i].word, gen->actualGen[i].fitness, wordLength);
                bestFitness = gen->actualGen[i].fitness;
                strcpy(best, gen->actualGen[i].word);
                if(bestFitness == wordLength) {
                    printf("\nFounded in generation %d\n%s\n", generation, gen->actualGen[i].word);
                    terminate(toGuess, gen);
                    return;
                }
            }
        }
        //printf("%d: %s %d/%d\n", generation, best, bestFitness, wordLength);
        generation++;
        strcpy(gen->newGen[0].word, best);
        gen->newGen[0].fitness = bestFitness;
        selection(totalFit, gen);

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
	int c = EOF;
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