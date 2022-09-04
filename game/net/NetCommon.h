#pragma once


#define USER_ID_LEN 32
#define MESSAGE_MAX_DATA_SIZE (100*100*8)
#define MESSAGE_MIN_DATA_SIZE sizeof(int32_t)

#define MESSAGE_ENTRY_SIZE__header sizeof(int32_t)

#define NULL_MESSAGE Message(NULL_CLIENT, nullptr, 0)

#define MESSAGE_TYPE__GetServerMessage 		0x1
#define MESSAGE_TYPE__CreateFaction 		0x2
#define MESSAGE_TYPE__GetWorldState 		0x3
#define MESSAGE_TYPE__UpdateObserverProperties	0x4
#define MESSAGE_TYPE__ServerShutdown 		0x5
