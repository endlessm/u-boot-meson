#include <common.h>
#include "usb_pcd.h"


#if defined(WRITE_TO_NAND_ENABLE)
#include <common.h>
#include <linux/ctype.h>
#include <linux/mtd/mtd.h>
#include <command.h>
#include <watchdog.h>
#include <malloc.h>
#include <asm/byteorder.h>
#include <jffs2/jffs2.h>
#include <nand.h>
#include <asm/arch/nand.h>
#include <linux/types.h>
#include <div64.h>
#include <linux/err.h>
char namebuf[20];
char databuf[4096];
//char listkey[1024];
int secukey_inited=0;
extern ssize_t uboot_key_init();
extern ssize_t uboot_get_keylist(char *keyname);
extern ssize_t uboot_key_read(char *keyname, char *keydata);
extern ssize_t uboot_key_write(char *keyname, char *keydata);
extern int nandkey_provider_register();
extern int key_set_version(char *device);
int ensure_secukey_init(void);
int cmd_secukey(int argc, char * const argv[], char *buf);
#endif

#if defined(WRITE_TO_EFUSE_ENABLE) || defined(WRITE_TO_NAND_ENABLE)
#define WRITE_TO_EFUSE_OR_NAND_ENABLE  1
#endif

#define SECUKEY_BYTES     512
#if defined(WRITE_TO_EFUSE_OR_NAND_ENABLE)
//test efuse read
#define EFUSE_READ_TEST_ENABLE									//enable efuse read test after efuse write success

//efuse version
#define EFUSE_VERSION_MESON3						"01:02:03"		//m3 version:01:xx:xx, machine id  sets 0x02:0x03
#define EFUSE_VERSION_MESON6						"02"				//m6 version:02

//extern 
#ifdef WRITE_TO_EFUSE_ENABLE
#ifdef CONFIG_AML_MESON3
extern int do_efuse_usb(int argc, char * const argv[], char *buf);			//extern in m3 platform
#elif defined(CONFIG_AML_MESON6)
extern int cmd_efuse(int argc, char * const argv[], char *buf);			//extern in m6 platform
#endif
#endif

/* hdcp 数据校验代码*/
#define DWORD unsigned int  //4 bytes
#define BYTE unsigned char   //1 byte
#define SHA1_MAC_LEN 20

typedef struct {
        DWORD state[5];
        DWORD count[2];
        BYTE buffer[64];
} SHA1_CTX;

void SHA1Reset(SHA1_CTX *context);
void SHA1Input(SHA1_CTX *context, BYTE *data, DWORD len);
void SHA1Result(SHA1_CTX *context, BYTE *digest);//20
void SHA1Transform_H(DWORD *state, BYTE *buffer); //5  64

#define rol(value, bits) (((value) << (bits)) | ((value) >> (32 - (bits))))

/* blk0() and blk() perform the initial expand. */
/* I got the idea of expanding during the round function from SSLeay */

#define blk0(i) (block->l[i] = (rol(block->l[i], 24) & 0xFF00FF00) | \
        (rol(block->l[i], 8) & 0x00FF00FF))
#define blk(i) (block->l[i & 15] = rol(block->l[(i + 13) & 15] ^ \
        block->l[(i + 8) & 15] ^ block->l[(i + 2) & 15] ^ block->l[i & 15], 1))

/* (R0+R1), R2, R3, R4 are the different operations used in SHA1 */
#define R0(v,w,x,y,z,i) \
        z += ((w & (x ^ y)) ^ y) + blk0(i) + 0x5A827999 + rol(v, 5); \
        w = rol(w, 30);
#define R1(v,w,x,y,z,i) \
        z += ((w & (x ^ y)) ^ y) + blk(i) + 0x5A827999 + rol(v, 5); \
        w = rol(w, 30);
#define R2(v,w,x,y,z,i) \
        z += (w ^ x ^ y) + blk(i) + 0x6ED9EBA1 + rol(v, 5); w = rol(w, 30);
#define R3(v,w,x,y,z,i) \
        z += (((w | x) & y) | (w & x)) + blk(i) + 0x8F1BBCDC + rol(v, 5); \
        w = rol(w, 30);
#define R4(v,w,x,y,z,i) \
        z += (w ^ x ^ y) + blk(i) + 0xCA62C1D6 + rol(v, 5); \
        w=rol(w, 30);

/* Hash a single 512-bit block. This is the core of the algorithm. */
void SHA1Transform_H(DWORD *state, BYTE *buffer)
{
        DWORD a, b, c, d, e;
        typedef union {
                BYTE c[64];
                DWORD l[16];
        } CHAR64LONG16;
        CHAR64LONG16 *block;

        DWORD workspace[16];
        block = (CHAR64LONG16 *)workspace;
        memcpy(block, buffer, 64);

        /* Copy context->state[] to working vars */
        a = state[0];
        b = state[1];
        c = state[2];
        d = state[3];
        e = state[4];
        /* 4 rounds of 20 operations each. Loop unrolled. */
        R0(a,b,c,d,e, 0); R0(e,a,b,c,d, 1); R0(d,e,a,b,c, 2); R0(c,d,e,a,b, 3);
        R0(b,c,d,e,a, 4); R0(a,b,c,d,e, 5); R0(e,a,b,c,d, 6); R0(d,e,a,b,c, 7);
        R0(c,d,e,a,b, 8); R0(b,c,d,e,a, 9); R0(a,b,c,d,e,10); R0(e,a,b,c,d,11);
        R0(d,e,a,b,c,12); R0(c,d,e,a,b,13); R0(b,c,d,e,a,14); R0(a,b,c,d,e,15);
        R1(e,a,b,c,d,16); R1(d,e,a,b,c,17); R1(c,d,e,a,b,18); R1(b,c,d,e,a,19);
        R2(a,b,c,d,e,20); R2(e,a,b,c,d,21); R2(d,e,a,b,c,22); R2(c,d,e,a,b,23);
        R2(b,c,d,e,a,24); R2(a,b,c,d,e,25); R2(e,a,b,c,d,26); R2(d,e,a,b,c,27);
        R2(c,d,e,a,b,28); R2(b,c,d,e,a,29); R2(a,b,c,d,e,30); R2(e,a,b,c,d,31);
        R2(d,e,a,b,c,32); R2(c,d,e,a,b,33); R2(b,c,d,e,a,34); R2(a,b,c,d,e,35);
        R2(e,a,b,c,d,36); R2(d,e,a,b,c,37); R2(c,d,e,a,b,38); R2(b,c,d,e,a,39);
        R3(a,b,c,d,e,40); R3(e,a,b,c,d,41); R3(d,e,a,b,c,42); R3(c,d,e,a,b,43);
        R3(b,c,d,e,a,44); R3(a,b,c,d,e,45); R3(e,a,b,c,d,46); R3(d,e,a,b,c,47);
        R3(c,d,e,a,b,48); R3(b,c,d,e,a,49); R3(a,b,c,d,e,50); R3(e,a,b,c,d,51);
        R3(d,e,a,b,c,52); R3(c,d,e,a,b,53); R3(b,c,d,e,a,54); R3(a,b,c,d,e,55);
        R3(e,a,b,c,d,56); R3(d,e,a,b,c,57); R3(c,d,e,a,b,58); R3(b,c,d,e,a,59);
        R4(a,b,c,d,e,60); R4(e,a,b,c,d,61); R4(d,e,a,b,c,62); R4(c,d,e,a,b,63);
        R4(b,c,d,e,a,64); R4(a,b,c,d,e,65); R4(e,a,b,c,d,66); R4(d,e,a,b,c,67);
        R4(c,d,e,a,b,68); R4(b,c,d,e,a,69); R4(a,b,c,d,e,70); R4(e,a,b,c,d,71);
        R4(d,e,a,b,c,72); R4(c,d,e,a,b,73); R4(b,c,d,e,a,74); R4(a,b,c,d,e,75);
        R4(e,a,b,c,d,76); R4(d,e,a,b,c,77); R4(c,d,e,a,b,78); R4(b,c,d,e,a,79);
        /* Add the working vars back into context.state[] */
        state[0] += a;
        state[1] += b;
        state[2] += c;
        state[3] += d;
        state[4] += e;
        /* Wipe variables */
        a = b = c = d = e = 0;

        memset(block, 0, 64);
}


/* SHA1Reset - Initialize new context */

void SHA1Reset(SHA1_CTX *context)
{
        /* SHA1 initialization constants */
        context->state[0] = 0x67452301;
        context->state[1] = 0xEFCDAB89;
        context->state[2] = 0x98BADCFE;
        context->state[3] = 0x10325476;
        context->state[4] = 0xC3D2E1F0;
        context->count[0] = context->count[1] = 0;
}


/* Run your data through this. */

void SHA1Input(SHA1_CTX* context, BYTE *_data, DWORD len)
{
        DWORD i, j;
        BYTE *data = _data;

        j = (context->count[0] >> 3) & 63;
        if ((context->count[0] += len << 3) < (len << 3))
                context->count[1]++;
        context->count[1] += (len >> 29);
        if ((j + len) > 63) {
                memcpy(&context->buffer[j], data, (i = 64-j));
                SHA1Transform_H(context->state, context->buffer);
                for ( ; i + 63 < len; i += 64) {
                        SHA1Transform_H(context->state, &data[i]);
                }
                j = 0;
        }
        else i = 0;
        memcpy(&context->buffer[j], &data[i], len - i);

}


/* Add padding and return the message digest. */

void SHA1Result(SHA1_CTX *context, BYTE *digest)
{
        DWORD i;
        BYTE finalcount[8];

        for (i = 0; i < 8; i++) {
                finalcount[i] = (BYTE)
                        ((context->count[(i >= 4 ? 0 : 1)] >>
                          ((3-(i & 3)) * 8) ) & 255);  /* Endian independent */
        }
        SHA1Input(context, (BYTE *) "\200", 1);
        while ((context->count[0] & 504) != 448) {
                SHA1Input(context, (BYTE *) "\0", 1);
        }
        SHA1Input(context, finalcount, 8);  /* Should cause a SHA1Transform_H()
                                              */
        for (i = 0; i < 20; i++) {
                digest[i] = (BYTE)
                        ((context->state[i >> 2] >> ((3 - (i & 3)) * 8)) &
                         255);
        }
        /* Wipe variables */
        i = 0;
        memset(context->buffer, 0, 64);
        memset(context->state, 0, 20);
        memset(context->count, 0, 8);
        memset(finalcount, 0, 8);
}
 /**************************************************************************
 * NOTES:       Test Vectors (from FIPS PUB 180-1) to verify implementation
 *              1- Input : "abc"
 *              Output : A9993E36 4706816A BA3E2571 7850C26C 9CD0D89D
 *              2- Input : "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq"
 *              Output : 84983E44 1C3BD26E BAAE4AA1 F95129E5 E54670F1
 *              2- Input : A million repetitions of 'a' - not applied (memory shortage)
 *              Output : 34AA973C D4C4DAA4 F61EEB2B DBAD2731 6534016F
 *              More test vectors can be obtained from FIPS web site
 ***************************************************************************/
void SHA1_Perform(BYTE *indata, DWORD inlen, BYTE *outdata) //calculate SHA-1 API
{
    SHA1_CTX sha;
    SHA1Reset(&sha);
    SHA1Input(&sha, indata, inlen);	
    SHA1Result(&sha, outdata);	
} 

typedef struct 
{
    unsigned char ksv[5];
    unsigned char rsv[3];
    unsigned char dpk[280];
    unsigned char sha[20];
}hdcp_llc_file;
#endif    /* WRITE_TO_EFUSE_OR_NAND_ENABLE */

int burn_board(const char *dev, void *mem_addr, u64 offset, u64 size)
{
	char	str[128];
	printf("burn_board!!!\n");
	printf("CMD: dev=%s, mem_addr=0x%x, offset=0xllx, size=0x%llx\n", dev, mem_addr, offset, size);
	if(!strncmp("nand", *dev, 4))
	{
		sprintf(str, "nand erase 0x%llx 0x%llx}", offset, size);
		printf("command:    %s\n", str);
		run_command(str, 0);
		sprintf(str, "nand write 0x%x 0x%llx 0x%llx}", mem_addr, offset, size);
		printf("command:    %s\n", str);
		run_command(str, 0);
	}
	else if(!strncmp("spi", *dev, 3))
	{
		run_command("sf probe 2", 0);
		sprintf(str, "sf erase 0x%llx 0x%llx}", offset, size);
		printf("command:    %s\n", str);
		run_command(str, 0);
		sprintf(str, "sf write 0x%x 0x%llx 0x%llx}", mem_addr, offset, size);
		printf("command:    %s\n", str);
		run_command(str, 0);
	}
	else if(!strncmp("emmc", *dev, 4))
	{
		sprintf(str, "mmc write 1 0x%x 0x%llx 0x%llx}", mem_addr, offset, size);
		printf("command:    %s\n", str);
		run_command(str, 0);
	}
	else
	{
		printf("Invalid Argument!\n");
		return -1;
	}
	return 0;
}

static int usb_bootm(const void *addr)
{
	char cmd[128];
	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd, "bootm %x", addr);
	return run_command(cmd, 0);
}

u32 checkcum_32(const unsigned char *buf, u32 len)
{
	u32 fake_len, chksum = 0;
	u32 *ptr = buf;
	int i;
	printf("buf=0x%08x, len=0x%x\n", buf, len);
	if(len%4)
	{
		fake_len = len - len%4 + 4;
		memset((buf+len), 0, (fake_len-len));
	}
	else
	{
		fake_len = len;
	}
	printf("fake_len=0x%x\n", fake_len);
	for(i=0; i<fake_len; i+=4, ptr++)
	{
		chksum += *ptr;
	}
	return chksum;
}


int usb_run_command (const char *cmd, char* buff)
{
	int ret = -1, flag = 0;
	u32 addr = 0, length = 0;
	u32 crc_value, crc_verify = 0;
	int argc;
	char *argv[CONFIG_SYS_MAXARGS + 1];	/* NULL terminated	*/
	unsigned long upgrade_step;
	printf("cmd: %s\n", cmd);
	
	memset(buff, 0, CMD_BUFF_SIZE);
	if(strncmp(cmd,"get update result",(sizeof("get update result")-1)) == 0){
		ret = usb_get_update_result();
		if(!ret)
		{
			strcpy(buff, "success");
		}
		else
		{
			strcpy(buff, "fail");
		}
		return ret;
	}
	else if(strncmp(cmd,"usb_bootm",(sizeof("usb_bootm")-1)) == 0){
		addr = *((u32*)(&cmd[60]));
		strcpy(buff, "okay");
		usb_bootm(addr);
		strcpy(buff, "fail");
		return -1;
	}
	else if(strncmp(cmd,"crc",(sizeof("crc")-1)) == 0){
		if ((argc = parse_line (cmd, argv)) == 0) {
			return -1;	/* no command at all */
		}
		addr = simple_strtoul (argv[1], NULL, 16);
		length = simple_strtoul (argv[2], NULL, 10);
		crc_verify = simple_strtoul (argv[3], NULL, 16);
		//crc_value = crc32 (0, (const uchar *) addr, length);
		crc_value = checkcum_32(addr, length);
		printf("crc_value=0x%x\n", crc_value);
		if(crc_verify == crc_value)
		{
			strcpy(buff, "success");
		}
		else
		{
			strcpy(buff, "failed");
		}
	}
	else if(strncmp(cmd,"cmd_in_mem",(sizeof("cmd_in_mem")-1)) == 0){
		char *cmd_in_mem = NULL;
		/* Extract arguments */
		if ((argc = parse_line (cmd, argv)) == 0) {
			return -1;	/* no command at all */
		}
		cmd_in_mem = (char *)simple_strtoul(argv[1], NULL, 0);
		printf("cmd_in_mem: %s\n", cmd_in_mem);
		if(run_command(cmd_in_mem, flag))
		{
			strcpy(buff, "fail");
			return -1;
		}
		else
		{
			strcpy(buff, "okay");
		}
	}
#if defined(WRITE_TO_EFUSE_OR_NAND_ENABLE)
	/*
		//efuse read/write command
		"efuse read version"	 	//cmd: efuse read version
		"efuse write version"	 	//cmd: efuse write version
		"efuse read mac"		 	//cmd: efuse read mac
		"efuse write mac "		 	//cmd: efuse write mac xx:xx:xx:xx:xx:xx
		"efuse read bt_mac"	 	//cmd: efuse read bt_mac
		"efuse write bt_mac "	 	//cmd: efuse write bt_mac xx:xx:xx:xx:xx:xx
		"efuse read wifi_mac"	 	//cmd: efuse read wifi_mac
		"efuse write wifi_mac "	 	//cmd: efuse write wifi_mac xx:xx:xx:xx:xx:xx
		"efuse read usid"	 		//cmd: efuse read usid
		"efuse write usid "	 		//cmd: efuse write usid xxxxx...
		"read hdcp"				//cmd: read hdcp
		"write hdcp:"			 	//cmd: write hdcp:xxxxx...		
	*/
	else if( !strncmp(cmd,"efuse",(sizeof("efuse")-1))  || 
		      !strncmp(cmd,"read hdcp",(sizeof("read hdcp")-1))  || 
		      !strncmp(cmd,"write hdcp:",(sizeof("write hdcp:")-1))){
		int argc;
		int i, ret, usid_flag = 0, hdcp_flag = 0, flag = 0;
		int usid_len, hdcp_len, len;
		int writeHdcp_flag = 1;
		char efuse_data[512], test_buff[512], hdcp_verify_data[512];
#ifdef  WRITE_TO_EFUSE_ENABLE	
		char *Argv1[3]={"efuse","read","hdcp"};
		char *Argv2[4]={"efuse","write","hdcp",""};
#elif defined(WRITE_TO_NAND_ENABLE)		
		char *Argv1[3]={"secukey","read","hdcp"};
		char *Argv2[4]={"secukey","write","hdcp",""};	
#endif		
		char *hdcp;
		char *argv[CONFIG_SYS_MAXARGS + 1];	/* NULL terminated	*/

		/* Extract arguments */
		if ((argc = parse_line (cmd, argv)) == 0) {
			return -1;	/* no command at all */
		}

#ifdef  WRITE_TO_EFUSE_ENABLE
		printf("usb_burning to efuse\n");
#elif defined(WRITE_TO_NAND_ENABLE)	
		printf("usb_burning to nand\n");
#endif
		memset(efuse_data,0,sizeof(efuse_data));
		memset(hdcp_verify_data,0,sizeof(hdcp_verify_data));
		
		//m3/m6 version
		if((!strncmp(argv[1],"write",sizeof("write"))) &&  (!strncmp(argv[2],"version",sizeof("version"))))
		{
			argc ++;	
			#ifdef CONFIG_AML_MESON3
				argv[3] = EFUSE_VERSION_MESON3;				//m3 version
				printf("CONFIG_AML_MESON3(version:%s)\n",argv[3]);
			#elif defined(CONFIG_AML_MESON6) 
				argv[3] = EFUSE_VERSION_MESON6;				//m6 version
				printf("CONFIG_AML_MESON6(version:%s)\n",argv[3]);
			#endif
		}


		//bt_mac/wifi_mac -->mac_bt/mac_wifi
		if(((!strncmp(argv[1],"read",sizeof("read")))||(!strncmp(argv[1],"write",sizeof("write")))) &&  
		      (!strncmp(argv[2],"bt_mac",sizeof("bt_mac"))))
			strcpy(argv[2], "mac_bt");
		else if(((!strncmp(argv[1],"read",sizeof("read")))||(!strncmp(argv[1],"write",sizeof("write")))) &&
	      		      (!strncmp(argv[2],"wifi_mac",sizeof("wifi_mac"))))
	      		 strcpy(argv[2], "mac_wifi"); 


		//hdcp
		if((!strncmp(argv[0],"read",sizeof("read"))) &&  (!strncmp(argv[1],"hdcp",sizeof("hdcp"))))
		{
			argv[0] = Argv1[0];
			argv[1] = Argv1[1];
			argv[2] = Argv1[2];
			argc = 3;
		}
		else if((!strncmp(argv[0],"write",sizeof("write"))) &&  (!strncmp(argv[1],"hdcp:",sizeof("hdcp:"))))
		{
			#define HDCP_DATA_ADDR	(volatile unsigned long *)(0x82000000)	//get hdcp data from address:0x82000000 
			hdcp = HDCP_DATA_ADDR;
					
			printf("receive hdcp_data=");
			for(i=0;i<288;i++)												//read 288 hdcp datas		
			{
				efuse_data[i] = *hdcp++;
				printf("%.2x:", efuse_data[i]);
			}	
			printf("\n");
			
			printf("receive hdcp_verify_data=");
			for(i=0;i<20;i++)													//read 20 hdcp verify datas		
			{
				hdcp_verify_data[i] = *hdcp++;
				printf("%.2x:", hdcp_verify_data[i]);
			}	
			printf("\n");

#ifdef  WRITE_HDCP_VERIFY_ENABLE	
		    	SHA1_CTX sha;
		    	BYTE Message_Digest[20];
		    	hdcp_llc_file *llc_key;

			memset(Message_Digest, 0, sizeof(Message_Digest));
			llc_key = (hdcp_llc_file *)efuse_data;

			printf("llc_key->ksv=");
			for(i=0;i<5;i++)
				printf("%.2x:", llc_key->ksv[i]);	
				
			printf("\nllc_key->rsv=");
			for(i=0;i<3;i++)
				printf("%.2x:", llc_key->rsv[i]);
				
			printf("\nllc_key->dpk=");
			for(i=0;i<280;i++)
				printf("%.2x:", llc_key->dpk[i]);
				
			printf("\nllc_key->sha=");
			for(i=0;i<20;i++)
				printf("%.2x:", llc_key->sha[i]);
			printf("\n");


			printf("start to verify hdcp data...\n");
		    	SHA1Reset(&sha);
		    	SHA1Input(&sha, (unsigned char*)llc_key, 288);
			SHA1Result(&sha, Message_Digest);

			printf("calculate hdcp_verify_data=");
			for(i=0; i<20; i++)
				printf("%.2x:", Message_Digest[i]);
			printf("\n");

			for(i=0; i<20; i++)
			{
				if(hdcp_verify_data[i] != Message_Digest[i])
				{
					writeHdcp_flag = 0;
					break;
				}
			}
#endif
			if(writeHdcp_flag)			//verify success,hdcp can write
			{
				memcpy(Argv2[3], efuse_data, SECUKEY_BYTES);					//copy hdcp datas
				argv[0] = Argv2[0];
				argv[1] = Argv2[1];
				argv[2] = Argv2[2];
				argv[3] = Argv2[3];
				argc = 4; 
			}
			else
			{
				sprintf(buff, "%s", "failed:(hdcp data verify not mach)");
				printf("%s\n",buff);
				return -1;
			}	
			
		}


#ifdef  WRITE_TO_NAND_ENABLE
		argv[0] = Argv2[0];			//if defined WRITE_TO_NAND_ENABLE,"efuse" convert to "secukey"
#endif

		//printf argv[0]--argv[argc-1]
		if((!strncmp(argv[1],"write",sizeof("write"))) &&  (!strncmp(argv[2],"hdcp",sizeof("hdcp"))))
		{
			for(i=0;i<argc-1;i++)  printf("argv[%d]=%s\n", i,argv[i]); 
			hdcp = argv[3];
			printf("argv[3]="); 
			for(i=0;i<SECUKEY_BYTES;i++)  printf("%02x:", *hdcp ++);  
			printf("\n"); 
		}
		else
			for(i=0;i<argc;i++)  printf("argv[%d]=%s\n", i,argv[i]);  
		

		
#ifdef  WRITE_TO_EFUSE_ENABLE	
#ifdef CONFIG_AML_MESON3
		ret = do_efuse_usb(argc, argv, buff);			//efuse read/write in m3 platform
#elif defined(CONFIG_AML_MESON6)
		ret = cmd_efuse(argc, argv, buff);				//efuse read/write in m6 platform
#endif
#elif defined(WRITE_TO_NAND_ENABLE)
		//write to nand
		if ((!strncmp(argv[1],"read",sizeof("read"))) && ( !strncmp(argv[2],"version",sizeof("version"))))
		{
			sprintf(buff, "%s", "failed:(version is not writen)");
			printf("%s\n",buff);
			return 0;
		}
		else if ((!strncmp(argv[1],"write",sizeof("write"))) && ( !strncmp(argv[2],"version",sizeof("version"))))
		{
			ret = ensure_secukey_init();
			if (ret==0 || ret==1)						//init nand success or already inited.
			{
				#ifdef CONFIG_AML_MESON3
					sprintf(buff, "success:(%s)", EFUSE_VERSION_MESON3);
				#elif defined(CONFIG_AML_MESON6) 
					sprintf(buff, "success:(%s)", EFUSE_VERSION_MESON6);
				#endif	
				printf("%s\n",buff);
				return 0;				
			}		
			else	 									//init nand failed!!
			{	
				printf("init nand failed!!\n");
				sprintf(buff, "failed:(write version failed)");	
				printf("%s\n",buff);	
				return -1;
			}
		}		

		ret = cmd_secukey(argc, argv, buff);									
#endif

		if(!ret)														
		{
#ifdef  WRITE_TO_EFUSE_ENABLE	
			//read version/mac/bt_mac/wifi_mac/usid/hdcp sucess 
			if(!strncmp(argv[1],"read",sizeof("read")))
			{														
				if(!strncmp(argv[2],"version",sizeof("version"))) 
				{
					#ifdef CONFIG_AML_MESON3
						sprintf(efuse_data, "%02x:%02x:%02x", buff[0],buff[1],buff[2]);		
					#elif defined(CONFIG_AML_MESON6) 
						sprintf(efuse_data, "%02x", buff[0]);
					#endif

					if(simple_strtol(&efuse_data[0], NULL, 16) != 0x00)
				    		sprintf(buff, "success:(%s)", efuse_data);
					else
						sprintf(buff, "%s", "failed:(version is not writen)");	
				
				}
				else if( !strncmp(argv[2],"mac",sizeof("mac")) || 
					     !strncmp(argv[2],"mac_bt",sizeof("mac_bt")) ||
					     !strncmp(argv[2],"mac_wifi",sizeof("mac_wifi")) )	
				{
					for(i=0;i<6;i++)
					{
						if(buff[i] != 0x00)
						{
							flag = 1;
							break;
						}		
					}
					if(flag)
					{													
						sprintf(efuse_data, "%02x:%02x:%02x:%02x:%02x:%02x", buff[0],buff[1],buff[2],buff[3],buff[4],buff[5]);	
						sprintf(buff, "success:(%s)", efuse_data);				//have writen mac/mac_bt/mac_wifi
					}						
					else
						sprintf(buff, "failed:(%s has been not writen)", argv[2]);	//haven't write mac/mac_bt/mac_wifi	
					
				}	
				else if( !strncmp(argv[2],"usid",sizeof("usid")) )	
				{
					printf("usid_len=%d, usid=%s\n", strlen(buff), buff);
					for(i=0; i<strlen(buff); i++)
					{
						if(buff[i] != 0x00)
						{
							usid_flag = 1;
							break;
						}
					}
					if(usid_flag)
					{
						//sprintf(buff, "%s", "success:(usid has been writen)");	//have writen usid
						memcpy(efuse_data, buff, strlen(buff));
						sprintf(buff, "success:(%s)", efuse_data);	
					}
					else
						sprintf(buff, "%s", "failed:(usid has been not writen)");	//haven't write usid	
						
				}
				else if( !strncmp(argv[2],"hdcp",sizeof("hdcp")) )	
				{
					printf("hdcp_data=");
					for(i=0; i<300; i++)
						printf("%.2x:",buff[i]);
					printf("\n");
					
					for(i=0; i<288; i++)
					{
						if(buff[i] != 0x00)
						{
							hdcp_flag = 1;
							break;
						}
					}
					if(hdcp_flag)
					{
						sprintf(buff, "%s", "success:(hdcp has been writen)");	//have writen hdcp
					}
					else
						sprintf(buff, "%s", "failed:(hdcp has been not writen)");	//haven't write hdcp		
				}	
			}			
#elif defined(WRITE_TO_NAND_ENABLE)		//nand
			if(!strncmp(argv[1],"read",sizeof("read")))
			{
				 if( !strncmp(argv[2],"mac",sizeof("mac")) || 
					     !strncmp(argv[2],"mac_bt",sizeof("mac_bt")) ||
					     !strncmp(argv[2],"mac_wifi",sizeof("mac_wifi")) )	
				{
					strcpy(namebuf,argv[2]);
					memset(databuf, 0, sizeof(databuf));

					len = uboot_key_read(namebuf, databuf);
					printf("%s_len = uboot_key_read(%s, databuf)\n",argv[2], namebuf);
					if(len>0)
					{
						printf("%s_len=%d\n",argv[2], len/2);
						printf("%s_data=%s\n", argv[2], buff);
						sprintf(efuse_data, "%s", buff);
						sprintf(buff, "success:(%s)", efuse_data);				 //have writen mac/mac_bt/mac_wifi
					}
					else if(len<0)
					{
						printf("%s_len=%d\n",argv[2], len);
						sprintf(buff, "failed:(%s has been not writen)", argv[2]);	//haven't write mac/mac_bt/mac_wifi													
					}	
					
				}
				else if( !strncmp(argv[2],"usid",sizeof("usid")) )	
				{
					strcpy(namebuf,"usid");
					memset(databuf, 0, sizeof(databuf));
					
					usid_len = uboot_key_read(namebuf, databuf);
					printf("usid_len = uboot_key_read(%s, databuf)\n",namebuf);
					if(usid_len>0)
					{
						printf("usid_len=%d\n",usid_len/2);
						printf("usid_data=%s\n",buff);	
						//sprintf(buff, "%s", "success:(usid has been writen)");	//have writen usid
						memcpy(efuse_data, buff, strlen(buff));
						sprintf(buff, "success:(%s)", efuse_data);	
					}	
					else if(usid_len<0)
					{
						printf("usid_len=%d\n",usid_len);
						sprintf(buff, "%s", "failed:(usid has been not writen)");	//haven't write usid	
					}	
						
				}
				else if( !strncmp(argv[2],"hdcp",sizeof("hdcp")) )	
				{	
					strcpy(namebuf,"hdcp");
					memset(databuf, 0, sizeof(databuf));

					hdcp_len = uboot_key_read(namebuf, databuf);
					printf("hdcp_len = uboot_key_read(%s, databuf)\n",namebuf);
					if(hdcp_len>0)
					{
						printf("hdcp_len=%d\n",hdcp_len/2);
						printf("hdcp_data=");
						for(i=0; i<512; i++)
							printf("%.2x:",buff[i]);
						printf("\n");		
						sprintf(buff, "%s", "success:(hdcp has been writen)");	//have writen hdcp						
					}	
					else if(hdcp_len<0)
					{
						printf("hdcp_len=%d\n",hdcp_len);
						sprintf(buff, "%s", "failed:(hdcp has been not writen)");	//haven't write hdcp							
					}	
						
				}	
			}
#endif			
			//write version/mac/bt_mac/wifi_mac/usid/hdcp sucess 
			else if( !strncmp(argv[1],"write",sizeof("write")))
			{
				if(!strncmp(argv[2],"version",sizeof("version")))			
				{
					#ifdef CONFIG_AML_MESON3
						sprintf(buff, "success:(%s)", EFUSE_VERSION_MESON3);
					#elif defined(CONFIG_AML_MESON6) 
						sprintf(buff, "success:(%s)", EFUSE_VERSION_MESON6);
					#endif					
				}
				else if( !strncmp(argv[2],"mac",sizeof("mac")) || 
					     !strncmp(argv[2],"mac_bt",sizeof("mac_bt")) || 
					     !strncmp(argv[2],"mac_wifi",sizeof("mac_wifi")) )
					sprintf(buff, "success:(%s)", argv[3]);
				else if( !strncmp(argv[2],"usid",sizeof("usid")) )	
					//sprintf(buff, "%s", "success:(write usid success)");
					sprintf(buff, "success:(%s)", argv[3]);
				else if( !strncmp(argv[2],"hdcp",sizeof("hdcp")) )	
					sprintf(buff, "%s", "success:(write hdcp success)");

//efuse write success, test efuse read
#if defined( WRITE_TO_EFUSE_ENABLE) && defined(EFUSE_READ_TEST_ENABLE)
				printf("#####efuse write %s success,now test efuse read %s:\n",argv[2], argv[2]);
				memset(test_buff,0,sizeof(test_buff));
				argv[0] 	= "efuse";	
				argv[1]  	= "read";	
				argv[2]  	= argv[2];
				argc = 3;
#ifdef CONFIG_AML_MESON3
				ret = do_efuse_usb(argc, argv, test_buff);			//efuse read/write in m3 platform
#elif defined(CONFIG_AML_MESON6)
				ret = cmd_efuse(argc, argv, test_buff);				//efuse read/write in m6 platform
#endif
				if(!ret)
				{
					if( !strncmp(argv[2],"version",sizeof("version")) )
					{
						#ifdef CONFIG_AML_MESON3
							sprintf(efuse_data, "%02x:%02x:%02x", test_buff[0],test_buff[1],test_buff[2]);		
						#elif defined(CONFIG_AML_MESON6) 
							sprintf(efuse_data, "%02x", test_buff[0]);
						#endif	
						printf("test success,version=%s\n", efuse_data);
					}	
					else if( !strncmp(argv[2],"mac",sizeof("mac")) || 
					     !strncmp(argv[2],"mac_bt",sizeof("mac_bt")) ||
					     !strncmp(argv[2],"mac_wifi",sizeof("mac_wifi")) )	
					{
						sprintf(efuse_data, "%02x:%02x:%02x:%02x:%02x:%02x", test_buff[0],test_buff[1],test_buff[2],test_buff[3],test_buff[4],test_buff[5]);
						printf("test success,%s=%s\n", argv[2], efuse_data);
					}	
					else if( !strncmp(argv[2],"usid",sizeof("usid")) ) 
					{
						printf("test success,%s=%s\n", argv[2], test_buff);
					}
					else if( !strncmp(argv[2],"hdcp",sizeof("hdcp")) ) 
					{
						printf("test success,%s=", argv[2]);
						for(i=0;i<512;i++)
							printf("%02x:", test_buff[i]);
						printf("\n");
					}						
				}
				else
					printf("test efuse read %s fail\n", argv[2]);	
#endif
			}

		}
		else														
		{
			//read version/mac/bt_mac/wifi_mac/usid/hdcp fail
			if(!strncmp(argv[1],"read",sizeof("read")))
				sprintf(buff, "failed:(read %s failed)", argv[2]);
									
			//write version/mac/bt_mac/wifi_mac/usid/hdcp fail
			else if(!strncmp(argv[1],"write",sizeof("write")))
				sprintf(buff, "failed:(write %s failed)", argv[2]);				
			
			printf("%s\n",buff);
			return -1;
		}	
	}
#endif      /* WRITE_TO_EFUSE_OR_NAND_ENABLE */	
	else
	{
		if(run_command(cmd, flag))
		{
			strcpy(buff, "fail");
			return -1;
		}
		else
		{
			strcpy(buff, "okay");
		}
	}

	printf("%s\n",buff);
	return 0;
}



#if defined(WRITE_TO_NAND_ENABLE)
int ensure_secukey_init(void)
{
	int error;
	char *cmd;

	if (secukey_inited){
		printk("nand already inited!!\n");
		return 1;
	}
	
	printk("should be inited first!\n");

	error = uboot_key_init();
	if(error >= 0){
		error = nandkey_provider_register();
		if(error >= 0){
			 error = key_set_version("nand");
			 if(error >= 0){
				printk("init key ok!!\n");
				secukey_inited = 1;
				return 0;
			}
		}
	}	
	else
	{
		printk("init error\n");
		return -1;
	}
	
	return -1;
}

char i_to_asc(char para)
{
	if(para>=0 && para<=9)
		para = para+'0';
	else if(para>=0xa && para<=0xf)
		para = para+'a'-0xa;
		
		return para;
}

char asc_to_i(char para)
{
	if(para>='0' && para<='9')
		para = para-'0';
	else if(para>='a' && para<='f')
		para = para-'a'+0xa;
	else if(para>='A' && para<='F')
		para = para-'A'+0xa;
		
		return para;
}

int cmd_secukey(int argc, char * const argv[], char *buf)
{
	int i,j, ret = 0,error;
	char *cmd;
	char *name;
	char *data;
	/* at least two arguments please */
	if (argc < 2)
		goto usage;
	cmd = argv[1];
	
	memset(buf, 0, CMD_BUFF_SIZE);
	memset(databuf, 0, sizeof(databuf));
	if (secukey_inited){
		if (argc > 2&&argc<5){
			if(!strcmp(cmd,"read")){
				if (argc>3)
					goto usage;
				name=argv[2];
				strcpy(namebuf,name);
				error=uboot_key_read(namebuf, databuf);
                for(i=0; i<CMD_BUFF_SIZE*2; i++)
                    printf(":%c", databuf[i]);
                printf("\n");
				if(error>=0){
					for (i=0,j=0; i<CMD_BUFF_SIZE*2; i++,j++){
							buf[j]= (((asc_to_i(databuf[i]))<<4) | (asc_to_i(databuf[++i])));
					}
					printf("%s is: ", namebuf);
					for(i=0; i<CMD_BUFF_SIZE; i++)
						printf(":%02x", buf[i]);
					printf("\n");
					return 0;
				}
				else{
					printk("read error!!\n");
					return -1;
				}
			}
			if(!strcmp(cmd,"write")){
				if (argc!=4)
					goto usage;
				name=argv[2];
				data=argv[3];
				strcpy(namebuf,name);
				
				for (i=0,j=0; i<SECUKEY_BYTES; i++,j++){
						databuf[j]= i_to_asc((data[i]>>4) & 0x0f);
						databuf[++j]= i_to_asc((data[i]) & 0x0f);
						printk("%02x:%02x:", databuf[j-1], databuf[j]);
				}
				printk("right here!!!\n");
				//memcpy(buf,databuf,SECUKEY_BYTES*2);
				error=uboot_key_write(namebuf, databuf);
				if(error>=0){
					printk("write key ok!!\n");
					return 0;
				}
				else{
					printk("write error!!\n");
					return -1;
				}	
			}
		}
	}
	else
		goto usage ;
		
usage:
	return 1;
}
#endif
