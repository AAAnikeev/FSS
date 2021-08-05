#include "db_manager.h"
#include <cstdlib>
#include <iostream>
#include <stdlib.h>

static const std::string main_table = "href_file";
static const std::string group_id_column_name = "group_id";
static const std::string file_id_column_name = "file_id";
static const std::string file_name_column_name = "file_name";
static const std::string file_path_column_name = "file_path";

full_file_id parseUrl(const std::string &url);

//TODO isolate DB_manager from PostgresDB, do it like interface

DB_manager::DB_manager(const std::string & DB_adr, int port)
{
    //TODO get this information some other way
    conn = PQsetdbLogin(DB_adr.c_str(), std::to_string(port).c_str(), "", "", "file_share", "postgres", "11111111");
    if ( PQstatus(conn) != CONNECTION_OK)
    {
        std::cerr<<"Connecttion to database failed: "<<PQerrorMessage(conn)<<std::endl;
        PQfinish(conn);
        throw std::runtime_error{"cannot connect to database"};
    }
}

file_struct DB_manager::getFileByUrl(const std::string & url){
    return  getFile(parseUrl(url));
}

full_file_id parseUrl(const std::string &url)
{
    auto pos_delimeter = url.find(";");//TODO replace this separator smth else
    if (pos_delimeter != url.npos)
    {
        std::string group_id = url.substr(0, pos_delimeter);
        std::string file_id = url.substr(pos_delimeter +1);
        return std::make_pair(group_id, file_id);
    }
    else{
        throw std::runtime_error{"parseUrl: wrong url format"};
    }
}



file_struct DB_manager::getFile(const full_file_id &file_id){
    PGresult   *res;
    file_struct ret_file;
    std::string request = "SELECT " + file_path_column_name + ", " + file_name_column_name
            + " FROM " + main_table +
            " WHERE " + group_id_column_name + "=\'" + file_id.first + "\' AND " + file_id_column_name + "=" + file_id.second + ";";
    res = PQexec(conn, request.c_str());
    if (PQresultStatus(res) != PGRES_TUPLES_OK)
    {
        PQclear(res);
        throw std::runtime_error{"getFilePath: cannot find file. Error"};
    }
    if (PQnfields(res) != 2 || PQntuples(res) != 1)
    {
        PQclear(res);
        throw std::runtime_error{"getFilePath: error in database structure"};
    }else{
        ret_file.path = PQgetvalue(res, 0, 0);
        ret_file.name = PQgetvalue(res, 0, 1);
    }
    PQclear(res);
    return ret_file;
}

int DB_manager::registerFile(const std::string &group_id, const file_struct &file)
{
    int new_file_id;
    try{
        new_file_id = getMaxFileId(group_id);
        new_file_id++;
        addFileRecord(group_id, std::to_string(new_file_id), file);
    }
    catch (...)
    {
        return -1;
    }
    return new_file_id;
}

int DB_manager::getMaxFileId(const std::string &group_id)
{
    PGresult   *res;
    int max_file_id;
    std::string request = "SELECT MAX(" + file_id_column_name +
            ") FROM " + main_table +
            " WHERE " + group_id_column_name + "=\'" + group_id + "\';";
    res = PQexec(conn, request.c_str());
    if (PQresultStatus(res) != PGRES_TUPLES_OK)
    {
        PQclear(res);
        throw std::runtime_error{"getMaxFileId: cannot find file. Error"};
    }
    if (PQnfields(res) != 1 || PQntuples(res) != 1)
    {
        PQclear(res);
        throw std::runtime_error{"getFilePath: error in database structure"};
    }else{
        max_file_id= atoi(PQgetvalue(res, 0, 0));
    }
    PQclear(res);
    return max_file_id;
}

int DB_manager::addFileRecord(const std::string &group_id, const std::string &file_id, const file_struct & file)
{
    PGresult   *res;
    std::string request = "INSERT INTO " + main_table +
            " (" + group_id_column_name + ", " + file_id_column_name + "," + file_name_column_name + ", " + file_path_column_name + ") " +
            " VALUES (\'" + group_id + "\', " + file_id + ", \'" + file.name + "\', \'" + file.path + "\');";
    res = PQexec(conn, request.c_str());
    if (!(PQresultStatus(res) == PGRES_TUPLES_OK || PQresultStatus(res) == PGRES_COMMAND_OK))
    {
        throw std::runtime_error{"addFileRecord: error when adding file into base, error:" + std::string(PQresultErrorMessage(res))};
    }
    return 1;
}
