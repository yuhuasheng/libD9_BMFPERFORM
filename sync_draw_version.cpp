#include "extension.h"
#include <fstream>
#include "util.h"
#include <dispatcher/dispatcher_itk.h>

#ifdef __cplusplus
extern "C" {
#endif
	int sync_draw_version(void* retValue) {
		printf("=========================================================\n");
		printf("sync_draw_version ��ʼִ��\n");
		printf("=========================================================\n");
		tag_t parentItemTag = NULLTAG;
		//		tag_t session_tag = NULLTAG; //�Ự����
		char *itemRev_Uid = NULL; //����汾uid
		char *parentId = NULL; //����ID
		char *parentVersion = NULL; //�汾��
		char *user_Id = NULL; //��ǰ��¼TC�û�

		//��ȡ�ַ������͵Ĳ���(����汾UID)
		USERARG_get_string_argument(&itemRev_Uid);

		//��ǰ��¼TC�û�
		USERARG_get_string_argument(&user_Id);

		//printf("user_id size == %s\n", strlen(user_Id));

		char *returnValue = (char *)MEM_alloc(128 * sizeof(char));
		memset(returnValue, 0, 128);
		strcpy(returnValue, "");

		FILE* logFile = NULL;
		CreateLogFile("sync_draw_version", &logFile);

		/*if (uid == NULL || user_Id == NULL || uid == "" || strlen(user_Id) < 0)
		{
			WriteLog(logFile, "����������\n");
			strcpy(returnValue, G2U("������������"));
			goto CLEANUP;
		}*/
		WriteLog(logFile, "�û�ID == %s\n", user_Id);
		WriteLog(logFile, "����汾uid == %s\n", itemRev_Uid);

		POM_AM__set_application_bypass(true); //������·

		//��ȡ���������汾
		ITK__convert_uid_to_tag(itemRev_Uid, &parentItemTag);

		if (parentItemTag == NULLTAG)
		{
			WriteLog(logFile, "uidתtagʧ��\n");
			strcpy(returnValue, G2U("����汾uid����ȷ��"));
			goto CLEANUP;
		}
		WriteLog(logFile, "uidתtag�ɹ�\n");
		//��ȡ�Ự
//		POM_ask_session(&session_tag);

		//��ȡ��ID
		ITKCALL((AOM_ask_value_string(parentItemTag, "item_id", &parentId)));
		WriteLog(logFile, "parentId == %s\n", parentId);

		//��ȡ���汾��
		ITKCALL((AOM_ask_value_string(parentItemTag, "item_revision_id", &parentVersion)));
		WriteLog(logFile, "parentVersion == %s\n", parentVersion);

		char ProviderName[128] = "FOXCONN";//Provider����
		char ServiceName[128] = "creorevise";//��������
		int request_args_num = 4;//��������
		//request_argsΪ����ֵ ��ֵ����ʽ "key=value",����"prop=xxx"
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
			&parentItemTag,// the array of primary objects  �������
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