#include "extension.h"
#include <fstream>
#include "util.h"
#include <dispatcher/dispatcher_itk.h>

#ifdef __cplusplus
extern "C" {
#endif
	int sync_draw_version(void* retValue) {
		printf("=========================================================\n");
		printf("sync_draw_version 开始执行\n");
		printf("=========================================================\n");
		tag_t parentItemTag = NULLTAG;
		//		tag_t session_tag = NULLTAG; //会话对象
		char *itemRev_Uid = NULL; //对象版本uid
		char *parentId = NULL; //对象ID
		char *parentVersion = NULL; //版本号
		char *user_Id = NULL; //当前登录TC用户

		//获取字符串类型的参数(对象版本UID)
		USERARG_get_string_argument(&itemRev_Uid);

		//当前登录TC用户
		USERARG_get_string_argument(&user_Id);

		//printf("user_id size == %s\n", strlen(user_Id));

		char *returnValue = (char *)MEM_alloc(128 * sizeof(char));
		memset(returnValue, 0, 128);
		strcpy(returnValue, "");

		FILE* logFile = NULL;
		CreateLogFile("sync_draw_version", &logFile);

		/*if (uid == NULL || user_Id == NULL || uid == "" || strlen(user_Id) < 0)
		{
			WriteLog(logFile, "参数不完整\n");
			strcpy(returnValue, G2U("参数不完整！"));
			goto CLEANUP;
		}*/
		WriteLog(logFile, "用户ID == %s\n", user_Id);
		WriteLog(logFile, "对象版本uid == %s\n", itemRev_Uid);

		POM_AM__set_application_bypass(true); //开启旁路

		//获取零组件对象版本
		ITK__convert_uid_to_tag(itemRev_Uid, &parentItemTag);

		if (parentItemTag == NULLTAG)
		{
			WriteLog(logFile, "uid转tag失败\n");
			strcpy(returnValue, G2U("对象版本uid不正确！"));
			goto CLEANUP;
		}
		WriteLog(logFile, "uid转tag成功\n");
		//获取会话
//		POM_ask_session(&session_tag);

		//获取父ID
		ITKCALL((AOM_ask_value_string(parentItemTag, "item_id", &parentId)));
		WriteLog(logFile, "parentId == %s\n", parentId);

		//获取父版本号
		ITKCALL((AOM_ask_value_string(parentItemTag, "item_revision_id", &parentVersion)));
		WriteLog(logFile, "parentVersion == %s\n", parentVersion);

		char ProviderName[128] = "FOXCONN";//Provider名称
		char ServiceName[128] = "creorevise";//服务名称
		int request_args_num = 4;//参数数量
		//request_args为参数值 键值对形式 "key=value",例如"prop=xxx"
		char** request_args = (char**)MEM_alloc(sizeof(char **));
		request_args[0] = (char *)MEM_alloc(sizeof(char[200]));
		request_args[1] = (char *)MEM_alloc(sizeof(char[200]));
		strcat(request_args[0], "user_id=");
		strcat(request_args[0], user_Id);
		strcat(request_args[1], "itemRev_Uid=");
		strcat(request_args[1], itemRev_Uid);

		tag_t trans_rqst = NULLTAG;
		WriteLog(logFile, "start send DISPATCHER_create_request!\n");
		ITKCALL(DISPATCHER_create_request(
			ProviderName,//the name of the provider that will process this request
			ServiceName,//the name of the service that will process this request
			3, //the priority to assign to the request(0 LOW to 3 HIGH)
			0, //the time at which to start this request
			0, //the time at which no more repeating requests of this same type will be processed.  
			//If the interval option for repeating is NOT selected, then this paramater is unused
			0, //the number of times to repeat this request
			// 0 - no repeating
			// 1 2 3 ... - number of times to repeat this task
			1,// the length of the primary and secondary object arrays
			&parentItemTag,// the array of primary objects  请求对象
			NULL,//the array of related secondary objects
			2, //the number of request arguments
			(const char **)request_args,//the array of request arguments in the format of
			NULL, // a string for use by the application creating the request for use in defining a type for the request (i.e. SYSTEM, THINCLIENT, etc.)
			0, //the number of datafiles you would like to upload
			0,
			0,
			&trans_rqst));
		WriteLog(logFile, "ending DISPATCHER_create_request!\n");
		strcpy(returnValue, G2U("Success"));

	CLEANUP:
		*((char**)retValue) = returnValue;
		//DOFREE(returnValue);
		DOFREE(parentId);
		DOFREE(parentVersion);
		DOFREE(user_Id);
		DOFREE(itemRev_Uid);
		return ITK_ok;
	}

#ifdef __cplusplus
}
#endif