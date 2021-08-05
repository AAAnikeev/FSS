#ifndef APIFSS_H
#define APIFSS_H
#include <cstdlib>
#include <boost/regex.hpp>

namespace FSS{
    std::string spr = "\r\n";//separator
    std::string endr = "\r\n\r\n";//endRequest/Response
}

//                                    REQUESTS:  APPE/GET\r\n
//                                                  APPE:
//                                                    FileName: name.txt\r\nFileSize: size\r\n\r\n
//                                                  GET:
//                                                    FileUrl: name.txt\r\n\r\n
//                                    RESPONSE:     OK/ER\r\n
//                                                            CODE_ERR\r\n\r\n
//                                                  APPE:
//                                                    FileName: name.txt\r\nFileSize: size\r\n\r\n
//                                                  GET:
//                                                    FileUrl: name.txt\r\n\r\n

struct FSSError{
    int code = 0;
    std::string mes = "";
};

inline std::string formGetRequest(std::string fileurl)
{
    return "GET" + FSS::spr + "FileUrl: " + fileurl + FSS::endr;
}

inline std::string formAddRequest(std::string group_id ,std::string fileName, size_t fileSize)
{
    return "APPE" + FSS::spr + "GroupId: " + group_id + FSS::spr + "FileName: " + fileName + FSS::spr +
            "FileSize: " + std::to_string(fileSize) + FSS::endr;
}

inline std::string parseByFileName(const std::string &response, FSSError *err)
{
    boost::regex regFileName("FileName: *(.+?)" + FSS::spr);
    boost::smatch mr;
    if (boost::regex_search(response, mr, regFileName))
    {
        err->code = 0;
        return mr[1];
    }
    else
    {
        err->code = 1;
        err->mes = "parseByFileName: cann't get file name from request";
        return std::string();
    }
}

inline bool resultIsSuccesful(std::string response, FSSError *err)
{
    if (response.substr(0,2) == "OK"){
        err->code = 0;
        return true;
    } else if(response.substr(0, 2) == "ER") {
        boost::regex regErrorCode("CodeError: *(.+?)" + FSS::spr);
        boost::smatch mr;
        if (boost::regex_search(response, mr, regErrorCode))
        {
            err->code = std::stoi(mr[1]);
            err->mes = std::string();//TODOA add error message
            return false;
        }
    }
    err->code = -1;
    err->mes = "getResult: cann't get error or error code";
    return false;
}

inline size_t parseByFileSize(const std::string &response, FSSError *err)
{
    boost::regex regFileSize("FileSize: *(.+?)" + FSS::spr);
    boost::smatch mr;
    if (boost::regex_search(response, mr, regFileSize))
    {
        err->code = 0;
        return std::stoull(mr[1]);
    }
    err->code = -1;
    err->mes = "parseByFileSize: cann't get file size from request";
    return size_t();
}

inline std::string parseByGroupId(const std::string &response, FSSError *err)
{
    boost::regex regGroupId("GroupId: *(.+?)" + FSS::spr);
    boost::smatch mr;
    if (boost::regex_search(response, mr, regGroupId))
    {
        err->code = 0;
        return mr[1];
    }
    else
    {
        err->code = -1;
        err->mes = "parseByGroupId: cann't get groupId";
        return std::string();
    }
}

inline std::string parseByFileUrl(const std::string & response, FSSError *err)
{
    boost::regex regFileName("FileUrl: *(.+?)" + FSS::spr);
    boost::smatch mr;
    if (boost::regex_search(response, mr, regFileName))
    {
        err->code = 0;
        return mr[1];
    }
    else
    {
        err->code = -1;
        err->mes = "parseByFileUrl: cann't get file url from request";
        return std::string();
    }
}


inline bool parseByFilenameAndFileSize(const std::string response,std::string *fileName, size_t *fileSize)
{
    FSSError errFN, errFS;
    *fileName = parseByFileName(response, &errFN);
    *fileSize  = parseByFileSize(response, &errFS);
    if (errFN.code == 0 && errFS.code == 0)
        return true;
    else
        return false;
}

#endif // APIFSS_H
