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


FAKE_VALUE_FUNC(Std_ReturnType, CanTp_SendSF, PduIdType, uint8*, PduLengthType);
FAKE_VALUE_FUNC(Std_ReturnType, CanTp_SendFF, PduIdType, PduLengthType);
FAKE_VOID_FUNC(CanTp_SendNextCF);
FAKE_VOID_FUNC(CanTp_FFReception, PduIdType, const PduInfoType*, CanPCI_Type*);
FAKE_VOID_FUNC(CanTp_SFReception, PduIdType, CanPCI_Type*, const PduInfoType*);
FAKE_VOID_FUNC(CanTp_CFReception, PduIdType, CanPCI_Type*, const PduInfoType*);
FAKE_VOID_FUNC(CanTp_FCReception, PduIdType, CanPCI_Type*);


/** ==================================================================================================================*\
                                TESTY JEDNOSTKOWE
\*====================================================================================================================*/


void Test_Of_CanTp_Init(void){
	//TEST1
    CanTp_ConfigType config = {
        .CanTpMainFunctionPeriod = 0.01,
        .CanTpMaxChannelCnt = 1,
        .pChannel = 0
    };

    CanTpInit(&config);
    
    TEST_CHECK(CanTpState == CANTP_ON);
	//TEST2
	CanTpInit(NULL);
    TEST_CHECK(CanTpState == CANTP_ON);  // Stan się nie zmienia
}


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


void Test_Of_CanTp_Shutdown(void){

    CanTpState = CANTP_ON;

    CanTp_Shutdown();

    TEST_CHECK(CanTpState == CANTP_OFF);
    TEST_CHECK(CanTpState != CANTP_ON);
	
	//TEST2
	  CanTp_Shutdown();
    TEST_CHECK(CanTpState == CANTP_OFF);
}


void TestOf_CanTp_Transmit(void){

    PduIdType txPduId = 0x01;
    uint8 sduData[8] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
    uint8 metaData[3] = {0x11, 0x22, 0x33};

    PduInfoType pduInfo = {
        .SduDataPtr = sduData,
        .SduLength = 6,
        .MetaDataPtr = metaData
    };

    RESET_FAKE(PduR_CanTpCopyTxData);
    RESET_FAKE(CanTp_SendSF);
    RESET_FAKE(PduR_CanTpTxConfirmation);
    RESET_FAKE(CanTp_SendFF);

    BufReq_ReturnType retPduR_CanTpCopyTxData[7] = {BUFREQ_OK, BUFREQ_E_NOT_OK, BUFREQ_E_OVFL, BUFREQ_OK, BUFREQ_OK, BUFREQ_OK, BUFREQ_OK};
    SET_RETURN_SEQ(PduR_CanTpCopyTxData, retPduR_CanTpCopyTxData, 7);

    Std_ReturnType retCanTp_SendSF[7] = {E_OK, E_OK, E_NOT_OK, E_OK, E_NOT_OK, E_NOT_OK, E_NOT_OK};
    SET_RETURN_SEQ(CanTp_SendSF, retCanTp_SendSF, 7);
    Std_ReturnType retCanTp_SendFF[7] = {E_OK, E_NOT_OK, E_NOT_OK, E_OK, E_NOT_OK, E_NOT_OK, E_NOT_OK};
    SET_RETURN_SEQ(CanTp_SendFF, retCanTp_SendFF, 7);

    // TEST1: 
    CanTpState = CANTP_OFF;
    Std_ReturnType ret = CanTp_Transmit(txPduId, &pduInfo);
    TEST_CHECK(ret == E_NOT_OK);



    // TEST2:

    p_n_sdu.tx.cfg.CanTpTxAddressingFormat = CANTP_EXTENDED;
    CanTpState = CANTP_ON;
    p_n_sdu.tx.taskState = CANTP_TX_WAIT;
    ret = CanTp_Transmit(txPduId, &pduInfo);

    TEST_CHECK(ret == E_OK);
    TEST_CHECK(p_n_sdu.tx.taskState == CANTP_TX_WAIT);
    TEST_CHECK(p_n_sdu.tx.has_meta_data == TRUE);
    TEST_CHECK(p_n_sdu.tx.saved_n_ta.CanTpNTa == 0x11);

    TEST_CHECK(PduR_CanTpCopyTxData_fake.call_count == 1);
    TEST_CHECK(CanTp_SendSF_fake.call_count == 1);

  //TEST3

    p_n_sdu.tx.cfg.CanTpTxAddressingFormat = CANTP_MIXED;
    CanTpState = CANTP_ON;
    p_n_sdu.tx.taskState = CANTP_TX_WAIT;
    ret = CanTp_Transmit(txPduId, &pduInfo);

    TEST_CHECK(ret == E_NOT_OK);
    TEST_CHECK(CanTp_ConfigPtr.pChannel.tx.CanTpTX_state == CANTP_TX_WAIT);
    TEST_CHECK(p_n_sdu.tx.has_meta_data == TRUE);
    TEST_CHECK(p_n_sdu.tx.saved_n_ae.CanTpNAe == 0x11);
    TEST_CHECK(PduR_CanTpCopyTxData_fake.call_count == 2);

    TEST_CHECK(PduR_CanTpTxConfirmation_fake.call_count == 1);
    TEST_CHECK(CanTp_SendSF_fake.call_count == 1);

  //TEST4

    p_n_sdu.tx.cfg.CanTpTxAddressingFormat = CANTP_NORMALFIXED;
    CanTpState = CANTP_ON;
    p_n_sdu.tx.taskState = CANTP_TX_WAIT;
    pduInfo.SduLength = 8;
    ret = CanTp_Transmit(txPduId, &pduInfo);

    TEST_CHECK(ret == E_OK);
    TEST_CHECK(CanTp_ConfigPtr.pChannel.tx.CanTpTX_state == CANTP_TX_WAIT);
    TEST_CHECK(p_n_sdu.tx.has_meta_data == TRUE);
    TEST_CHECK(p_n_sdu.tx.saved_n_sa.CanTpNSa == 0x11);
    TEST_CHECK(p_n_sdu.tx.saved_n_ta.CanTpNTa == 0x22);

    TEST_CHECK(CanTp_SendFF_fake.call_count == 1);
	
    //TEST5

    p_n_sdu.tx.cfg.CanTpTxAddressingFormat = CANTP_MIXED29BIT;
    CanTpState = CANTP_ON;
    p_n_sdu.tx.taskState = CANTP_TX_WAIT;
    pduInfo.SduLength = 8;
    ret = CanTp_Transmit(txPduId, &pduInfo);

    TEST_CHECK(ret == E_NOT_OK);
    TEST_CHECK(CanTp_ConfigPtr.pChannel.tx.CanTpTX_state == CANTP_TX_WAIT);
    TEST_CHECK(p_n_sdu.tx.has_meta_data == TRUE);
    TEST_CHECK(p_n_sdu.tx.saved_n_sa.CanTpNSa == 0x11);
    TEST_CHECK(p_n_sdu.tx.saved_n_ta.CanTpNTa == 0x22);
    TEST_CHECK(p_n_sdu.tx.saved_n_ae.CanTpNAe == 0x33);

    TEST_CHECK(CanTp_SendFF_fake.call_count == 2);

    //TEST6

    p_n_sdu.tx.cfg.CanTpTxAddressingFormat = CANTP_MIXED29BIT;
    CanTpState = CANTP_ON;
    p_n_sdu.tx.taskState = CANTP_TX_WAIT;
    pduInfo.SduLength = 3;
    ret = CanTp_Transmit(txPduId, &pduInfo);

    TEST_CHECK(ret == E_OK);
    TEST_CHECK(p_n_sdu.tx.has_meta_data == TRUE);
    TEST_CHECK(PduR_CanTpCopyTxData_fake.call_count == 3);


    //TEST7

    p_n_sdu.tx.cfg.CanTpTxAddressingFormat = CANTP_MIXED29BIT;
    CanTpState = CANTP_ON;
    p_n_sdu.tx.taskState = CANTP_TX_WAIT;
    pduInfo.MetaDataPtr = NULL;
    ret = CanTp_Transmit(txPduId, &pduInfo);

    TEST_CHECK(ret == E_OK);
    TEST_CHECK(p_n_sdu.tx.has_meta_data == FALSE);
    TEST_CHECK(PduR_CanTpCopyTxData_fake.call_count == 4);

     //TEST8

    p_n_sdu.tx.cfg.CanTpTxAddressingFormat = CANTP_MIXED29BIT;
    CanTpState = CANTP_ON;
    p_n_sdu.tx.taskState = CANTP_TX_PROCESSING;
    pduInfo.MetaDataPtr = NULL;
    ret = CanTp_Transmit(txPduId, &pduInfo);

    TEST_CHECK(ret == E_NOT_OK);
    TEST_CHECK(p_n_sdu.tx.has_meta_data == FALSE);
    TEST_CHECK(PduR_CanTpCopyTxData_fake.call_count == 4);


  
}


void Test_Of_CanTp_CancelTransmit(void){

    PduIdType txPduId = 1;

    CanTp_ConfigPtr.pChannel.tx.CanTpTxNSduId = 2;

    Std_ReturnType ret = CanTp_CancelTransmit(txPduId);
    TEST_CHECK(ret == E_NOT_OK);

    CanTp_ConfigPtr.pChannel.tx.CanTpTxNSduId = txPduId;
    ret = CanTp_CancelTransmit(txPduId);
    TEST_CHECK(ret == E_OK);

}


void Test_Of_CanTp_CancelReceive(void){
    PduIdType rxPduId = 0x01;

    CanTp_ConfigPtr.pChannel.rx.CanTpRxNSduId = 2;

    Std_ReturnType ret = CanTp_CancelReceive(rxPduId);
    TEST_CHECK(ret == E_NOT_OK);

    CanTp_ConfigPtr.pChannel.rx.CanTpRxNSduId = rxPduId;
    ret = CanTp_CancelReceive(rxPduId);
    TEST_CHECK(ret == E_OK);
}



void Test_Of_CanTp_ChangeParameter(void){
    PduIdType id = 0x01;
    CanTpGeneralgPtr.CanTpChangeParameterApi = 1; 

    CanTpState = CANTP_ON;
    p_n_sdu.rx.shared_params.taskState = CANTP_RX_WAIT;
    TPParameterType Parameter_Test = TP_BS;
    uint16 value = 10;

    Std_ReturnType ret = CanTp_ChangeParameter(id, Parameter_Test, value);
    TEST_CHECK(ret == E_OK);
    TEST_CHECK(p_n_sdu.rx.shared_params.params.bs == 10);

    ret = CanTp_ChangeParameter(id, TP_STMIN, 20);
    TEST_CHECK(ret == E_OK);
    TEST_CHECK(p_n_sdu.rx.shared_params.params.st_min == 20);

    ret = CanTp_ChangeParameter(id, TP_BC, 30);
    TEST_CHECK(ret == E_NOT_OK);

    CanTpState = CANTP_OFF;
    ret = CanTp_ChangeParameter(id, TP_BS, 40);
    TEST_CHECK(ret == E_NOT_OK);

    CanTpState = CANTP_ON;
    p_n_sdu.rx.shared_params.taskState = CANTP_RX_PROCESSING; 
    ret = CanTp_ChangeParameter(id, TP_BS, 40);
    TEST_CHECK(ret == E_NOT_OK);

    p_n_sdu.rx.shared_params.taskState = CANTP_RX_WAIT; 
    CanTpState = CANTP_ON;
    CanTpGeneralgPtr.CanTpChangeParameterApi = 0; 
    ret = CanTp_ChangeParameter(id, TP_BS, 40);
    TEST_CHECK(ret == E_NOT_OK);

    
}


void Test_Of_CanTp_ReadParameter(void){
    PduIdType id = 1;
    uint16 value;

    CanTpState = CANTP_ON;
    p_n_sdu.rx.shared_params.params.bs = 15;
    CanTpGeneralgPtr.CanTpReadParameterApi  = 1;

    Std_ReturnType ret = CanTp_ReadParameter(id, TP_BS, &value);
    TEST_CHECK(ret == E_OK);
    TEST_CHECK(value == 15);

    
    p_n_sdu.rx.shared_params.params.st_min = 25;
    ret = CanTp_ReadParameter(id, TP_STMIN, &value);
    TEST_CHECK(ret == E_OK);
    TEST_CHECK(value == 25);

    ret = CanTp_ReadParameter(id, TP_BC, &value);
    TEST_CHECK(ret == E_NOT_OK);

    CanTpState = CANTP_OFF;
    ret = CanTp_ReadParameter(id, TP_BS, &value);
    TEST_CHECK(ret == E_NOT_OK);

    CanTpState = CANTP_ON;
    CanTpGeneralgPtr.CanTpReadParameterApi  = 0; 
    ret = CanTp_ReadParameter(id, TP_BS, &value);
    TEST_CHECK(ret == E_NOT_OK);
}

void Test_Of_CanTp_MainFunction(){

    RESET_FAKE(PduR_CanTpRxIndication);
    RESET_FAKE(PduR_CanTpTxConfirmation);


    CanTp_TimerReset(&N_Ar_timer);
    CanTp_MainFunction();
    TEST_CHECK(N_Ar_timer.uiCounter == 0);

    CanTp_TimerStart(&N_Ar_timer);
    CanTp_MainFunction();
    TEST_CHECK(N_Ar_timer.uiCounter == 1);

    N_Ar_timer.uiCounter = 999;
    CanTp_MainFunction();
    TEST_CHECK(N_Ar_timer.uiCounter == 0);

    N_As_timer.uiCounter = 990;
    CanTp_TimerStart(&N_As_timer);
    CanTp_MainFunction();

    TEST_CHECK(N_As_timer.uiCounter == 991);
    TEST_CHECK(N_As_timer.eState == TIMER_ENABLE);
    TEST_CHECK(N_Bs_timer.uiCounter == 0);
    TEST_CHECK(N_Bs_timer.eState == TIMER_DISABLE);
    TEST_CHECK(N_Cs_timer.uiCounter == 0);
    TEST_CHECK(N_Cs_timer.eState == TIMER_DISABLE);

    TEST_CHECK(PduR_CanTpTxConfirmation_fake.call_count == 0);
    

    CanTp_TimerStart(&N_As_timer);
    CanTp_TimerStart(&N_Bs_timer);
    CanTp_TimerStart(&N_Cs_timer);

    CanTp_TimerStart(&N_Ar_timer);
    CanTp_TimerStart(&N_Br_timer);
    CanTp_TimerStart(&N_Cr_timer);

    N_As_timer.uiCounter = 999;
    N_Bs_timer.uiCounter = 999;
    N_Cs_timer.uiCounter = 999;
    N_Ar_timer.uiCounter = 999;
    N_Br_timer.uiCounter = 999;
    N_Cr_timer.uiCounter = 999;
    RESET_FAKE(PduR_CanTpTxConfirmation);
    RESET_FAKE(PduR_CanTpRxIndication);
    CanTp_MainFunction();
  
    TEST_CHECK(N_As_timer.uiCounter == 0);
    TEST_CHECK(N_As_timer.eState == TIMER_DISABLE);
    TEST_CHECK(N_Bs_timer.uiCounter == 0);
    TEST_CHECK(N_Bs_timer.eState == TIMER_DISABLE);
    TEST_CHECK(N_Cs_timer.uiCounter == 0);
    TEST_CHECK(N_Cs_timer.eState == TIMER_DISABLE);

    TEST_CHECK(N_Ar_timer.uiCounter == 0);
    TEST_CHECK(N_Ar_timer.eState == TIMER_DISABLE);
    TEST_CHECK(N_Br_timer.uiCounter == 0);
    TEST_CHECK(N_Br_timer.eState == TIMER_DISABLE);
    TEST_CHECK(N_Cr_timer.uiCounter == 0);
    TEST_CHECK(N_Cr_timer.eState == TIMER_DISABLE);

    TEST_CHECK(PduR_CanTpTxConfirmation_fake.call_count == 3);
    TEST_CHECK(PduR_CanTpRxIndication_fake.call_count == 3);

}


void Test_CanTp_RxIndication(void) {
    PduIdType rxPduId = 1;
    CanTpState = CANTP_ON;

    PduInfoType testPduInfo;
    uint8 testSduData[8] = {SF<<4, 0, 0, 0, 0, 0, 0, 0};
    testPduInfo.SduDataPtr = testSduData;

    RESET_FAKE(CanTp_FFReception);
    RESET_FAKE(CanTp_SFReception);
    RESET_FAKE(CanTp_FCReception);
    RESET_FAKE(CanTp_CFReception);
    RESET_FAKE(PduR_CanTpRxIndication);

   
    CanTpState = CANTP_ON;
    p_n_sdu.rx.shared_params.taskState = CANTP_RX_WAIT;

    CanTp_RxIndication(rxPduId, &testPduInfo);
    TEST_CHECK(CanTp_SFReception_fake.call_count == 1);

    testSduData[0] = FF<<4;
    testPduInfo.SduDataPtr = testSduData;

    CanTp_RxIndication(rxPduId, &testPduInfo);
    TEST_CHECK(CanTp_FFReception_fake.call_count == 1);

    testSduData[0] = FC<<4;
    testPduInfo.SduDataPtr = testSduData;

    CanTp_RxIndication(rxPduId, &testPduInfo);
    TEST_CHECK(CanTp_FCReception_fake.call_count == 1);

    testSduData[0] = CF<<4;
    testPduInfo.SduDataPtr = testSduData;

    CanTp_RxIndication(rxPduId, &testPduInfo);
    TEST_CHECK(CanTp_SFReception_fake.call_count == 1);
    TEST_CHECK(CanTp_FFReception_fake.call_count == 1);
    TEST_CHECK(CanTp_FCReception_fake.call_count == 1);

    p_n_sdu.rx.shared_params.taskState = CANTP_RX_PROCESSING;

    testSduData[0] = CF<<4;
    testPduInfo.SduDataPtr = testSduData;

    CanTp_RxIndication(rxPduId, &testPduInfo);
    TEST_CHECK(CanTp_CFReception_fake.call_count == 1);

    testSduData[0] = FC<<4;
    testPduInfo.SduDataPtr = testSduData;

    CanTp_RxIndication(rxPduId, &testPduInfo);
    TEST_CHECK(CanTp_FCReception_fake.call_count == 2);

    testSduData[0] = FF<<4;
    testPduInfo.SduDataPtr = testSduData;

    CanTp_RxIndication(rxPduId, &testPduInfo);
    TEST_CHECK(CanTp_FFReception_fake.call_count == 2);
    TEST_CHECK(PduR_CanTpRxIndication_fake.call_count == 1);
    TEST_CHECK(CanTp_ConfigPtr.pChannel.rx.CanTpRX_state == CANTP_RX_WAIT);

    p_n_sdu.rx.shared_params.taskState = CANTP_RX_PROCESSING;

    testSduData[0] = SF<<4;
    testPduInfo.SduDataPtr = testSduData;

    CanTp_RxIndication(rxPduId, &testPduInfo);
    TEST_CHECK(CanTp_SFReception_fake.call_count == 2);
    TEST_CHECK(PduR_CanTpRxIndication_fake.call_count == 2);
    TEST_CHECK(CanTp_ConfigPtr.pChannel.rx.CanTpRX_state == CANTP_RX_WAIT);

}


void Test_Of_CanTp_TxConfirmation(void){

  RESET_FAKE(PduR_CanTpCopyTxData);
  RESET_FAKE(CanTp_SendNextCF);
  RESET_FAKE(PduR_CanTpTxConfirmation);
  RESET_FAKE(PduR_CanTpRxIndication);

    
    CanTpState = CANTP_ON;
    CanTp_ConfigPtr.pChannel.rx.CanTpRxNSduId = 2;
    CanTp_ConfigPtr.pChannel.rx.CanTpRX_state = CANTP_RX_PROCESSING;
    CanTp_ConfigPtr.pChannel.tx.CanTpTxNSduId = 1;
    CanTp_ConfigPtr.pChannel.tx.CanTpTX_state = CANTP_TX_PROCESSING;

    CanTp_TxConfirmation(2, E_NOT_OK);
    TEST_CHECK(CanTp_ConfigPtr.pChannel.rx.CanTpRX_state == CANTP_RX_WAIT);
    TEST_CHECK(PduR_CanTpRxIndication_fake.call_count == 1 );

    
    CanTp_TxConfirmation(2, E_OK);
    TEST_CHECK(CanTp_ConfigPtr.pChannel.rx.CanTpRX_state == CANTP_RX_WAIT);
    TEST_CHECK(PduR_CanTpRxIndication_fake.call_count == 1 );

    CanTp_TxConfirmation(1, E_OK);
    TEST_CHECK(CanTp_SendNextCF_fake.call_count == 1 );

    CanTp_TxConfirmation(1, E_NOT_OK);
    TEST_CHECK(CanTp_ConfigPtr.pChannel.rx.CanTpRX_state == CANTP_RX_WAIT);
    TEST_CHECK(PduR_CanTpTxConfirmation_fake.call_count == 1);


}

/** ==================================================================================================================*\
                                TESTY FUNKCJI POMOCNICZYCH
\*====================================================================================================================*/

void Test_CanTp_TimerStart(void) {
    CanTp_Timer_type timer = {TIMER_DISABLE, 0, 100};

    CanTp_TimerStart(&timer);
    TEST_CHECK(timer.eState == TIMER_ENABLE);
    TEST_CHECK(timer.uiCounter == 0);

    timer.eState = TIMER_ENABLE;
    CanTp_TimerStart(&timer);
    TEST_CHECK(timer.eState == TIMER_ENABLE);
    TEST_CHECK(timer.uiCounter == 0);
}

void Test_Of_TimerReset(void){     
    CanTp_Timer_type timer = {TIMER_ENABLE, 50, 100};

    CanTp_TimerReset(&timer);
    TEST_CHECK(timer.eState == TIMER_DISABLE);
    TEST_CHECK(timer.uiCounter == 0);

    CanTp_TimerReset(&timer);
    TEST_CHECK(timer.eState == TIMER_DISABLE);
    TEST_CHECK(timer.uiCounter == 0);
}


void Test_CanTp_TimerTick(void) {
    CanTp_Timer_type timer = {TIMER_ENABLE, 0, 100};

    Std_ReturnType ret = CanTp_TimerTick(&timer);
    TEST_CHECK(ret == E_OK);
    TEST_CHECK(timer.uiCounter == 1);

    timer.uiCounter = UINT32_MAX;
    ret = CanTp_TimerTick(&timer);
    TEST_CHECK(ret == E_NOT_OK);
    TEST_CHECK(timer.uiCounter == UINT32_MAX);

    timer.eState = TIMER_DISABLE;
    ret = CanTp_TimerTick(&timer);
    TEST_CHECK(ret == E_OK);
    TEST_CHECK(timer.uiCounter == UINT32_MAX);  
}

void Test_CanTp_TimerTimeout(void) {
    CanTp_Timer_type timer = {TIMER_ENABLE, 100, 100};

    Std_ReturnType ret = CanTp_TimerTimeout(&timer);
    TEST_CHECK(ret == E_NOT_OK);

    timer.uiCounter = 50;
    ret = CanTp_TimerTimeout(&timer);
    TEST_CHECK(ret == E_OK);
}


TEST_LIST = {
    { "Test of CanTp_MainFunction", Test_Of_CanTp_MainFunction },
    { "Test of CanTp_ReadParameter", Test_Of_CanTp_ReadParameter },
    { "Test of CanTp_ChangeParameter", Test_Of_CanTp_ChangeParameter },
	  { "Test of CanTp_TimerStart", Test_CanTp_TimerStart },
    { "Test of CanTp_TimerReset", Test_Of_TimerReset },
    { "Test of CanTp_TimerTick", Test_CanTp_TimerTick },
    { "Test of CanTp_TimerTimeout", Test_CanTp_TimerTimeout },
    { "Test of CanTp_RxIndication", Test_CanTp_RxIndication },
    { "Test of CanTp_TxConfirmation", Test_Of_CanTp_TxConfirmation },
    { "Test of CanTp_CancelReceive", Test_Of_CanTp_CancelReceive },
    { "Test of CanTp_CancelTransmit", Test_Of_CanTp_CancelTransmit },
    { "Test of CanTp_Transmit", TestOf_CanTp_Transmit },
    { "Test of CanTp_Init", Test_Of_CanTp_Init },
    { "Test of CanTp_Shutdown", Test_Of_CanTp_Shutdown },
	{ "Test of CanTp_GetVersionInfo", Test_Of_CanTp_GetVersionInfo },
    { NULL, NULL }                           
};