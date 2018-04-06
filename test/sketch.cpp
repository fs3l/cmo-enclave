#include "./helper.h"
#include <sys/time.h>
#ifdef SGX_APP
#include "./sgx_helper.h"
#include "st"

#include "algo_u.h"
#else
#include "algo.h"
#endif

BOOST_AUTO_TEST_CASE(sketch_test)
{
    int32_t len = 100000;
    int32_t range = 1;
    int32_t* output = new int32_t[range];
    //printf("output addr = %p \n",output);
    memset(output,0,4*range);
    int32_t input[len];
    //printf("input addr = %p \n",input);
    for(int i = 0; i <len;i++){
        input[i] = rand()%len;
    }	
    struct timeval begin,end;
    gettimeofday(&begin,NULL);
    sketch(input,output,len);
    gettimeofday(&end,NULL);
    for(int i = 0;i<range;i++)
        printf("result =  %d\n",output[i]);
    printf("time spent=%ld\n",1000000*(end.tv_sec-begin.tv_sec)+end.tv_usec-begin.tv_usec);
}
