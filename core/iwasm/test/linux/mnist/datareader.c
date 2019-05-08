/*
 * Copyright (C) 2019 Intel Corporation.  All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// The data path is hard-coded in this part
#include "datareader.h"
#include <stdlib.h>

void readInData(double ***trainingData, double ***testData)
{
    char trainingDataPath[100] = "data/train.csv";
    char testDataPath[100] = "data/test.csv";
    int trainingPicNum = 42000;
    int testPicNum = 28000;
    (*trainingData) = getData(trainingDataPath, trainingPicNum, 785);
    (*testData) = getData(testDataPath, testPicNum, 784);
}

void readInMiniData(double ***trainingData, int minTestPicNum)
{
    char trainingDataPath[100] = "data/train.csv";
    int trainingPicNum = minTestPicNum;
    (*trainingData) = getData(trainingDataPath, trainingPicNum, 785);
}

double ** getData(char *path, int rowNum, int colNum)
{
    FILE* dataFile = fopen(path, "r");
    if (dataFile == NULL) {
        printf("Check if file path is correct");
        exit(1);
    }
    char head[10000];
    fgets(head, 10000, dataFile);
    double **data = (double **) malloc(sizeof(double*) * rowNum);
    int rowIndex, colIndex;
    char deliminator;
    int temp;
    for (rowIndex = 0; rowIndex < rowNum; rowIndex++) {
        data[rowIndex] = (double *) malloc(sizeof(double) * colNum);
        for (colIndex = 0; colIndex < colNum; colIndex++) {
            fscanf(dataFile, "%d%c", &temp, &deliminator);
            data[rowIndex][colIndex] = temp;
        }
    }
    fclose(dataFile);
    return data;
}
