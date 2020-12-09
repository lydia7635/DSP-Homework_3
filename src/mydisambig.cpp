#include <iostream>
#include "Ngram.h"

#define NGRAM_ORDER 2

using namespace std;

Vocab voc;
Ngram lm(voc, NGRAM_ORDER);

double getBigramProb(const char *w1, const char *w2)    // from Q&A
{
    VocabIndex wid1 = voc.getIndex(w1);
    VocabIndex wid2 = voc.getIndex(w2);

    if(wid1 == Vocab_None)  //OOV
        wid1 = voc.getIndex(Vocab_Unknown);
    if(wid2 == Vocab_None)  //OOV
        wid2 = voc.getIndex(Vocab_Unknown);

    VocabIndex context[] = { wid1, Vocab_None };
    return lm.wordProb( wid2, context);
}

int main(int argc, char *argv[])
{
    if(argc != 5) {
        fprintf(stderr, "Error! Usage: ./mydisambig [encoded file] [mapping] [language model] [output file]\n");
        exit(1);
    }
    
    char *inputFile = argv[1];
    char *mappingFile = argv[2];
    char *modelFile = argv[3];
    char *outputFile = argv[4];

    File inputFp(inputFile, "r");
    File mapFp(mappingFile, "r");

    File modelFp(modelFile, "r");
    lm.read(modelFp);
    modelFp.close();

    char *sentence;
    while((sentence = inputFp.getline()) != NULL) {
        double delta[][];        
    }

    printf("Prob = %lf\n", getBigramProb("Åo", "ºX"));
}
