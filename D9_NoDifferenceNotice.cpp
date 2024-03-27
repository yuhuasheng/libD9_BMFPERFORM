#include "extension.h"
#include "util.h"
#include <curl/curl.h>

#ifdef __cplusplus
extern "C" {
#endif


	//void sendGetHttpReq(const char* url) {
	//	CURL* curl;
	//	CURLcode res;
	//	curl = curl_easy_init();
	//	TC_write_syslog("http curl :: %d\n", curl);
	//	if (curl)
	//	{
	//		curl_easy_setopt(curl, CURLOPT_URL, url);
	//		curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10);
	//		curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10);
	//		res = curl_easy_perform(curl);
	//		curl_easy_cleanup(curl);
	//	}
	//	TC_write_syslog("http get status :: %d\n", res);
	//}


	int D9_NoDifferenceNotice(EPM_action_message_t msg) {
		char* taskUid = NULL;
		ITK__convert_tag_to_uid(msg.task, &taskUid);
		string url = "";
		string response = "";
		vector<string>* urlVec = new vector<string>();
		getPreferenceByName("D9_SpringCloud_URL", *urlVec);
		if ((*urlVec).size() > 0 && taskUid != NULL) {
			url = (*urlVec)[0] + "/tc-service/mntebom/noDifferenceNotice?taskUid=" + taskUid;
			httpGet(url, response, 60);
			TC_write_syslog("D9_NoDifferenceNotice http url == %s   response== %s \n", url.c_str(), response);
		}
		else {
			TC_write_syslog("D9_NoDifferenceNotice D9_SpringCloud_URL is not exsit !! , task uid == ", taskUid);
		}
		return ITK_ok;
	}

#ifdef __cplusplus
}
#endif