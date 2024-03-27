#include "extension.h"
#include "util.h"
#define SHEET_METAL "ME-SM"
#define PLASTIC "ME-PL"

extern "C" int POM_AM__set_application_bypass(logical bypass);
#ifdef __cplusplus
extern "C" {
#endif

	int D9_DCNFee(EPM_action_message_t msg)
	{
		int ifail = ITK_ok;

		char
			* dcnItemId = NULL,
			* dcnVersion = NULL,
			* drawItemId = NULL,
			* drawVersion = NULL,
			* projectId = NULL,
			* projectName = NULL,
			* hhpn = NULL,
			* customerPN = NULL,
			* partName = NULL,
			* description = NULL,
			* taskName = NULL,
			* reasonChange = NULL,
			* reasonType = NULL,
			* actualUser = NULL,
			* cost = NULL,
			* currency = NULL,
			* create_time = NULL,
			* newProjectId = NULL,
			* bu = "DT",
			* status = "Released",
			* dcnRecordName = "XPLM.DCN_FEE_RECORD";	


		tag_t
			current_task = NULLTAG,
			root_task = NULLTAG,
			newMoldFeeForm = NULLTAG,			
			dcnItemRev = NULLTAG,
			drawItemRev = NULLTAG,
			owning_user_tag = NULLTAG;

		tag_t
			* attach_object_list = NULLTAG,	
			* change_reason_object_list = NULLTAG,
			* solutionItem_object_list = NULLTAG;

		date_t
			create_date;

		string insertSql = "";
		string findSql = "";
		

		string modelNoPrefix,
			customerType,
			productLine;

		vector<tag_t> projectList;			
		projectList.clear();

		int
			attach_object_num = 0,
			projectNum = 0,
			change_reason_num = 0,
			solutionItem_object_num = 0;

		Environment* env = NULL;
		Connection* conn = NULL;
		Statement* pStmt = NULL;
		ResultSet* pRs = NULL;

		POM_AM__set_application_bypass(true);
		TC_write_syslog("\n=================   D9_DCNFee start   ==================\n");

		current_task = msg.task;

		ITKCALL(EPM_ask_root_task(msg.task, &root_task));
		ITKCALL(AOM_ask_value_string(root_task, "job_name", &taskName));

		TC_write_syslog("\n 开始处理流程：%s\n", taskName);
		ITKCALL(EPM_ask_attachments(root_task, EPM_target_attachment, &attach_object_num, &attach_object_list));

		for (size_t i = 0; i < attach_object_num; i++) {
			logical
				isDCN = false;			

			logical
				flag = false;
			ITKCALL(GTCTYPE_is_type_of(attach_object_list[i], "D9_DT_DCNRevision", &isDCN));

			if (isDCN) {
				dcnItemRev = attach_object_list[i];
				break;
			}
		}
		
		if (dcnItemRev == NULL) {
			goto CLEANUP;
		}

		ITKCALL(AOM_ask_value_string(dcnItemRev, "item_id", &dcnItemId));
		TC_write_syslog("dcnItemId == %s\n", dcnItemId);				

		ITKCALL(AOM_ask_value_string(dcnItemRev, "item_revision_id", &dcnVersion));
		TC_write_syslog("dcnVersion == %s\n", dcnVersion);

		/*if (isRelease(dcnItemRev)) {
			status = "Released";
		}*/

		ITKCALL(AOM_ask_creation_date(dcnItemRev, &create_date));

		create_time = (char*)MEM_alloc(sizeof(char) * 4096);
		formatDate(create_time, "%Y-%m-%d %H:%M", create_date);
		printf("create_date == %s\n", create_time);
		TC_write_syslog("create_date == %s\n", create_time);

		ITKCALL(getProjectInfo(dcnItemRev, projectList)); // 获取专案集合
		if (projectList.size() <= 0) {
			TC_write_syslog("dcnItemId == %s, dcnVersion == %s, 未指派专案\n", dcnItemId, dcnVersion);
			goto CLEANUP;
		}

		ITKCALL(AOM_ask_value_tags(dcnItemRev, "d9_ReasonForChangeTable", &change_reason_num, &change_reason_object_list)); // 获取变更原因条目

		ITKCALL(AOM_ask_value_tags(dcnItemRev, "CMHasSolutionItem", &solutionItem_object_num, &solutionItem_object_list));

		getConnection("D9_DB_XPLM", env, conn); // 连接数据库

		for (size_t j = 0; j < solutionItem_object_num; j++) {
			logical
				isMERev = false;
			drawItemRev = solutionItem_object_list[j];

			ITKCALL(GTCTYPE_is_type_of(drawItemRev, "D9_MEDesignRevision", &isMERev));
			if (!isMERev) {
				continue;
			}

			ITKCALL(AOM_ask_value_string(drawItemRev, "item_id", &drawItemId));
			TC_write_syslog("drawItemId == %s\n", drawItemId);

			ITKCALL(AOM_ask_value_string(drawItemRev, "item_revision_id", &drawVersion));
			TC_write_syslog("drawVersion == %s\n", drawVersion);

			if (startWith(drawItemId, SHEET_METAL)) {
				modelNoPrefix = SHEET_METAL;
			}
			else if (startWith(drawItemId, PLASTIC)) {
				modelNoPrefix = PLASTIC;
			}

			ITKCALL(AOM_ask_value_string(drawItemRev, "d9_HHPN", &hhpn));
			TC_write_syslog("hhpn == %s\n", hhpn);

			ITKCALL(AOM_ask_value_string(drawItemRev, "d9_CustomerPN", &customerPN));
			TC_write_syslog("customerPN == %s\n", customerPN);

			ITKCALL(AOM_ask_value_string(drawItemRev, "object_name", &partName));
			TC_write_syslog("partName == %s\n", U2G(partName));
					
			for (size_t k = 0; k < projectList.size(); k++) {
				ITKCALL(AOM_ask_value_string(projectList[k], "project_id", &projectId));
				TC_write_syslog("projectId == %s\n", projectId);

				ITKCALL(AOM_ask_value_string(projectList[k], "project_name", &projectName));
				TC_write_syslog("projectName == %s\n", U2G(projectName));

				newProjectId = (char*)MEM_alloc(sizeof(char) * 4096);

				strcat(newProjectId, projectId);
				strReplace(newProjectId, "p", ""); // 将p替换为空
				strReplace(newProjectId, "P", ""); // 将p替换为空
				findSql = "SELECT p.id platformFoundId, c.name customerName, l.name platformFoundProductLine from spas_platform_found p, spas_series s, spas_customer c, spas_product_line l WHERE p.id = '"
					+ (string)newProjectId + "' and p.project_series_id=s.id and s.customer_id=c.id and p.product_line_id=l.id";

				TC_write_syslog("findSql == %s\n", findSql.c_str());

				pStmt = conn->createStatement();
				pRs = pStmt->executeQuery(findSql);

				while (pRs->next()) {
					customerType = pRs->getString(2);
					TC_write_syslog("customerType == %s\n", customerType.c_str());

					productLine = pRs->getString(3);
					TC_write_syslog("productLine == %s\n", productLine.c_str());
					break;

				}
				for (size_t m = 0; m < change_reason_num; m++) {
					ITKCALL(AOM_ask_value_string(change_reason_object_list[m], "d9_ReasonOfChange", &reasonChange));
					TC_write_syslog("reasonChange == %s\n", U2G(reasonChange));

					ITKCALL(AOM_ask_value_string(change_reason_object_list[m], "d9_ReasonType", &reasonType));
					TC_write_syslog("reasonType == %s\n", U2G(reasonType));

					ITKCALL(AOM_ask_value_string(change_reason_object_list[m], "d9_Owner", &actualUser));
					TC_write_syslog("actualUser == %s\n", U2G(actualUser));

					ITKCALL(AOM_ask_value_string(change_reason_object_list[m], "d9_Cost", &cost));
					TC_write_syslog("cost == %s\n", cost);

					ITKCALL(AOM_ask_value_string(change_reason_object_list[m], "d9_Currency", &currency));
					TC_write_syslog("currency == %s\n", U2G(currency));

					insertSql = "MERGE INTO " + (string)dcnRecordName + " t1" +
						" USING (SELECT " + "'" + bu + "'" + " AS BU, " + "'" + projectId + "'" + " AS PROJECT_ID, " + "'" + projectName + "'" + " AS PROJECT_NAME, " + "'" + customerType + "'"
						+ " AS CUSTOMER_TYPE, " + "'" + productLine + "'" + " AS PRODUCT_LINE, " + "'" + dcnItemId + "'" + " AS DCN_NO, " +"'" + dcnVersion + "'" + " AS DCN_Version, "
						+ "'" + drawItemId + "'" + " AS MODEL_NO, " + +"'" + drawVersion + "'" + " AS MODEL_NO_Version, " + +"'" + modelNoPrefix + "'" + " AS MODEL_NO_PREFIX, " + +"'" + hhpn + "'" + " AS HHPN, "
						+ "'" + customerPN + "'" + " AS CUSTOMER_PN, " + +"'" + partName + "'" + " AS PART_NAME, " + "'" + reasonChange + "'" + " AS DESCRIPTION, " + "'" + reasonType + "'" + " AS REASON, "
						+ "'" + actualUser + "'" + " AS OWNER, " + "'" + cost + "'" + " AS COST_IMPACT, " + "'" + currency + "'" + " AS CURRENCY, " + "'" + status + "'" + " AS STATUS, " + "'" + create_time + "'" + " AS CREATE_DATE " + " FROM DUAL) t2"
						+ " ON (t1.DCN_NO = t2.DCN_NO AND t1.DCN_Version = t2.DCN_Version AND t1.DESCRIPTION = t2.DESCRIPTION AND t1.REASON = t2.REASON) "
						+ " WHEN MATCHED THEN "
						+ " UPDATE SET t1.PROJECT_ID = t2.PROJECT_ID, t1.PROJECT_NAME = t2.PROJECT_NAME, t1.CUSTOMER_TYPE = t2.CUSTOMER_TYPE, t1.PRODUCT_LINE = t2.PRODUCT_LINE, t1.MODEL_NO = t2.MODEL_NO,"
						+ " t1.MODEL_NO_Version = t2.MODEL_NO_Version, t1.MODEL_NO_PREFIX = t2.MODEL_NO_PREFIX, t1.HHPN = t2.HHPN, t1.CUSTOMER_PN = t2.CUSTOMER_PN, t1.PART_NAME = t2.PART_NAME," 
						+ " t1.OWNER = t2.OWNER, t1.COST_IMPACT = t2.COST_IMPACT, t1.CURRENCY = t2.CURRENCY, t1.STATUS = t2.STATUS, t1.CREATE_DATE = t2.CREATE_DATE, "
						+ " t1.RECORD_DATE = SYSDATE"
						+ " WHEN NOT MATCHED THEN "
						+ " INSERT (BU, PROJECT_ID, PROJECT_NAME, CUSTOMER_TYPE, PRODUCT_LINE, DCN_NO, DCN_Version, MODEL_NO, MODEL_NO_Version, MODEL_NO_PREFIX, HHPN, CUSTOMER_PN, PART_NAME, DESCRIPTION, REASON, OWNER, COST_IMPACT," 
						+ " CURRENCY, STATUS, CREATE_DATE, RECORD_DATE) "
						+ " VALUES(" + "'" + bu + "'," + "'" + projectId + "'," + "'" + projectName + "'," + "'" + customerType + "'," + "'" + productLine + "'," + "'" + dcnItemId + "'," + "'" + dcnVersion + "',"
						+ "'" + drawItemId + "'," + "'" + drawVersion + "'," + "'" + modelNoPrefix + "'," + "'" + hhpn + "'," + "'" + customerPN + "'," + "'" + partName + "'," + "'" + reasonChange + "',"						
						+ "'" + reasonType + "'," + "'" + actualUser + "'," + "'" + cost + "'," + "'" + currency + "'," + "'" + status + "'," + "'" + create_time + "'," + "SYSDATE)";

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
		DOFREE(dcnItemId);
		DOFREE(dcnVersion);
		DOFREE(change_reason_object_list);
		DOFREE(solutionItem_object_list);
		DOFREE(drawItemId);
		DOFREE(drawVersion);
		DOFREE(hhpn);
		DOFREE(customerPN);
		DOFREE(partName);
		DOFREE(projectId);
		DOFREE(projectName);
		DOFREE(reasonChange);
		DOFREE(reasonType);
		DOFREE(actualUser);
		DOFREE(cost);
		DOFREE(currency);
		TC_write_syslog("\n=================   D9_DCNFee   end   ==================\n");
		return ifail;
	}

#ifdef __cplusplus
}
#endif