#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <openssl/aes.h>
#include <openssl/rand.h>

typedef struct ticket { /* test field */
int ticketId;
char username[20];
    char date[20];
} USR_TICKET;

// a simple hex-print routine. could be modified to print 16 bytes-per-line
static void hex_print(const void* pv, size_t len)
{
const unsigned char * p = (const unsigned char*)pv;
if (NULL == pv)
    printf("NULL");
else
{
    size_t i = 0;
    for (; i<len;++i)
        printf("%04X ", *p++);
}
printf("\n");
}

// main entrypoint
int main(int argc, char **argv)
{
    int keylength;
    printf("Give a key length [only 128 or 192 or 256!]:\n");
    scanf("%d", &keylength);

    /* generate a key with a given length */
    unsigned char aes_key[keylength/8];
    memset(aes_key, 0, keylength/8);
    if (!RAND_bytes(aes_key, keylength/8))
        exit(-1);

    /* input struct creation */
    size_t inputslength = sizeof(USR_TICKET);
    USR_TICKET ticket;
    ticket.ticketId = 1;
    time_t now = time(NULL);
    strftime(ticket.date, 20, "%Y-%m-%d", localtime(&now));
    strcpy(ticket.username, "Hello AES");

    printf("Username - %s\n", ticket.username);
    printf("Ticket Id - %d\n", ticket.ticketId);
    printf("Date - %s\n", ticket.date);

    /* init vector */
    unsigned char iv_enc[AES_BLOCK_SIZE], iv_dec[AES_BLOCK_SIZE];
    RAND_bytes(iv_enc, AES_BLOCK_SIZE);
    memcpy(iv_dec, iv_enc, AES_BLOCK_SIZE);

    // buffers for encryption and decryption
    const size_t encslength = ((inputslength + AES_BLOCK_SIZE) / AES_BLOCK_SIZE) * AES_BLOCK_SIZE;
    unsigned char enc_out[encslength];
    unsigned char dec_out[inputslength];
    memset(enc_out, 0, sizeof(enc_out));
    memset(dec_out, 0, sizeof(dec_out));

    // so i can do with this aes-cbc-128 aes-cbc-192 aes-cbc-256
    AES_KEY enc_key, dec_key;
    AES_set_encrypt_key(aes_key, keylength, &enc_key);
    AES_cbc_encrypt((unsigned char *)&ticket, enc_out, encslength, &enc_key, iv_enc, AES_ENCRYPT);

    AES_set_decrypt_key(aes_key, keylength, &dec_key);
    AES_cbc_encrypt(enc_out, dec_out, encslength, &dec_key, iv_dec, AES_DECRYPT);

    printf("original:\t");
    hex_print((unsigned char *)&ticket, inputslength);

    printf("encrypt:\t");
    hex_print(enc_out, sizeof(enc_out));

    printf("decrypt:\t");
    hex_print(dec_out, sizeof(dec_out));

    USR_TICKET * dyc = (USR_TICKET *)dec_out;
    printf("Username - %s\n", dyc->username);
    printf("Ticket Id - %d\n", dyc->ticketId);
    printf("Date - %s\n", dyc->date);
    return 0;
}
