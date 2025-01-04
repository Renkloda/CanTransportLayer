#ifndef COMSTACK_TYPES_H
#define COMSTACK_TYPES_H

#include "Std_Types.h"
#include "Platform_Types.h"

typedef enum {
    BUFREQ_OK,       
    BUFREQ_E_NOT_OK, 
    BUFREQ_E_BUSY,     
    BUFREQ_E_OVFL 
}BufReq_ReturnType;

typedef uint16 PduIdType;
typedef uint32 PduLengthType;

typedef struct{
    uint8*        SduDataPtr;   
    uint8*        MetaDataPtr;  
    PduLengthType SduLength;    
}PduInfoType;

typedef enum {
    TP_DATACONF, 
    TP_DATARETRY,   
    TP_CONFPENDING  
}TpDataStateType;

typedef struct{
    TpDataStateType TpDataStateType; 
    PduLengthType   TxTpDataCnt;
}RetryInfoType;

typedef enum{
    TP_STMIN,  
    TP_BS,      
    TP_BC       
}TPParameterType;

#endif /* COMSTACK_TYPES_H */