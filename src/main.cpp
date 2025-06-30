
#include <evpp/http/http_server.h>
#include <evpp/event_loop.h>
void DefaultHandler(evpp::EventLoop* loop,
                    const evpp::http::ContextPtr& ctx,
                    const evpp::http::HTTPSendResponseCallback& cb) {
    std::stringstream oss;
    // oss << "func=" << __FUNCTION__ << " OK"
    //     << " ip=" << ctx->remote_ip() << "\n"
    //     << " uri=" << ctx->uri() << "\n";
    // << " body=" << ctx->body().ToString() << "\n";
    // ctx->AddResponseHeader("Content-Type", "application/octet-stream");
    // ctx->AddResponseHeader("Server", "evpp");
    int fd;
    if(ctx->uri()=="/"){
        fd = ::open("../html/file_management.html", O_RDONLY);
        ctx->AddResponseHeader("Content-Type", "text/html; charset=utf-8");
    }
    else if(ctx->uri()=="/file_management.css"){
        fd=::open("../html/file_management.css", O_RDONLY);
        ctx->AddResponseHeader("Content-Type", "text/css; charset=utf-8");
    }
    else if(ctx->uri()=="/file_management.js"){
        fd = ::open("../html/file_management.js", O_RDONLY);
        ctx->AddResponseHeader("Content-Type", "application/javascript; charset=utf-8");
    }
    else if(ctx->uri()=="/favicon.ico"){
        fd = ::open("../html/kicat.jpg", O_RDONLY);
        ctx->AddResponseHeader("Content-Type", "image/jpeg");
    }
    if(fd<0){
        LOG_ERROR << "open file error";
        cb("html/css/js/favicon.ico file not found");
        return;
    }
    std::string data;

    while (1)
    {
        char buf[8096];
        bzero(buf,sizeof(buf));
        ssize_t ret = ::read(fd, buf, sizeof(buf));
        if(ret<0){
            LOG_ERROR << "read file error";
            cb("html/css/js/favicon.ico file read error");
            return;
        }
        if(ret==0){
            ::close(fd);
            break;
        }
        else
            data.append(buf, ret);
    }
    // oss<< " body=" <<data << "\n";

    cb(data);
}

void FileListHandler(evpp::EventLoop* loop,
                  const evpp::http::ContextPtr& ctx,
                  const evpp::http::HTTPSendResponseCallback& cb){
    ctx->AddResponseHeader("Content-Type", "application/json; charset=utf-8");
    std::string body = "{\"files\":[{\"name\":\"a.txt\",\"location\":\"haung-ubuntu\",\"path\":\"../file/a.txt\",\"type\":\"txt\",\"comment\":\"test\"},{\"name\":\"b.txt\",\"location\":\"haung-ubuntu\",\"path\":\"../file/b.txt\",\"type\":\"txt\",\"comment\":\"test\"}]}";
    cb(body);
}

void DeleteHandler(evpp::EventLoop* loop,
                  const evpp::http::ContextPtr& ctx,
                  const evpp::http::HTTPSendResponseCallback& cb){
    std::string path=ctx->FindQueryFromURI(ctx->original_uri(),"path");
    int i = 0;
    while ((i = path.find("%2F", i)) != std::string::npos)
    {
        path.replace(i, 3, "/");
        ++i;
    }
    int ret=::remove(path.c_str());
    if(ret){
        ctx->AddResponseHeader("Content-Type", "application/json; charset=utf-8");
        ctx->set_response_http_code(500);
        std::string body = "{\"status\": \"error\", \"message\": \"No such file or directory\"}";
        cb(body);
    }
    else{
        // ctx->set_response_http_code(200);
        // ctx->AddResponseHeader("Content-Type", "application/json; charset=utf-8");
        cb("OK");
        // std::string body = "{\"status\": \"success\"}";
        // cb(body);
    }
}

void DownloadHandler(evpp::EventLoop* loop,
                  const evpp::http::ContextPtr& ctx,
                  const evpp::http::HTTPSendResponseCallback& cb){
    std::string path = ctx->FindQueryFromURI(ctx->original_uri(), "path");
    int i = 0;
    while ((i = path.find("%2F", i)) != std::string::npos)
    {
        path.replace(i, 3, "/");
        ++i;
    }
    int fd=::open(path.c_str(), O_RDONLY);
    if(fd<0){
        LOG_ERROR << "open file error";
        ctx->set_response_http_code(404);
        cb(" file not found");
        return;
    }
    std::string file_name(path.begin() + path.rfind("/", path.size() - 1) + 1, path.end());
    std::string data;
    while (1)
    {
        char buf[8096];
        bzero(buf, sizeof(buf));
        ssize_t ret = ::read(fd, buf, sizeof(buf));
        if (ret < 0)
        {
            LOG_ERROR << "read file error";
            cb("html/css/js/favicon.ico file read error");
            return;
        }
        if (ret == 0)
        {
            ::close(fd);
            break;
        }
        else
            data.append(buf, ret);
    }
    ctx->AddResponseHeader("Content-Type", "application/octet-stream");
    ctx->AddResponseHeader("Content-Disposition", "attachment; filename=" + file_name);
    cb(data);
}
int main(int argc, char *argv[])
{
    std::vector<int> ports = { 12345, 23456, 23457 };
    int port = 29099;
    int thread_num = 2;

    if (argc > 1) {
        if (std::string("-h") == argv[1] ||
            std::string("--h") == argv[1] ||
            std::string("-help") == argv[1] ||
            std::string("--help") == argv[1]) {
            std::cout << "usage : " << argv[0] << " <listen_port> <thread_num>\n";
            std::cout << " e.g. : " << argv[0] << " 8080 24\n";
            return 0;
        }
    }

    if (argc == 2) {
        port = atoi(argv[1]);
    } else if (argc == 3) {
        port = atoi(argv[1]);
        thread_num = atoi(argv[2]);
    }

    ports.push_back(port);

    evpp::http::Server server(thread_num);
    server.SetThreadDispatchPolicy(evpp::ThreadDispatchPolicy::kIPAddressHashing);
    server.RegisterDefaultHandler(&DefaultHandler);
    // server.RegisterHandler("/", &HostHandler);
    server.RegisterHandler("/api/files", &FileListHandler);
    server.RegisterHandler("/api/files/delete", &DeleteHandler);
    server.RegisterHandler("/api/files/download", &DownloadHandler);
    server.Init(ports);
    server.Start();
    while (!server.IsStopped()) {
        usleep(1);
    }
    return 0;
}
