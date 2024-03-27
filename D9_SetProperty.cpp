#include "json.hxx"
using nlohmann::json;
#include "extension.h"
#include "util.h"
#include "errorMsg.h"

extern "C" int POM_AM__set_application_bypass(logical bypass);
#ifdef __cplusplus
extern "C" {
#endif
	bool endsWith(const std::string& str, const std::string& suffix) {
		if (suffix.length() > str.length()) {
			return false;
		}
		return str.compare(str.length() - suffix.length(), suffix.length(), suffix) == 0;
	}
	void setActualUserEmailById(tag_t target, const char* userId, const char* property);
	int D9_Set_Property(EPM_action_message_t msg)
	{
		int ifail = ITK_ok,
			attach_object_num = 0,
			node_type = 0;

		tag_t
			current_task = NULLTAG,
			root_task = NULLTAG,
			target = NULLTAG,
			tag = NULLTAG,
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
			* actualUser = NULL,
			* itemId = NULL,
			* node_name = NULL,
			* userName = NULL;
		string str1,str2,str3;
		vector<string> spt1;
		vector<string> spt2;

		char* property = (char*)MEM_alloc(sizeof(char) * 256);
		memset(property, 0, 256);
		strcpy(property, "");

		char* value = (char*)MEM_alloc(sizeof(char) * 256);
		memset(value, 0, 256);
		strcpy(value, "");

		char* type = (char*)MEM_alloc(sizeof(char) * 256);
		memset(type, 0, 256);
		strcpy(type, "");

		TC_write_syslog("\n=================   D9_Set Propery v1.0   Start   ==================\n");

		POM_AM__set_application_bypass(true);

		map<string, string>* argMap = new map<string, string>();
		vector<string>* urlVec = new vector<string>();
		map<string, string>::reverse_iterator* argsIter = new map<string, string>::reverse_iterator();
		getHandlerArgs(msg.arguments, *argMap);

		for ((*argsIter) = (*argMap).rbegin(); (*argsIter) != (*argMap).rend(); (*argsIter)++) {
			string tmpKey = (*argsIter)->first;
			string tmpValue = (*argsIter)->second;
			if (tc_strcmp(tmpKey.c_str(), "property") == 0) {
				tc_strcpy(property, tmpValue.c_str());
			}
			else if (tc_strcmp(tmpKey.c_str(), "value") == 0) {
				tc_strcpy(value, tmpValue.c_str());
			}
			else if (tc_strcmp(tmpKey.c_str(), "type") == 0) {
				tc_strcpy(type, tmpValue.c_str());
			}
		}

		TC_write_syslog("property == %s\n", U2G(property));
		TC_write_syslog("value == %s\n", U2G(value));
		TC_write_syslog("type == %s\n", U2G(type));

		if (tc_strcmp(property, "") == 0) {
			TC_write_syslog("D9_SendIssueEmail Action handler -recipient param is null\n");
			goto CLEANUP;
		}

		if (tc_strcmp(value, "") == 0) {
			TC_write_syslog("D9_SendIssueEmail Action handler -type param is null\n");
			goto CLEANUP;
		}

		if (tc_strcmp(type, "") == 0) {
			TC_write_syslog("D9_SendIssueEmail Action handler -type param is null\n");
			goto CLEANUP;
		}

		current_task = msg.task;
		//ITKCALL(AOM_ask_value_string(current_task, "object_name", &current_name));
		ITKCALL(AOM_ask_value_string(current_task, "parent_name", &current_name));
		TC_write_syslog("current_name == %s\n", U2G(current_name));

		ITKCALL(EPM_ask_root_task(msg.task, &root_task)); // 获取流程根任务

		ITKCALL(AOM_ask_value_string(root_task, "job_name", &task_name));
		TC_write_syslog("job_name == %s\n", task_name);

		POM_ask_session(&session_tag);

		POM_ask_server_info(&node_type, &node_name);
		TC_write_syslog("node_type == %d\n", node_type);
		TC_write_syslog("node_name == %s\n", node_name);

		ITKCALL(EPM_ask_attachments(root_task, EPM_target_attachment, &attach_object_num, &attach_object_list));
		for (size_t i = 0; i < attach_object_num; i++) {
			target = attach_object_list[i];
			logical allow = false;
			str1 = type;
			spt1 = split(str1, ",");
			for (const string& type : spt1) {
				ITKCALL(GTCTYPE_is_type_of(target, type.c_str(), &allow));
				if (allow) {
					break;
				}
			}
			if (!allow) {
				continue;
			}
			ITKCALL(AOM_ask_value_string(target, "object_string", &object_string));
			TC_write_syslog("--Processing Target objects: %s\n", object_string);

			ITK__convert_tag_to_uid(target, &fristTargetUid);
			if (fristTargetUid == NULL) {
				continue;
			}
			TC_write_syslog("----The UUID of the first target: %s\n", fristTargetUid);
			str1 = value;
			str2 = property;
			spt1 = split(str1, ",");
			spt2 = split(str2, ",");
			ITKCALL(AOM_lock(target));
			for(int i=0;i<spt1.size();i++){
				string s1 = spt1[i];
				string s2 = spt2[i];
				if (endsWith(s1, "Mail")) {
					int objectType = 0;
					if (s1.compare("$RequestorMail") == 0) {
						TC_write_syslog("get Requestor\n");
						ITKCALL(AOM_ask_value_tag(target, "Requestor", &tag));
						objectType = 1;
					}
					else if (s1.compare("$ActualUserMail") == 0) {
						TC_write_syslog("get d9_ActualUserID \n");
						ITKCALL(AOM_ask_value_string(target, "d9_ActualUserID", &actualUser));
						objectType = 2;
					}
					else if (s1.compare("$IssueTLOwnerMail") == 0) {
						TC_write_syslog("Get d9_IssueTL\n");
						ITKCALL(AOM_ask_value_tag(target, "d9_IssueTL", &tag));
						objectType = 1;
					}
					else if (s1.compare("$IssueTLMail") == 0) {
						TC_write_syslog("Get d9_IssueTL_ActualUser\n");
						ITKCALL(AOM_ask_value_tag(target, "d9_IssueTL_ActualUser", &tag));
						objectType = 3;
					}
					else if (s1.compare("$IssueFixOwnerMail") == 0) {
						TC_write_syslog("Get d9_IssueFixOwner\n");
						ITKCALL(AOM_ask_value_tag(target, "d9_IssueFixOwner", &tag));
						objectType = 1;
					}
					else if (s1.compare("$IssueFixMail") == 0) {
						TC_write_syslog("Get d9_IssueFixOwner_ActualUser\n");
						ITKCALL(AOM_ask_value_tag(target, "d9_IssueFixOwner_ActualUser", &tag));
						objectType = 3;
					}
					else if (s1.compare("$IssueSEPMOwnerMail") == 0) {
						TC_write_syslog("get d9_IssueSEPM\n");
						ITKCALL(AOM_ask_value_tag(target, "d9_IssueSEPM", &tag));
						objectType = 1;
					}
					else if (s1.compare("$IssueSEPMMail") == 0) {
						TC_write_syslog("get d9_IssueSEPM_ActualUser\n");
						ITKCALL(AOM_ask_value_tag(target, "d9_IssueSEPM_ActualUser", &tag));
						objectType = 3;
					}
					else if (s1.compare("$IssueHWPMOwnerMail") == 0) {
						TC_write_syslog("get d9_IssueHWPM\n");
						ITKCALL(AOM_ask_value_tag(target, "d9_IssueHWPM", &tag));
						objectType = 1;
					}
					else if (s1.compare("$IssueHWPMMail") == 0) {
						TC_write_syslog("get d9_IssueHWPM_ActualUser\n");
						ITKCALL(AOM_ask_value_tag(target, "d9_IssueHWPM_ActualUser", &tag));
						objectType = 3;
					}
					else if (s1.compare("$IssueTesterOwnerMail") == 0) {
						TC_write_syslog("get d9_IssueTester\n");
						ITKCALL(AOM_ask_value_tag(target, "d9_IssueTester", &tag));
						objectType = 1;
					}
					else if (s1.compare("$IssueTesterMail") == 0) {
						TC_write_syslog("get d9_IssueTester_ActualUser\n");
						ITKCALL(AOM_ask_value_tag(target, "d9_IssueTester_ActualUser", &tag));
						objectType = 3;
					}
					else if (s1.compare("$IssuePEOwnerMail") == 0) {
						TC_write_syslog("get d9_IssuePE\n");
						ITKCALL(AOM_ask_value_tag(target, "d9_IssuePE", &tag));
						objectType = 1;
					}
					else if (s1.compare("$IssuePEMail") == 0) {
						TC_write_syslog("get d9_IssuePE_ActualUser\n");
						ITKCALL(AOM_ask_value_tag(target, "d9_IssuePE_ActualUser", &tag));
						objectType = 3;
					}
					if (objectType == 1) {
						if (tag != NULL) {
							ITKCALL(AOM_ask_value_tag(tag, "user", &tag));
							if (tag != NULL) {
								ITKCALL(AOM_ask_value_string(tag, "user_id", &itemId));
								TC_write_syslog("get user_id: %s\n", itemId);
								if (tc_strcmp(itemId, "") != 0) {
									setActualUserEmailById(target,itemId, s2.c_str());
								}
							}
						}
					}
					else if (objectType == 2) {						
						if (tc_strcmp(actualUser, "") != 0) {
							str3 = actualUser;
							size_t startPos = str3.find('(');
							size_t endPos = str3.find(')');
							if (startPos != string::npos && endPos != string::npos && startPos < endPos) {
								str3 = str3.substr(startPos + 1, endPos - startPos - 1);
								TC_write_syslog("str3: %s\n", str3);
								setActualUserEmailById(target,str3.c_str(),s2.c_str());
							}
						}
					}
					else if (objectType == 3) {
						if (tag != NULL) {
							ITKCALL(AOM_ask_value_string(tag, "d9_Email", &itemId));
							ITKCALL(AOM_set_value_string(target, s2.c_str(), itemId));
						}
					}
					continue;
				}
				if (s1.compare("$TaskName") == 0) {
					TC_write_syslog("get taskName\n");		
					ITKCALL(AOM_set_value_string(target, s2.c_str(), current_name));
					continue;
				}
				else if (s1.compare("$Requestor") == 0) {
					TC_write_syslog("get Requestor\n");
					ITKCALL(AOM_ask_value_tag(target, "Requestor", &tag));
					if (tag != NULL) {
						ITKCALL(AOM_ask_value_tag(tag, "user", &tag));
						if (tag != NULL) {
							ITKCALL(AOM_ask_value_string(tag, "user_id", &itemId));
							ITKCALL(AOM_ask_value_string(tag, "user_name", &userName));
							string strUserId = itemId;
							string strUserName = userName;
							string str = strUserName + "(" + strUserId + ")";
							TC_write_syslog("get user_id: %s\n", str.c_str());
							if (tc_strcmp(itemId, "") != 0) {
								ITKCALL(AOM_set_value_string(target, s2.c_str(), str.c_str()));
							}
						}
					}
					continue;
				}
				else if (s1.compare("$ActualUser") == 0) {
					TC_write_syslog("get d9_ActualUserID \n");
					ITKCALL(AOM_ask_value_string(target, "d9_ActualUserID", &actualUser));
					if (tc_strcmp(actualUser, "") != 0) {
						TC_write_syslog("str3: %s\n", actualUser);
						ITKCALL(AOM_set_value_string(target, s2.c_str(), actualUser));
					}
					continue;
				}
				else if (s1.compare("$IssueTL") == 0) {
					TC_write_syslog("Get d9_IssueTL_ActualUser\n");
					ITKCALL(AOM_ask_value_tag(target, "d9_IssueTL_ActualUser", &tag));
				}
				else if (s1.compare("$IssueTLOwner") == 0) {
					TC_write_syslog("Get d9_IssueTL\n");
					ITKCALL(AOM_ask_value_tag(target, "d9_IssueTL", &tag));
				}
				else if (s1.compare("$IssueFix") == 0) {
					TC_write_syslog("Get d9_IssueFixOwner_ActualUser\n");
					ITKCALL(AOM_ask_value_tag(target, "d9_IssueFixOwner_ActualUser", &tag));
				}
				else if (s1.compare("$IssueFixOwner") == 0) {
					TC_write_syslog("Get d9_IssueFixOwner\n");
					ITKCALL(AOM_ask_value_tag(target, "d9_IssueFixOwner", &tag));
				}
				else if (s1.compare("$IssueTester") == 0) {
					TC_write_syslog("get d9_IssueTester_ActualUser\n");
					ITKCALL(AOM_ask_value_tag(target, "d9_IssueTester_ActualUser", &tag));
				}
				else if (s1.compare("$IssueTesterOwner") == 0) {
					TC_write_syslog("get d9_IssueTester\n");
					ITKCALL(AOM_ask_value_tag(target, "d9_IssueTester", &tag));
				}
				else if (s1.compare("$IssueSEPM") == 0) {
					TC_write_syslog("get d9_IssueSEPM_ActualUser\n");
					ITKCALL(AOM_ask_value_tag(target, "d9_IssueSEPM_ActualUser", &tag));
				}
				else if (s1.compare("$IssueSEPMOwner") == 0) {
					TC_write_syslog("get d9_IssueSEPM\n");
					ITKCALL(AOM_ask_value_tag(target, "d9_IssueSEPM", &tag));
				}
				else if (s1.compare("$IssueHWPM") == 0) {
					TC_write_syslog("get d9_IssueHWPM_ActualUser\n");
					ITKCALL(AOM_ask_value_tag(target, "d9_IssueHWPM_ActualUser", &tag));
				}
				else if (s1.compare("$IssueHWPMOwner") == 0) {
					TC_write_syslog("get d9_IssueHWPM\n");
					ITKCALL(AOM_ask_value_tag(target, "d9_IssueHWPM", &tag));
				}
				else if (s1.compare("$IssuePE") == 0) {
					TC_write_syslog("get d9_IssuePE_ActualUser\n");
					ITKCALL(AOM_ask_value_tag(target, "d9_IssuePE_ActualUser", &tag));
				}
				else if (s1.compare("$IssuePEOwner") == 0) {
					TC_write_syslog("get d9_IssuePE\n");
					ITKCALL(AOM_ask_value_tag(target, "d9_IssuePE", &tag));
				}
				if (tag == NULL) {
					continue;
				}
				if (endsWith(s1, "Owner")) {
					ITKCALL(AOM_ask_value_tag(tag, "user", &tag));
					if (tag != NULL) {
						ITKCALL(AOM_ask_value_string(tag, "user_id", &itemId));
						ITKCALL(AOM_ask_value_string(tag, "user_name", &userName));
						string strUserId = itemId;
						string strUserName = userName;
						string str = strUserName + "(" + strUserId + ")";
						TC_write_syslog("get user_id: %s\n", str.c_str());
						if (tc_strcmp(itemId, "") != 0) {
							ITKCALL(AOM_set_value_string(target, s2.c_str(), str.c_str()));
						}
					}
				}
				else {
					ITKCALL(AOM_ask_value_string(tag, "d9_UserInfo", &itemId));
					TC_write_syslog("get itemId: %s\n", itemId);
					if (tc_strcmp(itemId, "") != 0) {
						ITKCALL(AOM_set_value_string(target, s2.c_str(), itemId));
					}
				}
			}
			ITKCALL(AOM_save_with_extensions(target));
			ITKCALL(AOM_unlock(target));
		}

	CLEANUP:
		POM_AM__set_application_bypass(false);
		(*urlVec).clear();
		urlVec = NULL;
		(*argMap).clear();
		argMap = NULL;
		argsIter = NULL;
		DOFREE(current_name);
		DOFREE(property);
		DOFREE(value);
		DOFREE(task_name);
		DOFREE(attach_object_list);
		DOFREE(change_desc);
		DOFREE(object_string);
		DOFREE(fristTargetUid);
		DOFREE(actualUser);
		DOFREE(change_desc);
		DOFREE(currentTargetUid);
		DOFREE(node_name);
		DOFREE(type);
		DOFREE(userName);
		return ifail;
	}

	void setActualUserEmailById(tag_t target,const char* userId, const char* property) {
		char** queryTypeName = (char**)MEM_alloc(sizeof(char*) * 1);
		char** queryTypeValue = (char**)MEM_alloc(sizeof(char*) * 1);
		queryTypeName[0] = (char*)MEM_alloc(sizeof(char*) * 64);
		queryTypeValue[0] = (char*)MEM_alloc(sizeof(char*) * 64);
		tag_t* resultTags = NULL;
		int resultCount = 0;
		tc_strcpy(queryTypeName[0], "ID");
		tc_strcpy(queryTypeValue[0], userId);
		query("__D9_Find_Actual_User", queryTypeName, queryTypeValue, 1, &resultTags, &resultCount);
		char* tempStr = NULL;
		if (resultCount != 0) {
			ITKCALL(AOM_ask_value_string(resultTags[0], "d9_Email", &tempStr));
			ITKCALL(AOM_set_value_string(target, property, tempStr))
		}
		DOFREE(queryTypeName);
		DOFREE(queryTypeValue);
		DOFREE(resultTags);
		DOFREE(tempStr);
	}


#ifdef __cplusplus
}
#endif
