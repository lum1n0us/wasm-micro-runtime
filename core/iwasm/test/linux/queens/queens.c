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

#include<stdio.h>

int abs(int a)
{
    if (a < 0)
        a = -a;
    return a;
}

int max = 8, sum = 0, a[8];

void show()
{
    for (int i = 0; i < max; i++) {
        printf("(%d,%d)\t", i, a[i]);
    }
    printf("\n");
}

int check(int n)
{
    for (int i = 0; i < n; i++) {
        if (a[i] == a[n] || abs(a[n] - a[i]) == (n - i))
            return 0;
    }
    return 1;
}

void eightQueen(int n)
{
    int i;
    if (n < max) {
        for (i = 0; i < max; i++) {
            a[n] = i;
            if (check(n))
                eightQueen(n + 1);
        }
    } else {
        sum++;
        show();
    }
}

int main()
{
    eightQueen(0);
    printf("%d\n", sum);
}
