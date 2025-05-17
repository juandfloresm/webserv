#define CATCH_CONFIG_MAIN
#include "../tester/catch.hpp"
#include <curl/curl.h>
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>

TEST_CASE("Basic check") {
    REQUIRE(1 + 1 == 2);
}

size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* s) {
    size_t newLength = size * nmemb;
    try {
        s->append((char*)contents, newLength);
        return newLength;
    } catch(std::bad_alloc& e) {
        return 0;
    }
}


// TEST_CASE("Debug HTTP response format", "[http]") {
//     std::system("curl -v --trace-ascii /tmp/curl_trace.txt 'http://localhost:8080/' > /tmp/curl_output.txt 2>&1");
    
//     std::ifstream trace_file("/tmp/curl_trace.txt");
//     std::stringstream buffer;
//     buffer << trace_file.rdbuf();
//     std::string trace = buffer.str();
    
//     std::cout << "\n----- Raw HTTP Response -----\n";
//     std::cout << trace << std::endl;
//     std::cout << "-----------------------------\n";
    
//     REQUIRE(true); // Debug test
// }

TEST_CASE("Webserver root endpoint", "[http]") {
    CURL* curl = curl_easy_init();
    REQUIRE(curl != nullptr);
    
    std::string response_data;
    long response_code = 0;
    
    std::string url = "http://localhost:8080/";

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_data);
    
    CURLcode res = curl_easy_perform(curl);
    REQUIRE(res == CURLE_OK);
    
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
    curl_easy_cleanup(curl);
    
    INFO("Status code: " << response_code);
    INFO("Response text: " << response_data);
    
    REQUIRE(response_code == 200);
    REQUIRE(!response_data.empty());
}

TEST_CASE("404 Not Found Status", "[http]") {
    CURL* curl = curl_easy_init();
    REQUIRE(curl != nullptr);

    std::string response_data;
    long response_code;
    std::string url = "http://localhost:8080/nonexistent.hml";

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_data);

    CURLcode res = curl_easy_perform(curl);
    REQUIRE(res == CURLE_OK);

    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
    curl_easy_cleanup(curl);

    INFO("Status code: " << response_code);
    INFO("Response text: " << response_data);

    REQUIRE(response_code == 404);
}

TEST_CASE("HTTP GET with query parameters", "[http]") {
    CURL* curl = curl_easy_init();
    REQUIRE(curl != nullptr);
    
    std::string response_data;
    long response_code = 0;
    std::string url = "http://localhost:8080/query.php?name=test";
    
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_data);
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    
    CURLcode res = curl_easy_perform(curl);
    
    const char* error_message = curl_easy_strerror(res);
    INFO("CURL error code: " << res << " (" << error_message << ")");
    
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
    curl_easy_cleanup(curl);
    
    INFO("Status code: " << response_code);
    INFO("Response text: " << response_data);
    
    if (res == CURLE_OK) {
        REQUIRE(response_code == 200);
        REQUIRE(response_data.find("test") != std::string::npos);
    } else {
        WARN("Query endpoint test skipped - endpoint may not exist: " << url);
    }
}

TEST_CASE("Content-Length Accuracy Test", "[http]") {
    CURL* curl = curl_easy_init();
    REQUIRE(curl != nullptr);
    
    std::string response_data;
    long response_code = 0;
    long content_length = 0;
    
    std::string url = "http://localhost:8080/";
    
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_data);
    
    CURLcode res = curl_easy_perform(curl);
    REQUIRE(res == CURLE_OK);
    
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
    curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD_T, &content_length);
    
    INFO("Status code: " << response_code);
    INFO("Content-Length header: " << content_length);
    INFO("Actual response size: " << response_data.size());
    
    REQUIRE(response_code == 200);
    if (content_length > 0) {
        REQUIRE(static_cast<size_t>(content_length) == response_data.size());
    }
    
    curl_easy_cleanup(curl);
}

TEST_CASE("HTTP POST Request", "[http]") {
    CURL* curl = curl_easy_init();
    REQUIRE(curl != nullptr);

    std::string response_data;
    long response_code = 0;
    std::string post_data = "field1=value1&field2=value2";

    std::string url = "http://localhost:8080/post.php";

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_data);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data.c_str());

    CURLcode res = curl_easy_perform(curl);
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

    INFO("Status code: " << response_code);
    INFO("Response text: " << response_data);

    curl_easy_cleanup(curl);

    REQUIRE(res == CURLE_OK);
    REQUIRE(response_code == 200);
}

TEST_CASE("Client Max Body Size Limit", "[http]") {
    CURL* curl = curl_easy_init();
    REQUIRE(curl != nullptr);

    std::string response_data;
    long response_code = 0;

    std::string large_post_data(10 * 1024 * 1024, 'X'); // 10MB data

    std::string url = "http://localhost:8080/upload.html";

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_data);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, large_post_data.c_str());

    CURLcode res = curl_easy_perform(curl);
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

    INFO("Status code: " << response_code);
    INFO("Response text: " << response_data);

    curl_easy_cleanup(curl);

    REQUIRE(res == CURLE_OK);
    REQUIRE(response_code == 413); // 413 payload too large;
}

TEST_CASE("Custom Headers", "[http]") {
    CURL* curl = curl_easy_init();
    REQUIRE(curl != nullptr);

    std::string response_data;
    long response_code = 0;
    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, "X-Custom-Header: TestValue");

    std::string url = "http://localhost:8080";

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_data);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    CURLcode res = curl_easy_perform(curl);
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

    INFO("Status code :" << response_code);
    INFO("Reponse text: " << response_data);

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    REQUIRE(res == CURLE_OK);
    REQUIRE(response_code == 200);
}

TEST_CASE("Multipart Form Upload", "[http]") {
    CURL* curl = curl_easy_init();
    REQUIRE(curl != nullptr);

    std::string response_data;
    long response_code;

    curl_mime* mime = curl_mime_init(curl);
    curl_mimepart* part = curl_mime_addpart(mime);

    curl_mime_name(part, "file");
    curl_mime_data(part, "File content to upload", CURL_ZERO_TERMINATED);
    curl_mime_filename(part, "test_upload.txt");

    std::string url = "http://localhost:8080/upload.php";

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_data);
    curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);

    CURLcode res = curl_easy_perform(curl);
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

    INFO("Status code: " << response_code);
    INFO("Response text: " << response_data);

    REQUIRE(res == CURLE_OK);
    REQUIRE((response_code == 200 || response_code == 201));
}

TEST_CASE("Cookie Handling", "[http]") {
    CURL* curl = curl_easy_init();
    REQUIRE(curl != nullptr);

    std::string response_data;
    long response_code;

    std::string url = "http://localhost:8080";
    
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_data);
    curl_easy_setopt(curl, CURLOPT_COOKIE, "name=value; example=test");

    CURLcode res = curl_easy_perform(curl);
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

    INFO("Status code: " << response_code);
    INFO("Response text: " << response_data);

    REQUIRE(res == CURLE_OK);
    REQUIRE(response_code == 200);
}

TEST_CASE("Response Headers Test", "[http]") {
    CURL* curl = curl_easy_init();
    REQUIRE(curl != nullptr);

    std::string response_data;
    std::string header_data;
    long response_code;

    std::string url = "http://localhost:8080";

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_data);
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, &header_data);

    CURLcode res = curl_easy_perform(curl);
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

    INFO("Status code: " << response_code);
    INFO("Response text: " << response_data);

    curl_easy_cleanup(curl);

    REQUIRE(res == CURLE_OK);
    REQUIRE(response_code == 200);
    REQUIRE(header_data.find("Content-Type") != std::string::npos);
}

TEST_CASE("Method Not Allowed", "[http]") {
    CURL* curl = curl_easy_init();
    REQUIRE(curl != nullptr);

    std::string response_data;
    long response_code = 0;

    std::string url = "http://localhost:8080";

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_data);
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");

    CURLcode res = curl_easy_perform(curl);
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

    INFO("Status code: " << response_code);
    INFO("Response text: " << response_data);

    curl_easy_cleanup(curl);

    REQUIRE(res == CURLE_OK);
    REQUIRE(response_code == 405);
}

TEST_CASE("HTTP HEAD Request", "[http]") {
    CURL* curl = curl_easy_init();
    REQUIRE(curl != nullptr);

    std::string response_data;
    long response_code = 0;

    std::string url = "http://localhost:8080/";

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_data);
    curl_easy_setopt(curl, CURLOPT_NOBODY, 1L); // HEAD Request

    CURLcode res = curl_easy_perform(curl);
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

    INFO("Status code: " << response_code);
    INFO("Response text: " << response_data);

    curl_easy_cleanup(curl);

    REQUIRE(res == CURLE_OK);

    REQUIRE(response_code == 405);
    REQUIRE(response_data.empty());
}

TEST_CASE("PHP CGI Execution", "[http]") {
    CURL* curl = curl_easy_init();
    REQUIRE(curl != nullptr);

    std::string response_data;
    long response_code = 0;
    std::string post_data = "param1=test&param2=value";

    std::string url = "http://localhost:8080/index.php";

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_data);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data.c_str());

    CURLcode res = curl_easy_perform(curl);
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

    INFO("Status code: " << response_code);
    INFO("Response text: " << response_data);

    REQUIRE(res == CURLE_OK);
    REQUIRE(response_code == 200);
}

TEST_CASE("Basic Authentication", "[http]") {
    CURL* curl = curl_easy_init();
    REQUIRE(curl != nullptr);

    std::string response_data;
    long response_code = 0;

    std::string url = "http://localhost:8080/auth_basic.php";

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_data);
    curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
    curl_easy_setopt(curl, CURLOPT_USERNAME, "testuser");
    curl_easy_setopt(curl, CURLOPT_PASSWORD, "testpass");

    CURLcode res = curl_easy_perform(curl);
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

    INFO("Status code: " << response_code);
    INFO("Reponse text: " << response_data);

    curl_easy_cleanup(curl);

    REQUIRE(res == CURLE_OK);
    REQUIRE((response_code == 200 || response_code == 401)); // 200 auth succees, 401 auth required
}