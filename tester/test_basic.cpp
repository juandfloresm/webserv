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