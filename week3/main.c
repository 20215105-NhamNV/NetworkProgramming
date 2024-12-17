#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <libxml/HTMLparser.h>

#define BUFFER_SIZE 10000

// Struct để lưu dữ liệu
struct Memory
{
    char *response;
    size_t size;
};

// Hàm callback để lưu dữ liệu từ curl vào bộ nhớ
size_t write_callback(void *ptr, size_t size, size_t nmemb, struct Memory *data)
{
    size_t new_size = data->size + size * nmemb;
    data->response = realloc(data->response, new_size + 1);
    if (data->response == NULL)
    {
        printf("Error reallocating memory.\n");
        return 0;
    }
    memcpy(&(data->response[data->size]), ptr, size * nmemb);
    data->size = new_size;
    data->response[new_size] = '\0';
    return size * nmemb;
}

// Hàm để crawl dữ liệu
void crawl_youtube(const char *url)
{
    CURL *curl;
    CURLcode res;
    struct Memory chunk = {.response = malloc(1), .size = 0};

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &chunk);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0");
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }

    // Phân tích cú pháp HTML
    htmlDocPtr doc = htmlReadMemory(chunk.response, chunk.size, NULL, NULL, HTML_PARSE_NOERROR | HTML_PARSE_NOWARNING);
    if (doc)
    {
        // Tìm kiếm các video và link
        xmlNode *root = xmlDocGetRootElement(doc);
        xmlNode *current_node = root->children;

        FILE *link_file = fopen("links.csv", "w");
        FILE *text_file = fopen("texts.csv", "w");
        FILE *video_file = fopen("videos.csv", "w");

        while (current_node)
        {
            if (current_node->type == XML_ELEMENT_NODE)
            {
                // Lọc link và text
                if (strcmp((char *)current_node->name, "a") == 0)
                {
                    xmlChar *href = xmlGetProp(current_node, (const xmlChar *)"href");
                    xmlChar *text = xmlNodeGetContent(current_node);

                    if (strstr((char *)href, "watch"))
                    {
                        fprintf(link_file, "%s\n", href);
                        fprintf(text_file, "%s\n", text);
                        fprintf(video_file, "%s\n", href);
                    }
                    xmlFree(href);
                    xmlFree(text);
                }
            }
            current_node = current_node->next;
        }

        fclose(link_file);
        fclose(text_file);
        fclose(video_file);
        xmlFreeDoc(doc);
    }

    free(chunk.response);
    curl_global_cleanup();
}

int main()
{
    const char *url = "https://www.youtube.com/@KTeam/videos"; // Thay YOUR_CHANNEL_NAME bằng tên kênh
    crawl_youtube(url);
    printf("Dữ liệu đã được lưu vào các file CSV!\n");
    return 0;
}
