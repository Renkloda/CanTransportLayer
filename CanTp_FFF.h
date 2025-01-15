#ifndef CANTP_FFF_H
#define CANTP_FFF_H


Std_ReturnType CanTp_SendSF(PduIdType id, uint8* SduDataPtr, PduLengthType SduLength);
Std_ReturnType CanTp_SendFF(PduIdType id, PduLengthType SduLength);
void CanTp_SendNextCF(void);
void CanTp_FFReception(PduIdType RxPduId, const PduInfoType* PduInfoPtr, CanPCI_Type* CanPCI);
void CanTp_SFReception(PduIdType RxPduId, CanPCI_Type* CanPCI, const PduInfoType* PduInfoPtr);
void CanTp_CFReception(PduIdType RxPduId, CanPCI_Type* CanPCI, const PduInfoType* PduInfoPtr);
void CanTp_FCReception(PduIdType RxPduId, CanPCI_Type* CanPCI);

#endif /* CANTP_FFF_H */
