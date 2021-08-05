#ifndef DB_MANAGER_H
#define DB_MANAGER_H
#include <postgresql/libpq-fe.h>
#include <string>

typedef std::pair<std::string, std::string> full_file_id;

struct file_struct
{
    std::string path;
    std::string name;
};

class DB_manager
{
public:
    DB_manager(const std::string &DB_adr = "127.0.0.1", int port = 5432 );
    file_struct getFileByUrl(const std::string &url);
    int registerFile(const std::string &group_id, const file_struct &file);
private:
    file_struct getFile(const full_file_id &file_id);
    PGconn * conn;
    int getMaxFileId(const std::string &group_id);
    int addFileRecord(const std::string &group_id, const std::string &file_id, const file_struct &file);
};

#endif // DB_MANAGER_H
