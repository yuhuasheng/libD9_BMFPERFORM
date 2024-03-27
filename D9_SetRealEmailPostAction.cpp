#include "extension.h"
#include "util.h"
#include "errorMsg.h"
#include <time.h>
extern "C" int POM_AM__set_application_bypass(logical bypass); 
#ifdef __cplusplus
extern "C" {
#endif

	string drawId(string actualUseId);
	int D9_SetRealEmailPostAction(METHOD_message_t* msg, va_list args) {
		printf("\n=================set_real_email_post_action start======================\n");
		TC_write_syslog("\n=================set_real_email_post_action start==================\n");
		int ifail = ITK_ok;		
		POM_AM__set_application_bypass(true);
		char* actualUserId = NULL;
		string strRealEmail;
		va_list item_args;
		va_copy(item_args, args);
		const char* item_id = va_arg(item_args, const char*);
		const char* item_name = va_arg(item_args, const char*);
		const char* type_name = va_arg(item_args, const char*);
		const char* rev_id = va_arg(item_args, const char*);
		tag_t* new_item = va_arg(item_args, tag_t*);
		tag_t* new_rev = va_arg(item_args, tag_t*);
		TC_write_syslog("item_id == %s\n", item_id);
		TC_write_syslog("item_name == %s\n", item_name);
		TC_write_syslog("rev_id == %s\n", rev_id);

		// 获取实际工作者
		ITKCALL(AOM_ask_value_string(new_rev[0], "d9_ActualUserID", &actualUserId));
		string strActualUserId = drawId(actualUserId);
		TC_write_syslog("actualUserId == %s\n", strActualUserId);

		Environment* env = NULL;
		Connection* conn = NULL;
		Statement* pStmt = NULL;
		ResultSet* pRs = NULL;

		getConnection("D9_DB_Info", env, conn);
		pStmt = conn->createStatement();
		string selectSql = "select u.notes from XPLM.spas_user u WHERE u.work_id = '" + strActualUserId + "'";		
		TC_write_syslog("selectSql == %s\n", selectSql.c_str());
		pRs = pStmt->executeQuery(selectSql);
		
		while (pRs->next())
		{
			strRealEmail = pRs->getString(1);
			TC_write_syslog("\n pRs->getString(1) = %s \n", strRealEmail.c_str());
		}
		if (strRealEmail.empty()) {
			goto CLEANUP;
		}

		// 写入真实邮箱
		ITKCALL(AOM_lock(new_rev[0])) //锁定设计对象
		ITKCALL(AOM_load(new_rev[0]));
		ITKCALL(AOM_set_value_string(new_rev[0], "d9_RealMail", strRealEmail.c_str()));
		ITKCALL(AOM_save_with_extensions(new_rev[0]));
		ITKCALL(AOM_unlock(new_rev[0])); //解锁设计对象

		TC_write_syslog("\n=================set_real_email_post_action end==================\n");
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
		DOFREE(actualUserId);
		POM_AM__set_application_bypass(false);
		return ifail;
	}

	string drawId(string actualUseId)
	{
		regex e("\\((.*)\\)");
		smatch m;
		if (regex_search(actualUseId, m, e)) {
			return m[1];
		}
		return "";
	}

#ifdef __cplusplus
}
#endif