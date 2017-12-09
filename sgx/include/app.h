#ifndef SGX_APP_H
#define SGX_APP_H

#include <sgx_urts.h>

int initialize_enclave(const char* enclave_file, const char* token_file_name,
                       sgx_enclave_id_t* eid);

void ocall_abort();
void ocall_print(const char* str);

#endif
