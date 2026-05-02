#include"HdQAntennaMIMOPathElectricFieldFileOutput.h"

#include"QzQFileBase.h"
#include"QzQDirectoryOperate.h"

namespace AntennaMIMOPathElectricFieldFileOutputStd {



	void OutPutLog(
		const std::string& outPutDirectoryPathName,
		const std::string& fileName,
		const std::string& logContext) {
		FileOperateStd::CreateNewDirectorys(outPutDirectoryPathName);

		std::string log_file_name;
		log_file_name.append(outPutDirectoryPathName);
		log_file_name.append("\\");
		log_file_name.append(fileName);
		std::string context;
		context.append(logContext);

		FileOperateStd::WriteStringToTxtFile(log_file_name.c_str(), context);
	}
}
