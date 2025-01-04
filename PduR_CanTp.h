#include "ComStack_Types.h"

void PduR_CanTpRxIndication (PduIdType id, Std_ReturnType result );

BufReq_ReturnType PduR_CanTpStartOfReception (PduIdType id, const PduInfoType* info, PduLengthType TpSduLength, PduLengthType* bufferSizePtr );

BufReq_ReturnType PduR_CanTpCopyRxData (PduIdType id, const PduInfoType* info, PduLengthType* bufferSizePtr );

BufReq_ReturnType PduR_CanTpCopyTxData (PduIdType id, const PduInfoType* info, const RetryInfoType* retry, PduLengthType* availableDataPtr );

void PduR_CanTpTxConfirmation (PduIdType id, Std_ReturnType result );
