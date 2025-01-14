#ifndef CAN_TP_H
#define CAN_TP_H

#include "ComStack_Types.h"

#define NULL_PTR ((void *)0x00)

#define CANTP_MODULE_ID (0x0Eu)
#define CANTP_SW_MAJOR_VERSION (0x00u)
#define CANTP_SW_MINOR_VERSION (0x01u)
#define CANTP_SW_PATCH_VERSION (0x00u)
#define CANTP_INIT_API_ID (0x01u)
#define CANTP_GET_VERSION_INFO_API_ID (0x07u)
#define CANTP_SHUTDOWN_API_ID (0x02u)
#define CANTP_TRANSMIT_API_ID (0x49u)
#define CANTP_CANCEL_TRANSMIT_API_ID (0x4Au)

#define CANTP_CAN_FRAME_SIZE (0x08u)


#define N_AS_TIMEOUT_VALUE 1000
#define N_BS_TIMEOUT_VALUE 1000
#define N_CS_TIMEOUT_VALUE 1000
#define N_AR_TIMEOUT_VALUE 1000
#define N_BR_TIMEOUT_VALUE 1000
#define N_CR_TIMEOUT_VALUE 1000

#define STMmin_TIMEOUT_VALUE 1000

typedef enum {
    CANTP_OFF, 
    CANTP_ON
} CanTpState_type;

typedef enum {
    CANTP_RX_WAIT,
    CANTP_RX_PROCESSING
} CanTpStateRX_type;

typedef enum {
    CANTP_TX_WAIT,                     
    CANTP_TX_PROCESSING
}CanTpStateTX_type;

typedef enum {
    SF = 0, // Single Frame
    FF = 1, // First Frame
    CF = 2, // Consecutive Frame 
    FC = 3, // Flow Control
    DEFAULT = 4
}FrameType;

typedef struct{
    FrameType eFrameType;
    uint32 uiFrameLenght; 
    uint8 uiSequenceNumber;
    uint8 uiBlockSize; 
    uint8 uiFlowStatus; 
    uint8 uiSeparationTime;
} CanPCI_Type;



typedef enum
{
    CANTP_EXTENDED,
    CANTP_MIXED,
    CANTP_MIXED29BIT,
    CANTP_NORMALFIXED,
    CANTP_STANDARD
} CanTp_AddressingFormatType;

typedef enum
{
    CANTP_FUNCTIONAL,
    CANTP_PHYSICAL
} CanTp_ComTypeType;

typedef struct
{
    uint8 CanTpNSa;
} CanTp_NSaType; 

typedef struct
{
    uint8 CanTpNTa; 
} CanTp_NTaType; 

typedef struct
{
    uint8 CanTpNAe; 
} CanTp_NAeType; 

typedef struct
{
    const PduIdType CanTpRxNPduRef;
    const uint16 CanTpRxNPduId; 
} CanTpRxNPduType; 

typedef struct
{
    const PduIdType CanTpTxFcNPduRef;
    const uint16 CanTpTxFcNPduConfirmationPduId;
}CanTpTxFcNPduType;


typedef struct
{
    const PduIdType CanTpTxNPduRef;
    const uint16 CanTpTxNPduId; 
} CanTpTxNPduType; 

typedef struct
{
    const PduIdType CanTpRxFcNPduRef;
    const uint16 CanTpRxFcNPduConfirmationPduId;
}CanTpRxFcNPduType;

typedef struct
{
    uint8 CanTpBs;
    float CanTpNar;
    float CanTpNbr;
    float CanTpNcr;
    const CanTp_AddressingFormatType CanTpRxAddressingFormat;
    uint16 CanTpRxNSduId;
    const CanTpState_type CanTpRxPaddingActivation;
    const CanTp_ComTypeType CanTpRxTaType;
    uint16 CanTpRxWftMax;
    const float CanTpSTmin;
    const PduIdType CanTpRxNSduRef;

    const CanTp_NSaType *CanTpNSa;
    const CanTp_NTaType *CanTpNTa;
    const CanTp_NAeType *CanTpNAe;

    CanTpRxNPduType CanTpRxNPdu;
    CanTpTxFcNPduType CanTpTxFcNPdu;

    CanTpStateRX_type CanTpRX_state;
}CanTp_RxNSduType;

typedef struct 
{
    float CanTpNas;
    float CanTpNbs;
    float CanTpNcs;
    boolean CanTpTc;
    const CanTp_AddressingFormatType CanTpTxAddressingFormat;
    uint16 CanTpTxNSduId;
    const CanTpState_type CanTpTxPaddingActivation;
    const CanTp_ComTypeType CanTpTxTaType;
    const PduIdType CanTpTxNSduRef;

    
    const CanTp_NSaType *CanTpNSa;
    const CanTp_NTaType *CanTpNTa;
    const CanTp_NAeType *CanTpNAe;

    CanTpRxNPduType CanTpTxNPdu;
    CanTpTxFcNPduType CanRpTxFcNPdu;

    CanTpStateTX_type CanTpTX_state;
}CanTp_TxNSduType;


typedef struct 
{
    CanTp_RxNSduType *rx;
    CanTp_TxNSduType *tx;
}CanTp_ChannelType;


typedef struct
{
    const float CanTpMainFunctionPeriod;
    const long long int CanTpMaxChannelCnt;
    CanTp_ChannelType *pChannel;   
}CanTp_ConfigType;

typedef struct{

    boolean CanTpChangeParameterApi;
    boolean CanTpDevErrorDetect;
    boolean CanTpDynIdSupport;
    boolean CanTpFlexibleDataRateSupport;
    boolean CanTpGenericConnectionSupport;
    boolean CanTpReadParameterApi;
    boolean CanTpVersionInfoApi;
    uint8 CanTpPaddingByte;
    
}CanTp_GeneralType;

typedef enum{
    TIMER_ENABLE,
    TIMER_DISABLE
} TimerState;

typedef struct{
    TimerState    eState;
    uint32        uiCounter; 
    const uint32  uiTimeout; 
} CanTp_Timer_type;


void CanTp_Init(const CanTp_ConfigType* CfgPtr);
void CanTp_GetVersionInfo(Std_VersionInfoType* versioninfo);
void CanTp_Shutdown(void);

Std_ReturnType CanTp_Transmit(PduIdType TxPduId, const PduInfoType* PduInfoPtr);
Std_ReturnType CanTp_CancelTransmit(PduIdType TxPduId);
Std_ReturnType CanTp_CancelReceive(PduIdType RxPduId);
Std_ReturnType CanTp_ChangeParameter(PduIdType id, TPParameterType parameter, uint16 value);
Std_ReturnType CanTp_ReadParameter(PduIdType id, TPParameterType parameter, uint16* value);

void CanTp_MainFunction(void); 

void CanTp_RxIndication(PduIdType RxPduId, const PduInfoType* PduInfoPtr);
void CanTp_TxConfirmation(PduIdType TxPduId, Std_ReturnType result);

#endif CAN_TP_H