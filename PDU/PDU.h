#pragma once

#define PDU_MAX_LOAD_SIZE 1500
#define PDU_MIN_LOAD_SIZE 46

#define PDU_QIANDAO 0xAA
#define PDU_SOF 0xAB
#ifdef PDU_ESC_ON
#define PDU_ESC 0x1B
#endif

#define PDU_ERROR_READ_FILE -1
#define PDU_ERROR_FILE_EMPTY -2

typedef enum PDU_Type_t{
    type_802_3,
    type_ethernet_v2
} PDU_Type;

typedef enum PDU_Load_Type_t{
    load_is_ipv4,
    load_is_ipv6,
    load_is_arp,
    load_is_rarp,
    load_is_any
} PDU_Load_Type;

typedef struct PDU_t{
    PDU_Type type;
    PDU_Load_Type load_type;
    char src_mac[17];
    char dst_mac[17];
    long load_length;
    char crc[4];
    unsigned char buffer[PDU_MAX_LOAD_SIZE+32];
} PDU;

PDU* new_PDU(PDU_Type type);
long pack_PDU(PDU* object,const char* source_file,const char* src_mac,const char* dst_mac,PDU_Load_Type loadtype);
void set_PDU_src_mac(const char* src_mac);
void set_PDU_dst_mac(const char* dst_mac);
void parse_PDU(PDU* object);