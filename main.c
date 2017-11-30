
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <malloc.h>
#include <time.h>


#define SIZE 1024*1024*8


float test_math(float * numbers){
    float out = 0.0;
    for(int i = SIZE-1; i >= 0; --i){
        out += 1.0/sqrt(numbers[i]);
    }
    return out;
}


float test_sse_scalar(float * numbers){
    float out = 0.0;
    float dst;
    for(int i = SIZE-1; i >= 0; --i){
        asm("rsqrtss %1, %0\n\t"
            : "=x" (dst)
            : "x" (numbers[i]));
        out += dst;
    }
    return out;
}


float test_sse_vector(float * numbers){
    float dst;
    float out = 0.0;
    for(int i=0; i<SIZE; i+=4){
        asm("movaps   %1,     %%xmm1\n\t"
            "rsqrtps  %%xmm1, %%xmm0\n\t"
            "movshdup %%xmm0, %%xmm1\n\t"
            "addps    %%xmm1, %%xmm0\n\t"
            "movhlps  %%xmm0, %%xmm1\n\t"
            "addss    %%xmm1, %%xmm0\n\t"
            "movss    %%xmm0, %0"
            : "=x" (dst)
            : "m" (numbers[i])
            : "xmm0", "xmm1"
        );
        out += dst;
    }
    return out;
}


float test_avx_vector(float * numbers){
    float dst;
    float out = 0.0;
    for(int i=0; i<SIZE; i+=8){
        asm("vmovaps    %1,     %%ymm1\n\t"
            "vrsqrtps   %%ymm1, %%ymm0\n\t"
            "vperm2f128 $1,     %%ymm0, %%ymm0, %%ymm1\n\t"
            "vaddps     %%ymm0, %%ymm1, %%ymm2\n\t"
            "vshufps    $177,   %%ymm2, %%ymm2, %%ymm0\n\t"  // _MM_SHUFFLE(2,3,0,1) = 177
            "vaddps     %%ymm0, %%ymm2, %%ymm0\n\t"
            "vshufps    $78,    %%ymm0, %%ymm0, %%ymm2\n\t"  // _MM_SHUFFLE(1,0,3,2) = 78
            "vaddps     %%ymm0, %%ymm2, %%ymm0\n\t"
            //"movss      %%xmm0, %0"  //  probably not needed as dst is xmm0
            : "=x" (dst)
            : "m" (numbers[i])
        );
        out += dst;
    }
    return out;
}


float test_carmack(float * numbers){
    int j;
	float x2, y;
    float out;
    const float threehalfs = 1.5F;

    for(int i = SIZE-1; i >= 0; --i){
        x2 = numbers[i] * 0.5F;
        y  = numbers[i];
        j  = * ( int * ) &y;
        j  = 0x5f3759df - ( j >> 1 );
        y  = * ( float * ) &j;
        y  = y * ( threehalfs - ( x2 * y * y ) );
        out += y;
    }

    return out;
}

void run(char * label, float (*func)(float *), float * numbers){
    float out;
    clock_t t;
    clock_t total = 0;
    for(int i = 0; i<100; i++){
        t = clock();
        out = func(numbers);
        total += clock() - t;
    }

    printf("| %10s | %6.1f | %4.1fms |\n", label, out, 1000*(double)(total)/CLOCKS_PER_SEC/100);
}

int main(int argc, char ** argv){

    float * numbers = memalign(32, SIZE*sizeof(float));
    for(int i=0; i<SIZE; ++i){
        numbers[i] = (float)i+1;
    }
    
    printf("+------------+--------+--------+\n");
    printf("| %10s | %6s | %6s |\n", "Method", "Result", "Time");
    printf("+------------+--------+--------+\n");
    run("Naive math", &test_math, numbers);
    run("Carmack", &test_carmack, numbers);
    run("SSE scalar", &test_sse_scalar, numbers);
    run("SSE packed", &test_sse_vector, numbers);
    run("AVX packed", &test_avx_vector, numbers);
    printf("+------------+--------+--------+\n");
    
    
    free(numbers);
    return 0;
}
