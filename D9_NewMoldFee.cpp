#include "extension.h"
#include "util.h"

extern "C" int POM_AM__set_application_bypass(logical bypass);
#ifdef __cplusplus
extern "C" {
#endif

	int D9_NewMoldFee(EPM_action_message_t msg)
	{
		int ifail = ITK_ok;

		char
			* itemId = NULL,			
			* version = NULL,
			* projectId = NULL,
			* projectName = NULL,
			* actualUser = NULL,
			* taskName = NULL,
			* partName = NULL,
			* currency = NULL,
			* hhpn = NULL,
			* create_time = NULL,
			* newMoldFee = NULL,
			* tableName = "XPLM.MATERIAL_MOLD_INFO";
			

		tag_t
			current_task = NULLTAG,
			root_task = NULLTAG,
			newMoldFeeForm = NULLTAG,
			MEItemRev = NULLTAG,
			MEItem = NULLTAG,
			reference_obj = NULLTAG,
			owning_user_tag = NULLTAG;

		tag_t
			* attach_object_list = NULLTAG,
			* reference_object_list = NULLTAG;

		date_t
			create_date;

		string insertSql = "";
		string keys = "";
		string values = "";			

		vector<tag_t> projectList;
		projectList.clear();

		int
			attach_object_num = 0,
			reference_object_num = 0;

		Environment* env = NULL;
		Connection* conn = NULL;
		Statement* pStmt = NULL;
		ResultSet* pRs = NULL;		

		POM_AM__set_application_bypass(true);
		TC_write_syslog("\n enter D9_NewMoldFee.....!\n");

		current_task = msg.task;

		ITKCALL(EPM_ask_root_task(msg.task, &root_task));
		ITKCALL(AOM_ask_value_string(root_task, "job_name", &taskName));

		TC_write_syslog("\n 开始处理流程：%s\n", taskName);
		ITKCALL(EPM_ask_attachments(root_task, EPM_target_attachment, &attach_object_num, &attach_object_list));
		for (size_t i = 0; i < attach_object_num; i++) {
			logical
				isRev = false;					

			ITKCALL(GTCTYPE_is_type_of(attach_object_list[i], "D9_MEDesignRevision", &isRev));
			if (isRev) {
				MEItemRev = attach_object_list[i];

//				ITKCALL(ITEM_ask_item_of_rev(MEItemRev, &MEItem)); // 对象版本获取对象
				ITKCALL(AOM_ask_creation_date(MEItemRev, &create_date));

				create_time = (char*)MEM_alloc(sizeof(char) * 4096);
				formatDate(create_time, "%Y-%m-%d %H:%M", create_date);
				printf("create_date == %s\n", create_time);
				TC_write_syslog("create_date == %s\n", create_time);

				ITKCALL(getProjectInfo(MEItemRev, projectList)); // 获取专案集合
				if (projectList.size() <= 0) {
					TC_write_syslog("未指派专案\n");
					goto CLEANUP;
				}

				ITKCALL(AOM_ask_value_string(projectList[0], "project_id", &projectId));
				TC_write_syslog("projectId == %s\n", projectId);

				ITKCALL(AOM_ask_value_string(projectList[0], "project_name", &projectName));
				TC_write_syslog("projectName == %s\n", U2G(projectName));

				ITKCALL(AOM_ask_value_tags(MEItemRev, "IMAN_reference", &reference_object_num, &reference_object_list));
				for (size_t j = 0; j < reference_object_num; j++) {
					logical
						isMoldFee = false;
					reference_obj = reference_object_list[j];
					ITKCALL(GTCTYPE_is_type_of(reference_obj, "D9_MoldInfo", &isMoldFee));
					if (isMoldFee) {
						ITKCALL(AOM_ask_value_string(MEItemRev, "item_id", &itemId));
						TC_write_syslog("itemId == %s\n", itemId);

						ITKCALL(AOM_ask_value_string(MEItemRev, "item_revision_id", &version));
						TC_write_syslog("item_revision_id == %s\n", version);

						ITKCALL(AOM_ask_value_string(MEItemRev, "object_name", &partName));
						TC_write_syslog("objectName == %s\n", U2G(partName));

						ITKCALL(AOM_ask_value_string(MEItemRev, "d9_HHPN", &hhpn));
						TC_write_syslog("hhpn == %s\n", hhpn);


						ITKCALL(AOM_ask_value_string(MEItemRev, "d9_ActualUserID", &actualUser));
						TC_write_syslog("actualUser == %s\n", U2G(actualUser));

						newMoldFeeForm = reference_obj;						
						break;
					}
				}


				if (newMoldFeeForm != NULL) {
					ITKCALL(AOM_ask_value_string(newMoldFeeForm, "d9_Currency", &currency));
					TC_write_syslog("currency == %s\n", U2G(currency));

					ITKCALL(AOM_ask_value_string(newMoldFeeForm, "d9_NewMoldFee", &newMoldFee));
					TC_write_syslog("newMoldFee == %s\n", newMoldFee);

					getConnection("D9_DB_XPLM", env, conn);
					pStmt = conn->createStatement();

					/*insertSql = "INSERT INTO " + (string)tableName + "(DRAWING_NO, DESCRIPTIONS, NEW_MOLD_FEE, CURRENCY, OWNER, CREATE_DATE) "
						+ " VALUES(" + "'" + itemId + "'," + "'" + objectName + "'," + "'" + newMoldFee + "'," + "'" + currency + "'," + "'" + actualUser + "'," + "SYSDATE)";*/

					insertSql = "MERGE INTO " + (string)tableName + " t1" +
						" USING (SELECT " + "'" + itemId + "'" + " AS ITEM_ID, " + "'" + version + "'" + " AS ITEM_REVISION_ID, " + "'" + projectId + "'" + " AS PROJECT_ID, " + "'" + projectName + "'" + " AS PROJECT_NAME, " 
						+ "'" + hhpn + "'" + " AS HHPN, " + "'" + partName + "'" + " AS PART_NAME, " + "'" + newMoldFee + "'"
						+ " AS NEW_MOLD_FEE, " + "'" + currency + "'" + " AS CURRENCY, " + "'" + actualUser + "'" + " AS OWNER, " + "'" + create_time + "'" + " AS CREATE_DATE " + " FROM DUAL) t2"
						+ " ON (t1.ITEM_ID = t2.ITEM_ID AND t1.ITEM_REVISION_ID = t2.ITEM_REVISION_ID) "
						+ " WHEN MATCHED THEN "
						+ " UPDATE SET t1.PROJECT_ID = t2.PROJECT_ID, t1.PROJECT_NAME = t2.PROJECT_NAME, t1.HHPN = t2.HHPN, t1.PART_NAME = t2.PART_NAME, t1.NEW_MOLD_FEE = t2.NEW_MOLD_FEE, t1.CURRENCY = t2.CURRENCY, t1.OWNER = t2.OWNER, t1.CREATE_DATE = t2. CREATE_DATE, t1.RECORD_DATE = SYSDATE"
						+ " WHEN NOT MATCHED THEN "
						+ " INSERT (ITEM_ID, ITEM_REVISION_ID, PROJECT_ID, PROJECT_NAME, HHPN, PART_NAME, NEW_MOLD_FEE, CURRENCY, OWNER, CREATE_DATE, RECORD_DATE) "
						+ " VALUES(" + "'" + itemId + "'," + "'" + version + "'," + "'" + projectId + "'," + "'" + projectName + "'," + "'" + hhpn + "'," + "'" + partName + "'," + "'" + newMoldFee + "'," + "'" + currency + "'," + "'" + actualUser + "'," + "'" + create_time + "'," + "SYSDATE)";

					TC_write_syslog("insertSql == %s\n", insertSql.c_str());

					pStmt = conn->createStatement();
					pStmt->executeUpdate(insertSql);
					insertSql = "";
				}
			}			
		}
		

	CLEANUP:	
		if (pRs != NULL)
		{
			pStmt->closeResultSet(pRs);
		}
		if (pStmt != NULL) {
			conn->terminateStatement(pStmt);
		}
		if (conn != NULL) {
			env->terminateConnection(conn);
		}
		if (env != NULL) {
			Environment::terminateEnvironment(env);
		}
		POM_AM__set_application_bypass(false);
		DOFREE(taskName);
		DOFREE(attach_object_list);
		DOFREE(reference_object_list);
		DOFREE(itemId);
		DOFREE(version);
		DOFREE(projectId);
		DOFREE(projectName);
		DOFREE(partName);
		DOFREE(hhpn);
		DOFREE(actualUser);
		DOFREE(currency);
		DOFREE(newMoldFee);
		return ifail;
	}

#ifdef __cplusplus
}
#endif