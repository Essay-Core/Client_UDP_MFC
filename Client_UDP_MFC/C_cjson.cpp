#include "stdafx.h"
#include "C_cjson.h"
#include <stdio.h>
#include <string.h>

#define _CRT_SECURE_NO_WARNINGS

C_cjson::C_cjson()
{
}


C_cjson::~C_cjson()
{
}

int C_cjson::cjson_make()
{
	//确定数据格式
	//创建数据
	//将数据写入配置文件
	/*
	{
		"Listener":	{
			"ip":	"0.0.0.0",
				"port" : 9211,
				"maxConn" : 100000
		},
		"DB" : {
				"ip":	"127.0.0.1",
					"port" : 5432,
					"maxConn" : 32,
					"uid" : "dxh",
					"passwd" : "123456",
					"dbName" : "db_cctv"
			}
	}
	*/

	//创建数据对象
	cJSON *root = cJSON_CreateObject();
	cJSON *listener = cJSON_CreateObject();
	cJSON *db = cJSON_CreateObject();

	//加入对象及键值
	cJSON_AddStringToObject(listener, "ip", "0.0.0.0");
	cJSON_AddNumberToObject(listener, "port", 9211);
	cJSON_AddNumberToObject(listener, "maxConn", 10000);
	
	cJSON_AddStringToObject(db, "ip", "127.0.0.1");
	cJSON_AddNumberToObject(db, "port", 5432);
	cJSON_AddNumberToObject(db, "maxConn", 32);
	cJSON_AddStringToObject(db, "uid", "dxh");
	cJSON_AddStringToObject(db, "passwd", "123456");
	cJSON_AddStringToObject(db, "dbName", "db_cctv");

	cJSON_AddItemToObject(root,"listener", listener);
	cJSON_AddItemToObject(root, "DB", db);

	//将数据格式转换成UTF-8
	const char *s = cJSON_Print(root);

	FILE *f = fopen("./config.ini", "wt");
	if (!f)
	{
		printf("fopen error\n");
		return -1;
	}

	int ret = fwrite(s, 1, strlen(s), f);
	
	fclose(f);

	return 0;
}

//解析json数据,获取某个字段的数据
int C_cjson::cjson_decode_str(cJSON *json_str, const char* ch, char* ret_str)
{
	if (!json_str || !ch)
		return -1;

	//获取某个对象的成员
	cJSON *item = cJSON_GetObjectItem(json_str, ch);

	//输出成员字符串
	if (item->type == cJSON_String)
	{
		sprintf(ret_str, "%s", item->valuestring);
	}

	return 0;
}

//解析json数据,获取某个字段的数据
int C_cjson::cjson_decode_num(cJSON *json_str, const char* ch)
{
	if (!json_str || !ch)
		return -1;
	//获取某个对象的成员
	cJSON *item = cJSON_GetObjectItem(json_str, ch);
	if (item->type == cJSON_Number)
		return  item->valueint;
	else
		return -1;
}
