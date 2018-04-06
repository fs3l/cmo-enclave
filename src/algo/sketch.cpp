#include "./shuffle_bucket.h"
#include "algo.h"
#include "cmo.h"
#include "cmo_queue.h"
#include "utils.h"
#define SCAN 1
#include <stdio.h>
#include <iostream>
#include <string>
#include <string.h>
#include <stdlib.h>
#include <cmath>
#include <list>
using namespace std;



typedef struct SHAstate_st {
    unsigned long h0, h1, h2, h3, h4;
    unsigned long Nl, Nh;
    unsigned long data[16];
    unsigned int num;
} SHA_CTX;

struct mht_node {
    unsigned char digest[20];
};


#define INIT_DATA_h0 0x67452301UL
#define INIT_DATA_h1 0xefcdab89UL
#define INIT_DATA_h2 0x98badcfeUL
#define INIT_DATA_h3 0x10325476UL
#define INIT_DATA_h4 0xc3d2e1f0UL

#define K_00_19 0x5a827999UL
#define K_20_39 0x6ed9eba1UL
#define K_40_59 0x8f1bbcdcUL
#define K_60_79 0xca62c1d6UL

#define F_00_19(b,c,d)  ((((c) ^ (d)) & (b)) ^ (d))
#define F_20_39(b,c,d)  ((b) ^ (c) ^ (d))
#define F_40_59(b,c,d)  (((b) & (c)) | (((b)|(c)) & (d)))
#define F_60_79(b,c,d)  F_20_39(b,c,d)


#  define HOST_c2l(c,l)   (l =(((unsigned long)(*((c)++)))<<24),          \
l|=(((unsigned long)(*((c)++)))<<16),          \
l|=(((unsigned long)(*((c)++)))<< 8),          \
l|=(((unsigned long)(*((c)++)))    )   )
#  define HOST_l2c(l,c)   (*((c)++)=(unsigned char)(((l)>>24)&0xff),      \
*((c)++)=(unsigned char)(((l)>>16)&0xff),      \
*((c)++)=(unsigned char)(((l)>> 8)&0xff),      \
*((c)++)=(unsigned char)(((l)    )&0xff)   )


# define ROTATE(a,n)     (((a)<<(n))|(((a)&0xffffffff)>>(32-(n))))
#  define Xupdate(a,ix,ia,ib,ic,id)     ( (a)=(ia^ib^ic^id),    \
ix=(a)=ROTATE((a),1)  \
)
#define HASH_LONG unsigned long
#define SHA_LONG unsigned long
#define SHA_LBLOCK 16
#define SHA_CBLOCK (SHA_LBLOCK*4)
#define HASH_CBLOCK SHA_CBLOCK
# define BODY_00_15(xi)           do {   \
T=E+K_00_19+F_00_19(B,C,D);     \
E=D, D=C, C=ROTATE(B,30), B=A;  \
A=ROTATE(A,5)+T+xi;         } while(0)

# define BODY_16_19(xa,xb,xc,xd)  do {   \
Xupdate(T,xa,xa,xb,xc,xd);      \
T+=E+K_00_19+F_00_19(B,C,D);    \
E=D, D=C, C=ROTATE(B,30), B=A;  \
A=ROTATE(A,5)+T;            } while(0)

# define BODY_20_39(xa,xb,xc,xd)  do {   \
Xupdate(T,xa,xa,xb,xc,xd);      \
T+=E+K_20_39+F_20_39(B,C,D);    \
E=D, D=C, C=ROTATE(B,30), B=A;  \
A=ROTATE(A,5)+T;            } while(0)

# define BODY_40_59(xa,xb,xc,xd)  do {   \
Xupdate(T,xa,xa,xb,xc,xd);      \
T+=E+K_40_59+F_40_59(B,C,D);    \
E=D, D=C, C=ROTATE(B,30), B=A;  \
A=ROTATE(A,5)+T;            } while(0)

# define BODY_60_79(xa,xb,xc,xd)  do {   \
Xupdate(T,xa,xa,xb,xc,xd);      \
T=E+K_60_79+F_60_79(B,C,D);     \
E=D, D=C, C=ROTATE(B,30), B=A;  \
A=ROTATE(A,5)+T+xa;         } while(0)


#define HASH_MAKE_STRING(c,s)   do {    \
unsigned long ll;               \
ll=(c)->h0; HOST_l2c(ll,(s));   \
ll=(c)->h1; HOST_l2c(ll,(s));   \
ll=(c)->h2; HOST_l2c(ll,(s));   \
ll=(c)->h3; HOST_l2c(ll,(s));   \
ll=(c)->h4; HOST_l2c(ll,(s));   \
} while (0)

static void HASH_BLOCK_DATA_ORDER(SHA_CTX *c, const void *p, size_t num)
{
    const unsigned char *data = (const unsigned char*)p;
    register unsigned int A, B, C, D, E, T, l;
    int i;
    SHA_LONG X[16];
    
    A = c->h0;
    B = c->h1;
    C = c->h2;
    D = c->h3;
    E = c->h4;
    
    for (;;) {
        for (i = 0; i < 16; i++) {
            HOST_c2l(data, l);
            X[i] = l;
            BODY_00_15(X[i]);
        }
        for (i = 0; i < 4; i++) {
            BODY_16_19(X[i], X[i + 2], X[i + 8], X[(i + 13) & 15]);
        }
        for (; i < 24; i++) {
            BODY_20_39(X[i & 15], X[(i + 2) & 15], X[(i + 8) & 15],
                       X[(i + 13) & 15]);
        }
        for (i = 0; i < 20; i++) {
            BODY_40_59(X[(i + 8) & 15], X[(i + 10) & 15], X[i & 15],
                       X[(i + 5) & 15]);
        }
        for (i = 4; i < 24; i++) {
            BODY_60_79(X[(i + 8) & 15], X[(i + 10) & 15], X[i & 15],
                       X[(i + 5) & 15]);
        }
        
        c->h0 = (c->h0 + A) & 0xffffffffL;
        c->h1 = (c->h1 + B) & 0xffffffffL;
        c->h2 = (c->h2 + C) & 0xffffffffL;
        c->h3 = (c->h3 + D) & 0xffffffffL;
        c->h4 = (c->h4 + E) & 0xffffffffL;
        
        if (--num == 0)
            break;
        
        A = c->h0;
        B = c->h1;
        C = c->h2;
        D = c->h3;
        E = c->h4;
    }
}

int hash_final(unsigned char *md, SHA_CTX *c)
{
    unsigned char *p = (unsigned char *)c->data;
    size_t n = c->num;
    
    p[n] = 0x80;                /* there is always room for one */
    n++;
    
    if (n > (HASH_CBLOCK - 8)) {
        memset(p + n, 0, HASH_CBLOCK - n);
        n = 0;
        HASH_BLOCK_DATA_ORDER(c, p, 1);
    }
    memset(p + n, 0, HASH_CBLOCK - 8 - n);
    
    p += HASH_CBLOCK - 8;
    (void)HOST_l2c(c->Nh, p);
    (void)HOST_l2c(c->Nl, p);
    p -= HASH_CBLOCK;
    HASH_BLOCK_DATA_ORDER(c, p, 1);
    c->num = 0;
    memset(p, 0, HASH_CBLOCK);
    
    HASH_MAKE_STRING(c, md);
    return 1;
}

int hash_update(SHA_CTX *c, const void *data_, size_t len)
{
    const unsigned char *data = (const unsigned char *)data_;
    unsigned char *p;
    HASH_LONG l;
    size_t n;
    
    if (len == 0)
        return 1;
    
    l = (c->Nl + (((HASH_LONG) len) << 3)) & 0xffffffffUL;
    /*
     * 95-05-24 eay Fixed a bug with the overflow handling, thanks to Wei Dai
     * <weidai@eskimo.com> for pointing it out.
     */
    if (l < c->Nl)              /* overflow */
        c->Nh++;
    c->Nh += (HASH_LONG) (len >> 29); /* might cause compiler warning on
                                       * 16-bit */
    c->Nl = l;
    
    n = c->num;
    if (n != 0) {
        p = (unsigned char *)c->data;
        
        if (len >= HASH_CBLOCK || len + n >= HASH_CBLOCK) {
            memcpy(p + n, data, HASH_CBLOCK - n);
            HASH_BLOCK_DATA_ORDER(c, p, 1);
            n = HASH_CBLOCK - n;
            data += n;
            len -= n;
            c->num = 0;
            memset(p, 0, HASH_CBLOCK); /* keep it zeroed */
        } else {
            memcpy(p + n, data, len);
            c->num += (unsigned int)len;
            return 1;
        }
    }
    
    n = len / HASH_CBLOCK;
    if (n > 0) {
        HASH_BLOCK_DATA_ORDER(c, data, n);
        n *= HASH_CBLOCK;
        data += n;
        len -= n;
    }
    
    if (len != 0) {
        p = (unsigned char *)c->data;
        c->num = (unsigned int)len;
        memcpy(p,data,len);
    }
    return 1;
}

void* sha1(void* message, int message_len, void* digest)//struct element_list** input, int n_ways, struct element_list* output)
{
    SHA_CTX *c = (SHA_CTX *)malloc(sizeof (SHA_CTX));
    
    unsigned char m[64];
    memset(c,0,sizeof(*c));
    int i=0;
    unsigned char* tmp = (unsigned char *)message;
    
    c->h0 = INIT_DATA_h0;
    c->h1 = INIT_DATA_h1;
    c->h2 = INIT_DATA_h2;
    c->h3 = INIT_DATA_h3;
    c->h4 = INIT_DATA_h4;
    
    hash_update(c,message,message_len);
    memset(m,0,64);
    
    hash_final(&(m[0]),c);
    memcpy(digest,m,20);
    //  printf("%s finished hashing\n",__func__);
    //  for (i=0;i<20;i++)
    //    printf("%p ",m[i]);
    //  printf("\n");
    if (c!=NULL)
        free(c);
}





int trivial_count(int* a, int len){
	list<int> la(a,a+len);
	la.sort();
	la.unique();
	return la.size();
}
/*
int find_first_zero(const map<int,int> &m){
	//return least bit which is 0
	auto it=m.begin();
	while(it!=m.end()){
		if(it->second==0)
			return (it->first);
		it++;
	}
	return 0;
}
*/

int find_first_zero(NobArray_p nob){
	int i = 1;
	while(i<nob->len){
		if(nob_read_at(nob,i)==0)
			return i-1;
		i++;
	}
}


int least1(char* digest){
	for(int index=19;index>=0; index--){
		char c=digest[index];
		if((c&1) != 0)
			return (19-index)*8+1;
		if((c&2) != 0)
			return (19-index)*8+2;
		if((c&4) != 0)
			return (19-index)*8+3;
		if((c&8) != 0)
			return (19-index)*8+4;
		if((c&16)!= 0)
			return (19-index)*8+5;
		if((c&32)!= 0)
			return (19-index)*8+6;
		if((c&64)!= 0)
			return (19-index)*8+7;
		if((c&128)!=0)
			return (19-index)*8+8;
	}
	cout<< "error least!" <<endl;
	return 0;
}
int least2(char* digest){
	for(int index=0;index<20; index++){
		char c=digest[index];
		if((c&1) != 0)
			return index*9+1;
		if((c&2) != 0)
			return index*8+2;
		if((c&4) != 0)
			return index*8+3;
		if((c&8) != 0)
			return index*8+4;
		if((c&16)!= 0)
			return index*8+5;
		if((c&32)!= 0)
			return index*8+6;
		if((c&64)!= 0)
			return index*8+7;
		if((c&128)!=0)
			return index*8+8;
	}
	cout<< "error least!" <<endl;
	return 0;
}

void print_nob_array(NobArray_p a){
	for(int i =0;i<a->len;i++)
		printf("bitmap[%d] = %d\n",i,nob_read_at(a,i));
}

static void _sketch(int32_t* input, int32_t* output,int32_t len)
{
	CMO_p rt = init_cmo_runtime();
	NobArray_p out = init_nob_array(rt,output,1);
	ReadObIterator_p it_raw = init_read_ob_iterator(rt,input,len);
	begin_leaky_sec(rt);
	int bm[20];
	memset(bm,0,20*4);
	NobArray_p b = init_nob_array(rt,bm,20);

	for(int i=0;i<len;i++){
		char message[100];
		//printf("message addr = %p",message);
		int raw_i = ob_read_next(it_raw);
		memcpy(message,&raw_i,4);
		char digest[20];
		//printf("digest addr = %p",digest);
		sha1(message,4,digest);


		int l = least1(digest);
	//	b[l]= 1;
		nob_write_at(b,l,1);
	//	cout<<"first 1 for hash of "<<raw_i<<" is "<<l<<endl;
	}

	//print_nob_array(b);
	int r=find_first_zero(b);
	//cout<<"the position of first 0 in bit map is "<<r<<endl;

	if(r==0)
		cout<<"unexcepted error!"<<endl;
	else{
		double ar = 0.77;
		int ans = pow(2,r)/ar;
		//cout<<"the result is "<< ans <<endl;
		nob_write_at(out,0,ans);
	}
	end_leaky_sec(rt);
        free_cmo_runtime(rt);
	//cout<< "The trivial count is "<< trivial_count(input,len) <<endl;

}

void sketch(int32_t* input, int32_t* output,int32_t len)
{
    _sketch(input,output,len);
}
