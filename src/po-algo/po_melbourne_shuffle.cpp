#include <stdio.h>
#include <unistd.h>
#include "./link_list.h"
#include "po_algo.h"
#include "cmo.h"

struct element {
        int key;
        int value;
};

void print_array(int* arr, int size)
{
     for (int i=0;i<size;i++)
        printf("id:%d, vl:%d \n",i,arr[i]);
}

void leakysec_distribute(int* perm, int* data, int* perm_t, int* data_t, int sqrtN, int max_elems, int i)
{
	CMO_p rt = init_cmo_runtime();
        ReadObIterator_p d = init_read_ob_iterator(rt, &data[i*sqrtN], sqrtN);
        ReadObIterator_p p = init_read_ob_iterator(rt, &perm[i*sqrtN], sqrtN*sqrtN);
        WriteObIterator_p od = init_write_ob_iterator(rt, &data_t[i*max_elems*sqrtN], max_elems*sqrtN);
        WriteObIterator_p op = init_write_ob_iterator(rt, &perm_t[i*max_elems*sqrtN], max_elems*sqrtN);
	
	int j,k,l, idT, key,v;
	int element_per_bucket = sqrtN;
	int buckets = sqrtN;
	int write_location, list_k, list_v;
    	struct list_head* rev_bucket;

        //preload();
	begin_leaky_sec(rt);
	rev_bucket = (struct list_head *) malloc (sizeof(struct list_head) * sqrtN); // allocate sqrt(input_size) number of link list heads to hold rev_buckets    
	for(l = 0; l < sqrtN; l++) {                   // setp 4 
		list_init(&rev_bucket[l]);
	}
	for(j = 0; j < element_per_bucket; j++) {                                               // step 7
		key =  ob_read_next(p);                               // step 9
		v =   ob_read_next(d);
                idT = key / (int) sqrtN;
		list_add(&rev_bucket[idT], key, v);                 // step 10
		if(rev_bucket[idT].size > max_elems) {                                                                  // step 14
			printf("OF ERROR : add more than plog(n) elements, rev_bucket %d cur size:%d, max:%d\n", k, rev_bucket[k].size, max_elems);     // step 15
			break;
		}
	}
	for(k = 0; k < buckets; k++) {
		//list_print(&rev_bucket[k]);
		write_location = k * max_elems * sqrtN + i * max_elems;
		l=0;
		while(l < max_elems) {
		       if(rev_bucket[k].size != 0){
                        	list_remove(&rev_bucket[k], &list_k, &list_v);
				ob_write_next(op, list_k);
				ob_write_next(od, list_v);
			}
			else{
				ob_write_next(od,-1);
				ob_write_next(op,-1);
			}
			l++;
		}
	}
	end_leaky_sec(rt);
	reset_write_ob(od);
	reset_write_ob(op);
	free(rev_bucket);
	free_cmo_runtime(rt);
}

void leakysec_cleanup(int* perm_t, int* data_t, int* data, int sqrtN, int max_elems, int i)
{
        int j,k,l,t1,t2,idT,key,v;
            j=k=l=0;
        int element_per_bucket = sqrtN;
        struct element *e;

        //preload();
        CMO_p rt = init_cmo_runtime();
        ReadObIterator_p d = init_read_ob_iterator(rt, &data_t[i*max_elems*sqrtN], max_elems*sqrtN);
        ReadObIterator_p p = init_read_ob_iterator(rt, &perm_t[i*max_elems*sqrtN], max_elems*sqrtN);
        NobArray_p od = init_nob_array(rt, data, sqrtN*sqrtN);

        begin_leaky_sec(rt); 
	for(j=0; j < max_elems * sqrtN; j++) {
		/*Skip dummy element identified by -1*/
                key = ob_read_next(d);
                v = ob_read_next(p); 
		if(key == -1) {	
                     continue;                                                                               // step 27
		}
		/*sort element within bucket according to perm*/
		nob_write_at(od,key,v);
	}

	end_leaky_sec(rt);
	free_cmo_runtime(rt);
}

void melbourn_shuffle(int* data, int* perm, int* data_t, int* perm_t, int* output, int size, int blowupfactor) {
        long    buckets;
        int     sqrtN, max_elems;
	int i;

        buckets = sqrtN = sqrt(size);                           // step 1                                                          // step 2
        max_elems = blowupfactor;                               //plogn
        printf("size:%d, Blowupfactor:%d\n", size, max_elems);

       /* Distribution phase*/
       for(i = 0; i < buckets; i++) {                                                                  // setp 4
		leakysec_distribute(perm, data, perm_t, data_t, sqrtN, max_elems, i);
        }

	//print_array(data_t, blowupfactor*size);
	//print_array(perm_t, blowupfactor*size);

        /* Clean up phase*/
        for(i = 0; i < buckets; i++) {                                                                                  // step 24
		leakysec_cleanup(perm_t, data_t,  output, sqrtN,  max_elems, i);
        }
        //print_array(output,size);
}

int po_melbourne_entry(int* perm, int* data, int* output, int size) {

	int sqrtN = sqrt(size);
	int blowupfactor = 1*log2(size); //plogn
	int data_t[blowupfactor*size], perm_t[blowupfactor*size];

	melbourn_shuffle(data, perm, data_t, perm_t, output, size, blowupfactor);
	return 1;
}

