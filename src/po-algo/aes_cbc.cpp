#include "algo.h"
#include "cmo.h"
#define BLOCKSIZE 128
/*struct cbcBlock
{
   int context[4];
};*/

void init_context(int* pt, int* iv, int size)
{
  //TODO
}

void xorBlock(int* in, int* iv)
{//TODO
	for (int i = 0; i < BLOCKSIZE; i++)
	{
           in[i] ^= iv[i];
	}

}

void PRF(int* pt, int k, int* cp)
{
  //TODO
  __asm(
    "nop\r\n"
   :::);
}

void enc_cbc(int* pt, int* iv, int k, int* ct, int size)
{
  CMO_p rt = init_cmo_runtime();
  ReadObIterator_p p = init_read_ob_iterator(rt, pt, size);
  ReadObIterator_p c = init_read_ob_iterator(rt, ct, size);
  //NobArray_p o = init_nob_array(rt, arr_out, len);
  int block_num = size / BLOCKSIZE;

   for (int i=0; i < block_num; i++)
   {
      int idx = i * BLOCKSIZE; 
      xorBlock(&pt[idx],iv);
      PRF(&pt[idx],k,&ct[idx]);
      iv = &ct[idx];
   }
}

int Aes_cbc(int size)
{
  int pt[size];
  int ct[size];
  int iv[128]; 
  int k;

  init_context(pt, iv, size);  
  enc_cbc(pt, iv, k, ct, size);  
}
