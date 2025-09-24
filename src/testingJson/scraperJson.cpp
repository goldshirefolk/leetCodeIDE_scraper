#include <curl/curl.h>
#include <fstream>
#include <iostream>
#include <jsoncpp/json/json.h>
#include <regex>
#include <string>
#include <vector>

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

typedef enum {
    LANG_CPP,
};

const std::string newLine_token = "\\n";
const std::vector<std::string> genericTokens = {"\\u200b", "\\t"};

class stringExtractor {
private:
    static std::string extractToken(const std::string &string, const std::string &start_token, const std::string &end_token,
                                    int *newStartPos) {
        int start = string.find(start_token);

        if (start == std::string::npos) {
            std::cout << "\n\nCould not token : (" << start_token << " | " << end_token
                      << "), for : " << string << "\n\n";
            exit(1);
        }
        start += start_token.length();

        int end = string.find(end_token, start);
        std::string finalString = string.substr(start, end - start);

        if (newStartPos)
            *newStartPos = end;
        return finalString;
    }

    static void detokenize(std::string &buffer) {

        /// Remove all the possible tokens we encounter
        for (int i = 0; i < genericTokens.size(); i++) {
            int token_pos_start = 0;
            int token_pos_end = 0;

            int token_length = (genericTokens[i]).length();

            while ((token_pos_end = buffer.find(genericTokens[i], token_pos_start)) != std::string::npos) {
                buffer.erase(token_pos_end, token_length);
                token_pos_start = token_pos_end - token_length;
            }
        }
    }

    static void processNewLine(const std::string &description, int *start, int end, int *pos, std::ostream &outStream) {
        *pos = end;
        while (*pos < description.length() &&
               description[*pos] == newLine_token[0] && description[*pos + 1] == newLine_token[1]) {
            outStream << '\n';

            (*pos) += 2;
        }

        *start = *pos;
    }

public:
    static int chosen_lang;

    static std::string nameFromLink(std::string &link) {
        return extractToken(link, "problems/", "/", NULL);
    }

    static std::string extractFromJson(std::string &json, const std::string &tag) {
        int newPos = 0;
        std::string json_extracted = extractToken(json, tag, ",\"", &newPos);

        json.erase(0, newPos);
        return json_extracted;
    }

    static void exportDescription(std::string &description, std::ostream &outStream) {
        int start = 0;
        int end = 0;
        std::string newLine_token = "\\n";

        while ((end = description.find(newLine_token, start)) != std::string::npos) {
            std::string buffer = description.substr(start, end - start);
            // int token_pos_start = 0;
            // int token_pos_end = 0;

            // // remove all the possible tokens we encounter
            // for (int i = 0; i < genericTokens.size(); i++) {
            //     int token_length = (genericTokens[i]).length();

            //     while ((token_pos_end = buffer.find(genericTokens[i], token_pos_start)) != std::string::npos) {
            //         buffer.erase(token_pos_end, token_length);
            //         token_pos_start = token_pos_end - token_length;
            //     }
            // }

            detokenize(buffer);

            outStream << buffer;

            // process newline
            // bool printed_newLine = 0;
            // int pos = end;
            // while (pos < description.length() &&
            //        description[pos] == newLine_token[0] && description[pos + 1] == newLine_token[1]) {
            //     outStream << '\n';
            //     printed_newLine = 1;

            //     pos += 2;
            // }

            int pos = 0;

            processNewLine(description, &start, end, &pos, outStream);

            // start = pos;
        }
    }
};

std::string cleanHTML(const std::string &html) {
    std::string result = html;

    // Remove HTML tags
    std::regex tagRegex("<[^>]*>");
    result = std::regex_replace(result, tagRegex, "");

    // Decode HTML entities
    result = std::regex_replace(result, std::regex("&amp;"), "&");
    result = std::regex_replace(result, std::regex("&lt;"), "<");
    result = std::regex_replace(result, std::regex("&gt;"), ">");
    result = std::regex_replace(result, std::regex("&quot;"), "\"");
    result = std::regex_replace(result, std::regex("&#39;"), "'");
    result = std::regex_replace(result, std::regex("&nbsp;"), " ");

    // Remove extra whitespace
    // result = std::regex_replace(result, std::regex("\\s+"), " ");

    return result;
}

int main() {
    std::cout << "=== Testing LeetCode GraphQL API ===\n"
              << std::endl;

    std::ifstream linkInput("link");

    std::string link;
    linkInput >> link;

    std::string problem_name = stringExtractor::nameFromLink(link);
    std::cout << problem_name << std::endl;

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

    std::cout << "Fetching problem details..." << std::endl;
    std::string problemDetail = getProblemDetail(problem_name);
    std::cout << "Problem detail response size: " << problemDetail.size() << " bytes\n"
              << std::endl;

    std::ofstream rawDesc_out("rawDesc");
    rawDesc_out << problemDetail;

    std::cout << "\n\n\n\n\n";
    std::cout << "=========EXTRACTED CONTENT=========" << "\n\n";

    std::string id = stringExtractor::extractFromJson(problemDetail, "\"questionId\":");
    std::cout << "Problem Id : " << id << std::endl;

    std::string title = stringExtractor::extractFromJson(problemDetail, "\"title\":");
    std::cout << "Problem title : " << title << std::endl;

    std::string content = stringExtractor::extractFromJson(problemDetail, "\"content\":");

    std::string difficulty = stringExtractor::extractFromJson(problemDetail, "\"difficulty\":");
    std::cout << "Problem difficulty : " << difficulty << std::endl;

    std::cout << "\n\n";
    std::cout << "=========PROBLEM DESCRIPTION=========" << "\n";

    std::string clean_html_description = cleanHTML(content);

    std::ofstream cleansedHtml_out("cleansedHtmlDesc");
    cleansedHtml_out << clean_html_description;
    // std::cout << cleanHTML(content) << "\n\n";

    stringExtractor::exportDescription(clean_html_description, std::cout);

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

    // std::cout
    //     << "=== Problem Detail Preview ===" << std::endl;
    // std::cout << problemDetail << "..." << std::endl;

    return 0;
}