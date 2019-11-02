#pragma once

#include "cJson/cJSON.h"

class C_cjson
{
public:
	C_cjson();
	~C_cjson();

	int cjson_make();
	//int cjson_decode(stSvrInfo *svrInfo, stDbInfo *dbInfo);
	int cjson_decode_str(cJSON *json_str, const char* ch, char* ret_str);
	int cjson_decode_num(cJSON *json_str, const char* ch);
};

int cjson_make();
//int cjson_decode(stSvrInfo *svrInfo, stDbInfo *dbInfo);
int cjson_decode_str(cJSON *json_str, const char* ch, char* ret_str);
int cjson_decode_num(cJSON *json_str, const char* ch);
