#include "CanTp.h"
#include "CanIf.h"
#include "PduR_CanTp.h"
#include "CanTp_FFF.h"


CanTpState_type CanTpState = CANTP_OFF;
CanTp_ConfigType *CanTp_ConfigPtr = NULL_PTR;
CanTp_GeneralType *CanTpGeneralgPtr = NULL_PTR;


typedef uint8 CanTp_FlowStatusType;

typedef struct
{
    uint8 can[CANTP_CAN_FRAME_SIZE];
    PduLengthType size;
    PduLengthType rmng;
} CanTp_NSduBufferType;

typedef struct
{
    const CanTp_RxNSduType *cfg;
    CanTp_NSduBufferType buf;
    uint8 meta_data_lower[0x04u];
    uint8 meta_data_upper[0x04u];
    CanTp_NSaType saved_n_sa;
    CanTp_NTaType saved_n_ta;
    CanTp_NAeType saved_n_ae;
    boolean has_meta_data;
    CanTp_FlowStatusType fs;
    uint32 st_min;
    uint8 bs;
    uint8 sn;
    uint16 wft_max;
    PduInfoType can_if_pdu_info;
    PduInfoType pdu_r_pdu_info;
    struct
    {
        CanTpStateRX_type taskState;
        // CanTp_FrameStateType state;
        struct
        {
            uint32 st_min;
            uint8 bs;
        } params;
    } shared_params;
} CanTp_RxConnectionType;

typedef struct
{
    const CanTp_TxNSduType *cfg;
    CanTp_NSduBufferType buf;
    uint8 meta_data[0x04u];
    CanTp_NSaType saved_n_sa;
    CanTp_NTaType saved_n_ta;
    CanTp_NAeType saved_n_ae;
    boolean has_meta_data;
    CanTp_FlowStatusType fs;
    uint32 target_st_min;
    uint32 st_min;
    uint16 bs;
    uint8 sn;
    PduInfoType can_if_pdu_info;
    CanTpStateTX_type taskState;
    // struct
    // {
        // CanTp_FrameStateType state;
    //     uint32 flag;
    // } shared;
} CanTp_TxConnectionType;

typedef struct
{
    CanTp_RxConnectionType rx;
    CanTp_TxConnectionType tx;
    // uint32 n[0x06u];
    // uint8_least dir;
    // uint32 t_flag;
} CanTp_NSduType;

void CanTpInit(const CanTp_ConfigType* CfgPtr) {
    uint8 value;
    if(CfgPtr != NULL_PTR){

        CanTp_ConfigPtr = CfgPtr;
        CanTpGeneralgPtr->CanTpChangeParameterApi = 1;
        CanTpGeneralgPtr->CanTpDevErrorDetect = 0;
        CanTpGeneralgPtr->CanTpDynIdSupport = 0;
        CanTpGeneralgPtr->CanTpFlexibleDataRateSupport = 0;
        CanTpGeneralgPtr->CanTpGenericConnectionSupport = 1;
        CanTpGeneralgPtr->CanTpPaddingByte = value;
        CanTpGeneralgPtr->CanTpReadParameterApi = 1;
        CanTpGeneralgPtr->CanTpVersionInfoApi = 1;
        CanTpState = CANTP_ON;
    }

}

void CanTp_GetVersionInfo(Std_VersionInfoType* versioninfo) {
    if (versioninfo != NULL_PTR) {
        versioninfo->vendorID = 0x00u;
        versioninfo->moduleID = (uint16)CANTP_MODULE_ID;
    	versioninfo->sw_major_version = CANTP_SW_MAJOR_VERSION;
    	versioninfo->sw_minor_version = CANTP_SW_MINOR_VERSION;
    	versioninfo->sw_patch_version = CANTP_SW_PATCH_VERSION;
    }
}

void CanTp_Shutdown(void){
    CanTpState = CANTP_OFF;
}

Std_ReturnType CanTp_Transmit(PduIdType TxPduId, const PduInfoType* PduInfoPtr){

    CanTp_NSduType *p_n_sdu = NULL_PTR;
    Std_ReturnType tmp_return = E_NOT_OK;
    BufReq_ReturnType BufReqState;


        if (CanTpState == CANTP_ON){

            if (PduInfoPtr->MetaDataPtr != NULL_PTR){
                p_n_sdu->tx.has_meta_data = TRUE;

                if (p_n_sdu->tx.cfg->CanTpTxAddressingFormat == CANTP_EXTENDED)
                {
                    p_n_sdu->tx.saved_n_ta.CanTpNTa = PduInfoPtr->MetaDataPtr[0x00u];
                }
                else if (p_n_sdu->tx.cfg->CanTpTxAddressingFormat == CANTP_MIXED)
                {
                    p_n_sdu->tx.saved_n_ae.CanTpNAe = PduInfoPtr->MetaDataPtr[0x00u];
                }
                else if (p_n_sdu->tx.cfg->CanTpTxAddressingFormat == CANTP_NORMALFIXED)
                {
                    p_n_sdu->tx.saved_n_sa.CanTpNSa = PduInfoPtr->MetaDataPtr[0x00u];
                    p_n_sdu->tx.saved_n_ta.CanTpNTa = PduInfoPtr->MetaDataPtr[0x01u];
                }
                else if (p_n_sdu->tx.cfg->CanTpTxAddressingFormat == CANTP_MIXED29BIT)
                {
                    p_n_sdu->tx.saved_n_sa.CanTpNSa = PduInfoPtr->MetaDataPtr[0x00u];
                    p_n_sdu->tx.saved_n_ta.CanTpNTa = PduInfoPtr->MetaDataPtr[0x01u];
                    p_n_sdu->tx.saved_n_ae.CanTpNAe = PduInfoPtr->MetaDataPtr[0x02u];
                }
                else
                {
                }


            }else
            {
                p_n_sdu->tx.has_meta_data = FALSE;
            }

            if ((p_n_sdu->tx.taskState != CANTP_TX_PROCESSING) && (PduInfoPtr->SduLength > 0x0000u) && (PduInfoPtr->SduLength < 0x0008u))
            {
                BufReqState = PduR_CanTpCopyTxData(TxPduId, PduInfoPtr, NULL, PduInfoPtr->SduLength);
                if(BufReqState == BUFREQ_OK){           
                    tmp_return = CanTp_SendSF(TxPduId, PduInfoPtr->SduDataPtr, PduInfoPtr->SduLength);
                }else if(BufReqState == BUFREQ_E_NOT_OK){
                    //CanTp_ResetTX();  
                    PduR_CanTpTxConfirmation(TxPduId, E_NOT_OK);
                    tmp_return = E_NOT_OK;
                }else{
                    //TODO 
                    tmp_return = E_OK;
                }


            }else if ((p_n_sdu->tx.taskState != CANTP_TX_PROCESSING) && (PduInfoPtr->SduLength >= 0x0008u) && (PduInfoPtr->SduLength <= 0x0FFFu)){
                if(CanTp_SendFF(TxPduId, PduInfoPtr->SduLength) == E_OK){
                    //TODO
                    tmp_return = E_OK;
                }else{
                    tmp_return = E_NOT_OK;
                }
            }else{
                tmp_return = E_NOT_OK;
            }
        }else{
            tmp_return = E_NOT_OK;
        }
    return tmp_return;    
}

Std_ReturnType CanTp_CancelTransmit(PduIdType TxPduId){

    Std_ReturnType tmp_return = E_NOT_OK;

    if(CanTp_ConfigPtr->pChannel->tx->CanTpTxNSduId == TxPduId ){
        PduR_CanTpTxConfirmation(CanTp_ConfigPtr->pChannel->tx->CanTpTxNSduId, E_NOT_OK);
        CanTp_ConfigPtr->pChannel->tx->CanTpTX_state = CANTP_TX_WAIT;
        tmp_return = E_OK;
    }
    else{
        tmp_return = E_NOT_OK;
    }
    return tmp_return;
}

Std_ReturnType CanTp_CancelReceive(PduIdType RxPduId){

    Std_ReturnType tmp_return = E_NOT_OK;

    if(CanTp_ConfigPtr->pChannel->rx->CanTpRxNSduId == RxPduId ){
        PduR_CanTpTxConfirmation(CanTp_ConfigPtr->pChannel->tx->CanTpTxNSduId, E_NOT_OK);
        CanTp_ConfigPtr->pChannel->tx->CanTpTX_state = CANTP_TX_WAIT;
        tmp_return = E_OK;
    }
    else{
        tmp_return = E_NOT_OK;
    }
    return tmp_return;
}

Std_ReturnType CanTp_ChangeParameter(PduIdType id, TPParameterType parameter, uint16 value){
    Std_ReturnType tmp_ret = E_NOT_OK;
    CanTpStateRX_type task_state;
    CanTp_NSduType *p_n_sdu;
    
    if (CanTpState == CANTP_ON){
        if(CanTpGeneralgPtr->CanTpChangeParameterApi == 1){
            task_state = p_n_sdu->rx.shared_params.taskState;

            if (task_state != CANTP_RX_PROCESSING){
                switch(parameter){
                    case TP_STMIN:
                    {
                        p_n_sdu->rx.shared_params.params.st_min = value;
                        tmp_ret = E_OK;
                        break;
                    }
                    case TP_BS:
                    {    
                        p_n_sdu->rx.shared_params.params.bs = value;
                        tmp_ret = E_OK;
                        break;
                    }    
                    case TP_BC:
                        break;
                    default:
                        break;
                }
            }
        }
    }
    return tmp_ret;
}


Std_ReturnType CanTp_ReadParameter(PduIdType id, TPParameterType parameter, uint16* value){
    Std_ReturnType tmp_ret = E_NOT_OK;
    CanTp_NSduType *p_n_sdu;
    uint16 temp_value;

    if (CanTpState == CANTP_ON){
        if(CanTpGeneralgPtr->CanTpReadParameterApi == 1){
            switch(parameter){
                case TP_STMIN:
                {
                    temp_value = p_n_sdu->rx.shared_params.params.st_min;

                    *value = temp_value;
                    tmp_ret = E_OK;
                    break;
                }
                case TP_BS:
                {    
                    temp_value = p_n_sdu->rx.shared_params.params.bs;

                    *value = temp_value;
                    tmp_ret = E_OK;
                    break;
                }    
                case TP_BC:
                    break;
                default:
                    break;
            }      
        }
    }
    return tmp_ret;
    
}

void CanTp_MainFunction(void) {


}

void CanTp_RxIndication( PduIdType RxPduId, const PduInfoType* PduInfoPtr){


}

void CanTp_TxConfirmation(PduIdType TxPduId, Std_ReturnType result){


}

