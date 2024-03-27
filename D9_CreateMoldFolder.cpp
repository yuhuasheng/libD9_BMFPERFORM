#include "extension.h"
#include "util.h"
#include "errorMsg.h"
#include <tc/folder.h>

extern "C" int POM_AM__set_application_bypass(logical bypass);
#ifdef __cplusplus
extern "C" {
#endif

	void getProjectPhase(tag_t itemRev, char** project);
	tag_t createFolder(char* folderName, tag_t  parentFolder);
	tag_t findChildFolder(tag_t pFolder, char* childFolderName, bool flag);
	string createModelFolder(tag_t itemRev, tag_t eebom);
	void getPCBA(tag_t bom, tag_t* pcab);
	vector<tag_t> findPhaseFolder(tag_t pFolder, char* eebomPhase);

	int D9_CreateMoldFolder(EPM_rule_message_t msg)
	{
		EPM_decision_t decision = EPM_go;
		int ifail = ITK_ok, counts = 0, pcabCount = 0;
		tag_t root_task, * target_tags, * relations = NULLTAG;
		logical isType = false;
		ITKCALL(ifail = EPM_ask_root_task(msg.task, &root_task));
		ITKCALL(ifail = EPM_ask_attachments(root_task, EPM_target_attachment, &counts, &target_tags));
		if (counts > 0) {
			for (int i = 0; i < counts; i++) {
				GTCTYPE_is_type_of(target_tags[i], "D9_BOMRequestRevision", &isType);
				if (isType) {
					TC_write_syslog("F41 WF Create Mold Folder Start");
					AOM_ask_value_tags(target_tags[i], "D9_BOMReq_PCBA_REL", &pcabCount, &relations);
					if (pcabCount > 0) {
						for (int p = 0; p < pcabCount; p++) {
							string result = createModelFolder(relations[p], target_tags[i]);
							if (result.length() > 0) {
								decision = EPM_nogo;
								EMH_store_error_s1(EMH_severity_user_error, VER_RULE_ERROR, result.c_str());
								break;
							}
						}
					}
					else {
						TC_write_syslog("bom is not D9_BOMRequestRevision type :  " + isType);
					}
					break;
				}
				else {
					TC_write_syslog("bom is not D9_BOMRequestRevision type :  " + isType);
				}
			}
		}
		return decision;
	}

	void getPCBA(tag_t bom, tag_t* pcab) {
		int count = 0;
		tag_t* relations = NULLTAG;
		AOM_ask_value_tags(bom, "D9_BOMReq_PCBA_REL", &count, &relations);
		if (count > 0) {
			*pcab = relations[0];
		}
	}

	string createModelFolder(tag_t itemRev, tag_t eebom) {
		string result = "";
		char* clsArray[] = { "Power&PI&INV&LED CONV BD","LED driver BD","LED Lighting BD","LED Lighting  Driver BD","DC JACK","Safety POWER BD" };
		char* ids, * modelName, * cif, * mGroup, * itemId, ** qry_entries, ** qry_values, * fmodeName, * deriTypeDc;
		AOM_ask_value_string(itemRev, "d9_MaterialGroup", &mGroup);
		if (tc_strcmp(mGroup, "B8X80") != 0) {
			return ITK_ok;
		}
		AOM_ask_value_string(itemRev, "project_ids", &ids);
		//AOM_ask_value_string(itemRev, "d9_ModelName", &modelName);
		AOM_ask_value_string(itemRev, "d9_FoxconnModelName", &fmodeName);
		AOM_ask_value_string(itemRev, "d9_DerivativeTypeDC", &deriTypeDc);

		if (strlen(deriTypeDc) > 0) {
			modelName = (char*)malloc(strlen(fmodeName) + strlen(deriTypeDc) + 1);
			sprintf(modelName, "%s_%s", fmodeName, deriTypeDc);
		}
		else {
			modelName = fmodeName;
		}
		AOM_ask_value_string(itemRev, "d9_PCBAssyClassification_L6", &cif);
		AOM_ask_value_string(itemRev, "item_id", &itemId);
		TC_write_syslog("F41 WF PCAB  %s---- >>  ids : %s   modelName: %s  PCBAssyClassification_L6: %s\n", itemId, ids, modelName, cif);
		//char* qs[2] = { "SPAS ID" ,"Name" };
		//char* qv[2] = { ids,"*" };
		qry_entries = (char**)MEM_alloc(sizeof(char*) * 1);
		qry_values = (char**)MEM_alloc(sizeof(char*) * 1);
		qry_entries[0] = (char*)MEM_alloc(sizeof(char*) * 64);
		qry_values[0] = (char*)MEM_alloc(sizeof(char*) * 64);
		tc_strcpy(qry_entries[0], "SPAS ID");
		//tc_strcpy(qry_entries[1], "Name");
		tc_strcpy(qry_values[0], ids);
		//tc_strcpy(qry_values[1], "*");
		tag_t* queryResults;
		int resultLength;
		tag_t qry;
		//ITKCALL(QRY_find2("__D9_Find_Project_Folder", &qry));
		ITKCALL(query("__D9_Find_Project_Folder", qry_entries, qry_values, 1, &queryResults, &resultLength));
		TC_write_syslog(" -- >> project ids : %s queryresult : %d \n", ids, resultLength);
		if (resultLength > 0) {
			char* projectPhase = NULL;
			//getProjectPhase(eebom, &projectPhase);
			AOM_ask_value_string(eebom, "d9_ProjectPhase", &projectPhase);
			TC_write_syslog("F41 WF PCAB  %s---- >>  projectPhase : %s \n", itemId, projectPhase);
			int iCount;
			tag_t* tContents;
			char* buFolder;
			tag_t ee = NULLTAG, psu = NULLTAG, layout = NULLTAG;
			AOM_ask_value_tags(queryResults[0], "contents", &iCount, &tContents);
			for (size_t k = 0; k < iCount; k++) {
				AOM_ask_value_string(tContents[k], "object_name", &buFolder);
				if (tc_strcmp(buFolder, G2U("EE")) == 0) {
					ee = tContents[k];
				}
				if (tc_strcmp(buFolder, G2U("Layout")) == 0) {
					layout = tContents[k];
				}
				if (tc_strcmp(buFolder, G2U("PSU")) == 0) {
					psu = tContents[k];
				}
			}
			bool isContainer = false;
			int arrayLen = sizeof(clsArray) / sizeof(clsArray[0]);
			for (int i = 0; i < arrayLen; i++) {
				if (strcmp(clsArray[i], cif) == 0) {
					isContainer = true;
					break;
				}
			}
			if (projectPhase != NULL && strlen(projectPhase) > 0) {
				vector<tag_t> eefs = findPhaseFolder(ee, projectPhase);
				vector<tag_t> psufs = findPhaseFolder(psu, projectPhase);
				vector<tag_t> layoutfs = findPhaseFolder(layout, projectPhase);
				if (!isContainer) {
					if (eefs.size() > 0) {
						for (int i = 0; i < eefs.size(); i++) {
							tag_t moldFolder = createFolder(modelName, eefs[i]);
							createFolder(cif, moldFolder);
						}
					}
					else {
						result.append("Project EE Phase Folder No Exist :").append(projectPhase);
					}
				}
				else {
					if (psufs.size() > 0) {
						for (int i = 0; i < psufs.size(); i++) {
							tag_t moldFolder = createFolder(modelName, psufs[i]);
							createFolder(cif, moldFolder);
						}
					}
					else {
						result.append("Project PSU Phase Folder No Exist :").append(projectPhase);
					}
				}
				if (layoutfs.size() > 0) {
					for (int i = 0; i < layoutfs.size(); i++) {
						tag_t moldFolder = createFolder(modelName, layoutfs[i]);
						createFolder(cif, moldFolder);
					}
				}
				else {
					result.append("Project Layout Phase Folder No Exist :").append(projectPhase);
				}
			}
			else {
				result = "BOM ProjectPhase Attribute is null !";
			}
		}
		else {
			result = "Unable To Find  Project Foder , Perhaps No project has been assigned !";
		}

		return result;
	}

	void getProjectPhase(tag_t itemRev, char** projectPhase) {
		AOM_ask_value_string(itemRev, "d9_ProjectPhase", projectPhase);
		//int n_referencers;
		//int* levels;
		//tag_t* referencers;
		//char** relations;
		//WSOM_where_referenced2(itemRev, 1, &n_referencers, &levels, &referencers, &relations);
		//for (int i = 0; i < n_referencers; i++) {
		//	if (tc_strcmp("D9_BOMReq_PCBA_REL", relations[i]) == 0) {
		//		tag_t item = NULLTAG, lastRev;
		//		ITEM_ask_item_of_rev(referencers[i], &item);
		//		ITEM_ask_latest_rev(item, &lastRev);
		//		AOM_ask_value_string(lastRev, "d9_ProjectPhase", projectPhase);
		//		break;
		//	}
		//}

	}


	vector<tag_t> findPhaseFolder(tag_t pFolder, char* eebomPhase) {
		int iCount = 0;
		tag_t* tContents;
		char* subFolder, * phaseName = (char*)malloc(2);
		logical isType = false;
		AOM_ask_value_tags(pFolder, "contents", &iCount, &tContents);
		printf("find folder size : %d \n", iCount);
		vector<tag_t>* phaseFolder = new vector<tag_t>();
		for (size_t k = 0; k < iCount; k++) {
			GTCTYPE_is_type_of(tContents[k], "D9_PhaseFolder", &isType);
			if (isType) {
				AOM_ask_value_string(tContents[k], "object_name", &subFolder);
				if (strlen(subFolder) >= 2) {
					tc_strncpy(phaseName, subFolder, 2);
					if (tc_strcmp(phaseName, eebomPhase) >= 0) {
						phaseFolder->push_back(tContents[k]);
					}
				}
			}
		}
		return *phaseFolder;
	}


	tag_t findChildFolder(tag_t pFolder, char* childFolderName, bool flag) {
		int iCount = 0;
		tag_t* tContents;
		char* subFolder;
		logical isType = false;
		AOM_ask_value_tags(pFolder, "contents", &iCount, &tContents);
		printf("find folder size : %d \n", iCount);
		for (size_t k = 0; k < iCount; k++) {
			AOM_ask_value_string(tContents[k], "object_name", &subFolder);
			if (flag) {
				if (startWith(subFolder, G2U(childFolderName)) == 1) {
					return tContents[k];
				}
			}
			else {
				if (tc_strcmp(subFolder, G2U(childFolderName)) == 0) {
					return tContents[k];
				}
			}
		}
		return NULLTAG;
	}
	tag_t createFolder(char* folderName, tag_t  parentFolder) {
		char* parentFolderName;
		AOM_ask_value_string(parentFolder, "object_name", &parentFolderName);
		TC_write_syslog("F41WF Create Folder %s----%s \n", parentFolderName, folderName);
		tag_t folder = findChildFolder(parentFolder, folderName, false);
		if (folder == NULLTAG) {
			FL_create2(folderName, folderName, &folder);
			AOM_save_with_extensions(folder);
			if (folder != NULLTAG) {
				AOM_load(parentFolder);
				AOM_refresh(parentFolder, false);
				FL_insert(parentFolder, folder, 999);
				AOM_save_with_extensions(parentFolder);
				AOM_unload(parentFolder);
				TC_write_syslog("F41WF  Create Folder Success%s----%s \n", parentFolderName, folderName);
			}
		}
		else {
			printf("ÒÑ¾­´æÔÚ  %s \n", folderName);
		}
		return folder;
	}

#ifdef __cplusplus
}
#endif

