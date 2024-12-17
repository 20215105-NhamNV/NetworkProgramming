CC = g++  # Trình biên dịch C++
CFLAGS = -Wall -pthread -std=c++17  # Các tùy chọn biên dịch
TARGET_SERVER = server
TARGET_CLIENT = client

all: $(TARGET_SERVER) $(TARGET_CLIENT)  # Xây dựng cả server và client

$(TARGET_SERVER): server.cpp log.cpp
	$(CC) $(CFLAGS) -o $(TARGET_SERVER) server.cpp log.cpp  # Biên dịch và liên kết server

$(TARGET_CLIENT): client.cpp log.cpp
	$(CC) $(CFLAGS) -o $(TARGET_CLIENT) client.cpp log.cpp  # Biên dịch và liên kết client

clean:
	rm -f $(TARGET_SERVER) $(TARGET_CLIENT)  # Xóa các tệp đã biên dịch