#pragma once


#define USER_ID_LEN 32
#define MESSAGE_MAX_DATA_SIZE (100*100*8)
#define MESSAGE_MIN_DATA_SIZE sizeof(int32_t)

#define MESSAGE_ENTRY_SIZE__header sizeof(int32_t)

#define NULL_MESSAGE Message(NULL_CLIENT, nullptr, 0)

#define MESSAGE_INFO_MESSAGE_LEN 256

#define USER_NAME_LEN  	32
#define USER_PASSWD_LEN	32

#define MESSAGE_TYPE__GetServerMessage          0x1
#define MESSAGE_TYPE__UserLogin                 0x2
#define MESSAGE_TYPE__UserLogout                0x3
#define MESSAGE_TYPE__UserRegister              0x4
#define MESSAGE_TYPE__GetObjInfoLib             0x5
#define MESSAGE_TYPE__CreateFaction             0x6
#define MESSAGE_TYPE__EditFaction               0x7
#define MESSAGE_TYPE__GetWorldState             0x8
#define MESSAGE_TYPE__GetAllFactions            0x9
#define MESSAGE_TYPE__GetChangedFactions        0x10
#define MESSAGE_TYPE__UpdateObserverProperties  0x11
#define MESSAGE_TYPE__Deploy                    0x12

