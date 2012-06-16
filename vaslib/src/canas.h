#ifndef CANAS_H
#define CANAS_H

#include<stdint.h>

//!  Number of payload bytes CAN 2.0A and 2.0B are able to transport.
#define CAN2AB_PAYLOAD 8

// for IDS Node Service, identifier distribution in byte 2
#define DISTRIBUTION_STANDARD 0
// flightpanels.com distribution 100 for user-defined distribution sheme
#define DISTRIBUTION_FP 100
// for IDS Node Service, header type in byte 3
#define HEADER_TYPE_CANAS 0

// Node service request codes
enum AS_NodeService {
    IDS = 0,
    STS = 7
    // others not implemented in plugin
};

//
enum AS_Type {
    AS_NODATA   = 0,
    AS_ERROR    = 1,
    AS_FLOAT    = 2,
    AS_LONG     = 3,
    AS_ULONG    = 4,
    AS_BLONG    = 5,
    AS_SHORT    = 6,
    AS_USHORT   = 7,
    AS_BSHORT   = 8,
    AS_CHAR     = 9,
    AS_UCHAR    = 10,
    AS_BCHAR    = 11,
    // Left out some types that are uneccesary at the moment
    AS_UCHAR4   = 16,
    // Left out some types that are uneccesary at the moment
    AS_ACHAR    = 23,
    AS_ACHAR2   = 24,
    AS_ACHAR4   = 25,
    // user defined (FP = flightpanels) type of sending 5 ascii charachters (4 in message, 1 in servie code)
    FP_ACHAR5   = 100
};

//! Union combining the CAN Aerospace message data types.
//
//! Binary data types will be handled through their unsigned variants.
typedef union canAS_Data_t {
    float       flt;
    uint32_t    uLong;      //!< Unsigned 32 bit integer.
    int32_t     sLong;      //!< Signed 32 bit integer.
    uint16_t    uShort[2];  //!< 2 x unsigned 16 bit integer.
    int16_t     sShort[2];  //!< 2 x signed 16 bit integer.
    uint8_t     uChar[4];   //!< 4 x unsigned 8 bit integer.
    int8_t      sChar[4];   //!< 4 x signed 8 bit integer.
    uint8_t     aChar[4];   //!< 4 x unsigned ASCII character.
} canAS_Data_t;


//! Struct combining the CAN Aerospace message header and message data.
typedef struct canAS_t {
    uint8_t         nodeId;         //!< Id of transmitting/receiving node.
    uint8_t         dataType;       //!< Id for CAN Aerospace message data type.
    uint8_t         serviceCode;    //!< Service code, see CAN Aerospace specification.
    uint8_t         messageCode;    //!< Message code, see CAN Aerospace specification.
    canAS_Data_t    data;           //!< CAN Aerospace message data.
} canAS_t;


//! Union combining the raw CAN payload data and CAN Aerospace message.
typedef union can_Data_t {
    uint8_t     byte[CAN2AB_PAYLOAD];   //!< Raw CAN message.
    canAS_t     aero;                   //!< CAN Aerospace message.
} can_Data_t;


//! Typedef for CAN messages.
typedef struct can_t {
    uint32_t    id;         //!< CAN id, used for both 11 and 29 bit id's.
    can_Data_t  msg;        //!< CAN message data bytes.
    uint8_t     dlc;        //!< Data length code, number of data bytes.
    uint8_t     id_is_29;   //!< Flag to distinguish 11 and 29 bit CAN id's.
    //uint8_t     reason;
    //uint8_t     prio;
    //uint8_t     this_send;
    //uint8_t     total_count;
} can_t;

#endif // CANAS_H
