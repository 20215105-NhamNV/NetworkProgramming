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
#include "log.h" // ghi log
#include <algorithm>
#include <openssl/hmac.h>
#include <curl/curl.h>
#include <openssl/hmac.h>
#include <openssl/evp.h>
#include <iomanip>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

using namespace std;
#define BUFFER_SIZE 16384

mutex client_mutex;       // bảo vệ map clients khỏi các truy cập đồng thời từ nhiều thread
map<int, string> clients; // Map lưu socket và username
map<string, string> accounts;
mutex register_mutex;

struct Flight
{
    string id;
    string airline;
    string departure;
    string destination;
    string startDate;
    string endDate;
    string quantity;
    string classType;
    string price;
    string time;
};

vector<Flight> flights;

// Add these structs after Flight struct
struct Booking
{
    string bookingId;
    string username;
    string flightId;
    string amount;
    string timestamp;
};

// Add these vectors and mutex for thread safety
vector<Booking> bookings;
mutex booking_mutex;

// Tải thông tin tài khoản từ file account.txt
void loadUsers(const string &filename)
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

void loadFlights(const string &filename)
{
    ifstream file(filename);
    if (!file.is_open())
    {
        cerr << "Not open file account.txt\n";
        exit(EXIT_FAILURE);
    }
    string line;
    while (getline(file, line))
    {
        Flight flight;
        stringstream ss(line);

        ss >> flight.id >> flight.airline >> flight.departure >> flight.destination >> flight.startDate >> flight.endDate >> flight.quantity >> flight.classType >> flight.price >> flight.time;

        if (ss.fail())
        {
            cerr << "Error parsing flight data\n";
            continue;
        }
        flights.push_back(flight);
    }
    file.close();
}

void loadBookings(const string &filename)
{
    ifstream file(filename);
    if (!file.is_open())
    {
        cerr << "Not open file account.txt\n";
        exit(EXIT_FAILURE);
    }
    string line;
    while (getline(file, line))
    {
        Booking booking;
        stringstream ss(line);

        ss >> booking.bookingId >> booking.username >> booking.flightId >> booking.amount >> booking.timestamp;

        if (ss.fail())
        {
            cerr << "Error parsing flight data\n";
            continue;
        }
        bookings.push_back(booking);
    }
    file.close();
}

// Hàm tạo HMAC SHA256
string generateHMAC(const string &key, const string &data)
{
    unsigned char hash[EVP_MAX_MD_SIZE];
    size_t hash_len = 0;

    // Sử dụng context mới (API OpenSSL 3.0)
    EVP_MAC *mac = EVP_MAC_fetch(NULL, "HMAC", NULL);
    EVP_MAC_CTX *ctx = EVP_MAC_CTX_new(mac);

    // Định cấu hình thuật toán HMAC-SHA256
    OSSL_PARAM params[] = {
        OSSL_PARAM_utf8_string("digest", (char *)"SHA256", 0),
        OSSL_PARAM_END};

    EVP_MAC_init(ctx, (unsigned char *)key.c_str(), key.length(), params);
    EVP_MAC_update(ctx, (unsigned char *)data.c_str(), data.length());
    EVP_MAC_final(ctx, hash, &hash_len, sizeof(hash));

    // Giải phóng bộ nhớ
    EVP_MAC_CTX_free(ctx);
    EVP_MAC_free(mac);

    // Chuyển đổi sang chuỗi hex
    ostringstream oss;
    for (size_t i = 0; i < hash_len; i++)
    {
        oss << hex << setw(2) << setfill('0') << (int)hash[i];
    }
    return oss.str();
}

size_t WriteCallback(void *contents, size_t size, size_t nmemb, string *userp)
{
    userp->append((char *)contents, size * nmemb);
    return size * nmemb;
}

string makeHttpRequest(const string &url, const string &jsonData)
{
    CURL *curl = curl_easy_init();
    string response;

    if (curl)
    {
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");

        json j = json::parse(jsonData);

        // Build parameters string
        string params;
        for (auto it = j.begin(); it != j.end(); ++it)
        {
            if (it != j.begin())
            {
                params += "&";
            }

            string value;
            if (it.value().is_string())
            {
                value = it.value().get<string>();
            }
            else
            {
                value = it.value().dump();
            }

            params += it.key() + "=" + value;
        }

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, params.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK)
        {
            cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << endl;
        }

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }
    return response;
}

// Xử lý client
void handleClient(int client_socket)
{
    char buffer[BUFFER_SIZE];
    string username;

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
                username = user; // Add this line
                send(client_socket, "loginSuccess", strlen("loginSuccess"), 0);
                // Ghi log server gửi
                logMessage("Sent to client: loginSuccess");
                lock_guard<mutex> lock(client_mutex);
                clients[client_socket] = username;
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
                lock_guard<mutex> lock(register_mutex);
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

    // Trang người dùng
    while (true)
    {
        memset(buffer, 0, BUFFER_SIZE);
        recv(client_socket, buffer, BUFFER_SIZE, 0);
        string command(buffer);
        // Ghi log message client gửi
        logMessage("Received from client: " + command);
        stringstream ss(command);
        string action, departure, destination, startDate, endDate, quantityMax, quantityMin, classType;
        ss >> action >> departure >> destination >> startDate >> endDate >> quantityMin >> quantityMax >> classType;

        // Tìm kiêm chuyên bay
        if (action == "search")
        {
            string searchResult;
            for (const Flight &flight : flights)
            {
                // Check departure condition
                if (departure != "all" && flight.departure != departure)
                    continue;

                // Check destination
                if (destination != "all" && flight.destination != destination)
                    continue;

                // Check dates
                if (startDate != "all" && flight.startDate != startDate)
                    continue;

                if (endDate != "all" && flight.startDate != endDate)
                    continue;

                // check quantity
                if (quantityMin != "all" && stoi(flight.quantity) < stoi(quantityMin))
                {
                    continue;
                }

                if (quantityMax != "all" && stoi(flight.quantity) > stoi(quantityMax))
                    continue;

                // Check class type
                if (classType != "all" && flight.classType != classType)
                    continue;

                int timeInMinutes = std::stoi(flight.time); // Chuyển từ string sang int
                int hours = timeInMinutes / 60;             // Tính số giờ
                int minutes = timeInMinutes % 60;           // Tính số phút còn lại
                std::string formattedTime = std::to_string(hours) + "h" + std::to_string(minutes) + "p";

                // Add matching flight to result
                searchResult += flight.id + " " + flight.airline + " " +
                                flight.departure + " " + flight.destination + " " +
                                flight.startDate + " " + flight.endDate + " " +
                                flight.quantity + " " + flight.classType + " " +
                                flight.price + " " + formattedTime + "\n";
            }

            // Send results to client
            if (searchResult.empty())
                searchResult = "Khong tim thay chuyen bay nao\n";

            send(client_socket, searchResult.c_str(), searchResult.length(), 0);
            logMessage("Sent to client: " + searchResult);
            break;
        }

        // săp xêp theo giá vé tăng dần
        else if (action == "ascePrice")
        {
            // Create copy of flights vector for sorting
            vector<Flight> asceFlights = flights;

            // Sort by price
            sort(asceFlights.begin(), asceFlights.end(),
                 [](const Flight &a, const Flight &b)
                 {
                     return stoi(a.price) < stoi(b.price);
                 });

            string asceResult;
            for (const Flight &flight : asceFlights)
            {
                int timeInMinutes = stoi(flight.time);
                int hours = timeInMinutes / 60;
                int minutes = timeInMinutes % 60;
                string formattedTime = to_string(hours) + "h" + to_string(minutes) + "p";

                asceResult += flight.id + " " + flight.airline + " " +
                              flight.departure + " " + flight.destination + " " +
                              flight.startDate + " " + flight.endDate + " " +
                              flight.quantity + " " + flight.classType + " " +
                              flight.price + " " + formattedTime + "\n";
            }

            send(client_socket, asceResult.c_str(), asceResult.length(), 0);
            logMessage("Sent to client: " + asceResult);
        }

        // sắp xếp theo giá vé giảm dần
        else if (action == "descPrice")
        {
            // Create copy of flights vector for sorting
            vector<Flight> descFlights = flights;

            // Sort by price
            sort(descFlights.begin(), descFlights.end(),
                 [](const Flight &a, const Flight &b)
                 {
                     return stoi(a.price) > stoi(b.price);
                 });

            string descResult;
            for (const Flight &flight : descFlights)
            {
                int timeInMinutes = stoi(flight.time);
                int hours = timeInMinutes / 60;
                int minutes = timeInMinutes % 60;
                string formattedTime = to_string(hours) + "h" + to_string(minutes) + "p";

                descResult += flight.id + " " + flight.airline + " " +
                              flight.departure + " " + flight.destination + " " +
                              flight.startDate + " " + flight.endDate + " " +
                              flight.quantity + " " + flight.classType + " " +
                              flight.price + " " + formattedTime + "\n";
            }

            send(client_socket, descResult.c_str(), descResult.length(), 0);
            logMessage("Sent to client: " + descResult);
        }

        // sắp xếp theo thời gian bay tăng dần
        else if (action == "asceTime")
        {
            // Create copy of flights vector for sorting
            vector<Flight> asceFlights = flights;

            // Sort by time
            sort(asceFlights.begin(), asceFlights.end(),
                 [](const Flight &a, const Flight &b)
                 {
                     return stoi(a.time) < stoi(b.time);
                 });

            string asceResult;
            for (const Flight &flight : asceFlights)
            {
                int timeInMinutes = stoi(flight.time);
                int hours = timeInMinutes / 60;
                int minutes = timeInMinutes % 60;
                string formattedTime = to_string(hours) + "h" + to_string(minutes) + "p";

                asceResult += flight.id + " " + flight.airline + " " +
                              flight.departure + " " + flight.destination + " " +
                              flight.startDate + " " + flight.endDate + " " +
                              flight.quantity + " " + flight.classType + " " +
                              flight.price + " " + formattedTime + "\n";
            }

            send(client_socket, asceResult.c_str(), asceResult.length(), 0);
            logMessage("Sent to client: " + asceResult);
        }

        // sắp xếp theo thời gian bay giảm dần
        else if (action == "descTime")
        {
            // Create copy of flights vector for sorting
            vector<Flight> descFlights = flights;

            // Sort by time
            sort(descFlights.begin(), descFlights.end(),
                 [](const Flight &a, const Flight &b)
                 {
                     return stoi(a.time) > stoi(b.time);
                 });

            string descResult;
            for (const Flight &flight : descFlights)
            {
                int timeInMinutes = stoi(flight.time);
                int hours = timeInMinutes / 60;
                int minutes = timeInMinutes % 60;
                string formattedTime = to_string(hours) + "h" + to_string(minutes) + "p";

                descResult += flight.id + " " + flight.airline + " " +
                              flight.departure + " " + flight.destination + " " +
                              flight.startDate + " " + flight.endDate + " " +
                              flight.quantity + " " + flight.classType + " " +
                              flight.price + " " + formattedTime + "\n";
            }

            send(client_socket, descResult.c_str(), descResult.length(), 0);
            logMessage("Sent to client: " + descResult);
        }
        else if (action == "booking")
        {
            string flightId = departure;

            // Find flight details
            Flight *selectedFlight = nullptr;
            for (auto &flight : flights)
            {
                if (flight.id == flightId)
                {
                    selectedFlight = &flight;
                    break;
                }
            }

            if (!selectedFlight)
            {
                send(client_socket, "Flight not found", strlen("Flight not found"), 0);
                continue;
            }

            string app_id = "2554";
            string key1 = "sdngKKJmqEMzvh5QQcdD2A9XBSKUNaYn";
            string Key2 = "trMrHtvjo6myautxDUiAcYsVtaeQ8nhf";

            // Generate transaction ID
            srand(time(nullptr));
            int transID = rand() % 1000000;

            // Get current timestamp
            time_t now = time(nullptr);
            char timestamp[7];
            strftime(timestamp, sizeof(timestamp), "%y%m%d", localtime(&now));

            string app_trans_id = string(timestamp) + "_" + to_string(transID);

            string app_user = "user123";
            int amount = stoi(selectedFlight->price);
            long app_time = time(nullptr) * 1000; // Current time in milliseconds

            string embed_data = "{\"redirecturl\":\"https://zalopay.vn/\"}";
            string item = "[{}]";

            // Create data string for MAC
            string data = app_id + "|" + app_trans_id + "|" + app_user + "|" +
                          to_string(amount) + "|" + to_string(app_time) + "|" +
                          embed_data + "|" + item;

            string mac = generateHMAC(key1, data);

            // Create order data

            string orderData = json{
                {"app_id", app_id},
                {"app_trans_id", app_trans_id},
                {"app_user", app_user},
                {"app_time", app_time},
                {"item", json::parse(item)},
                {"embed_data", json::parse(embed_data)},
                {"amount", amount},
                {"description", "Payment for flight " + flightId},
                {"bank_code", ""},
                {"mac", mac}}.dump(); // Chuyển thành chuỗi JSON

            // Make HTTP request
            string response = makeHttpRequest("https://sb-openapi.zalopay.vn/v2/create", orderData);

            // Parse response
            json responseJson = json::parse(response);

            if (responseJson["return_code"] == 1)
            {
                string order_url = responseJson["order_url"];

                // Create booking record
                lock_guard<mutex> lock(booking_mutex);
                Booking booking{
                    to_string(transID), // bookingId
                    username,           // username/email
                    flightId,           // flightId
                    to_string(amount),  // amount
                    to_string(app_time) // timestamp
                };
                bookings.push_back(booking);

                // Write to bookings.txt
                ofstream bookingFile("bookings.txt", ios::app);
                bookingFile << booking.bookingId << " "
                            << booking.username << " "
                            << booking.flightId << " "
                            << booking.amount << " "
                            << booking.timestamp << endl;
                bookingFile.close();

                // Send order_url to client
                send(client_socket, order_url.c_str(), order_url.length(), 0);
                logMessage("Sent to client: " + order_url);
            }
            else
            {
                // Send error message
                send(client_socket, "Booking failed", strlen("Booking failed"), 0);
                logMessage("Booking failed: " + response);
            }
        }
        // else if (action == "receive")
        // {
        // }
    }

    // Thêm client vào danh sách

    // tạo mutex để ngăn nhiều client thao tác đồng thời vào map clients
    // lock_guard<mutex> lock(client_mutex);
    // clients[client_socket] = username;

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

    // lưu thông tin tài khoản mật khẩu từ file users.txt vào accounts
    loadUsers("users.txt");
    loadFlights("flights.txt");
    loadBookings("bookings.txt");

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