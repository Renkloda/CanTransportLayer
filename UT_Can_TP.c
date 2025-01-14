/** ==================================================================================================================*\
  @file UT_Can_TP.c

  @brief Testy jednostkowe do CanTp
\*====================================================================================================================*/

#include "acutest.h"
#include "Std_Types.h"

#include "CanTp.c"   

#include <stdio.h>
#include <string.h>

#include "fff.h"

DEFINE_FFF_GLOBALS; 

FAKE_VALUE_FUNC(BufReq_ReturnType, PduR_CanTpCopyTxData, PduIdType, const PduInfoType*, const RetryInfoType*, PduLengthType*);
FAKE_VOID_FUNC(PduR_CanTpTxConfirmation, PduIdType, Std_ReturnType);
FAKE_VOID_FUNC(PduR_CanTpRxIndication, PduIdType, Std_ReturnType);
FAKE_VALUE_FUNC(BufReq_ReturnType, PduR_CanTpCopyRxData, PduIdType, const PduInfoType*, PduLengthType*);
FAKE_VALUE_FUNC(BufReq_ReturnType, PduR_CanTpStartOfReception, PduIdType, const PduInfoType*, PduLengthType, PduLengthType*);

FAKE_VALUE_FUNC(Std_ReturnType, CanIf_Transmit, PduIdType, const PduInfoType*);

FAKE_VOID_FUNC(CanTp_SendNextCF);

uint8 PduR_CanTpCopyTxData_sdu_data[20][7];
PduLengthType *PduR_CanTpCopyTxData_availableDataPtr; 
PduLengthType* PduR_CanTpCopyRxData_buffSize_array;
PduLengthType *PduR_CanTpStartOfReception_buffSize_array;
CanTpState_type CanTpState = CANTP_OFF;

BufReq_ReturnType PduR_CanTpCopyTxData_FF(PduIdType id, const PduInfoType* info, const RetryInfoType* retry, PduLengthType* availableDataPtr){
    static int i = 0;
    int iCtr;
    i = PduR_CanTpCopyTxData_fake.call_count - 1;
    for(iCtr = 0; iCtr < info->SduLength; iCtr++ ){
      info->SduDataPtr[iCtr] = PduR_CanTpCopyTxData_sdu_data[i][iCtr];
    }
    *availableDataPtr = PduR_CanTpCopyTxData_availableDataPtr[i];
    return PduR_CanTpCopyTxData_fake.return_val_seq[i];
}

BufReq_ReturnType PduR_CanTpStartOfReception_FF(PduIdType id, const PduInfoType* info, PduLengthType TpSduLength, PduLengthType* bufferSizePtr){
    static int i = 0;
    i = PduR_CanTpStartOfReception_fake.call_count - 1;
    *bufferSizePtr = PduR_CanTpStartOfReception_buffSize_array[i];
   return PduR_CanTpStartOfReception_fake.return_val_seq[i];
}

BufReq_ReturnType PduR_CanTpCopyRxData_FF(PduIdType id, const PduInfoType* info, PduLengthType* bufferSizePtr){
    static int i = 0;
    i = PduR_CanTpCopyRxData_fake.call_count - 1;
    *bufferSizePtr = PduR_CanTpCopyRxData_buffSize_array[i];
    return PduR_CanTpCopyRxData_fake.return_val_seq[i];
}


/** ==================================================================================================================*\
                                TESTY JEDNOSTKOWE
\*====================================================================================================================*/

/**
  @brief Test inicjalizacji

  Funkcja testująca inicjalizacji CanTp.
*/

void Test_Of_CanTp_Init(void){
	//TEST1
    CanTp_ConfigType config = {
        .CanTpMainFunctionPeriod = 0.01,
        .CanTpMaxChannelCnt = 1,
        .pChannel = NULL
    };

    CanTpInit(&config);
    
    TEST_CHECK(CanTpState == CANTP_ON);
	//TEST2
	CanTpInit(NULL);
    TEST_CHECK(CanTpState == CANTP_ON);  // Stan się nie zmienia
}

/**
  @brief Test wersji

  Funkcja testująca zapisywana wersję CanTp.
*/
void Test_Of_CanTp_GetVersionInfo(void)
{
    Std_VersionInfoType versionInfo;

    // TEST1
    CanTp_GetVersionInfo(&versionInfo);

    TEST_CHECK(versionInfo.vendorID == 0x00u);
    TEST_CHECK(versionInfo.moduleID == CANTP_MODULE_ID);
    TEST_CHECK(versionInfo.sw_major_version == CANTP_SW_MAJOR_VERSION);
    TEST_CHECK(versionInfo.sw_minor_version == CANTP_SW_MINOR_VERSION);
    TEST_CHECK(versionInfo.sw_patch_version == CANTP_SW_PATCH_VERSION);

    // TEST2
    CanTp_GetVersionInfo(NULL);
      
}

/**
  @brief Test Wyłączenia modułu

  Funkcja testująca wyłączania modułu CanTp.
*/
void Test_Of_CanTp_Shutdown(void){

    CanTpState = CANTP_ON;

    CanTp_Shutdown();

    TEST_CHECK(CanTpState == CANTP_OFF);
    TEST_CHECK(CanTpState != CANTP_ON);
	
	//TEST2
	  CanTp_Shutdown();
    TEST_CHECK(CanTpState == CANTP_OFF);
}

/**
  @brief Test żądania transmisji

  Funkcja testująca żądanie transmisji PDU.
*/
void TestOf_CanTp_Transmit(void){

    PduIdType txPduId = 0x01;
    uint8 sduData[8] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
    PduInfoType pduInfo = {
        .SduDataPtr = sduData,
        .SduLength = 8,
        .MetaDataPtr = NULL
    };

    // TEST1: 
    CanTpState = CANTP_OFF;
    Std_ReturnType ret = CanTp_Transmit(txPduId, &pduInfo);
    TEST_CHECK(ret == E_NOT_OK);

    // TEST2:
    CanTpState = CANTP_ON;
    p_n_sdu->tx.taskState = CANTP_TX_WAIT;
    ret = CanTp_Transmit(txPduId, &pduInfo);

    TEST_CHECK(ret == E_OK);
    TEST_CHECK(p_n_sdu->tx.taskState == CANTP_TX_WAIT);
    TEST_CHECK(p_n_sdu->tx.has_meta_data == FALSE);
	
	// TEST3: Nieprawidłowe dane
    pduInfo.SduLength = 0;
    ret = CanTp_Transmit(txPduId, &pduInfo);
    TEST_CHECK(ret == E_NOT_OK);

  
}

/**
  @brief Test przerwania transmisji

  Funkcja testująca przerwanie transmisji PDU.
*/
void Test_Of_CanTp_CancelTransmit(void){

    PduIdType txPduId = 1;

    // TEST1: Brak dopasowania ID

    CanTp_ConfigPtr->pChannel->tx->CanTpTxNSduId = 2;

    Std_ReturnType ret = CanTp_CancelTransmit(txPduId);
    TEST_CHECK(ret == E_NOT_OK);

    // TEST2: Dopasowane ID
    CanTp_ConfigPtr->pChannel->tx->CanTpTxNSduId = txPduId;
    ret = CanTp_CancelTransmit(txPduId);
    TEST_CHECK(ret == E_OK);

}

/**
  @brief Test przerwania odbioru

  Funkcja testująca przerwanie odbioru PDU.
*/
void Test_Of_CanTp_CancelReceive(void){
    PduIdType rxPduId = 0x01;

    CanTp_ConfigPtr->pChannel->rx->CanTpRxNSduId = 2;

    Std_ReturnType ret = CanTp_CancelReceive(rxPduId);
    TEST_CHECK(ret == E_NOT_OK);

    // TEST2: Dopasowane ID
    CanTp_ConfigPtr->pChannel->rx->CanTpRxNSduId = rxPduId;
    ret = CanTp_CancelReceive(rxPduId);
    TEST_CHECK(ret == E_OK);
}


/**
  @brief Test zmiany wartosci parametru

  Funkcja testująca zmiane wartosci parametru.
*/

void Test_Of_CanTp_ChangeParameter(void){
   PduIdType id = 0x01;

    // Scenariusz 1: Zmiana TP_BS
    CanTpState = CANTP_ON;
    p_n_sdu->rx.shared_params.taskState = CANTP_RX_WAIT;

    Std_ReturnType ret = CanTp_ChangeParameter(id, TP_BS, 10);
    TEST_CHECK(ret == E_OK);
    TEST_CHECK(p_n_sdu->rx.shared_params.params.bs == 10);

    // Scenariusz 2: Zmiana TP_STMIN
    ret = CanTp_ChangeParameter(id, TP_STMIN, 20);
    TEST_CHECK(ret == E_OK);
    TEST_CHECK(p_n_sdu->rx.shared_params.params.st_min == 20);

    // Scenariusz 3: Nieobsługiwany parametr
    ret = CanTp_ChangeParameter(id, TP_BC, 30);
    TEST_CHECK(ret == E_NOT_OK);

    // Scenariusz 4: Moduł wyłączony
    CanTpState = CANTP_OFF;
    ret = CanTp_ChangeParameter(id, TP_BS, 40);
    TEST_CHECK(ret == E_NOT_OK);

      // Scenariusz 5: RX w stanie processing
    CanTpState = CANTP_ON;
    p_n_sdu->rx.shared_params.taskState = CANTP_RX_PROCESSING; 
    ret = CanTp_ChangeParameter(id, TP_BS, 40);
    TEST_CHECK(ret == E_NOT_OK);

    CanTpState = CANTP_ON;
    CanTpGeneralgPtr->CanTpChangeParameterApi = 0; 
    ret = CanTp_ChangeParameter(id, TP_BS, 40);
    TEST_CHECK(ret == E_NOT_OK);

    
}

/**
  @brief Test odczytu wartosci parametru

  Funkcja testująca odczyt wartosci parametru.
*/

void Test_Of_CanTp_ReadParameter(void){
    PduIdType id = 1;
    uint16 value;

    // Scenariusz 1: Odczyt TP_BS
    CanTpState = CANTP_ON;
    p_n_sdu->rx.shared_params.params.bs = 15;

    Std_ReturnType ret = CanTp_ReadParameter(id, TP_BS, &value);
    TEST_CHECK(ret == E_OK);
    TEST_CHECK(value == 15);

    // Scenariusz 2: Odczyt TP_STMIN
    p_n_sdu->rx.shared_params.params.st_min = 25;
    ret = CanTp_ReadParameter(id, TP_STMIN, &value);
    TEST_CHECK(ret == E_OK);
    TEST_CHECK(value == 25);

    // Scenariusz 3: Nieobsługiwany parametr
    ret = CanTp_ReadParameter(id, TP_BC, &value);
    TEST_CHECK(ret == E_NOT_OK);

    // Scenariusz 4: Moduł wyłączony
    CanTpState = CANTP_OFF;
    ret = CanTp_ReadParameter(id, TP_BS, &value);
    TEST_CHECK(ret == E_NOT_OK);

    CanTpState = CANTP_ON;
    CanTpGeneralgPtr->CanTpChangeParameterApi = 0; 
    ret = CanTp_ReadParameter(id, TP_BS, &value);
    TEST_CHECK(ret == E_NOT_OK);
}

/**
  @brief Test zarządzania modułem

  Funkcja testująca zarządzanie modułem CanTp.
*/

void Test_Of_CanTp_MainFunction(){
    // Scenariusz 1: Brak aktywnych timerów
    CanTp_TimerReset(&N_Ar_timer);
    CanTp_MainFunction();
    TEST_CHECK(N_Ar_timer.uiCounter == 0);

    // Scenariusz 2: Praca timerów RX
    CanTp_TimerStart(&N_Ar_timer);
    CanTp_MainFunction();
    TEST_CHECK(N_Ar_timer.uiCounter == 1);

    // Scenariusz 3: Timeout dla timerów RX
    N_Ar_timer.uiCounter = 99;
    CanTp_MainFunction();
    TEST_CHECK(N_Ar_timer.uiCounter == 100);
    TEST_CHECK(PduR_CanTpRxIndication_fake.call_count == 1);
}


void Test_CanTp_RxIndication(void) {
    PduIdType rxPduId = 1;
    uint8 sfData[8] = {0x00, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
    PduInfoType sfPdu = {.SduDataPtr = sfData, .SduLength = 8};

    // Scenariusz 1: Single Frame
    CanTpState = CANTP_ON;
    p_n_sdu = calloc(1, sizeof(CanTp_NSduType));
    p_n_sdu->rx.shared_params.taskState = CANTP_RX_WAIT;

    CanTp_RxIndication(rxPduId, &sfPdu);
    TEST_CHECK(p_n_sdu->rx.taskState != CANTP_RX_WAIT);

    // Cleanup
    free(p_n_sdu);
}



/**
  @brief Test Potwierdzenia transmisji

  Funkcja testującapotwierdzenie transmisji PDU.
*/
void Test_Of_CanTp_TxConfirmation(void){
    PduIdType txPduId = 1;

    // Scenariusz 1: Transmisja poprawna
    CanTpState = CANTP_ON;
    p_n_sdu->tx.taskState = CANTP_TX_PROCESSING;
    p_n_sdu->tx.cfg = calloc(1, sizeof(CanTp_TxNSduType));
    p_n_sdu->tx.cfg->CanTpTxNSduId = txPduId;

    CanTp_TxConfirmation(txPduId, E_OK);
    TEST_CHECK(p_n_sdu->tx.taskState == CANTP_TX_PROCESSING);

    // Scenariusz 2: Transmisja nieudana
    CanTp_TxConfirmation(txPduId, E_NOT_OK);
    TEST_CHECK(p_n_sdu->tx.taskState == CANTP_TX_WAIT);

}

/** ==================================================================================================================*\
                                TESTY FUNKCJI POMOCNICZYCH
\*====================================================================================================================*/
/**
  @brief Test Startu odbiornika

  Funkcja testująca reset odbiornika.
*/
void Test_CanTp_TimerStart(void) {
    CanTp_Timer_type timer = {TIMER_DISABLE, 0, 100};

    // Scenariusz 1: Timer wyłączony
    CanTp_TimerStart(&timer);
    TEST_CHECK(timer.eState == TIMER_ENABLE);
    TEST_CHECK(timer.uiCounter == 0);

    // Scenariusz 2: Timer już włączony
    timer.eState = TIMER_ENABLE;
    CanTp_TimerStart(&timer);
    TEST_CHECK(timer.eState == TIMER_ENABLE);
    TEST_CHECK(timer.uiCounter == 0);
}
/**
  @brief Test Resetu 

  Funkcja testująca reset timera.
*/
void Test_Of_TimerReset(void){     
    CanTp_Timer_type timer = {TIMER_ENABLE, 50, 100};

    // Scenariusz 1: Timer aktywny
    CanTp_TimerReset(&timer);
    TEST_CHECK(timer.eState == TIMER_DISABLE);
    TEST_CHECK(timer.uiCounter == 0);

    // Scenariusz 2: Timer wyłączony
    CanTp_TimerReset(&timer);
    TEST_CHECK(timer.eState == TIMER_DISABLE);
    TEST_CHECK(timer.uiCounter == 0);
}

/**
  @brief Test Wyliczania ticków timera

  Funkcja zliczająca ticki zegara
*/
void Test_CanTp_TimerTick(void) {
    CanTp_Timer_type timer = {TIMER_ENABLE, 0, 100};

    // Scenariusz 1: Timer aktywny
    Std_ReturnType ret = CanTp_TimerTick(&timer);
    TEST_CHECK(ret == E_OK);
    TEST_CHECK(timer.uiCounter == 1);

    // Scenariusz 2: Przekroczenie limitu
    timer.uiCounter = UINT32_MAX;
    ret = CanTp_TimerTick(&timer);
    TEST_CHECK(ret == E_NOT_OK);
    TEST_CHECK(timer.uiCounter == UINT32_MAX);

    // Scenariusz 3: Timer wyłączony
    timer.eState = TIMER_DISABLE;
    ret = CanTp_TimerTick(&timer);
    TEST_CHECK(ret == E_OK);
    TEST_CHECK(timer.uiCounter == UINT32_MAX);  // Licznik nie powinien się zmieniać
}

void Test_CanTp_TimerTimeout(void) {
    CanTp_Timer_type timer = {TIMER_ENABLE, 100, 100};

    // Scenariusz 1: Timeout
    Std_ReturnType ret = CanTp_TimerTimeout(&timer);
    TEST_CHECK(ret == E_NOT_OK);

    // Scenariusz 2: Brak timeoutu
    timer.uiCounter = 50;
    ret = CanTp_TimerTimeout(&timer);
    TEST_CHECK(ret == E_OK);
}


TEST_LIST = {
    { "Test of CanTp_MainFunction", Test_Of_CanTp_MainFunction },
    { "Test of CanTp_ReadParameter", Test_Of_CanTp_ReadParameter },
    { "Test of CanTp_ChangeParameter", Test_Of_CanTp_ChangeParameter },
	{ "Test of CanTp_TimerStart", Test_CanTp_TimerStart },
    // { "Test of CanTp_TimerReset", Test_CanTp_TimerReset },
    { "Test of CanTp_TimerTick", Test_CanTp_TimerTick },
    { "Test of CanTp_TimerTimeout", Test_CanTp_TimerTimeout },
    { "Test of CanTp_TxConfirmation", Test_Of_CanTp_TxConfirmation },
    { "Test of CanTp_CancelReceive", Test_Of_CanTp_CancelReceive },
    { "Test of CanTp_CancelTransmit", Test_Of_CanTp_CancelTransmit },
    { "Test of CanTp_Transmit", TestOf_CanTp_Transmit },
    { "Test of CanTp_Init", Test_Of_CanTp_Init },
    { "Test of CanTp_Shutdown", Test_Of_CanTp_Shutdown },
	{ "Test of CanTp_GetVersionInfo", Test_Of_CanTp_GetVersionInfo },
    { NULL, NULL }                           
};