#include "extension.h"
#include "util.h"
#include "errorMsg.h"
#include <time.h>

extern "C" int POM_AM__set_application_bypass(logical bypass);
using namespace oracle::occi;
#ifdef __cplusplus
extern "C" {
#endif
	int D9_CreateDCNPostAction(METHOD_message_t* msg, va_list args) {
		printf("\n=================D9_CreateDCNPostAction start======================\n");
		TC_write_syslog("\n=================D9_CreateDCNPostAction start==================\n");
		int ifail = ITK_ok;
		char
			*bu = NULL,
			*itemId = NULL,
			*version = NULL,
			*objectType = NULL,
			*tableName = "DCN_RECORD",
			*objectName = NULL,
			*create_time = NULL,
			*DCNFormName = NULL;

		date_t
			create_date;
		char
			** prop_names = (char**)MEM_alloc(sizeof(char **));		

		tag_t
			DCNItem = NULLTAG,
			DCNItemRevison = NULLTAG,
			target_obj = NULLTAG,
			relType = NULLTAG,
			DCNCostImpactForm = NULLTAG,
			primary_object = NULLTAG,
			secondary_object = NULLTAG,
			relation_type = NULLTAG,
			user_data = NULLTAG;
			
		int
			tagCount = 0,
			num = 0,
			prop_count = 0,
			arg_cnt = 0;

		tag_t 
			*tag_list = NULLTAG,
			*new_relation = NULLTAG;

		logical
			isDT_DCN_Rev = false,
			isMNT_DCN_Rev = false,
			is_PRT_DCN_Rev = false,
		    is_DCN_CostImpact_Form = false;

		string insertSql = "";	
		string findTDCNSql = "";
		string keys = "";
		string values = "";

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

		va_list item_args;
		va_copy(item_args, args);

		primary_object = va_arg(item_args, tag_t);
		secondary_object = va_arg(item_args, tag_t);
		relation_type = va_arg(item_args, tag_t);
		user_data = va_arg(item_args, tag_t);
		new_relation = va_arg(item_args, tag_t *);		

		POM_AM__set_application_bypass(true); //开启旁路
		vector<string> dcnTypeVec;
		getPreferenceByName("D9_DCN_Revision_Type", dcnTypeVec);
		for (size_t i = 0; i < dcnTypeVec.size(); i++)
		{
			GTCTYPE_is_type_of(primary_object, dcnTypeVec[i].c_str(), &isDT_DCN_Rev);
			if (isDT_DCN_Rev)
			{
				bu = "DT";
				break;
			}

			GTCTYPE_is_type_of(primary_object, dcnTypeVec[i].c_str(), &isMNT_DCN_Rev);
			if (isMNT_DCN_Rev)
			{
				bu = "MNT";
				break;
			}

			GTCTYPE_is_type_of(primary_object, dcnTypeVec[i].c_str(), &is_PRT_DCN_Rev);
			if (is_PRT_DCN_Rev)
			{
				bu = "PRT";
				break;
			}
		}
		if (!isDT_DCN_Rev && !isMNT_DCN_Rev && !is_PRT_DCN_Rev)
		{
			goto CLEANUP;			
		}

		DCNItemRevison = primary_object;

		ITKCALL(AOM_ask_value_string(DCNItemRevison, "item_id", &itemId)); //对象ID
		printf("item_id == %s\n", itemId);
		TC_write_syslog("item_id == %s\n", itemId);

		ITKCALL(AOM_ask_value_string(DCNItemRevison, "item_revision_id", &version));
		printf("item_revision_id == %s\n", version);
		TC_write_syslog("item_revision_id == %s\n", version);

		/*ITKCALL(AOM_ask_value_string(secondary_object, "object_name", &objectName));
		printf("object_name == %s\n", objectName);*/		

		GTCTYPE_is_type_of(secondary_object, "D9_DCNCostImpact", &is_DCN_CostImpact_Form);
		if (!is_DCN_CostImpact_Form)
		{
			goto CLEANUP;			
		}

		DCNCostImpactForm = secondary_object;

		ITKCALL(AOM_ask_value_string(DCNCostImpactForm, "object_name", &DCNFormName));
		printf("DCNFormName == %s\n", DCNFormName);

		ITKCALL(AOM_ask_creation_date(DCNItemRevison, &create_date));	

//		DATE_date_to_string(create_date, "%Y-%m-%d", &create_time);

		create_time = (char*)MEM_alloc(sizeof(char) * 4096);

		formatDate(create_time, "%Y-%m-%d %H:%M:%S", create_date);
		printf("create_date == %s\n", create_time);
		TC_write_syslog("create_date == %s\n", create_time);

//		printf("creation_date == %s\n", create_date);
//		TC_write_syslog("creation_date == %s\n", create_date);		

		/*if (true)
		{
			goto CLEANUP;
		}*/	

		getPreferenceByName("D9_DB_XPLM", *dbInfoVec);
		getHashMapPreference(*dbInfoVec, *dbInfoMap);

		for ((*dbInfoIter) = (*dbInfoMap).rbegin(); (*dbInfoIter) != (*dbInfoMap).rend(); (*dbInfoIter)++) {
			string tmpKey = (*dbInfoIter)->first;
			string tmpValue = (*dbInfoIter)->second;
			if (strcmp(tmpKey.c_str(), "ip") == 0) {
				printf("ip decoded== %s\n", base64_decode(tmpValue));
				strcpy(ip, base64_decode(tmpValue).c_str());
			}
			else if (strcmp(tmpKey.c_str(), "username") == 0) {
				printf("userName decoded== %s\n", base64_decode(tmpValue));
				strcpy(userName, base64_decode(tmpValue).c_str());
			}
			else if (strcmp(tmpKey.c_str(), "password") == 0) {
				printf("password decoded== %s\n", base64_decode(tmpValue));
				strcpy(password, base64_decode(tmpValue).c_str());
			}
			else if (strcmp(tmpKey.c_str(), "port") == 0) {
				printf("port decoded== %s\n", base64_decode(tmpValue));
				strcpy(port, base64_decode(tmpValue).c_str());
			}
			else if (strcmp(tmpKey.c_str(), "sid") == 0) {
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
			goto CLEANUP;
		}

		printf("createEnvironment success... \n");
		TC_write_syslog("createEnvironment success... \n");

		conn = env->createConnection(userName, password, connectStr);
		if (NULL == conn) {
			printf("createConnection error... \n");
			TC_write_syslog("createConnection error... \n");
			goto CLEANUP;
		}

		printf("createConnection success... \n");
		TC_write_syslog("createConnection success... \n");
		
			
		if (DCNCostImpactForm != NULL)
		{		
//			pStmt = conn->createStatement();

//			string *findTDCNSql = new string();
			
			ITKCALL(AOM_ask_prop_names(DCNCostImpactForm, &prop_count, &prop_names));			
			for (size_t k = 0; k < prop_count; k++)
			{

				printf("prop_names == %s\n", prop_names[k]);

				if (strcmp("d9_CustomerRequest", prop_names[k]) != 0 && strcmp("d9_DFXImprovement", prop_names[k]) != 0 && strcmp("d9_DesignImprovement_ME", prop_names[k]) != 0 
					&& strcmp("d9_DesignImprovement_Others", prop_names[k]) != 0 && strcmp("d9_ProcessImprovement", prop_names[k]) != 0) {
					continue;
				}								

				findTDCNSql = "SELECT COUNT(*) FROM " + (string)tableName + " WHERE DCN_NO = '" + (string)itemId + "' AND REASON = '" + (string)prop_names[k] + "'";
				printf("findTDCNSql == %s\n", findTDCNSql.c_str());
				TC_write_syslog("findTDCNSql == %s\n", findTDCNSql.c_str());	

				pStmt = conn->createStatement();
				pRs = pStmt->executeQuery(findTDCNSql);
				while (pRs->next())
				{
					num = pRs->getInt(1);
					if (num <= 0)
					{
						keys = keys + "BU,";
						values = values + "'" + bu + "',";					

						keys = keys + "DCN_NO,";
						values = values + "'" + itemId + "',";

						keys = keys + "REASON,";
						values = values + "'" + prop_names[k] + "',";
						

						keys = keys + "CREATE_DATE";
						values = values + "'" + create_time + "'";
						
						insertSql = "INSERT INTO " + (string)tableName + "(" + keys + ") VALUES(" + values + ")";
							
						printf("insertSql == %s\n", insertSql.c_str());
						TC_write_syslog("insertSql == %s\n", insertSql.c_str());

						pStmt = conn->createStatement();
						pStmt->executeUpdate(insertSql);

						keys = "";
						values = "";
					}
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

		DOFREE(itemId);
		DOFREE(version);
//		DOFREE(create_date);
		DOFREE(tag_list);
		DOFREE(prop_names);
		if (dbInfoVec != NULL)
		{
			(*dbInfoVec).clear();
			dbInfoVec = NULL;
		}
		
		if (dbInfoMap != NULL)
		{
			(*dbInfoMap).clear();
			dbInfoMap = NULL;

		}		
		dbInfoIter = NULL;

		printf("\n=================  D9_CreateDCNPostAction end   ======================\n");
		TC_write_syslog("\n=================  D9_CreateDCNPostAction end   ==================\n");
		return ifail;
	}




#ifdef __cplusplus
}
#endif

