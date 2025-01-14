#include "CanIf.h"
#include "CanTp.h"
#include "PduR_CanTp.h"
#include "CanTp_FFF.h"


CanTpState_type CanTpState = CANTP_OFF;
CanTp_NSduType *p_n_sdu = NULL_PTR;
CanTp_ConfigType *CanTp_ConfigPtr = NULL_PTR;
CanTp_GeneralType *CanTpGeneralgPtr = NULL_PTR;


typedef uint8 CanTp_FlowStatusType;

CanTp_Timer_type N_Ar_timer =   {TIMER_DISABLE, 0, N_AR_TIMEOUT_VALUE};
CanTp_Timer_type N_Br_timer =   {TIMER_DISABLE, 0, N_BR_TIMEOUT_VALUE};
CanTp_Timer_type N_Cr_timer =   {TIMER_DISABLE, 0, N_CR_TIMEOUT_VALUE};
CanTp_Timer_type N_As_timer =   {TIMER_DISABLE, 0, N_AS_TIMEOUT_VALUE};
CanTp_Timer_type N_Bs_timer =   {TIMER_DISABLE, 0, N_BS_TIMEOUT_VALUE};
CanTp_Timer_type N_Cs_timer =   {TIMER_DISABLE, 0, N_CS_TIMEOUT_VALUE};
CanTp_Timer_type STMmin_timer = {TIMER_DISABLE, 0, STMmin_TIMEOUT_VALUE};

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
} CanTp_TxConnectionType;

typedef struct
{
    CanTp_RxConnectionType rx;
    CanTp_TxConnectionType tx;
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
                    CanTp_ConfigPtr->pChannel->tx->CanTpTX_state = CANTP_TX_WAIT; 
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

void CanTp_MainFunction(void){
    /* Wypełnia [SWS_CANTP_00164]*/
    CanTp_TimerTick(&N_Ar_timer);
    CanTp_TimerTick(&N_Br_timer);
    CanTp_TimerTick(&N_Cr_timer);

    CanTp_TimerTick(&N_As_timer);
    CanTp_TimerTick(&N_Bs_timer);
    CanTp_TimerTick(&N_Cs_timer);

   if(N_Br_timer.eState == TIMER_ENABLE){
        if(CanTp_TimerTimeout(&N_Cr_timer) == E_NOT_OK){
            PduR_CanTpRxIndication(CanTp_ConfigPtr->pChannel->rx->CanTpRxNSduId, E_NOT_OK);
            CanTp_ConfigPtr->pChannel->rx->CanTpRX_state = CANTP_RX_WAIT;
       }
       
   }
   if(N_Cr_timer.eState == TIMER_ENABLE){
       if(CanTp_TimerTimeout(&N_Cr_timer) == E_NOT_OK){
            PduR_CanTpRxIndication(CanTp_ConfigPtr->pChannel->rx->CanTpRxNSduId, E_NOT_OK);
            CanTp_ConfigPtr->pChannel->rx->CanTpRX_state = CANTP_RX_WAIT;
       }
   }
   if(N_Ar_timer.eState == TIMER_ENABLE){
       if(CanTp_TimerTimeout(&N_Ar_timer) == E_NOT_OK){
            PduR_CanTpRxIndication(CanTp_ConfigPtr->pChannel->rx->CanTpRxNSduId, E_NOT_OK);
            CanTp_ConfigPtr->pChannel->rx->CanTpRX_state = CANTP_RX_WAIT;
       }
   }
    if(N_Cs_timer.eState == TIMER_ENABLE){
       if(CanTp_TimerTimeout(&N_Cs_timer) == E_NOT_OK){
            PduR_CanTpTxConfirmation(CanTp_ConfigPtr->pChannel->tx->CanTpTxNSduId, E_NOT_OK);
            CanTp_ConfigPtr->pChannel->tx->CanTpTX_state = CANTP_TX_WAIT;       }
   }
    if(N_As_timer.eState == TIMER_ENABLE){
       if(CanTp_TimerTimeout(&N_As_timer) == E_NOT_OK){
            PduR_CanTpTxConfirmation(CanTp_ConfigPtr->pChannel->tx->CanTpTxNSduId, E_NOT_OK);
            CanTp_ConfigPtr->pChannel->tx->CanTpTX_state = CANTP_TX_WAIT;
       }
   }
    if(N_Bs_timer.eState == TIMER_ENABLE){
       if(CanTp_TimerTimeout(&N_Bs_timer) == E_NOT_OK){
            PduR_CanTpTxConfirmation(CanTp_ConfigPtr->pChannel->tx->CanTpTxNSduId, E_NOT_OK);
            CanTp_ConfigPtr->pChannel->tx->CanTpTX_state = CANTP_TX_WAIT;
       }
   }
}


void CanTp_RxIndication(PduIdType RxPduId, const PduInfoType* PduInfoPtr){
    
    CanPCI_Type CanPCI;    

    if(CanTpState == CANTP_ON){
        if( p_n_sdu->rx.shared_params.taskState == CANTP_RX_WAIT){
        
            CanTp_GetPCI(PduInfoPtr, &CanPCI);

            if(CanPCI.eFrameType == FF){
                CanTp_FirstFrameReception(RxPduId, PduInfoPtr, &CanPCI);
            }
            else if(CanPCI.eFrameType == SF){
                CanTp_SingleFrameReception(RxPduId, &CanPCI, PduInfoPtr);           
            } 
            else if(CanPCI.eFrameType == FC){
                CanTp_FlowControlReception(RxPduId, &CanPCI);
            }
            else
            {
                p_n_sdu->rx.taskState = CANTP_RX_WAIT; 
            } 
        }
        else if(p_n_sdu->rx.taskState == CANTP_RX_PROCESSING){
             CanTp_GetPCI(PduInfoPtr, &CanPCI);
             if(CanPCI.eFrameType == CF){
                CanTp_ConsecutiveFrameReception(RxPduId, &CanPCI, PduInfoPtr);
            }
            else if(CanPCI.eFrameType == FC){
                CanTp_FlowControlReception(RxPduId, &CanPCI);
            }
            else if(CanPCI.eFrameType == FF){            
                PduR_CanTpRxIndication (CanTp_ConfigPtr->pChannel->rx->CanTpRxNSduId, E_NOT_OK);
                CanTp_ConfigPtr->pChannel->rx->CanTpRX_state = CANTP_RX_WAIT;
                CanTp_FirstFrameReception(RxPduId, PduInfoPtr, &CanPCI);
            }
            else if(CanPCI.eFrameType == SF){            
                PduR_CanTpRxIndication (CanTp_ConfigPtr->pChannel->rx->CanTpRxNSduId, E_NOT_OK);
                CanTp_ConfigPtr->pChannel->rx->CanTpRX_state = CANTP_RX_WAIT;
                CanTp_SingleFrameReception(RxPduId, &CanPCI, PduInfoPtr);
            }
            else{
            }
        }
        else {    
            CanTp_GetPCI(PduInfoPtr, &CanPCI);
            if(CanPCI.eFrameType == FC){
                CanTp_FlowControlReception(RxPduId, &CanPCI);
            }
            else if(CanPCI.eFrameType == FF) {            
                PduR_CanTpRxIndication (CanTp_ConfigPtr->pChannel->rx->CanTpRxNSduId, E_NOT_OK);
                CanTp_ConfigPtr->pChannel->rx->CanTpRX_state = CANTP_RX_WAIT;
                CanTp_FirstFrameReception(RxPduId, PduInfoPtr, &CanPCI);
            }
            else if(CanPCI.eFrameType == SF) {            
                PduR_CanTpRxIndication (CanTp_ConfigPtr->pChannel->rx->CanTpRxNSduId, E_NOT_OK);
                CanTp_ConfigPtr->pChannel->rx->CanTpRX_state = CANTP_RX_WAIT;
                CanTp_SingleFrameReception(RxPduId, &CanPCI, PduInfoPtr);
            }
            else {
                PduR_CanTpRxIndication (CanTp_ConfigPtr->pChannel->rx->CanTpRxNSduId, E_NOT_OK);
                CanTp_ConfigPtr->pChannel->rx->CanTpRX_state = CANTP_RX_WAIT;
            }
        }
    }
}




void CanTp_TxConfirmation(PduIdType TxPduId, Std_ReturnType result){
if( CanTpState == CANTP_ON ){  
    if(CanTp_ConfigPtr->pChannel->rx->CanTpRxNSduId == TxPduId){
        if(CanTp_ConfigPtr->pChannel->rx->CanTpRX_state == CANTP_RX_PROCESSING ){
            if(result == E_NOT_OK){
                PduR_CanTpRxIndication(CanTp_ConfigPtr->pChannel->rx->CanTpRxNSduId, E_NOT_OK);
                CanTp_ConfigPtr->pChannel->rx->CanTpRX_state = CANTP_RX_WAIT;
            }
        }
        else{} 
    }
    if(CanTp_ConfigPtr->pChannel->tx->CanTpTxNSduId == TxPduId ){
        if(result == E_OK){
            if(CanTp_ConfigPtr->pChannel->tx->CanTpTX_state == CANTP_TX_PROCESSING)
            {
               CanTp_SendNextCF();               
            }
            else{}
        }
        else{
            PduR_CanTpTxConfirmation(CanTp_ConfigPtr->pChannel->tx->CanTpTxNSduId, E_NOT_OK);
            CanTp_ConfigPtr->pChannel->rx->CanTpRX_state = CANTP_RX_WAIT;
        }
    }
    else{}
}

}

static Std_ReturnType CanTp_GetPCI(const PduInfoType* CanData, CanPCI_Type* CanFrameInfo){
    Std_ReturnType ret = E_OK;

    if(NE_NULL_PTR(CanData) && NE_NULL_PTR(CanFrameInfo) && (NE_NULL_PTR(CanData->SduDataPtr))){
        CanFrameInfo->eFrameType = DEFAULT;
        CanFrameInfo->uiFrameLenght = 0;
        CanFrameInfo->uiBlockSize = 0;
        CanFrameInfo->uiFlowStatus = 0;
        CanFrameInfo->uiSeparationTime = 0;
        CanFrameInfo->uiSequenceNumber = 0;

        switch((CanData->SduDataPtr[0]) >> 4){
            case SF:
                CanFrameInfo->eFrameType = SF;
                CanFrameInfo->uiFrameLenght = CanData->SduDataPtr[0];
            break;
            case FF:
                CanFrameInfo->eFrameType = FF;
                if( (CanData->SduDataPtr[0] & 0x0F) | CanData->SduDataPtr[1] ) {
                    CanFrameInfo->uiFrameLenght =  CanData->SduDataPtr[0] & 0x0F;
                    CanFrameInfo->uiFrameLenght =  (CanFrameInfo->uiFrameLenght << 8) | CanData->SduDataPtr[1]; 
                }
                else{
                    CanFrameInfo->uiFrameLenght =  CanData->SduDataPtr[2];
                    CanFrameInfo->uiFrameLenght =  (CanFrameInfo->uiFrameLenght << 8) | CanData->SduDataPtr[3]; 
                    CanFrameInfo->uiFrameLenght =  (CanFrameInfo->uiFrameLenght << 8) | CanData->SduDataPtr[4];
                    CanFrameInfo->uiFrameLenght =  (CanFrameInfo->uiFrameLenght << 8) | CanData->SduDataPtr[5];
                }
            break;
            case CF:
                CanFrameInfo->eFrameType = CF;
                CanFrameInfo->uiSequenceNumber= (CanData->SduDataPtr[0] & 0x0F );
            break;
            case FC:
                CanFrameInfo->eFrameType = FC;
                CanFrameInfo->uiFlowStatus = CanData->SduDataPtr[0] & 0x0F; 
                CanFrameInfo->uiBlockSize = CanData->SduDataPtr[1]; 
                CanFrameInfo->uiSeparationTime = CanData->SduDataPtr[2]; 
            break;
            default:
                CanFrameInfo->eFrameType = DEFAULT;
                ret = E_NOT_OK;
            break;
        }
    }
    else{
        ret = E_NOT_OK;
    }
    return ret;
}


void CanTp_TimerStart(CanTp_Timer_type *pTimer){
    pTimer->eState = TIMER_ENABLE;
}

void CanTp_TimerReset(CanTp_Timer_type *pTimer){
    pTimer->eState = TIMER_DISABLE;
    pTimer->uiCounter = 0;
}


Std_ReturnType CanTp_TimerTick(CanTp_Timer_type *pTimer){
    Std_ReturnType ret = E_OK;   
    if(pTimer->eState == TIMER_ENABLE){
        if(pTimer->uiCounter < UINT32_MAX){
            pTimer->uiCounter++;
        }
        else{
            ret = E_NOT_OK;
        }
    }
    return ret;
}

Std_ReturnType CanTp_TimerTimeout(const CanTp_Timer_type *pTimer){
    if(pTimer->uiCounter >= pTimer->uiTimeout){
        return E_NOT_OK;
    }
    else{
        return E_OK;
    }
}