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

/*This program is a demostration of MNIST recognition task implemented in C programming language*/
//No bias included in this version of the program
//This is a fast demo of the system, more work need to be done decomposing the parts of the net to make it accepting constructing the net
//with different kind of layers, deveritive matrix is essential to be included into it
// It's better to take hidden layers as an array of matrixs begin modifying codes - 2015-9-5
//#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include "simplenet.h"
#include "datareader.h"

void quadCostFunc(Vector *output, Vector *det, int label)
{
    for (int i = 0; i < output->len; i++) {
        if (i == label)
            det->data[i] = output->data[i] - 1;
        else
            det->data[i] = output->data[i];
    }
}

void mnistTest()
{
    // Prepare data
    double **trainingData, **testData;
    int trainingPicNum = 42000;
    readInData(&trainingData, &testData);

    // Init the network
    SimpleNet myNet;
    int layerNum = 3;
    int *layerSizes = (int *) malloc(sizeof(int) * layerNum);
    layerSizes[0] = 724, layerSizes[1] = 88, layerSizes[2] = 10;
    initNetWork(&myNet, layerNum, layerSizes);

    // Load pretrained weights
    //loadMat(&myNet.fls[0].weight, "fc0weight");
    //loadMat(&myNet.fls[1].weight, "fc1weight");
    //loadVector(&myNet.fls[0].bias, "fc0bias");
    //loadVector(&myNet.fls[1].bias, "fc1bias");

    // Params for learning
    double stepFactor = 0.00000005;
    int maxIteration = 3; //233 original //Epoch num, as they always set it to 50 in Currennt

    // Training by backprpagation
    clear(&myNet);
    for (int i = 0; i < maxIteration; i++) {
        double loss = 0;
        for (int j = 0; j < 10; j++) { // j < trainingPicNum
            int di = j; //data index
            forward(&myNet, trainingData[di] + 1);
            loss += l2loss(&myNet, (int) trainingData[di][0]);
            backward(&myNet, trainingData[di][0], &quadCostFunc, stepFactor);
        }
        loss /= trainingPicNum;
        printf("L2 Loss: %lf\n", loss);
        update(&myNet);
    }

    // Test overfitting result
    int right = 0;
    for (int j = 0; j < 10; j++) { // j < testPicNum
        forward(&myNet, trainingData[j] + 1);
        int res = selectFromOutput(&myNet);
        right += (trainingData[j][0] == (double) res);
    }
    printf("Accuracy: %lf\n", right / (double) trainingPicNum);

    // Save weight
    saveMat(&myNet.fls[0].weight, "fc0weight");
    saveMat(&myNet.fls[1].weight, "fc1weight");
    saveVector(&myNet.fls[0].bias, "fc0bias");
    saveVector(&myNet.fls[1].bias, "fc1bias");

    // Predict
    int testPicNum = 28000;
    FILE *res = fopen("prediction.csv", "w");
    fprintf(res, "ImageId,Label\n");
    for (int j = 0; j < 10; j++) {
        forward(&myNet, testData[j]);
        int label = selectFromOutput(&myNet);
        fprintf(res, "%d,%d\n", j + 1, label);
    }
    fclose(res);

}

int main()
{
    mnistTest();
    return 0;
}
