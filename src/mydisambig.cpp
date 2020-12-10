#include <iostream>
#include <string.h>
#include <map>
#include "Ngram.h"

#define NGRAM_ORDER 2
#define MAX_LINELEN 1024
#define MAX_CAND 3072
#define BIG5UNIT 3
#define NEGINF -10000

using namespace std;

typedef struct Cand {
    char **cand_list;
    unsigned int cand_num;
} CAND;

Vocab voc;
Ngram lm(voc, NGRAM_ORDER);
double *delta[MAX_LINELEN];
int *bestBack[MAX_LINELEN];
int candNum[MAX_LINELEN];

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

double getOneProb(const char *w1)    // from Q&A
{
    VocabIndex wid1 = voc.getIndex(w1);

    if(wid1 == Vocab_None)  //OOV
        wid1 = voc.getIndex(Vocab_Unknown);

    VocabIndex context[] = { Vocab_None };
    return lm.wordProb( wid1, context);
}

void buildMap(File *mapFp, map<string, CAND *> &ZBmap)
{
    char *lineBuf;
    while((lineBuf = (*mapFp).getline()) != NULL) {
        VocabString words[MAX_CAND] = {};
        CAND *cand = (CAND *)malloc(sizeof(CAND));
        char *key = (char *)malloc(sizeof(char) * BIG5UNIT);
        cand->cand_num = Vocab::parseWords(lineBuf, words, MAX_CAND) - 1;
        memcpy(key, words[0], BIG5UNIT);
        cand->cand_list = (char **)malloc(sizeof(char *) * (cand->cand_num));

        for(int i = 0; i < cand->cand_num; ++i) {
            cand->cand_list[i] = (char *)malloc(sizeof(char) * BIG5UNIT);
            memcpy(cand->cand_list[i], words[i + 1], BIG5UNIT);
        }
        string keystr(key);
        ZBmap[keystr] = cand;
    }
    return;
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
    map<string, CAND *> ZBmap;
    map<string, CAND *>::iterator mapIter;
    map<string, CAND *>::reverse_iterator mapIter_r;

    buildMap(&mapFp, ZBmap);
    
    mapFp.close();

    File modelFp(modelFile, "r");
    lm.read(modelFp);
    modelFp.close();

    FILE *outputFp = fopen(outputFile, "w");

    char *sentence;

    while((sentence = inputFp.getline()) != NULL) {
        // check sentence_len
        VocabString words[MAX_LINELEN] = {};
        int sentenceLen = Vocab::parseWords(sentence, words, MAX_LINELEN);
        string keystr(words[0]);

        // initial for Viterbi
        candNum[0] = ZBmap[keystr]->cand_num;
        delta[0] = (double *)malloc(sizeof(double) * candNum[0]);
        for(int i = 0; i < candNum[0]; ++i) {
            delta[0][i] = getOneProb(ZBmap[keystr]->cand_list[i]);
        }

        for(int i = 1; i < sentenceLen; ++i) {
            string preKeyStr(words[i - 1]), curKeyStr(words[i]);
            candNum[i] = ZBmap[curKeyStr]->cand_num;
            delta[i] = (double *)malloc(sizeof(double) * candNum[i]);
            bestBack[i] = (int *)malloc(sizeof(int) * candNum[i]);

            for(int j = 0; j < candNum[i]; ++j) {
                char *curWord = ZBmap[curKeyStr]->cand_list[j];
                delta[i][j] = NEGINF;

                for(int k = 0; k < candNum[i - 1]; ++k) {
                    char *preWord = ZBmap[preKeyStr]->cand_list[k];
                    double temp_delta = getBigramProb(preWord, curWord) + delta[i-1][k];
                    if(temp_delta > delta[i][j]) {
                        delta[i][j] = temp_delta;
                        bestBack[i][j] = k;
                    }
                }
            }
        }
        int bestDelta = NEGINF;
        int bestTail;
        for(int i = 0; i < candNum[sentenceLen - 1]; ++i) {
            if(delta[sentenceLen - 1][i] > bestDelta) {
                bestDelta = delta[sentenceLen - 1][i];
                bestTail = i;
            }
        }

        char bestPath[sentenceLen * BIG5UNIT + 9];

        for(int i = 0; i < sentenceLen * BIG5UNIT + 9; ++i)
            bestPath[i] = ' ';

        memcpy(bestPath, "<s> ", 4);
        memcpy(&bestPath[sentenceLen * BIG5UNIT + 4], "</s>\n", 5);

        for(int i = sentenceLen - 1; i > 0; --i) {
            string keyStr(words[i]);
            memcpy(&bestPath[i * BIG5UNIT + 4],ZBmap[keyStr]->cand_list[bestTail], 2);
            bestTail = bestBack[i][bestTail];
        }
        string keyStr(words[0]);
        memcpy(&bestPath[4], ZBmap[keyStr]->cand_list[bestTail], 2);

        fwrite(bestPath, sizeof(char), sentenceLen * BIG5UNIT + 9, outputFp);
    }

    inputFp.close();
    fclose(outputFp);
    return 0;
}
