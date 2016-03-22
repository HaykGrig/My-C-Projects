#ifndef RSAENCDEC
#define RSAENCDEC
	int RSA_EncDec_block(const void *message, size_t m_size, BIGNUM *key, BIGNUM *n, void **output, size_t *output_size)
#endif
