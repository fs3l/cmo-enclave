#include "algo.h"
#include "cmo.h"
#define BLOCKSIZE 32

void init_context(int* pt, int* iv, int size)
{
  for(int i = 0; i < size; ++i)
  {
     pt[i] = 1;
     if (i < BLOCKSIZE) iv[i] = 2;
  }
}

void xorBlock(ReadObIterator_p pr,WriteObIterator_p pw,int* iv)
{
	for (int i = 0; i < BLOCKSIZE; i++)
	{
	   ob_write_next(pw, ob_read_next(pr) ^ iv[i]);
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
  ReadObIterator_p pr = init_read_ob_iterator(rt, pt, size);
  ReadObIterator_p c = init_read_ob_iterator(rt, ct, size);
  WriteObIterator_p pw = init_write_ob_iterator(rt, pt, size);
  
  int block_num = size / BLOCKSIZE;

   begin_leaky_sec(rt);
   for (int i=0; i < block_num; i++)
   {
      int idx = i * BLOCKSIZE; 
      xorBlock(pr, pw, iv);
      PRF(&pt[idx],k,&ct[idx]);
      iv = &ct[idx];
   }
   end_leaky_sec(rt);
}

int Aes_cbc(int size)
{
  int pt[size];
  int ct[size];
  int iv[32]; 
  int k;

  init_context(pt, iv, size);  
  enc_cbc(pt, iv, k, ct, size);  
}
