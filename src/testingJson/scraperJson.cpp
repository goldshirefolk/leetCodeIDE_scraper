#include <curl/curl.h>
#include <fstream>
#include <iostream>
#include <jsoncpp/json/json.h>
#include <string>

size_t WriteCallback(void *contents, size_t size, size_t nmemb, std::string *response) {
    size_t totalSize = size * nmemb;
    response->append((char *)contents, totalSize);
    return totalSize;
}

std::string makeGraphQLRequest(const std::string &query, const std::string &variables = "{}") {
    CURL *curl = curl_easy_init();
    std::string response;

    if (!curl)
        return "";

    // Prepare the GraphQL request
    std::string postData = "{\"query\":\"" + query + "\",\"variables\":" + variables + "}";

    // Escape the query (simplified)
    std::string escapedQuery;
    for (char c : query) {
        if (c == '\n')
            escapedQuery += "\\n";
        else if (c == '\"')
            escapedQuery += "\\\"";
        else
            escapedQuery += c;
    }

    postData = "{\"query\":\"" + escapedQuery + "\",\"variables\":" + variables + "}";

    struct curl_slist *headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, "User-Agent: Mozilla/5.0");

    curl_easy_setopt(curl, CURLOPT_URL, "https://leetcode.com/graphql");
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    CURLcode res = curl_easy_perform(curl);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    return response;
}

// Get problem list (50 problems at a time)
std::string getProblemList(int skip = 0, int limit = 50) {
    std::string query = R"(
        query problemsetQuestionList($categorySlug: String, $limit: Int, $skip: Int, $filters: QuestionListFilterInput) {
            problemsetQuestionList: questionList(
                categorySlug: $categorySlug
                limit: $limit
                skip: $skip
                filters: $filters
            ) {
                total: totalNum
                questions: data {
                    acRate
                    difficulty
                    frontendQuestionId: questionFrontendId
                    paidOnly: isPaidOnly
                    title
                    titleSlug
                    topicTags {
                        name
                    }
                }
            }
        }
    )";

    std::string variables = R"({
        "categorySlug": "",
        "skip": )" + std::to_string(skip) +
                            R"(,
        "limit": )" + std::to_string(limit) +
                            R"(,
        "filters": {}
    })";

    return makeGraphQLRequest(query, variables);
}

// Get specific problem details by title slug (like "two-sum")
std::string getProblemDetail(const std::string &titleSlug) {
    std::string query = R"(
        query questionData($titleSlug: String!) {
            question(titleSlug: $titleSlug) {
                questionId
                questionFrontendId
                title
                titleSlug
                content
                difficulty
                exampleTestcases
                codeSnippets {
                    lang
                    langSlug
                    code
                }
            }
        }
    )";

    std::string variables = R"({"titleSlug": ")" + titleSlug + "\"}";

    return makeGraphQLRequest(query, variables);
}

class stringExtractor {
private:
    static std::string extractToken(const std::string &string, const std::string &start_token, const std::string &end_token,
                                    int *newStartPos) {
        int start = string.find(start_token);

        if (start == std::string::npos) {
            std::cout << "\n\nCould not token : (" << start_token << "|" << end_token
                      << "), for : " << string << "\n\n";
            exit(1);
        }
        start += start_token.length();

        if (newStartPos)
            *newStartPos = start;

        std::string finalString = string.substr(start, string.find(end_token, start) - start);
        return finalString;
    }

public:
    static std::string nameFromLink(const std::string &link) {
        // std::string token = "problems/";
        // int start = link.find(token);

        // if (start == std::string::npos) {
        //     std::cout << "\n\nCould not find name in link!\n\n";
        //     return "";
        // }
        // start += token.length();

        // std::string finalString = link.substr(start, link.find('/', start) - start);
        // return finalString;

        return extractToken(link, "problems/", "/", NULL);
    }

    static std::string extractFromJson(std::string &json, const std::string &tag) {
    }
};

// std::string nameFromLink(std::string link) {
//     std::string token = "problems/";
//     int start = link.find(token);

//     if (start == std::string::npos) {
//         std::cout << "\n\nCould not find name in link!\n\n";
//         return "";
//     }
//     start += token.length();

//     std::string finalString = link.substr(start, link.find('/', start) - start);
//     return finalString;
// }

int main() {
    std::cout << "=== Testing LeetCode GraphQL API ===\n"
              << std::endl;

    std::string link;
    std::cin >> link;

    std::string problem_name = stringExtractor::nameFromLink(link);
    std::cout << problem_name;

    // // Test 1: Get problem list
    // std::cout << "1. Fetching problem list..." << std::endl;
    // std::string problemList = getProblemList(0, 5);  // First 5 problems
    // std::cout << "Problem list response size: " << problemList.size() << " bytes\n"
    //           << std::endl;

    // // Save raw response
    // std::ofstream listFile("problem_list.json");
    // listFile << problemList;
    // listFile.close();
    // std::cout << "Problem list saved to problem_list.json\n"
    //           << std::endl;

    // Test 2: Get specific problem details
    std::cout << "2. Fetching Two Sum problem details..." << std::endl;
    std::string problemDetail = getProblemDetail(problem_name);
    std::cout << "Problem detail response size: " << problemDetail.size() << " bytes\n"
              << std::endl;

    // // Save raw response
    // std::ofstream detailFile("two_sum_detail.json");
    // detailFile << problemDetail;
    // detailFile.close();
    // std::cout << "Problem detail saved to two_sum_detail.json\n"
    //           << std::endl;

    // // Show preview of responses
    // std::cout << "=== Problem List Preview ===" << std::endl;
    // std::cout << problemList.substr(0, 500) << "...\n"
    //           << std::endl;

    std::cout << "=== Problem Detail Preview ===" << std::endl;
    std::cout << problemDetail << "..." << std::endl;

    return 0;
}