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
    long response_code = 0;

    curl_mime* mime = curl_mime_init(curl);
    curl_mimepart* part = curl_mime_addpart(mime);

    const unsigned char minimal_content[] = { 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A, 0x00 };

    curl_mime_name(part, "fileToUpload");
    curl_mime_type(part, "image/png");
    curl_mime_data(part, (const char*)minimal_content, sizeof(minimal_content));
    curl_mime_filename(part, "minimal_image.png");

    std::string url = "http://localhost:8080/upload_script.php";

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_data);
    curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);

    CURLcode res = curl_easy_perform(curl);
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

    curl_mime_free(mime);

    INFO("Status code: " << response_code);
    INFO("Response text: " << response_data);

    curl_easy_cleanup(curl);

    REQUIRE(res == CURLE_OK);
    REQUIRE(response_code == 200);
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

TEST_CASE("Content Type Test", "[http]") {
    struct FileType {
        std::string path;
        std::string expected_type;
        bool should_exist;
    };

    std::vector<FileType> files = {
        {"/index.html", "text/html", true},
        {"/", "text/html", true},
        {"/index.php", "text/html", true},
        {"/query.php", "text/html", true},
        {"/post.php", "text/html", true},
        {"/upload_script.php", "text/html", true},
        {"/api/index.php", "text/html", true},
        {"/api/index.html", "text/html", true},
        {"/style.css", "text/css", false},
        {"/script.js", "application/javascript", true},
        {"/data.json", "application/json", true},

        {"/somefile.xyz", "application/octet-stream", false}
    };

    for (const auto& file : files) {
        SECTION("Content-Type for " + file.path) {
            CURL* curl = curl_easy_init();
            REQUIRE(curl != nullptr);

            std::string response_data;
            std::string header_data;
            long response_code = 0;

            std::string url = "http://localhost:8080" + file.path;

            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_data);
            curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_HEADERDATA, &header_data);

            CURLcode res = curl_easy_perform(curl);
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

            INFO("Status code: " << response_code);
            INFO("Response headers: " << header_data);

            curl_easy_cleanup(curl);

            if (response_code == 200) {
                std::string lowercase_headers = header_data;
                std::transform(lowercase_headers.begin(), lowercase_headers.end(),
                            lowercase_headers.begin(), ::tolower);
                
                std::string expected_type = file.expected_type;
                std::transform(expected_type.begin(), expected_type.end(), expected_type.begin(), ::tolower);
                
                REQUIRE(lowercase_headers.find(expected_type) != std::string::npos);
                REQUIRE(lowercase_headers.find("content-type") != std::string::npos);
            } else if (file.should_exist) {
                FAIL("Expected file not found error: " << file.path << " (Status: " << response_code << ")");
            } else {
                INFO("Optional file " << file.path << " returned status " << response_code);
            }
        }
    }
}

TEST_CASE("HTTP Status Code - 414", "[http]") {
    CURL* curl = curl_easy_init();
    REQUIRE(curl != nullptr);

    std::string response_data;
    long response_code;

    std::string long_path = "/";
    for (int i = 0; i < 10000; i++) {
        long_path += "very_long_path";
    }
    long_path += ".html";

    std::string url = "http://localhost:8080" + long_path;

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_data);

    CURLcode res = curl_easy_perform(curl);
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

    INFO("Status code: " << response_code);
    INFO("Response text: " << response_data);

    curl_easy_cleanup(curl);

    REQUIRE(res == CURLE_OK);
    REQUIRE(response_code == 414);
}

TEST_CASE("HTTP Status Code - 406", "[http]") {
    CURL* curl = curl_easy_init();
    REQUIRE(curl != nullptr);

    std::string response_data;
    long reponse_code = 0;
    struct curl_slist* headers = NULL;

    headers = curl_slist_append(headers, "Accept: application/nonexistent-format");

    std::string url = "http://localhost:8080/";

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_data);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    CURLcode res = curl_easy_perform(curl);
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &reponse_code);

    INFO("Status code: " << reponse_code);
    INFO("Response text: " << response_data);

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    REQUIRE(res == CURLE_OK);
    REQUIRE(reponse_code == 406);
}

TEST_CASE("HTTP Status code - 205", "[http]") {
    CURL* curl = curl_easy_init();
    REQUIRE(curl != nullptr);

    std::string response_data;
    long response_code = 0;

    std::string post_data = "submit=1";

    std::string url = "http://localhost:8080/upload.html";

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_data);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data.c_str());
    
    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    CURLcode res = curl_easy_perform(curl);
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

    curl_slist_free_all(headers);

    INFO("Status code: " << response_code);
    INFO("Response text: " << response_data);

    curl_easy_cleanup(curl);

    REQUIRE(res == CURLE_OK);
    REQUIRE(response_code == 205);
}

TEST_CASE("HTTP Status Code - 412", "[http]") {
    CURL* curl = curl_easy_init();
    REQUIRE(curl != nullptr);

    std::string response_data;
    long response_code = 0;
    struct curl_slist* headers = NULL;

    headers = curl_slist_append(headers, "If-Match: \"some-etag-value\"");

    std::string url = "http://localhost:8080/";

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_data);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    CURLcode res = curl_easy_perform(curl);
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

    curl_slist_free_all(headers);

    INFO("Status code: " << response_code);
    INFO("Response text: " << response_data);

    REQUIRE(res == CURLE_OK);
    REQUIRE(response_code == 412);
}

TEST_CASE("HTTP Status Code - 416", "[http]") {
    CURL* curl = curl_easy_init();
    REQUIRE(curl != nullptr);

    std::string response_data;
    long response_code = 0;
    struct curl_slist* headers = NULL;

    headers = curl_slist_append(headers, "Range: bytes=100-200");

    std::string url = "http://localhost:8080/";

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_data);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    CURLcode res = curl_easy_perform(curl);
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    INFO("Status code: " << response_code);
    INFO("Response text: " << response_data);

    REQUIRE(res == CURLE_OK);
    REQUIRE(response_code == 416);
}

TEST_CASE("HTTP Status Code - 417", "[http]") {
    CURL* curl = curl_easy_init();
    REQUIRE(curl != nullptr);

    std::string response_data;
    long response_code = 0;
    struct curl_slist* headers = NULL;

    headers = curl_slist_append(headers, "Expect: 100-continue");

    std::string url = "http://localhost:8080/";

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_data);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    CURLcode res = curl_easy_perform(curl);
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    INFO("Status code: " << response_code);
    INFO("Response tex: " << response_data);

    REQUIRE(res == CURLE_OK);
    REQUIRE(response_code == 417);
}