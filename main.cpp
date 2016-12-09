//
//  main.cpp
//  GoogleTrends Scraper
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

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/replace.hpp>

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

int main(int argc, char* argv[]) {
    std::string url(argv[1]);
    // std::string url("http://www.google.com/trends/fetchComponent?hl=en-US&date=today%203-m&q=debt,%2Fm%2F02lc8s,brexit&tz=Etc/GMT%2B4&content=1&cid=TIMESERIES_GRAPH_0&export=3");
    // std::string fname(argv[2]);
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
        // Muted until file out feature completed to pipe to file
        // std::cout << "\nGot successful response from " << url << std::endl;
        std::string jsonAsString = "";
        // Response looks good - done using Curl now.  Try to parse the results
        // and print them out.
        Json::Value jsonData;
        Json::Reader jsonReader;

        // START 'cleanJsonData' function
        // Removes Google nonsense from json returned as string
        std::smatch matches;
        regex_search(*httpData.get(), matches, std::regex("(\\{.*\\})", std::regex::optimize));
        jsonAsString += matches[1];

        // Customizations for string returned from Trends Server
        boost::erase_all(jsonAsString, "new Date");
        boost::replace_all(jsonAsString, "(", "[");
        boost::replace_all(jsonAsString, ")", "]");
        boost::erase_all(jsonAsString, ",,");
        // END 'cleanJsonData' function

        if (jsonReader.parse(jsonAsString, jsonData)) {
            // Muted until file out feature completed to pipe to file
            // std::cout << "Successfully parsed JSON data" << std::endl;
            // std::cout << "\nJSON data received:" << std::endl;
            
            // Store headers for CSV file
            std::string headString("");
            for(auto const& colus : jsonData["table"]["cols"]) {
                headString = headString + colus["label"].asString() + ",";
            }
            std::cout << headString << std::endl;

            // Array to store Trends data for printing:
            for(auto const& row : jsonData["table"]["rows"]) {

                // Ensure data present in row before printing
                if(row["c"].size() > 2) {
                    // Initialize empty string to store data for CSV
                    std::string rowString("");

                    // First element is always date; add this to rowString
                    std::string dateString(row["c"][0]["f"].asString());
                    boost::replace_all(dateString, ",", "");
                    rowString = rowString + dateString + ",";

                    // Within each day, get each datapoint (Exclude weekends here?)
                    for(int subindex = 1; subindex < row["c"].size(); subindex++) {
                        rowString = rowString + row["c"][subindex]["v"].asString() + ",";
                    }
                    rowString = rowString + "\n";

                    // Completed rowString printed to console
                    std::cout << rowString;
                }
                // std::cout << "FName == " << fname << std::endl;
            }
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