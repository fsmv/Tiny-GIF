#include <stdlib.h>
#include <string.h>
#include "Dictionary.h"
#include "LZW.h"

/**
 * Appends the element to the array extending it if necessary
 *
 * @param arr array to append to
 * @param size size of the array currently
 * @param index position to add to
 * @param code element to add
 */
void setCode(uint16_t **arr, size_t *size, size_t index, const uint16_t code) {
    if(index >= *size) {
        uint16_t *temp = malloc(sizeof(uint16_t) * 2 * index);
        memcpy(temp, *arr, *size * sizeof(uint16_t));
        free(*arr);
        *size = 2 * index;
        *arr = temp;
    }

    (*arr)[index] = code;
}

/**
 * Appends the element to the array extending it if necessary
 *
 * @param arr array to append to
 * @param size size of the array currently
 * @param index position to add to
 * @param ch element to add
 */
void setChar(char **arr, size_t *size, size_t index, const char ch) {
    if(index >= *size) {
        char *temp = malloc(sizeof(char) * 2 * index);
        memcpy(temp, *arr, *size * sizeof(char));
        free(*arr);
        *size = 2 * index;
        *arr = temp;
    }

    (*arr)[index] = ch;
}

void LZW_Compress(const char *string, uint16_t **code, size_t *codec) {
    Dictionary dict;
    dict_init(&dict);

    size_t resultLen = 64;
    uint16_t *result = malloc(sizeof(uint16_t) * resultLen);
    size_t resultIndex = 0;

    size_t symLen = 16;
    uint8_t *currSym = malloc(sizeof(uint8_t) * symLen);
    size_t symIndex = 0;
    size_t charIndex = 0;
    while(string[charIndex] != '\0') {
        setChar((char**)&currSym, &symLen, symIndex, string[charIndex]);
        symIndex++;

        if(!dict_contains(&dict, currSym, symIndex)) {
            //If the dictionary does not cantain the symbol, add it
            setCode(&result, &resultLen, resultIndex, dict_add(&dict, currSym, symIndex));

            //if we go over 16 bits reset the dictionary
            if(dict.currIndex == 0xFFFF) {
                dict_free(&dict);
                dict_init(&dict);
                setCode(&result, &resultLen, resultIndex, CLEAR_CODE);
            }

            //set the current symbol to last char read
            currSym[0] = string[charIndex];
            symIndex = 1;
            resultIndex++;
        }

        charIndex++;
    }

    setChar((char**)&currSym, &symLen, symIndex, '\0');
    setCode(&result, &resultLen, resultIndex, dict_add(&dict, currSym, symIndex+1));
    resultIndex++;

    *code = result;
    *codec = resultIndex;

    free(currSym);
    dict_free(&dict);
}

void LZW_Decompress(const uint16_t *code, const size_t codec, char **string) {
    Dictionary dict;
    dict_init(&dict);

    size_t symLen = 16;
    uint8_t *currSym = malloc(sizeof(uint8_t) * symLen);
    size_t symIndex = 0;

    size_t resultLen = 32;
    char *result = malloc(sizeof(char) * resultLen);
    size_t resultIndex = 0;

    int i;
    for(i = 0; i < codec; i++) {
        uint8_t *phrase = NULL;
        size_t phraseLen;

        //If the clear code shows up reset the dictionary
        if(code[i] == CLEAR_CODE) {
            dict_free(&dict);
            dict_init(&dict);
            continue;
        }

        if(dict_search(&dict, code[i], &phrase, &phraseLen)) {
            //output phrase
            resultIndex += phraseLen;
            setChar(&result, &resultLen, resultIndex, '\0');
            memcpy(result + resultIndex - phraseLen, phrase, phraseLen);

            //Add currSym + phrase[0] to dictionary
            setChar((char **) &currSym, &symLen, symIndex, phrase[0]);
            if(symIndex > 0) { //if == 0 its already in the dictionary
                dict_add(&dict, currSym, symIndex+1);
            }

            //currSym = phrase
            free(currSym);
            currSym = phrase;
            symLen = symIndex = phraseLen;
        }else{
            //currSym += currSym[0]
            setChar((char **)&currSym, &symLen, symIndex, currSym[0]);
            symIndex++;

            //output currSym
            resultIndex += symIndex;
            setChar(&result, &resultLen, resultIndex, '\0');
            memcpy(result + resultIndex - symIndex, currSym, symIndex);

            //add currSym to dictionary
            dict_add(&dict, currSym, symIndex);
            free(phrase); //allocated in dict_search
        }
    }

    *string = result;

    free(currSym);
    dict_free(&dict);
}