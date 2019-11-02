#include "stdafx.h"
#include "F_cjson.h"

int getJsStr(char *json_str, const char* ch, char* ret_str)
{
	if (!json_str || !ch)
		return -1;

	//获取json数据句柄
	cJSON *json = cJSON_Parse(json_str);
	//获取某个对象的成员
	cJSON *item = cJSON_GetObjectItem(json, ch);

	//输出成员字符串
	if (item->type == cJSON_String)
	{
		sprintf(ret_str, "%s", item->valuestring);
	}

	return 0;
}

int getJsInt(char *json_str, const char* ch, int *ret_int)
{
	if (!json_str || !ch)
		return -1;

	//获取json数据句柄
	cJSON *json = cJSON_Parse(json_str);

	//获取某个对象的成员
	cJSON *item = cJSON_GetObjectItem(json, ch);

	if (item->type == cJSON_Number)
	{
		*ret_int = item->valueint;
	}

	return 0;
}

//将文件转还为json格式的数据
int insertJsonStr(char*jsonStr, const char* ch, char* subStr)
{
	if (!jsonStr || !ch || !subStr)
	{
		return -1;
	}

	cJSON *root;
	//创建json数据对象,即创建一个{}
	root = cJSON_CreateObject();

	//加入键值及字符串
	cJSON_AddStringToObject(root, ch, subStr);

	//数据格式转换为utf-8
	char *s = cJSON_PrintUnformatted(root);

	if (root)
	{
		//删除json数据
		cJSON_Delete(root);
	}

	strcpy(jsonStr, s);

	return 0;
}