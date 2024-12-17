#include <iostream>
#include <thread>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include <string>
#include <mutex>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <cstring>
#include "log.h"

using namespace std;
#define BUFFER_SIZE 16384

// mutex client_mutex; // bảo vệ map clients khỏi các truy cập đồng thời từ nhiều thread
// map<int, string> clients; // Map lưu socket và username
map<string, string> accounts;

// Hàm ghi log
// void logMessage(const string &message)
// {
//     ofstream logFile("log.txt", ios::app); // Mở file với chế độ append
//     if (logFile.is_open())
//     {
//         // Ghi thời gian vào log
//         time_t now = time(0);
//         char *dt = ctime(&now);
//         dt[strlen(dt) - 1] = '\0';
//         logFile << "[" << dt << "] " << message << endl;
//         logFile.close();
//     }
//     else
//     {
//         cerr << "Unable to open log file" << endl;
//     }
// }

// Tải thông tin tài khoản từ file account.txt
void loadAccounts(const string &filename)
{
    ifstream file(filename);
    if (!file.is_open())
    {
        cerr << "Not open file account.txt\n";
        exit(EXIT_FAILURE);
    }
    string line, username, password;
    while (getline(file, line))
    {
        stringstream ss(line);
        ss >> username >> password;
        accounts[username] = password;
    }
    file.close();
}

// Xử lý client
void handleClient(int client_socket)
{
    char buffer[BUFFER_SIZE];
    string username;

    // Đăng nhập hoặc đăng ký
    while (true)
    {
        memset(buffer, 0, BUFFER_SIZE);
        recv(client_socket, buffer, BUFFER_SIZE, 0);
        string command(buffer);
        // Ghi log message client gửi
        logMessage("Received from client: " + command);
        stringstream ss(command);
        string action, user, pass;
        ss >> action >> user >> pass;

        // đăng nhập
        if (action == "login")
        {
            if (accounts.count(user) && accounts[user] == pass)
            {
                send(client_socket, "loginSuccess", strlen("loginSuccess"), 0);
                // Ghi log server gửi
                logMessage("Sent to client: loginSuccess");
                break;
            }
            else
            {
                send(client_socket, "loginFail", strlen("loginFail"), 0);
                // Ghi log server gửi
                logMessage("Sent to client: loginFail");
            }
        }

        // đăng ký
        else if (action == "register")
        {
            if (!accounts.count(user))
            {
                accounts[user] = pass;
                ofstream file("users.txt", ios::app);
                file << user << " " << pass << endl;
                file.close();
                send(client_socket, "registerSuccess", strlen("registerSuccess"), 0);
                // Ghi log server gửi
                logMessage("Sent to client: registerSuccess");
            }
            else
            {
                send(client_socket, "exist", 5, 0);
                // Ghi log server gửi
                logMessage("Sent to client: exist");
            }
        }
        else if (action == "exit")
        {
            send(client_socket, "goodbye", 7, 0);
            // Ghi log server gửi
            logMessage("Sent to client: goodbye");
            break;
        }
    }

    // Thêm client vào danh sách
    {
        // tạo mutex để ngăn nhiều client thao tác đồng thời vào map clients
        // lock_guard<mutex> lock(client_mutex);
        // clients[client_socket] = username;
    }

    // nhận tin nhắn từ client
    // while (true)
    // {
    //     memset(buffer, 0, BUFFER_SIZE);
    //     int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
    //     if (bytes_received <= 0)
    //     {
    //         // tạo mutex để ngăn nhiều client xóa phần tử của map đồng thời
    //         lock_guard<mutex> lock(client_mutex);
    //         clients.erase(client_socket);
    //         close(client_socket);
    //         break;
    //     }

    //     string message(buffer);

    //     // kiểm tra cú pháp gửi tin nhắn
    //     if (message.find('.') != string::npos)
    //     {
    //         size_t pos = message.find('.');
    //         string recipient = message.substr(0, pos); // tên người nhận
    //         string content = message.substr(pos + 1);  // nội dùng tin nhắn

    //         // tạo mutex để đảm bảo chỉ 1 client đang duyệt qua clients để gửi tin nhắn đúng địa chỉ
    //         lock_guard<mutex> lock(client_mutex);
    //         bool found = false;
    //         for (const auto &[sock, uname] : clients)
    //         {
    //             // nếu tìm thấy người nhận thì gửi tin nhắn
    //             if (uname == recipient)
    //             {
    //                 string formatted_message = "(" + username + ": " + content + ")";
    //                 send(sock, formatted_message.c_str(), formatted_message.length(), 0);
    //                 found = true;
    //                 break;
    //             }
    //         }
    //         // nếu không tìm thấy người nhận
    //         if (!found)
    //         {
    //             string error_message = "User " + recipient + " not exist!";
    //             send(client_socket, error_message.c_str(), error_message.length(), 0);
    //         }
    //     }
    //     // sai cú pháp gửi tin nhắn
    //     else
    //     {
    //         string error_message = "send again: <username>.<messenge>";
    //         send(client_socket, error_message.c_str(), error_message.length(), 0);
    //     }
    // }
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        cerr << "Using: ./server <PortNumber>\n";
        return EXIT_FAILURE;
    }

    int PORT = stoi(argv[1]);

    // tạo tcp socket
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1)
    {
        cerr << "Can't create socket\n";
        return EXIT_FAILURE;
    }

    sockaddr_in server_addr = {};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // gán địa chỉ cho socket
    if (bind(server_socket, (sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        cerr << "Bind error\n";
        return EXIT_FAILURE;
    }

    // nghe connection từ client
    if (listen(server_socket, 10) < 0)
    {
        cerr << "listen error\n";
        return EXIT_FAILURE;
    }

    cout << "Server running on port: " << PORT << "\n";

    // lưu thông tin tài khoản mật khẩu từ file account.txt vào accounts
    loadAccounts("users.txt");

    // tạo mảng động threads kiểu thread
    vector<thread> threads;

    while (true)
    {
        sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_socket = accept(server_socket, (sockaddr *)&client_addr, &client_len);
        if (client_socket < 0)
        {
            cerr << "Accept error\n";
            continue;
        }

        // tạo 1 thread vào cuối vecto threads
        threads.emplace_back(handleClient, client_socket);
    }

    for (auto &t : threads)
    {
        t.join();
    }

    close(server_socket);
    return 0;
}