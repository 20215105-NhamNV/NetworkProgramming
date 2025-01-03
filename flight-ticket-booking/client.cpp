#include <iostream>
#include <thread>
#include <string>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include <mutex>

using namespace std;

#define BUFFER_SIZE 16384

mutex cout_mutex;

// Nhận tin nhắn từ server
// void receiveMessages(int server_socket)
// {
//     char buffer[BUFFER_SIZE];
//     while (true)
//     {
//         bzero(buffer, BUFFER_SIZE);
//         int bytes_received = recv(server_socket, buffer, BUFFER_SIZE, 0);
//         if (bytes_received <= 0)
//         {
//             lock_guard<mutex> lock(cout_mutex);
//             cout << "Server not connect.\n";
//             close(server_socket);
//             exit(EXIT_FAILURE);
//         }
//         lock_guard<mutex> lock(cout_mutex);
//         cout << buffer << endl;
//     }
// }

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        cerr << "Using: ./client <IPAddress> <PortNumber>\n";
        return EXIT_FAILURE;
    }

    string server_ip = argv[1];
    int PORT = stoi(argv[2]);

    // Tạo socket
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1)
    {
        cerr << "Can't create socket\n";
        return EXIT_FAILURE;
    }

    sockaddr_in server_addr = {};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    if (inet_pton(AF_INET, server_ip.c_str(), &server_addr.sin_addr) <= 0)
    {
        cerr << "IP invalid\n";
        return EXIT_FAILURE;
    }

    // Kết nối tới server
    if (connect(client_socket, (sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        cerr << "Can't connect to server\n";
        return EXIT_FAILURE;
    }
    cout << "Server connect successfully!\n";

    // Đăng nhập hoặc đăng ký
    while (true)
    {
        string command;
        cout << "\nTrang đăng kí, đăng nhập\n";
        cout << "----------------------------------------------------------\n";
        cout << "Đăng kí đăng nhập: (login/register username password)\n";
        cout << "Thoát: (exit)\n";
        getline(cin, command);
        send(client_socket, command.c_str(), command.length(), 0);

        char response[BUFFER_SIZE];
        bzero(response, BUFFER_SIZE);
        recv(client_socket, response, BUFFER_SIZE, 0);

        if (strcmp(response, "loginSuccess") == 0)
        {
            while (true)
            {
                cout << "\nTrang người dùng\n";
                cout << "----------------------------------------------------------\n";
                cout << "1.Tìm kiêm chuyên bay\n";
                cout << "2.So sánh giá vé\n";
                cout << "3.So sánh thời gian bay\n";
                cout << "4.Đặt vé máy bay và thanh toán trực tuyên\n";
                cout << "5.Xem mã vé điện tư\n";
                cout << "6.Quan lý vé đã đặt\n";
                cout << "Nhập lựa chọn của bạn(từ 1 đên 5):\n";
                int choose;
                cin >> choose;
                if (choose == 1)
                {
                    cin.ignore();
                    cout << "Nhập tiêu chí tìm kiêm:" << "\n"
                         << "(search departure destination startDate endDate quantityMin quantityMax classType)\n";
                    string userInput;
                    getline(cin, userInput);
                    send(client_socket, userInput.c_str(), userInput.length(), 0);

                    char userResponse[BUFFER_SIZE];
                    bzero(userResponse, BUFFER_SIZE);
                    recv(client_socket, userResponse, BUFFER_SIZE, 0);
                    cout << "\nKêt quả tìm kiêm:\n";
                    cout << "----------------------------------------------------------\n";
                    cout << "ID Airline Departure Destination StartDate EndDate Quantity ClassType Price Time\n";
                    cout << userResponse << endl;
                    cout << "----------------------------------------------------------\n";
                }
                else if (choose == 2)
                {
                    cin.ignore();
                    cout << "Giá vé tăng dân: (ascePrice)" << "\n"
                         << "Giá vé giảm dân: (descPrice)\n";
                    string userInput;
                    getline(cin, userInput);
                    send(client_socket, userInput.c_str(), userInput.length(), 0);

                    char userResponse[BUFFER_SIZE];
                    bzero(userResponse, BUFFER_SIZE);
                    recv(client_socket, userResponse, BUFFER_SIZE, 0);
                    cout << "\nKêt quả tìm kiêm:\n";
                    cout << "----------------------------------------------------------\n";
                    cout << "ID Airline Departure Destination StartDate EndDate Quantity ClassType Price Time\n";
                    cout << userResponse << endl;
                    cout << "----------------------------------------------------------\n";
                }
                else if (choose == 3)
                {
                    cin.ignore();
                    cout << "Thời gian bay tăng dân: (asceTime)" << "\n"
                         << "Thời gian bay giảm dân: (descTime)\n";
                    string userInput;
                    getline(cin, userInput);
                    send(client_socket, userInput.c_str(), userInput.length(), 0);

                    char userResponse[BUFFER_SIZE];
                    bzero(userResponse, BUFFER_SIZE);
                    recv(client_socket, userResponse, BUFFER_SIZE, 0);
                    cout << "\nKêt quả tìm kiêm:\n";
                    cout << "----------------------------------------------------------\n";
                    cout << "ID Airline Departure Destination StartDate EndDate Quantity ClassType Price Time\n";
                    cout << userResponse << endl;
                    cout << "----------------------------------------------------------\n";
                }
                else if (choose == 4)
                {
                    cin.ignore();
                    cout << "Nhập mã vé:(booking flightId)" << "\n";
                    string userInput;
                    getline(cin, userInput);
                    send(client_socket, userInput.c_str(), userInput.length(), 0);

                    char userResponse[BUFFER_SIZE];
                    bzero(userResponse, BUFFER_SIZE);
                    recv(client_socket, userResponse, BUFFER_SIZE, 0);
                    cout << "\nTruy cập vào url sau đê thanh toán:\n";
                    cout << "----------------------------------------------------------\n";
                    cout << userResponse << endl;
                    cout << "----------------------------------------------------------\n";
                }
                else if (choose == 5)
                {
                    cin.ignore();
                    cout << "Xem mã vé điện tư: (receive bookingId email)" << "\n";
                    string userInput;
                    getline(cin, userInput);
                    send(client_socket, userInput.c_str(), userInput.length(), 0);

                    char userResponse[BUFFER_SIZE];
                    bzero(userResponse, BUFFER_SIZE);
                    recv(client_socket, userResponse, BUFFER_SIZE, 0);
                    cout << "----------------------------------------------------------\n";
                    cout << userResponse << endl;
                    cout << "----------------------------------------------------------\n";
                }
                else if (choose == 6)
                {
                    cin.ignore();
                    cout << "Xem mã vé điện tư: (receive bookingId email)" << "\n";
                    string userInput;
                    getline(cin, userInput);
                    send(client_socket, userInput.c_str(), userInput.length(), 0);

                    char userResponse[BUFFER_SIZE];
                    bzero(userResponse, BUFFER_SIZE);
                    recv(client_socket, userResponse, BUFFER_SIZE, 0);
                    cout << "----------------------------------------------------------\n";
                    cout << userResponse << endl;
                    cout << "----------------------------------------------------------\n";
                }

                else
                {
                    cout << "Tính năng chưa hoạt động\n";
                    break;
                }
            }
        }
        else if (strcmp(response, "loginFail") == 0)
        {
            cout << "Wrong username or password. Try again!.\n";
        }
        else if (strcmp(response, "registerSuccess") == 0)
        {
            cout << "Register successful!\n";
        }
        else if (strcmp(response, "exist") == 0)
        {
            cout << "username already exist! Try again.!\n";
        }
        else if (strcmp(response, "goodbye") == 0)
        {
            cout << "exit!\n";
            break;
        }
        else
        {
            break;
            cout << "Error! Try again.!\n";
        }
    }

    // Bắt đầu nhận tin nhắn
    // thread receiver(receiveMessages, client_socket);

    // Gửi tin nhắn
    // while (true)
    // {
    //     string message;
    //     getline(cin, message);
    //     send(client_socket, message.c_str(), message.length(), 0);
    // }

    // receiver.join();
    close(client_socket);
    return 0;
}
