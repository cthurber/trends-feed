//
//  main.cpp
//  Stratspp
//
//  Created by Chris Thurber on 11/18/16.
//  Copyright © 2016 Chris Thurber. All rights reserved.
//

// main.cpp
#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <regex>
#include <boost/regex.hpp>
<<<<<<< HEAD
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/replace.hpp>
=======
>>>>>>> 33f3049e1e7c698ff23a072e1d51b0b39774eadf

#include <curl/curl.h>
#include <json/json.h>

namespace {
    std::size_t callback(
            const char* in,
            std::size_t size,
            std::size_t num,
            std::string* out)
    {
        const std::size_t totalBytes(size * num);
        out->append(in, totalBytes);
        return totalBytes;
    }
}

int main() {
    const std::string url("http://www.google.com/trends/fetchComponent?hl=en-US&date=today%203-m&q=debt,%2Fm%2F02lc8s,brexit&tz=Etc/GMT%2B4&content=1&cid=TIMESERIES_GRAPH_0&export=3");

    CURL* curl = curl_easy_init();

    // Set remote URL.
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

    // Don't bother trying IPv6, which would increase DNS resolution time.
    curl_easy_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);

    // Don't wait forever, time out after 10 seconds.
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10);

    // Follow HTTP redirects if necessary.
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    // Response information.
    int httpCode(0);
    std::unique_ptr<std::string> httpData(new std::string());
    // std::unique_ptr<std::string> jsonAsString(new std::string());

    // Hook up data handling function.
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, callback);

    // Hook up data container (will be passed as the last parameter to the
    // callback handling function).  Can be any pointer type, since it will
    // internally be passed as a void pointer.
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, httpData.get());

    // Run our HTTP GET command, capture the HTTP response code, and clean up.
    curl_easy_perform(curl);
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
    curl_easy_cleanup(curl);

    if (httpCode == 200) {
        std::cout << "\nGot successful response from " << url << std::endl;
        std::string jsonAsString = "";
        // Response looks good - done using Curl now.  Try to parse the results
        // and print them out.
        Json::Value jsonData;
        Json::Reader jsonReader;

        // Removes Google nonsense from json returned as string
        std::smatch matches;
        regex_search(*httpData.get(), matches, std::regex("(\\{.*\\})", std::regex::optimize));
        jsonAsString += matches[1];

        // Customizations for string returned from Trends Server
        boost::erase_all(jsonAsString, "new Date");
        boost::replace_all(jsonAsString, "(", "[");
        boost::replace_all(jsonAsString, ")", "]");
        boost::erase_all(jsonAsString, ",,");

        // Debuggin mech.
        std::cout << jsonAsString << std::endl;

        if (jsonReader.parse(jsonAsString, jsonData)) {
            std::cout << "Successfully parsed JSON data" << std::endl;
            std::cout << "\nJSON data received:" << std::endl;
            // std::cout << jsonData.toStyledString() << std::endl;

            const std::string dateString(jsonData["date"].asString());
            const std::size_t unixTimeMs(
                    jsonData["milliseconds_since_epoch"].asUInt64());
            const std::string timeString(jsonData["time"].asString());

            std::cout << "Natively parsed:" << std::endl;
            std::cout << "\tDate string: " << dateString << std::endl;
            std::cout << std::endl;
        }
        else {
            std::cout << "Could not parse HTTP data as JSON" << std::endl;
            std::cout << "HTTP data was:\n" << *httpData.get() << std::endl;
            
            return 1;
        }
    }
    else {
        std::cout << "Couldn't GET from " << url << " - exiting" << std::endl;
        return 1;
    }

    return 0;
}