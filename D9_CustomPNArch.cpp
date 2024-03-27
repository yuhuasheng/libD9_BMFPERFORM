#include "extension.h"
#include "util.h"
#include <tc/folder.h>
extern "C" int POM_AM__set_application_bypass(logical bypass);

#ifdef __cplusplus
extern "C" {
#endif

	int D9_CustomPNArch(EPM_rule_message_t msg) {

		EPM_decision_t decision = EPM_go;
		int ifail = ITK_ok, counts = 0, tagCount = 0, cnt=0, iCount;
		int iAttachmentCount = 0;

		tag_t tRootTask = NULL_TAG;
		tag_t *tAttachments = NULL, *cnTags = NULL,*tContents = NULL;
		char *error_msg,*tempStr, *itemProjectValue;		
		  
		char **qry_entries = NULL,**qry_values = NULL;

		POM_AM__set_application_bypass(true);
		error_msg = (char*)MEM_alloc(sizeof(char) * 9000);
		itemProjectValue = (char *)MEM_alloc(sizeof(char) * 100);
		memset(error_msg, '\0', 9000);
		tc_strcpy(error_msg, "");

		// 获取目标对象
		EPM_ask_root_task(msg.task, &tRootTask);
		EPM_ask_attachments(tRootTask, EPM_target_attachment, &iAttachmentCount, &tAttachments);
		for (size_t i = 0; i < iAttachmentCount; i++) {
			vector<string> *itemProjectVec = new vector<string>();
			char *objectName = NULL ;
			tag_t tTargetObj = NULL_TAG, qry = NULLTAG;
			tag_t tItemRev = NULL_TAG, tItem = NULL_TAG;
			logical isRev = false;
			tTargetObj = tAttachments[i];
			GTCTYPE_is_type_of(tTargetObj, "ItemRevision", &isRev);
			if (!isRev) {
				continue;
			}			
			tItemRev = tTargetObj;			
			if (NULL_TAG == tItemRev) {
				continue;
			}
			// 获取对象项目信息				
			getProjectInfo(tItemRev, *itemProjectVec);
			printf("itemProjectVec size is: %d\n", (*itemProjectVec).size());
			for (size_t j = 0; j < (*itemProjectVec).size(); j++)
			{
				// 获取目标对象项目信息
				tc_strcpy(itemProjectValue, (*itemProjectVec)[j].c_str());
				TC_write_syslog("itemProjectValue is %s\n", itemProjectValue);
				printf("itemProjectValue is: %s\n", itemProjectValue);

				qry_entries = (char **)MEM_alloc(sizeof(char*) * 1);
				qry_values = (char **)MEM_alloc(sizeof(char*) * 1);
				qry_entries[0] = (char *)MEM_alloc(sizeof(char*) * 64);
				qry_values[0] = (char *)MEM_alloc(sizeof(char*) * 64);

				tc_strcpy(qry_entries[0], "SPAS_ID");
				tc_strcpy(qry_values[0], itemProjectValue);

				ITKCALL(QRY_find2("__D9_Find_CustomPN_Folder", &qry));

				ITKCALL(QRY_execute(qry, 1, qry_entries, qry_values, &cnt, &cnTags));
				printf("search  event resultNum == %d\n", cnt);
				TC_write_syslog("search  event resultNum == %d\n", cnt);
				if (cnt<=0) {
					continue;
				}

				AOM_ask_value_tags(cnTags[0], "contents", &iCount, &tContents);
				if (iCount<=0) {
					continue;
				}
				tag_t af = NULL,afTmp = NULL;
				char *an = NULL,*newAn = NULL;
				logical isF = false;
				for (size_t k = 0; k < iCount; k++) {
					ITKCALL(AOM_ask_value_string(tContents[k], "object_name", &an));					
					if (tc_strcmp(an, G2U("產品設計協同工作區")) == 0) {
						TC_write_syslog("+++++++++++++++++++++++++\n");
						af = tContents[k];
						break;
					}
				}
				if (af == NULL) {
					continue;
				}

				AOM_ask_value_tags(af, "contents", &iCount, &tContents);
				for (size_t k = 0; k < iCount; k++) {
					ITKCALL(AOM_ask_value_string(tContents[k], "object_name", &an));
					if (tc_strcmp(an, G2U("自編物料協同工作區")) == 0) {	
						TC_write_syslog("--------------------------\n");
						af = tContents[k];
						break;
					}
				}				
				if (af == NULL) {
					continue;
				}
				printf("11111111111 %d\n", cnt);
				ITKCALL(AOM_ask_value_string(tItemRev, "d9_BoardType_L6", &newAn));
				TC_write_syslog("board type %s\n", newAn);
				AOM_ask_value_tags(af, "contents", &iCount, &tContents);
				for (size_t k = 0; k < iCount; k++) {
					GTCTYPE_is_type_of(tContents[k], "Folder", &isF);
					if (!isF) {
						continue;
					}
					ITKCALL(AOM_ask_value_string(tContents[k], "object_name", &an));
					if (tc_strcmp(newAn, an) == 0) {
						TC_write_syslog("not found board folder\n");
						afTmp = tContents[k];
						break;
					}
				}
				
				
				if (afTmp == NULL) {
					//创建文件夹
					TC_write_syslog("create new folder\n");
					FL_create2(newAn, newAn, &afTmp);
					AOM_save_with_extensions(afTmp);
					TC_write_syslog("create new folder2\n");
					ITKCALL(AOM_load(af));
					ITKCALL(AOM_refresh(af, false));
					ITKCALL(FL_insert(af, afTmp, 999));
					ITKCALL(AOM_save_with_extensions(af));
					ITKCALL(AOM_unload(af));
					printf("create new folder success!\n");
					TC_write_syslog("create new folder\n");
					af = afTmp;
					printf("333333 %d\n", cnt);
				}else {
					af = afTmp;
				}
				printf("4444444444444 %d\n", cnt);
				ITKCALL(ITEM_ask_item_of_rev(tItemRev, &tItem));
				if (!isExistItem(af, tItem)) {
					 ITKCALL(AOM_load(af));
					 ITKCALL(AOM_refresh(af, false));
					 ITKCALL(FL_insert(af, tItem, 999));
					 ITKCALL(AOM_save_with_extensions(af));
					 ITKCALL(AOM_unload(af));
				}

			}

			itemProjectVec->clear();
			itemProjectVec = NULL;
		}

	CLEANUP:
		DOFREE(cnTags);
		DOFREE(tContents);

		return decision;
	}








#ifdef __cplusplus
}
#endif