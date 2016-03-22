#include <stdio.h>
#include <stdlib.h>
#include <openssl/bn.h>
#include <stdlib.h>
#include <assert.h>

int RSA_EncDec_block(const void *message, size_t m_size, BIGNUM *key, BIGNUM *n, void **output, size_t *output_size)
{
    assert(m_size > 0 && message != 0 && key != 0 && output != 0 && output_size);
    BIGNUM *bnm = BN_bin2bn(message, m_size, NULL);
    int status = 0;
    if (bnm != NULL) {
        BIGNUM *out = BN_new();
        BN_CTX *ctx = BN_CTX_new();
        status = BN_mod_exp(out, bnm, key, n, ctx);
        if (status != 0) {
            int size = BN_num_bytes(out);
            *output_size = size;
            *output = malloc(size);
            *output_size = BN_bn2bin(out, *output);
            status = 1;
        }
        BN_free(out);
        BN_CTX_free(ctx);
    }
    return status;
}
