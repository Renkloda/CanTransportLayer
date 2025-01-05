#ifndef CANTP_FFF_H
#define CANTP_FFF_H


Std_ReturnType CanTp_SendSF(PduIdType id, uint8* SduDataPtr, PduLengthType SduLength);
Std_ReturnType CanTp_SendFF(PduIdType id, PduLengthType SduLength);

#endif /* CANTP_FFF_H */
