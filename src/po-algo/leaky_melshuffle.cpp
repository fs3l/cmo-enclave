#include <stdio.h>
#include <unistd.h>
#include "./link_list.h"
#include "po-algo.h"
#include "cmo.h"

struct element {
        int key;
        int value;
};


void leakysec_bucket_permmul(int* perm, int* data, int* perm_t, int* data_t, int sqrtN, int max_elems, int i)
{
	int j,k,l, idT, key,v;
	int element_per_bucket = sqrtN;
	int buckets = sqrtN;
	int write_location, list_k, list_v;
    	struct list_head* rev_bucket;

        //preload();
	CMO_p rt = init_cmo_runtime();
	ReadObIterator_p d = init_read_ob_iterator(rt, data, sqrtN);
	ReadObIterator_p p = init_read_ob_iterator(rt, perm, sqrtN);
	NobArray_p od = init_nob_array(rt, data_t, max_elems*sqrtN);
	NobArray_p op = init_nob_array(rt, perm_t, max_elems*sqrtN);


	rev_bucket = (struct list_head *) malloc (sizeof(struct list_head) * sqrtN); // allocate sqrt(input_size) number of link list heads to hold rev_buckets    
	for(l = 0; l < sqrtN; l++) {                   // setp 4 
		list_init(&rev_bucket[l]);
	}
	for(j = 0; j < element_per_bucket; j++) {                                               // step 7
		key =  ob_read_next(p);                               // step 9
		v =   ob_read_next(d);
                idT = key / (int) sqrtN;
		list_add(&rev_bucket[idT], key, v);                 // step 10
		//printf("adding rev_bucket %d k:%d, v:%d\n",idT, perm[i*element_per_bucket+j], data[i*element_per_bucket+j]);
		if(rev_bucket[idT].size > max_elems) {                                                                  // step 14
			printf("OF ERROR : add more than plog(n) elements, rev_bucket %d cur size:%d, max:%d\n", k, rev_bucket[k].size, max_elems);     // step 15
			break;
		}
	}

	for(k = 0; k < buckets; k++) {
		//list_print(&rev_bucket[k]);
	        /* add dummy elements*/
		/*while(rev_bucket[k].size < max_elems) {
			list_add(&rev_bucket[k], -1, -1);                                       // step 18
		}*/
		/*seek approprite location ( i.e. within each bucket at ith position )*/
		write_location = k * max_elems * sqrtN + i * max_elems;
		l=0;
		while(l < max_elems) {
		       if(rev_bucket[k].size != 0){
                        	list_remove(&rev_bucket[k], &list_k, &list_v);
				nob_write_at(od, list_k,list_v);
			}
			else{
                            nob_write_at(od,-1,-1);
			}
                        //write_location++;
			l++;
		}
	}
	free(rev_bucket);
	free_cmo_runtime(rt);
}

void leakysec_bucket_sort(int* perm_t, int* data_t, int* data, int sqrtN, int max_elems, int i)
{
        int j,k,l,t1,t2,idT,key,v;
            j=k=l=0;
        int element_per_bucket = sqrtN;
        struct element *e;
        struct element *bucketM = (struct element *) malloc (element_per_bucket * sizeof(struct element));
        //struct element *clean_rev_bucket = (struct element *) malloc (max_elems * sizeof(struct element) * (int) sqrtN);    

        //preload();
        CMO_p rt = init_cmo_runtime();
        ReadObIterator_p d = init_read_ob_iterator(rt, data_t, max_elems*sqrtN);
        ReadObIterator_p p = init_read_ob_iterator(rt, perm_t, max_elems*sqrtN);
        NobArray_p od = init_nob_array(rt, data, sqrtN);

	/*for(k = 0; k < max_elems * sqrtN; k++) {                                                        // step 25
		clean_rev_bucket[k].key = ob_read_next(p);
		clean_rev_bucket[k].value = ob_read_next(d);
	}*/
       
	for(j=0; j < max_elems * sqrtN; j++) {
		/*Skip dummy element identified by -1*/
                e->key = ob_read_next(p);
                e->value = ob_read_next(d); 
		if(e->key == -1) {
		     j++;	
                     continue;                                                                               // step 27
		}
		/*sort element within bucket according to perm*/
		idT = e->key % (int) sqrtN;                                    // step 28
                bucketM[idT].value = e->value;
	}
	/* write output array*/
	for( l=0; l<sqrtN;l++)
	{
           nob_write_at(od, l,bucketM[l].value);    // step 29
	}

	free(bucketM);
	free_cmo_runtime(rt);
}
void melbourn_shuffle(int* data, int* perm, int* data_t, int* perm_t, int size, int blowupfactor) {

        long    buckets;
        int     sqrtN, max_elems;
	int i;
        /* calculate various sizes */

        buckets = sqrtN = sqrt(size);                           // step 1                              
                                                                // step 2
        max_elems = blowupfactor;                               //plogn
        printf("size:%d, Blowupfactor:%d\n", size, max_elems);

       /* Distribution phase*/
       // distribute(perm, data, perm_t, data_t, sqrtN, max_elems);
       for(i = 0; i < buckets; i++) {                                                                  // setp 4
          leakysec_bucket_permmul(perm, data, perm_t, data_t, sqrtN, max_elems, i);
        }
        /*
         * DEBUG:
         * You can use print_array() to print temp contents
         * printf("\n Printing TEMP array\n");
         * print_array(perm_t, max_elems*size);
         */

        /* Clean up phase*/
        //cleanup(perm_t, data_t, data, sqrtN, max_elems);
        for(i = 0; i < buckets; i++) {                                                                                  // step 24
            leakysec_bucket_sort(perm_t, data_t,  data, sqrtN,  max_elems, i);
        }
        //print_array(data,size);
}

int melbourne_entry(int* perm, int* data, int size) {

    int sqrtN = sqrt(size);
    int blowupfactor = 1*log2(size); //plogn
    int data_t[blowupfactor*size], perm_t[blowupfactor*size];

    melbourn_shuffle(data, perm, data_t, perm_t, size, blowupfactor);
   return 1;
}

