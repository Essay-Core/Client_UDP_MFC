#pragma once

typedef struct stCtrlMsg
{
	int type;

}stCtrlMsg;

typedef struct stHeadMsg  
{
	UINT32 msg_id;
	UINT32 data[350];
}stHeadMsg;


typedef enum
{
	SOCK_MSG_NULL,
	SOCK_MSG_START_SEND_DATA,
	SOCK_MSG_SENDING_DATA,
	SOCK_MSG_STOP_SEND_DATA,
	SOCK_MSG_START_GET_DATA,	//开始
	SOCK_MSG_STOP_GET_DATA		//结束

} MSG_ID;


typedef struct msg
{
	UINT32 msg_id;
	UINT32 data[350]; //_DATA

}_MSG, *pMSG;

typedef struct send_data
{
	UINT32 length;
	UINT32 sequNum; //add 2019年6月25日13:46:03
	UINT32 data[340];
	
}_DATA, *pDATA;


typedef struct stAdcMsg
{


}stAdcMsg;

typedef struct stFpgaMsg
{
	char m_fpga_reg[32];
	char m_fpga_val[32];

}stFpgaMsg;

//////////////////////////////////
typedef enum 
{
	FPGA_OPERA_NULL,
	FPGA_OPERA_WRITE,
	FPGA_OPERA_READ,
	FPGA_OPERA_ADC_WRITE,
	FPGA_OPERA_ADC_READ
}FPGA_OPERA_ID;

typedef struct stAdcConfig
{
	UINT32 channel;
	UINT32 operation; 
	UINT32 size;
	UINT32 addr;
	UINT32 data;
}stAdcConfig;

typedef struct stFpgaConfig
{
	UINT32 reg;
	UINT32 val;
}stFpgaConfig;

typedef struct stHeadFpgaConfig
{
	UINT32 opera;	//FPGA_OPERA_ID
	UINT32 dada[5]; //stAdcConfig | stFpgaConfig
}stHeadFpgaConfig;

// ADD TIME: 2019年6月22日14:30:06
typedef struct stInitMsg
{
	char local_ip[32];
	char local_port[32];

	char msg_ip[32];
	char msg_port[12];

	char config_ip[32];
	char config_port[12];

	char fpga_reg[12];
	char fpga_val[12];

	char adc_channel[12];
	char adc_size[12];
	char adc_addr[12];
	char adc_data[12];

}stInitMsg;

typedef struct stIpPortInitMsg
{
	char ip_romote[32];
	char ip_local[32];

	int port_ctrl;
	int port_data;
	int port_config;
	
}stIpPortInitMsg;
