/*
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */

/*
 *   Author:zixiang
 *   Email :zixiangcode@gmail.com
 */

#include <stdio.h>

#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

void trans_32x32(int M, int N, int A[N][M], int B[M][N]);
void trans_64x64(int M, int N, int A[N][M], int B[M][N]);
void trans_61x67(int M, int N, int A[N][M], int B[M][N]);

/*
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded.
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N]) {
    if (M == 32 && N == 32) {
        trans_32x32(M, N, A, B);
    }
    if (M == 64 && N == 64) {
        trans_64x64(M, N, A, B);
    }
    if (M == 61 && N == 67) {
        trans_61x67(M, N, A, B);
    }
}

/*
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started.
 */
char trans_32x32_desc[] = "Fast 32x32 transpose";
void trans_32x32(int M, int N, int A[N][M], int B[M][N]) {
    for (int i = 0; i < N; i += 8) {      // 外层分块的行
        for (int j = 0; j < M; j += 8) {  // 外层分块的列
            for (int k = i; k < i + 8; k++) {   // 按行读 A，空间局部性很好。读完就按列写入 B
                int a0 = A[k][j];               // 读到这的时候，A[k][j]后面的七个元素已经在缓存了，这个不命中，下面a1~a7全命中
                int a1 = A[k][j + 1];
                int a2 = A[k][j + 2];
                int a3 = A[k][j + 3];
                int a4 = A[k][j + 4];
                int a5 = A[k][j + 5];
                int a6 = A[k][j + 6];
                int a7 = A[k][j + 7];
                B[j][k] = a0;                   // 这时候已经把A[k][j]驱逐了，但是我们已经保存了a1~a7了
                B[j + 1][k] = a1;
                B[j + 2][k] = a2;
                B[j + 3][k] = a3;
                B[j + 4][k] = a4;
                B[j + 5][k] = a5;
                B[j + 6][k] = a6;
                B[j + 7][k] = a7;
            }
        }
    }
}

char trans_64x64_desc[] = "Fast 64x64 transpose";
void trans_64x64(int M, int N, int A[N][M], int B[M][N]) {
    int a0 = 0, a1 = 0, a2 = 0, a3 = 0, a4 = 0, a5 = 0, a6 = 0, a7 = 0;
    for (int i = 0; i < N; i += 8) {
        for (int j = 0; j < M; j += 8) {
            for (int k = i; k < i + 4; k++) {
                // 取 A 的左上和右上两块
                a0 = A[k][j + 0];
                a1 = A[k][j + 1];
                a2 = A[k][j + 2];
                a3 = A[k][j + 3];
                a4 = A[k][j + 4];
                a5 = A[k][j + 5];
                a6 = A[k][j + 6];
                a7 = A[k][j + 7];
                // 放到 B 的左上右上处
                B[j + 0][k] = a0;
                B[j + 1][k] = a1;
                B[j + 2][k] = a2;
                B[j + 3][k] = a3;
                B[j + 0][k + 4] = a4;
                B[j + 1][k + 4] = a5;
                B[j + 2][k + 4] = a6;
                B[j + 3][k + 4] = a7;
            }
            for (int k = j; k < j + 4; k++) {
                // 存下每块 B 中右上角的小块，作为本地 buffer
                a0 = B[k][i + 4];
                a1 = B[k][i + 5];
                a2 = B[k][i + 6];
                a3 = B[k][i + 7];
                // 拿到 A 的左下角第 3 块
                a4 = A[i + 4][k];
                a5 = A[i + 5][k];
                a6 = A[i + 6][k];
                a7 = A[i + 7][k];
                // 复制到刚才已经存到本地的 B 的位置处
                B[k][i + 4] = a4;
                B[k][i + 5] = a5;
                B[k][i + 6] = a6;
                B[k][i + 7] = a7;
                // 然后把 B 刚才存到本地的变量，复制到其应该在的位置，也就是 B 的左下角处
                B[k + 4][i + 0] = a0;
                B[k + 4][i + 1] = a1;
                B[k + 4][i + 2] = a2;
                B[k + 4][i + 3] = a3;
            }
            // 处理 B 的右下角，这就是正常利用变量转置了
            for (int k = i + 4; k < i + 8; k++) {
                a4 = A[k][j + 4];
                a5 = A[k][j + 5];
                a6 = A[k][j + 6];
                a7 = A[k][j + 7];
                B[j + 4][k] = a4;
                B[j + 5][k] = a5;
                B[j + 6][k] = a6;
                B[j + 7][k] = a7;
            }
        }
    }
}

char trans_61x67_desc[] = "Fast 61x67 transpose";
void trans_61x67(int M, int N, int A[N][M], int B[M][N]) {
    for (int i = 0; i < N; i += 16) {
        for (int j = 0; j < M; j += 16) {
            for (int row = i; row < i + 16 && row < N; row++) {
                for (int col = j; col < j + 16 && col < M; col++) {
                    B[col][row] = A[row][col];
                }
            }
        }
    }
}

/*
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N]) {
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }
}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions() {
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc);

    /* Register any additional transpose functions */
    registerTransFunction(trans, trans_desc);
    registerTransFunction(trans_32x32, trans_32x32_desc);
    registerTransFunction(trans_64x64, trans_64x64_desc);
    registerTransFunction(trans_61x67, trans_61x67_desc);
}

/*
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N]) {
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}
