#include "CanTp.h"
#include "CanIf.h"
#include "PduR_CanTp.h"


CanTpState_type CanTpState = CANTP_OFF;

void CanTpInit(const CanTp_ConfigType* CfgPtr) {

    CanTpState = CANTP_ON;
}

void CanTp_GetVersionInfo(Std_VersionInfoType* versioninfo) {

}

void CanTp_Shutdown(void){
    CanTpState = CANTP_OFF;
}

Std_ReturnType CanTp_Transmit(PduIdType TxPduId, const PduInfoType* PduInfoPtr){

    return E_OK;
}

Std_ReturnType CanTp_CancelTransmit(PduIdType TxPduId){

    return E_OK;
}

Std_ReturnType CanTp_CancelReceive(PduIdType RxPduId){

    return E_OK;
}

Std_ReturnType CanTp_ChangeParameter(PduIdType id, TPParameterType parameter, uint16 value){
    return E_OK;
}


Std_ReturnType CanTp_ReadParameter(PduIdType id, TPParameterType parameter, uint16* value){

    return E_OK;
}

void CanTp_MainFunction(void) {


}

void CanTp_RxIndication( PduIdType RxPduId, const PduInfoType* PduInfoPtr){


}

void CanTp_TxConfirmation(PduIdType TxPduId, Std_ReturnType result){


}

