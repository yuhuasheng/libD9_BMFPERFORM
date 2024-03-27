
#include "util.h"
#include "extension.h"
#include "preference.h"

extern "C" int POM_AM__set_application_bypass(logical bypass);
string getSevValue(vector<string> vec, string RPNValue);
bool checkRPNRange(string RPNRange, string str);
#ifdef __cplusplus
extern "C" {
#endif

	int D9_IRSeverityIRDELL(METHOD_message_t* msg, va_list args) {

		TC_write_syslog("\n================= D9_IRSeverityIRDELL Start ==================\n");

		int ifail = ITK_ok;

		tag_t prop_tag;
		char** value;
		va_list largs;
		va_copy(largs, args);
		prop_tag = va_arg(largs, tag_t);
		value = va_arg(largs, char**);

		char
			* itemId = NULL,
			* objectType = NULL,
			* version = NULL,
			* productImpactChar = NULL,
			* customerImpactDellChar = NULL,
			* likeihoodChar = NULL;


		va_end(largs);

		//printf("--------------------PROJECT_ask_object_string_value--------------------\n");

		tag_t itemRev = NULLTAG;
		*value = NULL;

		vector<string> vec;

		string productImpact,
			customerImpactDell,
			likeihood;

		vector<string> vector1,
			vector2,
			vector3;

		string productImpactNew,
			customerImpactDellNew,
			likeihoodNew,
			RPNValue,
			sevValue;

		/* ====== get owning object ====== */
		METHOD_PROP_MESSAGE_OBJECT(msg, itemRev);


		ITKCALL(WSOM_ask_object_type2(itemRev, &objectType));
		TC_write_syslog("objectType == %s\n", objectType);

		ITKCALL(AOM_ask_value_string(itemRev, "item_id", &itemId));
		TC_write_syslog("itemId == %s\n", itemId);

		ITKCALL(AOM_ask_value_string(itemRev, "item_revision_id", &version));
		TC_write_syslog("version == %s\n", version);

		ITKCALL(AOM_ask_value_string(itemRev, "d9_IRProductImpact", &productImpactChar));
		TC_write_syslog("productImpactChar == %s\n", productImpactChar);

		ITKCALL(AOM_ask_value_string(itemRev, "d9_IRCustomerImpactDell", &customerImpactDellChar));
		TC_write_syslog("customerImpactDellChar == %s\n", customerImpactDellChar);

		ITKCALL(AOM_ask_value_string(itemRev, "d9_IRLikelihood", &likeihoodChar));
		TC_write_syslog("likeihoodChar == %s\n", likeihoodChar);

		if (containStr(productImpactChar, "(") == 0 && containStr(productImpactChar, ")") == 0) {
			goto CLEANUP;
		}

		if (containStr(customerImpactDellChar, "(") == 0 && containStr(customerImpactDellChar, ")") == 0) {
			goto CLEANUP;
		}

		if (containStr(likeihoodChar, "(") == 0 && containStr(likeihoodChar, ")") == 0) {
			goto CLEANUP;
		}


		productImpact = (string)productImpactChar;
		customerImpactDell = (string)customerImpactDellChar;
		likeihood = (string)likeihoodChar;

		string_replace(productImpact, ")", "");
		string_replace(customerImpactDell, ")", "");
		string_replace(likeihood, ")", "");

		TC_write_syslog("productImpact == %s\n", productImpact);
		TC_write_syslog("customerImpactDell == %s\n", customerImpactDell);
		TC_write_syslog("likeihood == %s\n", likeihood);

		vector1 = split(productImpact, "(");
		vector2 = split(customerImpactDell, "(");
		vector3 = split(likeihood, "(");

		productImpactNew = clearAllSpace(vector1[1]);
		TC_write_syslog("productImpactNew == %s\n", productImpactNew);
		if (productImpactNew.empty() || !isNumeric(productImpactNew.c_str())) { // 判断是否为空或者不是数字
			productImpactNew = "0";
		}

		customerImpactDellNew = vector2[1];
		TC_write_syslog("customerImpactDellNew == %s\n", customerImpactDellNew);
		if (customerImpactDellNew.empty() || !isNumeric(customerImpactDellNew.c_str())) { // 判断是否为空或者不是数字
			customerImpactDellNew = "0";
		}

		likeihoodNew = vector3[1];
		TC_write_syslog("likeihoodNew == %s\n", likeihoodNew);
		if (likeihoodNew.empty() || !isNumeric(likeihoodNew.c_str())) { // 判断是否为空或者不是数字
			likeihoodNew = "0";
		}

		RPNValue = multiply(multiply(productImpactNew, customerImpactDellNew), likeihoodNew); // 计算分数
		TC_write_syslog("RPNValue == %s\n", RPNValue);

		getPreferenceByName(D9_DELL_ISSUE_RPN_SEV, vec); // 获取首选项

		sevValue = getSevValue(vec, RPNValue);

		if (*value != NULL) MEM_free(*value);
		*value = (char*)MEM_alloc(strlen(sevValue.c_str()) + 1);

		strcpy(*value, sevValue.c_str());

		TC_write_syslog("\n================= D9_IRSeverityIRDELL End ==================\n");

	CLEANUP:
		DOFREE(objectType);
		DOFREE(itemId);
		DOFREE(version);
		DOFREE(productImpactChar);
		DOFREE(customerImpactDellChar);
		DOFREE(likeihoodChar);
		return ifail;
	}

#ifdef __cplusplus
}
#endif

/*
* 获取严重等级数值
*/
string getSevValue(vector<string> vec, string RPNValue) {
	for (int i = 0; i < vec.size(); i++) {
		string str = vec[i];
		vector<string> spl = split(str, "=");
		string RPNRange = spl[0];
		string sevValue = spl[1];

		if (checkRPNRange(RPNRange, RPNValue)) {
			return sevValue;
		}
	}

	return "";
}

/*
* 判断RPN值区间
*/
bool checkRPNRange(string RPNRange, string str) {
	vector<string> spl = split(RPNRange, ",");
	string str1 = spl[0];
	string str2 = spl[1];


	if (strcmp(str1.c_str(), ("[" + str).c_str()) == 0 || strcmp(str2.c_str(), (str + "]").c_str()) == 0) {
		return true;
	}

	string_replace(RPNRange, "[", "");
	string_replace(RPNRange, "]", "");
	string_replace(RPNRange, "(", "");
	string_replace(RPNRange, ")", "");

	spl = split(RPNRange, ",");
	str1 = spl[0];
	str2 = spl[1];

	int value = atoi(str.c_str());

	int min = 0;
	int max = 0;
	if (isNumeric(str1.c_str())) { // 判断是否为数字
		min = atoi(str1.c_str());
	}

	if (isNumeric(str2.c_str())) {
		max = atoi(str2.c_str());
	}

	if (max > 0) {
		if (value > min && value < max) {
			return true;
		}
	}
	else {
		if (value > min) {
			return true;
		}
	}

	return false;
}