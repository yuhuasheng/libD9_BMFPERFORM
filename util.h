#ifndef UTIL
#define UTIL

#pragma comment(lib, "oraocci19.lib")
#pragma comment(lib, "oraocci19d.lib")

//#include <tc/iman.h>                -----------20221222
#include <tc/emh.h>
#include <pom/pom/pom_errors.h>
//#include <tc/tc.h>                -----------20221222
#include <pom/pom/pom.h>

#include <stdio.h>
#include <string.h>
#include <string>
#include <fclasses/tc_date.h>
#include <time.h>
#include <base_utils/Mem.h>
#include <tc/preferences.h>
#include <vector>
#include <tc/tc_macros.h> //ITKCALL 的头文件
#include <tccore/aom_prop.h>
#include <tccore/grm.h>
//#include <tccore/iman_grmtype.h>                -----------20221222
//#include "ADOSql.h"

#include <io.h>
//#include <direct.h>
#include <tccore/aom.h>
//#include "common_itk_util.h"

#include <vector>
#include <map>
//#include <itk/mem.h>                -----------20221222
#include <stdio.h>
#include <stdlib.h>
#include <map>
#include <string>
#include <iostream>

//#include <tc/tc.h>                 -----------20221222
#include <epm/cr.h>
#include <epm/signoff.h>
#include <epm/epm.h>
#include <tccore/workspaceobject.h>
#include <tccore/aom.h>
#include <tccore/aom_prop.h>
#include <tccore/grm.h>
#include <ae/dataset.h>
#include <ae/datasettype.h>
#include <fclasses/tc_date.h>
#include <user_exits/epm_toolkit_utils.h>
#include <epm/epm.h>
#include <tc/tc_arguments.h>
#include <ict/ict_userservice.h>
#include <io.h>
#include <direct.h>
#include <tc/emh.h>
#include <tccore/item.h>
#include <bom/bom.h>
#include <sa/am.h>
#include <sa/person.h>
#include <ug_va_copy.h>
#include <epm/epm.h>
#include <epm/epm_task_template_itk.h>
#include <schmgt/schmgt_bridge_itk.h>
#include <ae/dataset_msg.h>
#include <cfm/cfm.h>
#include <epm/epm_toolkit_tc_utils.h>
#include <regex>
#include <ics/ics.h>
#include <ics/ics2.h>
#include <lov/lov.h>
#include <res\res_itk.h>
#include <property/prop.h>

#include <tc/envelope.h>

#include <ae/dataset.h>
#include <nls/nls.h>
#include <time.h>
#include <regex>
#include <occi.h>
#include <errno.h>
#include <fstream>
#include <dispatcher/dispatcher_itk.h>
#include <curl/curl.h>
#include <cassert>
//#include <sql.h>

#define DOFREE(obj)								\
{												\
	if(obj != NULL)										\
	{											\
		MEM_free(obj);							\
		obj = NULL;								\
	}											\
}


#define TCFREE(obj)								\
	{												\
	if(obj)										\
	{											\
	MEM_free(obj);							\
	obj = NULL;								\
	}											\
}

extern "C" int POM_AM__set_application_bypass(logical bypass);
using namespace std;
using namespace oracle::occi;
class Messagebox {
public:
	static const int infomation = EMH_severity_information;
	static const int warning = EMH_severity_warning;
	static const int error = EMH_severity_error;
	static const int userError = EMH_severity_user_error;
	static int post(const char* infomation, int TYPE)
	{

		EMH_store_error_s1(TYPE, EMH_USER_error_base, infomation);
		return EMH_USER_error_base;
	}
};

#define _CRT_SECURE_NO_WARNINGS
#define BUFFER_SIZE 256
#define MAX_PATH_LENGTH 2000
#define MAX_PRINTLINE_LENGTH 2000
#define BUFSIZE				512
#define ERRORCODE_MESSAGE (EMH_USER_error_base + 110)

bool regex_matchstr(string search_string, std::tr1::regex reg);
vector<string> getReviseBeforeItemRevList(tag_t *rev_list, int count);
void current_time(date_t * date_tag);
void formatDate(char* drawingDate, const char* formdate, date_t date);
bool getPreferenceByName(string prefername, vector<string> &preference_vec);
int getHashMapPreference(vector<string> preference_vec, map<string, string> &mp);
int get_RevMasterForm_from_rev(tag_t itemRev, tag_t* itemRevForm);
vector<string> split(string str, string pattern);
int messageBox(string infomation);
//bool getPreferenceInfo(vector<string> &preference_vec,pre_infosql &preference_info);
string get_curret_time(char* formdate);
bool change_state_from_revMasterForm(map<int, string> &targets, string Attribute);
void substr(char *a, int m, int n, char *b);
int query(char* qry_name, char** attr_names, char** values, int attr_cnt, tag_t **results, int* results_cnt);
int query_sort(char* qry_name, char** attr_names, char** values, int attr_cnt, tag_t **results, int* results_cnt);
int export_dataset_file(tag_t dataset, char *ref_name, char *ext, char **filename, char **original_name, char *dirPath);
int execmd(char* cmd, char* result);
int import_dataset_file(tag_t dataset, char* ref_name, char* ext, char* fullfilename, char* original_name);
int create_dataset(char* ds_type, char* ref_name, char* ds_name, char* fullfilename, char* relation_name, tag_t parent_rev, tag_t* dataset);
int FindTargetTool(tag_t ds_type, const char *ref_name, tag_t *target_tool);
void file_copy(const char *srcFile, const char *desFile);
void str_replace(char* str1, char* str2, char* str3);
void setRelated(tag_t parent_tag, tag_t child_tag, const char * relateTypeName);
string read_file(char* pathname);
bool isNumeric(const  char * str);
bool isDigNumeric(const char * str);
void WriteLog(FILE * logFile, const char* format, ...);
void CreateLogFile(char* FunctionName, FILE** logFile);
char* U2G(const char* utf8);
char* G2U(const char* gb2312);
char* G2B5(char* gb2312);
string getTemplateId(vector<string> vec, char* templateName);
logical checkItemType(vector<string>  tagTypeVec, tag_t  rev_tag);
char* getProjectInfo(tag_t item_tag);
bool getProjectInfo(tag_t item_tag, vector<string> &preference_vec);
int getProjectInfo(tag_t item_tag, vector<tag_t>& vec);
int GTCTYPE_is_type_of(tag_t object_tag, const char *typeName, logical *isType);
int checkTagAttr(vector<string>  tagAttrVec, tag_t  rev_tag, char **error_msg);
int checkStrAttr(vector<string>  tagAttrVec, tag_t  rev_tag, char **error_msg);
boolean isExistItem(tag_t folder_tag, tag_t item_tag);
string getMaterialGroup(char *groupName, vector<string> preference_vec);
int startWith(char *str, char * p);
int endWith(char *str, char * p);
int containStr(char* str, char* p);
int strReplace(char strRes[], char from[], char to[]);
void string_replace(string& s1, const string& s2, const string& s3);
logical checkStructure(tag_t itemRev);
logical hasChildBOMLine(tag_t tItemRev);
static inline bool is_base64(unsigned char c);
std::string base64_encode(char const* bytes_to_encode, int in_len); // base64 编码
std::string base64_decode(std::string & encoded_string); // base64 解码
logical checkProName(tag_t primaryTag, char* proName);
void createFolder(std::string dir);
void deleteFolder(std::string dir);
std::string getTempFolder(std::string dir);
bool createFile(std::string fileName);
void appendContent(const char* fileName, vector<string> content);
bool is_special_dir(const char* path);
bool is_dir(int attrib);
void show_error(const char* file_name);
void get_file_path(const char* path, const char* file_name, char* file_path);
void delete_file(const char* path);
void removeDir(const char* dirPath);
void getFiles(string path, vector<string>& files);
boolean rename(string oldName, string newName);
void getConnection(string prefername, Environment* &env, Connection* &conn);
int validateProp(tag_t object_tag, const char* propertyName, logical* verdict);
bool isBom(tag_t object_tag);
size_t ReceiveData(void* contents, size_t size, size_t nmemb, void* stream);
CURLcode httpPost(const std::string& url, const std::string& data, std::string& response, int timeout);
CURLcode httpPostFormData(const std::string& url, const std::string& data, std::string& response, int timeout);
CURLcode httpGet(const std::string& url, std::string& response, int timeout);
bool isRelease(tag_t object_tag);
bool checkSignRecord(tag_t object_tag);
int getHandlerArgs(TC_argument_list_t* arguments, map<string, string>& mp);
unsigned char ToHex(unsigned char x);
unsigned char FromHex(unsigned char x);
std::string UrlEncode(const std::string& str);
std::string UrlDecode(const std::string& str);
boolean checkExistItem(tag_t folder_tag, tag_t item_tag, char* propName);
int updateProp(tag_t item, const char* propName, const char* propValue);
string& clearHeadTailSpace(string& str);
string& clearAllSpace(string& str);
string multiply(string num1, string num2);
string getTcCode(char* utf8);
string getExternCode(char* str);
string getPropValue(tag_t obj, char* propName);
#endif