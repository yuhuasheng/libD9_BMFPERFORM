#include "extension.h"
#include "errorMsg.h"
#include "util.h"

extern "C" int POM_AM__set_application_bypass(logical bypass);
using namespace oracle::occi;
int checkBOMAttr(char *parentId, char *parentVersion, ResultSet *pRs, char **error_msg);

#ifdef __cplusplus
extern "C" {
#endif

	int D9_CheckBOMProperties(EPM_rule_message_t msg)
	{
		EPM_decision_t decision = EPM_go;
		int
			ifail = ITK_ok,
			attach_object_num = 0,
			structCount = 0;

		tag_t
			root_task = NULL_TAG,
			target_obj = NULL_TAG,
			tItemRev = NULL_TAG,
			window = NULL_TAG,
			top_bomline = NULL_TAG;

		tag_t
			*attach_object_list = NULLTAG,
			*struct_tag_list = NULLTAG;

		char
			*error_msg = NULL,			
			*parentId = NULL,
			*parentVersion = NULL;

		string
			childId,
			childVersion;

		Environment *env = NULL;
		Connection *conn = NULL;
		Statement *pStmt = NULL;
		ResultSet *pRs = NULL;

		char *ip = new char[32];
		char *userName = new char[32];
		char *password = new char[32];
		char *port = new char[32];
		char *sid = new char[32];
		char *connectStr = new char[128];
		strcpy(connectStr, "");

		vector<string> *dbInfoVec = new vector<string>();
		map<string, string> *dbInfoMap = new map<string, string>();
		map<string, string>::reverse_iterator *dbInfoIter = new map<string, string>::reverse_iterator();
		getPreferenceByName("D9_DB_Info", *dbInfoVec);
		getHashMapPreference(*dbInfoVec, *dbInfoMap);
		for ((*dbInfoIter) = (*dbInfoMap).rbegin(); (*dbInfoIter) != (*dbInfoMap).rend(); (*dbInfoIter)++) {
			string tmpKey = (*dbInfoIter)->first;
			string tmpValue = (*dbInfoIter)->second;
			if (strcmp(tmpKey.c_str(), "IP") == 0) {
				printf("ip decoded== %s\n", base64_decode(tmpValue));
				strcpy(ip, base64_decode(tmpValue).c_str());
			}
			else if (strcmp(tmpKey.c_str(), "UserName") == 0) {
				printf("userName decoded== %s\n", base64_decode(tmpValue));
				strcpy(userName, base64_decode(tmpValue).c_str());
			}
			else if (strcmp(tmpKey.c_str(), "Password") == 0) {
				printf("password decoded== %s\n", base64_decode(tmpValue));
				strcpy(password, base64_decode(tmpValue).c_str());
			}
			else if (strcmp(tmpKey.c_str(), "Port") == 0) {
				printf("port decoded== %s\n", base64_decode(tmpValue));
				strcpy(port, base64_decode(tmpValue).c_str());
			}
			else if (strcmp(tmpKey.c_str(), "SID") == 0) {
				printf("sid decoded== %s\n", base64_decode(tmpValue));
				strcpy(sid, base64_decode(tmpValue).c_str());
			}
		}

		printf("ip == %s\n", ip);
		TC_write_syslog("ip == %s\n", ip);
		printf("userName == %s\n", userName);
		TC_write_syslog("userName == %s\n", userName);
		printf("password == %s\n", password);
		TC_write_syslog("password == %s\n", password);
		printf("port == %s\n", port);
		TC_write_syslog("port == %s\n", port);
		printf("sid == %s\n", sid);
		TC_write_syslog("sid == %s\n", sid);

		tc_strcat(connectStr, ip);		
		tc_strcat(connectStr, ":");
		tc_strcat(connectStr, port);
		tc_strcat(connectStr, "/");
		tc_strcat(connectStr, sid);
		printf("connectStr == %s\n", connectStr);
		TC_write_syslog("connectStr == %s\n", connectStr);

		env = Environment::createEnvironment();
		if (NULL == env) {			
			printf("createEnvironment error... \n");
			TC_write_syslog("createEnvironment error... \n");
			decision = EPM_nogo;
			goto CLEANUP;
		}

		printf("createEnvironment success... \n");
		TC_write_syslog("createEnvironment success... \n");

		conn = env-> createConnection(userName, password, connectStr);
		if (NULL == conn) {
			printf("createConnection error... \n");
			TC_write_syslog("createConnection error... \n");
			decision = EPM_nogo;
			goto CLEANUP;
		}

		printf("createConnection success... \n");
		TC_write_syslog("createConnection success... \n");

		POM_AM__set_application_bypass(true); // ������·

		printf("\n=================     D9_CheckBOMProperties start      ======================\n");
		TC_write_syslog("\n=================   D9_CheckBOMProperties   start   ==================\n");

		error_msg = (char*)MEM_alloc(sizeof(char) * 9000);
		memset(error_msg, '\0', 9000);
		tc_strcpy(error_msg, "");

		ITKCALL(EPM_ask_root_task(msg.task, &root_task)); // ��ȡ���̸�����
		ITKCALL(EPM_ask_attachments(root_task, EPM_target_attachment, &attach_object_num, &attach_object_list));

		string *findTableSql = new string();
		
		for (size_t i = 0; i < attach_object_num; i++)
		{
			logical
				isRev = true,
				isDS = false,
				isForm = false,
				isBOMViewRev = false;
			target_obj = attach_object_list[i];

			GTCTYPE_is_type_of(target_obj, "Dataset", &isDS);
			if (isDS) // �ж��Ƿ�Ϊ���ݼ�
			{
				continue;
			}
				
			GTCTYPE_is_type_of(target_obj, "PSBOMViewRevision", &isBOMViewRev);
			if (isBOMViewRev) // �ж��Ƿ�ΪBOM��ͼ
			{
				continue;
			}

			GTCTYPE_is_type_of(target_obj, "Form", &isForm);
			if (isForm) // �ж��Ƿ�ΪForm
			{
				continue;
			}


			ITKCALL(GTCTYPE_is_type_of(target_obj, "ItemRevision", &isRev));
			if (!isRev) // �ж��Ƿ�Ϊ����汾
			{
				continue; 
			}

			else
			{
				tItemRev = target_obj;
				ITKCALL((AOM_ask_value_string(tItemRev, "item_id", &parentId))); // ��ȡ��ID
				printf("parentId == %s\n", parentId);

				ITKCALL((AOM_ask_value_string(tItemRev, "item_revision_id", &parentVersion))); // ��ȡ���汾��
				printf("parentVersion == %s\n", parentVersion);

				*findTableSql = "BEGIN bomline_procedure(:1, :2, :3); end;";
				pStmt = conn->createStatement(*findTableSql);
				pStmt->setString(1, parentId);
				pStmt->setString(2, parentVersion);
				pStmt->registerOutParam(3, OCCICURSOR);
				int updateCount = pStmt->execute();				
				pRs = pStmt->getCursor(3);
				ITKCALL(checkBOMAttr(parentId, parentVersion, pRs, &error_msg)); // �ж�BOMLine�����Ƿ���д
				//while (pRs->next())
				//{
				//	char tempStr[256];
				//	childId = pRs->getString(1);						
				//	printf("childId == %s\n", childId.c_str());
				//	childVersion = pRs->getString(2);
				//	printf("childVersion == %s\n", childVersion.c_str());	
				//	memset(tempStr, '\0', 256);
				//	sprintf(tempStr, G2B5("������汾IDΪ:%s, ������汾��Ϊ:%s, ��ǰ�Ӷ���汾IDΪ:%s, �Ӷ���汾��Ϊ:%s, ��������δ��д\n"), parentId, parentVersion, childId.c_str(), childVersion.c_str());
				//	tc_strcat(error_msg, tempStr);
				//	//printf(error_msg);
				//}
			}
		}
		
		pStmt->closeResultSet(pRs);
		conn->terminateStatement(pStmt);
		env->terminateConnection(conn);
		Environment::terminateEnvironment(env);
		//printf("error_msg == %s\n", error_msg);

		if (tc_strcmp(error_msg, "") != 0)
		{
			printf("error tip return");			
			decision = EPM_nogo;
			error_msg[strlen(error_msg) - 1] = 0;
			EMH_store_error_s1(EMH_severity_user_error, VER_RULE_ERROR, error_msg);
			ifail = VER_RULE_ERROR;
			goto CLEANUP;
		}
	CLEANUP:
		(*dbInfoVec).clear();
		dbInfoVec = NULL;
		(*dbInfoMap).clear();
		dbInfoMap = NULL;
		dbInfoIter = NULL;
		DOFREE(attach_object_list);
		DOFREE(parentId);
		DOFREE(parentVersion);

		printf("\n=================     D9_CheckBOMProperties end      ======================\n");
		TC_write_syslog("\n=================   D9_CheckBOMProperties   end   ==================\n");
		return decision;	
	}

#ifdef __cplusplus
}
#endif


/*
 �ж�BOMLine�����Ƿ���д
*/
int checkBOMAttr(char *parentId, char *parentVersion, ResultSet *pRs, char **error_msg)
{
	int  rcode = ITK_ok;
	string
		childId,
		childVersion,
		matType,
		matGroup,
		manuID,
		manuPN,
		UN,
		procMethods;

	while (pRs->next())
	{
		char tempStr[1024];
		childId = pRs->getString(1);
		printf("childId == %s\n", childId.c_str());

		childVersion = pRs->getString(2);
		printf("childVersion == %s\n", childVersion.c_str());

		matType = pRs->getString(3);
		printf("matType == %s\n", matType.c_str());

		matGroup = pRs->getString(4);
		printf("matGroup == %s\n", matGroup.c_str());

		manuID = pRs->getString(5);
		printf("manuID == %s\n", manuID.c_str());

		manuPN = pRs->getString(6);
		printf("manuPN == %s\n", manuPN.c_str());

		UN = pRs->getString(7);
		printf("UN == %s\n", UN.c_str());

		procMethods = pRs->getString(8);
		printf("procMethods == %s\n", procMethods.c_str());

		memset(tempStr, '\0', 1024);
		sprintf(tempStr, G2B5("������汾IDΪ:%s, ������汾��Ϊ:%s, ��ǰ�Ӷ���汾IDΪ:%s, �Ӷ���汾��Ϊ:%s, "), parentId, parentVersion, childId, childVersion);
		tc_strcat(*error_msg, tempStr);
		if (matType.empty()) // �ж��Ƿ�Ϊ��
		{
			sprintf(tempStr, G2B5("������������, ֵΪ��,����д;"));
			tc_strcat(*error_msg, tempStr);
			tc_strcat(*error_msg, "\n");
			continue;
		}

		if (matGroup.empty()) // �ж��Ƿ�Ϊ��
		{
			sprintf(tempStr, G2B5("����Ⱥ������, ֵΪ��,����д;"));
			tc_strcat(*error_msg, tempStr);
			tc_strcat(*error_msg, "\n");
			continue;
		}

		if (manuID.empty())
		{
			sprintf(tempStr, G2B5("��Ӧ������, ֵΪ��,����д;"));
			tc_strcat(*error_msg, tempStr);
		}

		if (manuPN.empty())
		{
			sprintf(tempStr, G2B5("��Ӧ���Ϻ�����, ֵΪ��,����д;"));
			tc_strcat(*error_msg, tempStr);
		}
		
		if (UN.empty())
		{
			sprintf(tempStr, G2B5("������λ����, ֵΪ��,����д;"));
			tc_strcat(*error_msg, tempStr);
		}

		if (procMethods.empty())
		{
			sprintf(tempStr, G2B5("�ɹ���ʽ����, ֵΪ��,����д;"));
			tc_strcat(*error_msg, tempStr);
		}
		tc_strcat(*error_msg, "\n");
		
	}

CLEANUP:
	//error_msg[strlen(*error_msg) - 1] = 0;
	//tc_strcat(*error_msg, "\n");
	return rcode;

}