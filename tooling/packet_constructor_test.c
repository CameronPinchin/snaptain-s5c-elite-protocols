#include <openssl/aes.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>


#define DST_IP              "172.19.10.0"
#define USERNAME            "guanxukeji"
#define PASSWORD            "gxrdw60"
#define FH_AES_KEY          "guanxukj@fh8620."
#define HDR_LEN             82
#define DST_PORT            8866

#define SA struct sockaddr

static int build_login_packet(uint8_t *buf)
{
    if(buf == NULL){
        return -1;
    }
    memset(buf, 0, 96);
    buf[0] = 0x00;                  // device_type
    buf[1] = 0x51;                  // header_len
    buf[2] = 0x00;                  // param_0 = 0
    buf[3] = 0x01;                  // cmd_id -- 0x01 == login
    buf[4] = 0x01;                  // seq_id
    buf[9] = 0x00;                  // padding
    strncpy((char *)&buf[10], USERNAME, 32);
    strncpy((char *)&buf[42], PASSWORD, 36);
    buf[78] = 0x0001;
    buf[82] = 0x00;

    return 83;
}

static int aes_ecb_encrypt(const uint8_t *plaintext, int len, const uint8_t *key, uint8_t *ciphertext)
{
    AES_KEY aes_key;
    AES_set_encrypt_key(key, 128, &aes_key);

    int block_count = (len + 15)/16;
    for(int i = 0; i < block_count; i++){
        AES_ecb_encrypt(plaintext + i*16, ciphertext + i*16, &aes_key, AES_ENCRYPT);
    }
    return block_count * 16;
}

static int build_wire_packet(uint8_t *ciphertext, int cipher_len, int plaintext_len, uint8_t *wire_packet)
{
    wire_packet[0] = 0x49;      // 'I'
    wire_packet[1] = 0x54;      // 'T' Magic numbers (letters) found in the library file
    int last_block_offset = ((plaintext_len - 1) / 16) * 16;
    int iVar3 = last_block_offset + 20;
    memcpy(&wire_packet[2], &iVar3, 4);
    memcpy(&wire_packet[6], &plaintext_len, 4);
    memcpy(&wire_packet[10], ciphertext, cipher_len);
    return iVar3 + 6; // final packet size of 106
}

int main()
{
    uint8_t plaintext[96] = {0};
    int plaintext_len = build_login_packet(plaintext);
    if(plaintext_len == -1){
        return EXIT_FAILURE;
    }

    uint8_t ciphertext[96] = {0};
    int ciphertext_len = aes_ecb_encrypt(plaintext, plaintext_len, (uint8_t *)FH_AES_KEY, ciphertext);

    uint8_t wire_packet[256] = {0};
    int wire_packet_len = build_wire_packet(ciphertext, ciphertext_len, plaintext_len, wire_packet);
    printf("Wire packet (%d bytes):\n", wire_packet_len);
    for (int i = 0; i < wire_packet_len; i++) {
        printf("%02x ", wire_packet[i]);
        if ((i+1) % 16 == 0) printf("\n");
    }
    printf("\n");
}





