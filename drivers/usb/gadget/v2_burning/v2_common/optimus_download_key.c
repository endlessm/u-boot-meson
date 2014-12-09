/*
 * =====================================================================================
 *
 *       Filename:  v2_download_key.c
 *
 *        Version:  1.0
 *        Created:  2013/9/4 14:10:07
 *       Compiler:  gcc
 *
 *         Author:  Sam Wu (yihui.wu@amlogic.com)
 *   Organization:  Amlogic Inc.
 *
 *       Revision:  none
 *    Description:  Funcitions and command to burn keys with key_unify driver
 *
 * =====================================================================================
 */
#include "../v2_burning_i.h"
#include <amlogic/keyunify.h>

#ifndef CMD_BUFF_SIZE
#define CMD_BUFF_SIZE (512)
#endif// #ifndef CMD_BUFF_SIZE

int decrypt_hdcp_license_to_raw_value(const char* keyName, const u8* keyVal, const unsigned keyValLen, char* errInfo,
                              const u8** keyRealVal, unsigned* keyRealValLen,
                              char* decryptBuf, const unsigned decryptBufSz)
{
    int ret = 0;
    decryptBuf = decryptBuf;//avoid compiler warning as not used

    DWN_MSG("hdcp down in len %d\n", keyValLen);
    if(288 == keyValLen)//288 means license data is raw, not including the 20Bytes sha value
    {
        return 0;//ok, it's raw data if size is 288
    }
    else if(308 == keyValLen)
    {
        const unsigned shaSumLen = 20;
        const unsigned licLen = keyValLen - shaSumLen;
        const u8* orgSum = keyVal + licLen;
        u8 genSum[shaSumLen];

        sha1_csum((u8*)keyVal, licLen, genSum);

        ret = memcmp(orgSum, genSum, shaSumLen);
        if(ret){
            const unsigned fmtStrLen = shaSumLen * 2 + 2;
            char org_sha1Str[fmtStrLen];
            char gen_sha1Str[fmtStrLen];

            optimus_hex_data_2_ascii_str(orgSum, shaSumLen, org_sha1Str, fmtStrLen);
            optimus_hex_data_2_ascii_str(genSum, shaSumLen, gen_sha1Str, fmtStrLen);
            sprintf(errInfo, "failed:hdcp, orgSum[%s] != genSum[%s]\n", org_sha1Str, gen_sha1Str);
            DWN_ERR(errInfo);

            return EINVAL;
        }

        *keyRealValLen = licLen;
        return 0;
    }
    else
    {
        sprintf(errInfo, "failed:hdcp len %d is invalid\n", keyValLen);
        DWN_ERR(errInfo);
        return -EINVAL;
    }

    return 0;
}

int decrypt_mac_str_fmt_4_media(const char* keyName, const u8* keyVal, const unsigned keyValLen, char* errInfo,
                              const u8** keyRealVal, unsigned* keyRealValLen,
                              char* decryptBuf, const unsigned decryptBufSz)
{
    int index = 0;
    const char* keyDevice = NULL;

    if(17 != keyValLen)//288 means license data is raw, not including the 20Bytes sha value
    {
        sprintf(errInfo, "failed:mac len %d is invalid, must be 17\n", keyValLen);
        DWN_ERR(errInfo);
        return -EINVAL;
    }

    for(index = 2; index < 17; index += 3){
        const char c = keyVal[index];
        if(':' != c){
            sprintf(errInfo, "failed:L%d:mac str(%s) fmt err at index[%d]\n", __LINE__, keyVal, index);
            DWN_ERR(errInfo);
            return -EINVAL;
        }
    }
    
    for(index = 0; index < 17; index += 3){
        int k = 0;
        for(k = 0; k < 2; ++k){
            const char c = keyVal[index + k];
            if(!( (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F') )){
                sprintf(errInfo, "failed:L%d:mac str(%s) fmt err at index[%d]\n", __LINE__, keyVal, index);
                DWN_ERR(errInfo);
                return -EINVAL;
            }
        }
    }

    keyDevice = key_unify_query_key_device((char*)keyName);
    DWN_MSG("write %s as %s\n", keyName, keyDevice);
    if(strcmp("efusekey", keyDevice))//not efusekey, not need to decrypt
    {
        return 0;
    }

    *keyRealVal     = (u8*)decryptBuf;//change the keyRealVal to decryptBuf
    *keyRealValLen  = 6;

    for(index = 0; index < 17; index += 3){
        const char *theByteStr = (const char*)keyVal + index;
        int k = 0;
        unsigned byteSum = 0;

        for(k = 0; k < 2; ++k){
            const char c    = *theByteStr++;

            if(c >= '0' && c <= '9'){
                byteSum += c - '0' + 0x0;
            }
            else if (c >= 'a' && c <= 'f'){
                byteSum += c - 'a' + 0xa;
            }
            else if (c >= 'A' && c <= 'F'){
                byteSum += c - 'A' + 0XA;
            }
            else{
                sprintf(errInfo, "failed:Exception when burn key for efuse, c=%x\n", c);
                return -EINVAL;
            }
            byteSum <<= 4 * (1 - k);
        }
        DWN_DBG("byteSum=0x%x\n", byteSum);
        *decryptBuf++ = byteSum;
    }

    return 0;
}

/* 
 * Check or Decrypt the key from usb to real key value
 *Only depending the keyName to decide whether the key value needed special disposed !!
 * */
int check_or_decrypt_raw_usb_key_value(const char* keyName, const u8* keyVal, const unsigned keyValLen, char* errInfo,
                              const u8** keyRealVal, unsigned* keyRealValLen)
{
    int ret = 0;
    char* keyDecryptBuf             = (char*)OPTIMUS_KEY_DECRYPT_BUF;
    const unsigned keyDecryptBufSz  = OPTIMUS_KEY_DECRYPT_BUF_SZ;

    *keyRealVal = keyVal;
    *keyRealValLen = keyValLen;
 
    //do with the special key value
    if(!strcmp("hdcp", keyName))
    {
        ret = decrypt_hdcp_license_to_raw_value(keyName, keyVal, keyValLen, errInfo, 
                                keyRealVal, keyRealValLen, keyDecryptBuf, keyDecryptBufSz);
    }
    else if (!strcmp("mac", keyName) || !strcmp("mac_bt", keyName) || !strcmp("mac_wifi", keyName))
    {
        ret = decrypt_mac_str_fmt_4_media(keyName, keyVal, keyValLen, errInfo, 
                                keyRealVal, keyRealValLen, keyDecryptBuf, keyDecryptBufSz);
    }
    else if(!strcmp("your_key_name", keyName))
    {
        //TODO:Add your key decrypt or check code here
    }

    return ret;
}

/*
 *This fucntion called by mwrite command, mread= bulkcmd "download key .." + n * download transfer, for key n==1
 *Attentions: "return value is the key length" if burn sucess

 *@keyName: key name in null-terminated c style string
 *@keyVal: key value download from USB, "the value for sepecial keyName" may need de-encrypt by user code
 *@keyValLen: the key value downloaded from usb transfer! 
 *@errInfo: start it with success if burned ok, or format error info into it tell pc burned failed
 */
unsigned v2_key_burn(const char* keyName, const u8* keyVal, const unsigned keyValLen, char* errInfo)
{
    int ret = 0;
    const u8* keyRealVal = NULL;//the real value disposed/de-encrypted by user code
    unsigned keyRealValLen = 0;

    ret = check_or_decrypt_raw_usb_key_value(keyName, keyVal, keyValLen, errInfo, 
                                            &keyRealVal, &keyRealValLen);
    if(ret){
        DWN_ERR("Fail to check_or_decrypt_raw_usb_key_value, ret=0x%x\n", ret);
        return 0;
    }

    ret = key_unify_write((char*)keyName, (unsigned char*)keyRealVal, keyRealValLen);
    DWN_MSG("%s, ret 0x%x\n", __func__, ret);
    ret = ret >=0 ? keyValLen : 0;

    return ret;
}

static int _key_format_val_2_str_if_needed(const char* keyName, 
        const unsigned keyValLen, u8* keyVal, const unsigned reallen, unsigned* fmtLen)
{
    int rc = __LINE__;

    if(keyValLen == reallen){
        return 0;
    }
    if(keyValLen < reallen){
        DWN_ERR("bufsz %d < real sz %d\n", keyValLen, reallen);
        return __LINE__;
    }
    //keyValLen > reallen
    if(6 == reallen)
    {
        if (!strcmp("mac", keyName) || !strcmp("mac_bt", keyName) || !strcmp("mac_wifi", keyName))
        {
            char* decryptBuf = (char*)OPTIMUS_KEY_DECRYPT_BUF;
            int i = 0;

            rc = optimus_hex_data_2_ascii_str(keyVal, reallen, decryptBuf, keyValLen);
            if(rc){
                DWN_ERR("Fail to format mac hex data to str, rc=%d\n", rc);
                return __LINE__;
            }
            for(i = 0; i < reallen; ++i){
                *keyVal++ = decryptBuf[i * 2];
                *keyVal++ = decryptBuf[i * 2 + 1];
                *keyVal++ = ':';
            }
            *--keyVal = 0;
            *fmtLen = reallen * 3 - 1;

            return 0;
        }
    }

    return rc;
}

/*
 *This fucntion called by mread command, mread= bulkcmd "upload key .." + n * upload transfer, for key n==1
 *Attentions: return 0 if success, else failed
 *@keyName: key name in null-terminated c style string
 *@keyVal: the buffer to read back the key value
 *@keyValLen: keyVal len is strict when read, i.e, user must know the length of key he/she wnat to read!! 
 *@errInfo: start it with success if burned ok, or format error info into it tell pc burned failed
 */
int v2_key_read(const char* keyName, u8* keyVal, const unsigned keyValLen, char* errInfo, unsigned* fmtLen)
{
    unsigned reallen = 0;
    unsigned keyIsBurned = -1;
    unsigned keypermit = -1;
    int rc = 0;

    rc = key_unify_query((char*)keyName, &keyIsBurned, &keypermit);
    if(rc < 0 || 1 != keyIsBurned){
        sprintf(errInfo, "failed to query key state, rc %d, keyIsBurned=%d\n", rc, keyIsBurned);
        DWN_ERR(errInfo);
        return __LINE__;
    }

    rc = key_unify_read((char*)keyName, keyVal, keyValLen, &reallen);
    if(rc < 0 || !reallen){
        sprintf(errInfo, "failed:key_read rc %d, reallen(%d), want len(%d)\n", rc, reallen, keyValLen);
        DWN_ERR(errInfo);
        return __LINE__;
    }

    *fmtLen = reallen;
    rc = _key_format_val_2_str_if_needed(keyName, keyValLen, keyVal, reallen, fmtLen);

    return rc;
}

//key command: 1, key init seed_in_str; 2, key uninit
int v2_key_command(const int argc, char * const argv[], char *info)
{
    const char* keyCmd = argv[1];
    int rcode = 0;
    int subCmd_argc = argc - 1;
    char* const * subCmd_argv = argv + 1;

    DWN_DBG("argc=%d, argv[%s, %s, %s, %s]\n", argc, argv[0], argv[1], argv[2], argv[3]);
    if(argc < 2){
        printf("argc < 2, key sub-command init/uninit read/write/query not found\n");
        return __LINE__;
    }

    if(!strcmp("init", keyCmd))
    {
        if(argc < 3){
            sprintf(info, "failed:cmd [key init] must take argument (seedNum)\n");
            DWN_ERR(info);
            return __LINE__;
        }

        u64 seedNum = simple_strtoull(subCmd_argv[1], NULL, 16);
        if(!seedNum){
            sprintf(info, "failed:seedNum %s illegal\n", argv[2]);
            DWN_ERR(info);
            return __LINE__;
        }

        rcode = key_unify_init((char*)&seedNum, sizeof(seedNum));

        DWN_MSG("seedNum is 0x%llx, rcode %d\n", seedNum, rcode);
    }
    else if(!strcmp("uninit", keyCmd))
    {
        rcode = key_unify_uninit();
    }
    else if(!strcmp("is_burned", keyCmd))
    {
        if(subCmd_argc < 2){
            sprintf(info, "failed: %s %s need a keyName\n", argv[0], argv[1]);
            DWN_ERR(info);
            return __LINE__;
        }
        const char* queryKey = subCmd_argv[1];
        unsigned keyIsBurned = -1;
        unsigned keypermit = -1;
        rcode = key_unify_query((char*)queryKey, &keyIsBurned, &keypermit);
        if(rcode < 0){
            sprintf(info, "failed to query key state, rcode %d\n", rcode);
            DWN_ERR(info);
            return __LINE__;
        }
        rcode = (1 == keyIsBurned) ? 0 : __LINE__;
        sprintf(info, "%s:key[%s] was %s burned yet(keystate %d, keypermit 0x%x)\n", 
                rcode ? "failed" : "success", queryKey, rcode ? "NOT" : "DO", keyIsBurned, keypermit);
    }
    else if(!strcmp("can_write", keyCmd) || !strcmp("can_read", keyCmd))
    {
        if(subCmd_argc < 2){
            sprintf(info, "failed: %s %s need a keyName\n", argv[0], argv[1]);
            DWN_ERR(info);
            return __LINE__;
        }
        const char* queryKey = subCmd_argv[1];
        unsigned keyIsBurned = -1;
        unsigned keypermit = -1;
        rcode = key_unify_query((char*)queryKey, &keyIsBurned, &keypermit);
        if(rcode < 0){
            sprintf(info, "failed to query key state, rcode %d\n", rcode);
            DWN_ERR(info);
            return __LINE__;
        }
        int writeCmd = !strcmp("can_write", keyCmd);
        unsigned canWrite = ( 0xa == ( (keypermit>>4) & 0xfu ) );
        unsigned canRead  = ( 0xa == ( keypermit & 0xfu ) );
        rcode = writeCmd ? !canWrite : !canRead;
        sprintf(info, "%s:key[%s] %s %s (keystate %d, keypermit 0x%x)\n", 
                rcode ? "failed" : "success", queryKey, rcode ? "NOT" : "DO", keyCmd, keyIsBurned, keypermit);
    }
    else if(!strcmp("write", keyCmd))
    {
        /*
         *
         *key write [keyName keyValueInStr]
         *write direct, not support to deencrypt or verify, debug pipe, don't use to in PC tools
         *Attentions it support at most 512-6 Bytes!
         */

        const char* keyName = subCmd_argv[1];
        const char* keyValInStr = subCmd_argv[2];

        if(subCmd_argc < 3){
            sprintf(info, "failed: %s %s need a keyName and keyValInStr\n", argv[0], argv[1]);
            DWN_ERR(info);
            return __LINE__;
        }

        rcode = v2_key_burn(keyName, (u8*)keyValInStr, strlen(keyValInStr), info);
        rcode = (strlen(keyValInStr) == rcode) ? 0 : __LINE__;
    }
    else if(!strcmp("read", keyCmd) || !strcmp("get_len", keyCmd))
    {
        /*
         *key read [keyName], read directly to info buffer
         *debug pipe, support at most 512-6 bytes, and PLS DON'T use in PC tools
         *
         * 
         */
        unsigned actualLen = 0;
        const int cswBufLen = CMD_BUFF_SIZE - sizeof("success") + 1;
        const char* keyName = subCmd_argv[1];
        unsigned char* keyValBuf = (unsigned char*)info + CMD_BUFF_SIZE - cswBufLen;
        unsigned ReadBufLen = cswBufLen;

        if(subCmd_argc < 2){
            sprintf(info, "failed: %s %s need a keyName\n", argv[0], argv[1]);
            DWN_ERR(info);
            return __LINE__;
        }

        const int is_query = !strcmp("get_len", keyCmd) ;
        if(is_query) {
            keyValBuf  = (u8*)OPTIMUS_KEY_DECRYPT_BUF;
            ReadBufLen = OPTIMUS_KEY_DECRYPT_BUF_SZ;
        }

        rcode = v2_key_read(keyName, keyValBuf, ReadBufLen, info, &actualLen);
        if(is_query)
        {
            if(rcode)
                sprintf(info, "success%u", actualLen);
            else
                sprintf(info, "failed:at get_len rc %d\n", rcode);
        }
        DWN_MSG("key[%s] len(%d), rc(%d)\n", keyName, actualLen, rcode);

        rcode = rcode >=0 ? 0 : rcode;
    }
    else if(!strcmp("get_fmt", keyCmd))
    {
        char* fmt = NULL;
        fmt = key_unify_query_key_format((char*)subCmd_argv[1]);
        sprintf(info, "success:%s\n", fmt);
    }
    else{
        sprintf(info, "failed:Error keyCmd[%s]\n", keyCmd);
        DWN_ERR(info);
        rcode = __LINE__;
    }

    DWN_DBG("rcode 0x%x\n", rcode);
    return rcode;
}

