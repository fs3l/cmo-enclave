#include "./helper.h"
#include "po_algo.h"
#include "cmo.h"
#include "utils.h"

void read_file(char *name, int32_t* perm, int32_t len) {
	int32_t		count;
	FILE		*fp;

	fp = fopen(name, "rb");
	if(fp == NULL) {
		perror("read_file error!\n");
		exit(1);
	}

	printf("Start reading %d perms:\n", len);
	
	for(int32_t i=0; i<len; i++){
		if(fscanf(fp, "%d ", &perm[i]) != 1)
			break;	
	}	
	fclose(fp);
	fisher_yates_shuffle<int32_t>(perm,len);
	/*for(int i=0; i<len; i++){
		printf("Perm:%d\n",perm[i]);	
	}*/
}

void po_melbourne_shuffle(int32_t* data, int32_t* perm, int32_t* data_t, int32_t* perm_t, int32_t* output, int32_t size, int32_t max_elems) {
        long    buckets;
        int32_t     sqrtN;
        int32_t i;

        buckets = sqrtN = ceil(sqrt(size));                           // step 1
        printf("size:%d, Blow_up_factor:%d\n", size, max_elems);

       /* Distribution phase*/
       for(i = 0; i < buckets; i++) {                                                                  // setp 4
                leakysec_distribute(perm, data, perm_t, data_t, sqrtN, max_elems, i);
        }

        //print_array(data_t, max_elems*size);
        //print_array(perm_t, max_elems*size);

        /* Clean up phase*/
        for(i = 0; i < buckets; i++) {                                                                                  // step 24
                leakysec_cleanup(perm_t, data_t,  output, sqrtN,  max_elems, i);
        }
        //print_array(output,size);
}

BOOST_AUTO_TEST_CASE(po_melbourne_shuffle_test)
{
  int32_t len = 25600;
  int32_t blow_up_factor = ceil(log2(len));
  int32_t* input = new int32_t[len];//gen_random_sequence(len);
  int32_t* output = new int32_t[len];
  int32_t* perm_t = new int32_t[len*blow_up_factor];
  int32_t* data_t = new int32_t[len*blow_up_factor];
  read_file("./melbourne_input.txt", input, len);
  po_melbourne_shuffle(input, input, data_t, perm_t, output, len, blow_up_factor);
  for (int32_t i = 0; i < len; ++i) BOOST_CHECK(output[i] == i);
  delete[] input;
  delete[] data_t;
  delete[] perm_t;
}

