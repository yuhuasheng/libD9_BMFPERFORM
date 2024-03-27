#include "json.hxx"
using nlohmann::json;
#include "extension.h"
#include "util.h"
#include "errorMsg.h"

extern "C" int POM_AM__set_application_bypass(logical bypass);
#ifdef __cplusplus
extern "C" {
#endif
	wstring stringToUnicode(const string& str) {
		wstring wstr(str.begin(), str.end());     
		return wstr; 
	}
	const char* hexToChar(const string& hexString) {
		string charString;
		charString.reserve(hexString.length() / 2);
		for (size_t i = 0; i < hexString.length(); i += 2) {
			string byteString = hexString.substr(i, 2);
			char byte = static_cast<char>(stoi(byteString, nullptr, 16));
			charString.push_back(byte);
		}
		return charString.c_str();
	}
	char* concatenate(char** strArray, int size) {
		// 计算总长度
		int totalLength = 0;
		for (int i = 0; i < size; i++) {
			totalLength += strlen(strArray[i]);
		}
		// 分配足够的内存来存储拼接后的字符串
		char* result = (char*)malloc((totalLength + 1) * sizeof(char));
		if (result == NULL) {
			fprintf(stderr, "内存分配失败");
			exit(1);
		}
		// 拼接字符串
		int currentIndex = 0;
		for (int i = 0; i < size; i++) {
			strcpy_s(result + currentIndex, strlen(strArray[i]) + 1, strArray[i]);
			currentIndex += strlen(strArray[i]);
		}
		return result;
	}

	int D9_SendIssueEmail(EPM_action_message_t msg)
	{
		int ifail = ITK_ok,
			attach_object_num = 0,
			node_type = 0;

		tag_t
			current_task = NULLTAG,
			root_task = NULLTAG,
			target = NULLTAG,
			tag = NULLTAG,
			actualUserTag = NULLTAG,
			session_tag = NULLTAG;

		tag_t
			* attach_object_list = NULLTAG;

		string attachmentsUid = "";
		string temp = "";
		string URL = "";
		string params = "taskName=";

		char
			* current_name = NULL,
			* task_name = NULL,
			* fristTargetUid = NULL,
			* currentTargetUid = NULL,
			* object_string = NULL,
			* change_desc = NULL,
			* node_name = NULL,
			* dueDate = NULL,
			* html = NULL,
			* note = NULL,
			* actualUser = NULL,
			* actualUserEmail = NULL,
			* emial = NULL;
		string str;
		string resp;
		json jsonBody;
		map<string, string> httpmap;
		vector<string> spt1;
		vector<string> revicerList;
		
		note = (char*)MEM_alloc(sizeof(char) * 500);
		dueDate = (char*)MEM_alloc(sizeof(char) * 50);
		char* subject = (char*)MEM_alloc(sizeof(char) * 256);
		memset(subject, 0, 256);
		strcpy(subject, "");

		char* comment = (char*)MEM_alloc(sizeof(char) * 256);
		memset(comment, 0, 256);
		strcpy(comment, "");

		char* recipient = (char*)MEM_alloc(sizeof(char) * 256);
		memset(recipient, 0, 256);
		strcpy(recipient, "");

		char* type = (char*)MEM_alloc(sizeof(char) * 256);
		memset(type, 0, 256);
		strcpy(type, "");

		TC_write_syslog("\n=================   D9_SendIssueEmail v1.2   Start   ==================\n");

		POM_AM__set_application_bypass(true);

		map<string, string>* argMap = new map<string, string>();
		vector<string>* urlVec = new vector<string>();
		map<string, string>::reverse_iterator* argsIter = new map<string, string>::reverse_iterator();
		getHandlerArgs(msg.arguments, *argMap);

		for ((*argsIter) = (*argMap).rbegin(); (*argsIter) != (*argMap).rend(); (*argsIter)++) {
			string tmpKey = (*argsIter)->first;
			string tmpValue = (*argsIter)->second;
			if (tc_strcmp(tmpKey.c_str(), "subject") == 0) {
				tc_strcpy(subject, tmpValue.c_str());
			}
			else if (tc_strcmp(tmpKey.c_str(), "comment") == 0) {
				tc_strcpy(comment, tmpValue.c_str());
			}
			else if (tc_strcmp(tmpKey.c_str(), "recipient") == 0) {
				tc_strcpy(recipient, tmpValue.c_str());
			}
			else if (tc_strcmp(tmpKey.c_str(), "type") == 0) {
				tc_strcpy(type, tmpValue.c_str());
			}
		}

		TC_write_syslog("subject == %s\n", U2G(subject));
		TC_write_syslog("comment == %s\n", U2G(comment));
		TC_write_syslog("recipient == %s\n", U2G(recipient));
		TC_write_syslog("type == %s\n", U2G(type));


		if (tc_strcmp(subject, "") == 0) {
			TC_write_syslog("D9_SendIssueEmail Action handler -subject param is null\n");
			goto CLEANUP;
		}

		if (tc_strcmp(comment, "") == 0) {
			TC_write_syslog("D9_SendIssueEmail Action handler -comment param is null\n");
			goto CLEANUP;
		}

		if (tc_strcmp(recipient, "") == 0) {
			TC_write_syslog("D9_SendIssueEmail Action handler -recipient param is null\n");
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
			str = type;
			spt1 = split(str, ",");
			for (const string& type : spt1) {
				ITKCALL(GTCTYPE_is_type_of(target, type.c_str(), &allow));
				if (allow) {
					break						;
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
			ITKCALL(AOM_ask_value_string(target, "d9_ActualUserID", &actualUser));
			if (tc_strcmp(actualUser, "") == 0) {
				TC_write_syslog("----d9_ActualUserID is null skip .\n");
				continue;
			}
			str = actualUser;
			size_t startPos = str.find('(');
			size_t endPos = str.find(')');
			if (startPos != string::npos && endPos != string::npos && startPos < endPos) {
				str = str.substr(startPos + 1, endPos - startPos - 1);
				TC_write_syslog("str: %s\n", str);
				char** queryTypeName = (char**)MEM_alloc(sizeof(char*) * 1);
				char** queryTypeValue = (char**)MEM_alloc(sizeof(char*) * 1);
				queryTypeName[0] = (char*)MEM_alloc(sizeof(char*) * 64);
				queryTypeValue[0] = (char*)MEM_alloc(sizeof(char*) * 64);
				tag_t* resultTags = NULL;
				int resultCount = 0;
				tc_strcpy(queryTypeName[0], "ID");
				tc_strcpy(queryTypeValue[0], str.c_str());
				query("__D9_Find_Actual_User", queryTypeName, queryTypeValue, 1, &resultTags, &resultCount);
				if (resultCount != 0) {
					actualUserTag = resultTags[0];
					ITKCALL(AOM_ask_value_string(actualUserTag, "d9_Email", &actualUserEmail));
				}
				DOFREE(queryTypeName);
				DOFREE(queryTypeValue);
				DOFREE(resultTags);
			}
			date_t dueDate_t;
			ITKCALL(AOM_ask_value_date(target, "due_date", &dueDate_t));
			formatDate(dueDate, "%Y/%m/%d", dueDate_t);
			if (tc_strcmp(dueDate, "0/01/00") == 0) {
				dueDate = (char*)MEM_alloc(sizeof(char) * 10);
				dueDate = tc_strcpy(dueDate,hexToChar("28E784A129"));
			}
			TC_write_syslog("----endDate == %s\n", dueDate);
			int n = 0;
			//ITKCALL(AOM_ask_value_strings(root_task, "fnd0Instructions",&n, &noteList));
			ITKCALL(AOM_ask_value_string(current_task, "fnd0Instructions", &note));
			// note = concatenate(noteList, sizeof(noteList) / sizeof(noteList[0]));
			if (tc_strcmp(note, "") == 0) {
				note = (char*)MEM_alloc(sizeof(char) * 10);
				note = tc_strcpy(note, hexToChar("28E784A129"));
			}
			TC_write_syslog("----note == %s\n", U2G(note));
			str = recipient;
			spt1 = split(str, ",");
			for (const string& s : spt1) {
				if (s.compare("$REQUESTOR") == 0) {
					TC_write_syslog("get d9_ActualUserID\n");
					tag = actualUserTag;
				}else if (s.compare("$IssueTL") == 0) {
					TC_write_syslog("Get d9_IssueTL_ActualUser\n");
					ITKCALL(AOM_ask_value_tag(target, "d9_IssueTL_ActualUser", &tag));
				}else if (s.compare("$IssueFixOwner") == 0) {
					TC_write_syslog("Get d9_IssueFixOwner_ActualUser\n");
					ITKCALL(AOM_ask_value_tag(target, "d9_IssueFixOwner_ActualUser", &tag));
				}else if (s.compare("$IssueTester") == 0) {
					TC_write_syslog("get d9_IssueTester_ActualUser\n");
					ITKCALL(AOM_ask_value_tag(target, "d9_IssueTester_ActualUser", &tag));
				}else if (s.compare("$IssueSEPM") == 0) {
					TC_write_syslog("get d9_IssueSEPM_ActualUser\n");
					ITKCALL(AOM_ask_value_tag(target, "d9_IssueSEPM_ActualUser", &tag));
				}
				else if (s.compare("$IssueHWPM") == 0) {
					TC_write_syslog("get d9_IssueHWPM_ActualUser\n");
					ITKCALL(AOM_ask_value_tag(target, "d9_IssueHWPM_ActualUser", &tag));
				}
				else if (s.compare("$IssuePE") == 0) {
					TC_write_syslog("get d9_IssuePE_ActualUser\n");
					ITKCALL(AOM_ask_value_tag(target, "d9_IssuePE_ActualUser", &tag));
				}
				if (tag == NULL) {
					continue;
				}
				ITKCALL(AOM_ask_value_string(tag, "d9_Email", &emial));
				if (tc_strcmp(emial, "") == 0) {
					continue;
				}
				revicerList.push_back(emial);
				TC_write_syslog("get email  == %s\n", emial);
			}
		}

		if (revicerList.size() == 0) {
			TC_write_syslog("D9_SendIssueEmail not found reciver,Bay bay~\n");
			goto CLEANUP;
		}

		getPreferenceByName("D9_SpringCloud_URL", *urlVec);
		if ((*urlVec).size() == 0) {
			TC_write_syslog("D9_SendIssueEmail not found D9_SpringCloud_URL config,Bay bay~\n");
			goto CLEANUP;
		}
		URL = (*urlVec)[0] + "/tc-mail/teamcenter/sendMail3";
		//URL =  "http://localhost:8454/teamcenter/sendMail3";
		TC_write_syslog("URL: %s \n", URL.data());	
		html = (char*)MEM_alloc(sizeof(char) * 1024 * 4);
		/*
			<!doctype html>
			<html lang="en">
			<head>
				<meta charset="UTF-8">
				<meta name="viewport" content="width=device-width, user-scalable=no, initial-scale=1.0, maximum-scale=1.0, minimum-scale=1.0">
				<meta http-equiv="X-UA-Compatible" content="ie=edge">
				<title>Document</title>
			</head>
			<body>
			當前任務：%s <br>
			流程名稱：%s <br>
			截止日期：%s <br>
			注釋：%s <br>
			說明：%s <br>
			Issue發起者：%s%s <br>
			<br>
			<h3 style = "margin: 0">Teamcenter 系統自動定時發送，請勿回復郵件！</h3>
			</body>
			</html>
		*/
		sprintf_s(html, 1024 * 4, U2G(hexToChar("3C21646F63747970652068746D6C3E0A3C68746D6C206C616E673D22656E223E0A3C686561643E0A202020203C6D65746120636861727365743D225554462D38223E0A202020203C6D657461206E616D653D2276696577706F72742220636F6E74656E743D2277696474683D6465766963652D77696474682C20757365722D7363616C61626C653D6E6F2C20696E697469616C2D7363616C653D312E302C206D6178696D756D2D7363616C653D312E302C206D696E696D756D2D7363616C653D312E30223E0A202020203C6D65746120687474702D65717569763D22582D55412D436F6D70617469626C652220636F6E74656E743D2269653D65646765223E0A202020203C7469746C653E446F63756D656E743C2F7469746C653E0A3C2F686561643E0A3C626F64793E0AE795B6E5898DE4BBBBE58B99EFBC9A2573203C62723E0AE6B581E7A88BE5908DE7A8B1EFBC9A2573203C62723E0AE688AAE6ADA2E697A5E69C9FEFBC9A2573203C62723E0AE6B3A8E9878BEFBC9A2573203C62723E0AE8AAAAE6988EEFBC9A2573203C62723E0A4973737565E799BCE8B5B7E88085EFBC9A25732573203C62723E0A3C62723E0A3C6833207374796C65203D20226D617267696E3A2030223E5465616D63656E74657220E7B3BBE7B5B1E887AAE58B95E5AE9AE69982E799BCE98081EFBC8CE8AB8BE58BBFE59B9EE5BEA9E983B5E4BBB6EFBC813C2F68333E0A3C2F626F64793E0A3C2F68746D6C3E")), U2G(current_name), U2G(object_string), U2G(dueDate), U2G(comment), U2G(note),U2G(actualUser),U2G(actualUserEmail));
		TC_write_syslog("html: %s \n", html);
		str = "";
		for (const string& s : revicerList) {
			if (str.size() > 0) {
				str.append(",");
			}
			str.append(s);
		}
		// str = u8"jian-jun.fan@foxconn.com";
		
		httpmap.insert(make_pair("sendTo", str));
		temp = object_string;
		temp += " ";
		temp += subject;
		httpmap.insert(make_pair("subject", temp));
		httpmap.insert(make_pair("htmlmsg", G2U(html)));
		jsonBody = json(httpmap);
		str = "data=" + UrlEncode(jsonBody.dump());
		httpPostFormData(URL, str, resp, 10);
		TC_write_syslog("resp: %s \n", resp);

	CLEANUP:
		POM_AM__set_application_bypass(false); 
		(*urlVec).clear();
		urlVec = NULL;
		(*argMap).clear();
		argMap = NULL;
		argsIter = NULL;
		DOFREE(current_name);
		DOFREE(html);
		DOFREE(dueDate);
		DOFREE(subject);
		DOFREE(note);
		DOFREE(actualUser);
		DOFREE(comment);
		DOFREE(recipient);
		DOFREE(task_name);
		DOFREE(attach_object_list);
		DOFREE(change_desc);
		DOFREE(object_string);
		DOFREE(fristTargetUid);
		DOFREE(actualUserEmail);
		DOFREE(change_desc);
		DOFREE(emial);
		DOFREE(currentTargetUid);
		DOFREE(node_name);
		DOFREE(type);
		return ifail;
	}

	
#ifdef __cplusplus
}
#endif
