#include <cstdlib>
#include <iostream>
#include <fstream>
#include <mutex>

#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/regex.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/filesystem.hpp>

#include <experimental/filesystem>

#include "include/apifss.h"
#include "db_manager.h"
#include <syslog.h>
#include <cstring>

using boost::asio::ip::tcp;

boost::asio::io_service service;

typedef boost::shared_ptr<boost::asio::ip::tcp::socket> socket_ptr;

std::string getFileSaveDirectory();

DB_manager db_manager;
std::mutex db_m, generator_m;

//We can use transactions in DB, but now we have only one select and insert request,
file_struct getFileByUrl(std::string fileUrl)
{
    std::lock_guard<std::mutex> lock(db_m);
    return db_manager.getFileByUrl(fileUrl);
}

int registerFileIntoDB(std::string group_id, file_struct file){
    std::lock_guard<std::mutex> lock(db_m);
    return db_manager.registerFile(group_id, file);
}

std::string generateRandomString()//TODO: может быть дважды сгенерирован один и тот же код, это может вызвать проблемы, когда много файлов с одним именем будет записано
{
    std::lock_guard<std::mutex> lock(generator_m);
    static boost::uuids::random_generator generator;
    boost::uuids::uuid uuid = generator();
    return boost::uuids::to_string(uuid);
}

int downloadToFileFromSocket(const std::string &pathToFile, socket_ptr socket, const std::size_t &fileSize,
                              boost::asio::streambuf& receive_buffer );
void writeFileToSocket(socket_ptr socket , std::string pathToFile);
void client_session(socket_ptr sock);
std::string generateErrorStr(int error_code);
bool checkFolder(std::string path);


int main(int argc, char* argv[])
{
    int port = 1234;
    if (argc == 3 && strcmp(argv[1], "-p") == 0 && atoi(argv[2]) != 0){
        port = atoi(argv[2]);
    } else if (argc != 1){
        std::cerr<<"Wrong parametrs. You can run file share service in two ways:\n"<<
                   "1) Without any arguments and it will listening on port 1234 by default\n"<<
                   "2) With arguments -p PORT_NUM"<<std::endl;
        return 1;
    }
    if (!checkFolder(getFileSaveDirectory())){
        std::cerr<<"We don't have permissions to save files into:" + getFileSaveDirectory()<<std::endl;
        return 1;
    }
    boost::asio::ip::tcp::endpoint ep( boost::asio::ip::tcp::v4(), port); // listen on 1234
    std::cout << "File sharing service starting listening on port:"<<port<<std::endl;
    boost::asio::ip::tcp::acceptor acc(service, ep);
    while ( true)
    {
        socket_ptr sock(new boost::asio::ip::tcp::socket(service));
        acc.accept(*sock);
        boost::thread( boost::bind(client_session, sock));
    }
    return 0;
}

std::string getFileSaveDirectory(){
    if (strcmp(std::getenv("USER"), "root") == 0)
        return "/home/fss/FSS";//That folder was created when install our package
    std::string fsspath = std::string(std::getenv("HOME")) + "/FSS";
    boost::system::error_code ec;
    if (!boost::filesystem::create_directory(fsspath, ec) && ec != 0){
        std::cerr<<"Error in access directory:"<<fsspath<<" Error:"<<ec<<std::endl;
    }
    return fsspath;
}

bool checkFolder(std::string path){
    std::ofstream out;
    path += "/test.XXXXXX";
    char filename[path.size()];
    strcpy(filename, path.c_str());
    int fd = mkstemp(filename);    // Creates and opens a new temp file r/w.
                                   // Xs are replaced with a unique number.
    if (fd == -1) return false;        // Check we managed to open the file.
    close(fd);
    unlink(filename);              // Delete the temporary file.
    return true;
}


void client_session(socket_ptr sock)
 {
    boost::asio::streambuf sBuf;
    std::istream istr{ &sBuf};
    size_t sizeBuf = boost::asio::read_until(*sock, sBuf,  FSS::endr);
    const char* dataTmp = boost::asio::buffer_cast<const char*>(sBuf.data());
    std::string headers(dataTmp);
    sBuf.consume(sizeBuf);
    std::string requestType = headers.substr(0, headers.find_first_of(FSS::spr));

    if (requestType == "APPE")
    {
        std::string group_id;
        std::string filename;
        size_t fileSize;

        if (!parseByFilenameAndFileSize(headers, &filename, &fileSize))
        {
            sock->close();
            return;
        }
        FSSError errGr, errFN, errFS;
        group_id = parseByGroupId(headers, &errGr);
        filename = parseByFileName(headers, &errFN);
        fileSize = parseByFileSize(headers, &errFS);
        if (errGr.code != 0 || errFN.code != 0 || errFS.code != 0 ){
            sock->close();
            return;
        }

        std::string path = getFileSaveDirectory() + generateRandomString() + "_" + filename;
        file_struct file{filename, path};

        if (downloadToFileFromSocket(path, sock, fileSize, sBuf)){
            int file_id = registerFileIntoDB(group_id, file);
            std::string responseStr;
            if (file_id != -1)
            {
                responseStr = "OK" + FSS::spr + std::string("FileUrl: " +group_id + ";" + std::to_string(file_id) + FSS::endr);
            }
            else
            {
                responseStr = generateErrorStr(1);
            }
            boost::asio::write(*sock, boost::asio::buffer(responseStr) );
        }
    }
    else if (requestType == "GET")
    {
        boost::regex regFileUrl("FileUrl: *(.+?)" + FSS::endr);
        boost::smatch mr;
        if (!boost::regex_search(headers, mr, regFileUrl))
        {
            std::cout<<"Wrong format GET request"<<std::endl;
            sock->close();
            return;
        }
        std::string fileUrl = mr[1];
        try{
            file_struct file = getFileByUrl(fileUrl);
            unsigned long long fileSize = std::experimental::filesystem::file_size(file.path);

            std::string responseStr = "OK" + FSS::spr + std::string("FileName: " + file.name + FSS::spr +
                                                                          "FileSize:" + std::to_string(fileSize) + FSS::endr);

            boost::asio::write(*sock, boost::asio::buffer(responseStr) );
            writeFileToSocket(sock, file.path);
        }
        catch (...)
        {
            boost::asio::write(*sock, boost::asio::buffer(generateErrorStr(2)) );
        }

    } else {
        boost::asio::write(*sock, boost::asio::buffer(generateErrorStr(-1)) );
    }

    sock->close();
}

std::string generateErrorStr(int error_code)
{
    std::string responseStr = std::string("ER" + FSS::spr + "CodeError: ") + std::to_string(error_code) + FSS::endr;
    return responseStr;
}

int downloadToFileFromSocket(const std::string &pathToFile, socket_ptr socket, const std::size_t &fileSize,
                              boost::asio::streambuf& receive_buffer )
{
    std::ofstream out;
    out.open(pathToFile);
    if (!out.is_open())
    {
        throw std::runtime_error{"int downloadToFileFromSocket(): cann't open file:" + pathToFile};
    }

    size_t get_early = receive_buffer.size();
    if (get_early != 0)
    {
        out<< &receive_buffer;
    }
    std::size_t getting_bytes = get_early;

    while (getting_bytes != fileSize)
    {
        boost::system::error_code ec;
        size_t recieve_bytes = boost::asio::read(*socket, receive_buffer, boost::asio::transfer_at_least(1), ec);
        if (ec && ec != boost::asio::error::eof)
        {
            std::cout<<"error"<<std::endl;
        }
        out<< &receive_buffer;
        getting_bytes += recieve_bytes;

        if (ec == boost::asio::error::eof)
        {
            break;
        }
    }

    return getting_bytes == fileSize;
}

void writeFileToSocket(socket_ptr socket , std::string pathToFile)
{
    size_t fileSize = std::experimental::filesystem::file_size(pathToFile);
    std::ifstream ifs;
    ifs.open(pathToFile, std::ifstream::in);
    if (!ifs.is_open())
    {
        throw std::runtime_error{"void writeFileToSocket: cann't open file:" + pathToFile};
    }
    size_t buffer_size = 1 <<10;
    char *buffer = new char[buffer_size];

    size_t total_transfered_bytes = 0;

    while (ifs)
    {
        ifs.read(buffer, buffer_size);
        size_t count = ifs.gcount();

        if (!count)
            break;
        total_transfered_bytes += boost::asio::write(*socket, boost::asio::buffer(buffer, count));
    }
    assert(total_transfered_bytes == fileSize);
    delete buffer;
}
