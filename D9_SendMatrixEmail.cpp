#include "extension.h"
#include "util.h"
#include "errorMsg.h"

int checkDataset(tag_t itemRev, char* datasetType, vector<string>& list);

extern "C" int POM_AM__set_application_bypass(logical bypass);
#ifdef __cplusplus
extern "C" {
#endif

	int D9_SendMatrixEmail(EPM_action_message_t msg)
	{
		int ifail = ITK_ok,
			attach_object_num = 0,
			node_type = 0;

		tag_t 
			current_task = NULLTAG,
			root_task = NULLTAG,
			target = NULLTAG,
			session_tag = NULLTAG;

		tag_t
			* attach_object_list = NULLTAG;

		string attachmentsUid = "";
		string result = "";		
		string URL = ""; 
		string params = "taskName=";

		char
			* current_name = NULL,
			* task_name = NULL,
			* fristTargetUid = NULL,
			* currentTargetUid = NULL,
			* object_string = NULL,
			* change_desc = NULL,
			* node_name = NULL;

		char* subject = (char*)MEM_alloc(sizeof(char) * 256);
		memset(subject, 0, 256);
		strcpy(subject, "");

		char* comment = (char*)MEM_alloc(sizeof(char) * 256);
		memset(comment, 0, 256);
		strcpy(comment, "");

		char *datasetType = (char*)MEM_alloc(sizeof(char) * 256);
		memset(datasetType, 0, 256);
		strcpy(comment, "");

		TC_write_syslog("\n=================   D9_SendMatrixEmail   start   ==================\n");

		POM_AM__set_application_bypass(true);

		vector<string> datasetUidVec;
		datasetUidVec.clear();

		map<string, string>* argMap = new map<string, string>();
		vector<string>* urlVec = new vector<string>();
		map<string, string>::reverse_iterator* argsIter = new map<string, string>::reverse_iterator();
		getHandlerArgs(msg.arguments, *argMap);

		for ((*argsIter) = (*argMap).rbegin(); (*argsIter) != (*argMap).rend(); (*argsIter)++) {
			string tmpKey = (*argsIter)->first;
			string tmpValue = (*argsIter)->second;
			if (tc_strcmp(tmpKey.c_str(), "subject") == 0) {
				tc_strcpy(subject, tmpValue.c_str());
			} else if (tc_strcmp(tmpKey.c_str(), "comment") == 0) {
				tc_strcpy(comment, tmpValue.c_str());
			} else if (tc_strcmp(tmpKey.c_str(), "datasetType") == 0) {
				tc_strcpy(datasetType, tmpValue.c_str());
			}
		}

		TC_write_syslog("subject == %s\n", U2G(subject));
		TC_write_syslog("comment == %s\n", comment);
		TC_write_syslog("datasetType == %s\n", datasetType);


		if (tc_strcmp(subject, "") == 0) {
			TC_write_syslog("D9_SendMatrixEmail Action handler -subject param is null\n");
			goto CLEANUP;
		}

		if (tc_strcmp(comment, "") == 0) {
			TC_write_syslog("D9_SendMatrixEmail Action handler -comment param is null\n");
			goto CLEANUP;
		}
		if (tc_strcmp(datasetType, "") == 0) {
			TC_write_syslog("D9_SendMatrixEmail Action handler -datasetType param is null\n");
			goto CLEANUP;
		}

		current_task = msg.task;
//		ITKCALL(AOM_ask_value_string(current_task, "object_name", &current_name));
		ITKCALL(AOM_ask_value_string(current_task, "parent_name", &current_name));
		TC_write_syslog("current_name == %s\n", U2G(current_name));

		ITKCALL(EPM_ask_root_task(msg.task, &root_task)); // 获取流程根任务
		ITKCALL(AOM_ask_value_string(root_task, "job_name", &task_name));
		TC_write_syslog("job_name == %s\n", task_name);

//		POM_ask_session(&session_tag);

//		POM_ask_server_info(&node_type, &node_name);
//		TC_write_syslog("node_type == %d\n", node_type);
//		TC_write_syslog("node_name == %s\n", node_name);

		
		ITKCALL(EPM_ask_attachments(root_task, EPM_target_attachment, &attach_object_num, &attach_object_list));
		for (size_t i = 0; i < attach_object_num; i++) {
			logical
				isRev = false,
				is_DT_DCN_Rev = false;

			target = attach_object_list[i];
			ITKCALL(GTCTYPE_is_type_of(target, "ItemRevision", &isRev));

			if (!isRev) {
				continue;
			}

			if (tc_strcmp(comment, "Yes") == 0) { // 获取变更描述字段
				ITKCALL(GTCTYPE_is_type_of(target, "D9_DT_DCNRevision", &is_DT_DCN_Rev));
				if (is_DT_DCN_Rev) {
					ITKCALL(AOM_ask_value_string(target, "d9_ChangeDescription", &change_desc));
					TC_write_syslog("change_desc == %s\n", U2G(change_desc));
				}
			}
			ITKCALL(AOM_ask_value_string(target, "object_string", &object_string));
			TC_write_syslog("--处理目标对象：%s\n", object_string);
			if (fristTargetUid == NULL && checkSignRecord(target)) {
				ITK__convert_tag_to_uid(target, &fristTargetUid);
				TC_write_syslog("----第一个目标的UID：%s\n", fristTargetUid);
			}

			checkDataset(target, datasetType, datasetUidVec);			
		}

		
		params += UrlEncode(task_name);

		params += "&subject=";
		params += UrlEncode(subject);

		params += "&currentName=";
		params += UrlEncode(current_name);

		if (change_desc != NULL) {
			params += "&changeDesc=";
			params += UrlEncode(change_desc);
		}


		if (fristTargetUid != NULL) {
			params += "&firstTargetUid=";
			params += UrlEncode(fristTargetUid);
		}		

		for (size_t k = 0; k < datasetUidVec.size(); k++) {
			attachmentsUid += (string)datasetUidVec[k] + ";";
		}
		attachmentsUid = attachmentsUid.substr(0, attachmentsUid.length() - 1);
		TC_write_syslog("----附件UIDS：%s\n", attachmentsUid);
		params += "&attachments=";
		params += UrlEncode(attachmentsUid);		
		
		getPreferenceByName("D9_SpringCloud_URL", *urlVec);
		if ((*urlVec).size() > 0) {
			URL = (*urlVec)[0] + "/tc-service/teamcenter/sendMatrixEmail?" + params;
//			URL =  "http://localhost:8888/teamcenter/sendMatrixEmail?" + params;
			TC_write_syslog("----URL：%s \n", URL.data());
		}
		
		CURLcode res = httpGet(URL, result, 10);
		TC_write_syslog("http get status :: %d\n", res);


	CLEANUP:
		POM_AM__set_application_bypass(false);
		(*urlVec).clear();
		urlVec = NULL;
		(*argMap).clear();
		argMap = NULL;
		datasetUidVec.clear();
		argsIter = NULL;
		DOFREE(current_name);
		DOFREE(task_name);
		DOFREE(attach_object_list);
		DOFREE(change_desc);
		DOFREE(object_string);
		DOFREE(fristTargetUid);
		DOFREE(change_desc);
		return ifail;
	}
#ifdef __cplusplus
}
#endif

int checkDataset(tag_t itemRev, char *datasetType, vector<string>& list) {
	int rcode = ITK_ok,
		tagCount = 0;

	tag_t* tag_list = NULLTAG;
	char
		* uid = NULL,
		* targetType = NULL;

	ITKCALL(AOM_ask_value_tags(itemRev, "IMAN_specification", &tagCount, &tag_list));
	for (size_t i = 0; i < tagCount; i++) {
		ITKCALL(WSOM_ask_object_type2(tag_list[i], &targetType));
		TC_write_syslog("\n IMAN_specification属性下的targetType == %s\n", targetType);
		if (containStr(datasetType, targetType)) {
			ITK__convert_tag_to_uid(tag_list[i], &uid);
			list.push_back(uid);
		}
	}

CLEANUP:
	DOFREE(tag_list);
	DOFREE(targetType);
	DOFREE(uid);
	return rcode;
}