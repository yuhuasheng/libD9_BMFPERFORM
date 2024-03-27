#include "extension.h"
#include "util.h"
#include "errorMsg.h"
#include <tc/folder.h>

extern "C" int POM_AM__set_application_bypass(logical bypass);
#ifdef __cplusplus
extern "C" {
#endif


	int D9_PostDCN2Agile(EPM_rule_message_t msg)
	{
		EPM_decision_t decision = EPM_go;
		vector<string>* urlVec = new vector<string>();
		int  counts=0,i=0;
		tag_t root_task = NULLTAG;
		tag_t* target_tags = NULLTAG;
		char* object_type = NULL,*uid=NULL;
		tag_t revType = NULLTAG;
		tag_t typeTag = NULLTAG;
		logical isRev;
		vector<tag_t> ecnVec;
		string url = "", result = "";
		POM_AM__set_application_bypass(true);
		ITKCALL(EPM_ask_root_task(msg.task, &root_task));
		ITKCALL(EPM_ask_attachments(root_task, EPM_target_attachment, &counts, &target_tags));

		for (i = 0; i < counts; i++) {

			ITKCALL(TCTYPE_ask_type("D9_PRT_DCNRevision", &revType));
			ITKCALL(TCTYPE_ask_object_type(target_tags[i], &typeTag));
			ITKCALL(TCTYPE_is_type_of(typeTag, revType, &isRev));
			if (isRev) {			
				ITKCALL(WSOM_ask_object_type2(target_tags[i], &object_type));
				TC_write_syslog("Process object type >> > % s\n", object_type);				
				ecnVec.push_back(target_tags[i]);				
			}
		}


		if (ecnVec.size() != 1) {
			decision = EPM_nogo;
			EMH_store_error_s1(EMH_severity_user_error, VER_RULE_ERROR, "ERROR : Multi ECN in the process !\n");
			TC_write_syslog("ERROR : Please check ECN in the process !\n");
		}

		ITK__convert_tag_to_uid(ecnVec[0], &uid);

		getPreferenceByName("D9_SpringCloud_URL", *urlVec);
		if ((*urlVec).size() > 0) {
			url = (*urlVec)[0] + "/tc-integrate/prt/syncPrtBOM?uid="+ (string)uid;
			TC_write_syslog("url == %s\n", url.c_str());
		}
		CURLcode res = httpGet(url, result, 30);

		if (result.compare("success")!=0) {

			EMH_store_error_s1(EMH_severity_user_error, VER_RULE_ERROR, "ERROR : sYNC EBOM to Agile Faied !\n");
			decision = EPM_nogo;
		}

		

	CLEANUP:
		POM_AM__set_application_bypass(false); // πÿ±’≈‘¬∑
		DOFREE(object_type);
		DOFREE(target_tags);
	
		
		return decision;
	}


#ifdef __cplusplus
}
#endif