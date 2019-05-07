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

/*
 Simple linear algebra operation for the network
 */

#ifndef SIMPLELA
#define SIMPLELA
#include <stdbool.h>

typedef struct Vector {
    double *data;
    int len;
} Vector;

typedef struct Mat {
    double **data;
    int rowNum;
    int colNum;
} Mat;

double * getVecSpace(int size);
double ** getMatSpace(int inputLayerSize, int outputLayerSize);
void clearVector(Vector *vec);
void clearMat(Mat *mat);
void vplusv(Vector *vec, Vector *delta, double factor);
void vcpv(Vector *des, Vector *src);
void mplusm(Mat *m, Mat *dm, double factor);
void vmv(Vector *in_vec, Mat *mat, Vector *out_vec, bool mtrans);
void vvm(Vector *lvec, Vector *rvec, Mat *mat, double sf);
void printVector(Vector *vec);
void saveVector(Vector *v, char filename[]);
void loadVector(Vector *v, char filename[]);
void saveMat(Mat* m, char filename[]);
void loadMat(Mat *m, char filename[]);
#endif
