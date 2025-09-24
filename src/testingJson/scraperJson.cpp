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

    // Escape query
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

// Get specific problem details by title
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
    LANG_CPP
} languages;

bool problem_has_images = false;
const std::string image_warning = "!!! Problem has images, it is best you use the link !!!";

std::string problem_id;
std::string problem_title;
std::string problem_difficulty;

const std::string newLine_token = "\\n";

const std::vector<std::string> genericDescTokens = {"\\u200b", "\\t"};
const std::vector<std::string> descSectionTokens = {"Example", "Constraints"};

const std::string exampleSymbol_start = "(";
const std::string exampleSymbol_end = ")";

const std::string constraintsLineSymbol_start = "(*) ";

const std::vector<std::string> descSectionSymbols_start = {"/*"};
const std::vector<std::string> descSectionSymbols_end = {"*/"};
const std::vector<char> descSectionSymbols = {'=', '#', '-'};
const std::vector<int> maxSectioNewLines = {2, 2, 1};

const int sectionMax = 1;
int descSectionSymbolLength = 32;
const int descLineMaxLength = 96;

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
        for (int i = 0; i < genericDescTokens.size(); i++) {
            int token_pos_start = 0;
            int token_pos_end = 0;

            int token_length = (genericDescTokens[i]).length();

            while ((token_pos_end = buffer.find(genericDescTokens[i], token_pos_start)) != std::string::npos) {
                buffer.erase(token_pos_end, token_length);
                token_pos_start = token_pos_end - token_length;
            }
        }
    }

    static void processSection(std::string &buffer, int *section, std::ostream &outStream, int lang) {

        // if (*section > sectionMax)
        //     return;

        if (*section <= sectionMax) {
            std::string token = descSectionTokens[*section];

            if (buffer.find(token) != std::string::npos) {
                std::string sectionSymbolBuffer;
                sectionSymbolBuffer.append(descSectionSymbolLength, descSectionSymbols[2]);

                outStream << sectionSymbolBuffer << '\n';
                (*section)++;
            }
        }

        /// Examples Section
        if (*section == 1) {
            if (buffer.find(descSectionTokens[0]) != std::string::npos) {
                buffer.insert(0, exampleSymbol_start);
                buffer.insert(buffer.length() - 1, exampleSymbol_end);
            }
        }
    }

    static void processNewLine(const std::string &description, int *start, int end, std::ostream &outStream,
                               const int maxNewLines) {
        int pos = end;
        int current = 0;
        while (pos < description.length() && description[pos] == newLine_token[0] && description[pos + 1] == newLine_token[1]) {
            if (current < maxNewLines)
                outStream << '\n';

            (pos) += 2;
            current++;
        }

        *start = pos;
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

    typedef enum {
        SECTION_START = 2,
        SECTION_EXAMPLES = 1,
        SECTION_CONSTRAINTS = 1
    } sectionMaxNewLines;

    static void exportProblemHeader(std::ostream &outStream, int lang) {
        outStream << descSectionSymbols_start[lang] << "\n";

        std::string startBuffer;

        startBuffer += problem_id + ". " + problem_title + " [" + problem_difficulty + "]";
        startBuffer = std::regex_replace(startBuffer, std::regex("\""), "");

        outStream << startBuffer << '\n';

        /// Show link
        startBuffer.clear();

        startBuffer = "https://leetcode.com/problems/";
        startBuffer += problem_title;
        startBuffer = std::regex_replace(startBuffer, std::regex(" "), "-");
        startBuffer = std::regex_replace(startBuffer, std::regex("\""), "");
        std::transform(startBuffer.begin(), startBuffer.end(), startBuffer.begin(), ::tolower);
        startBuffer += '/';
        outStream << startBuffer << '\n';

        if (problem_has_images) {
            outStream << '\n';
            outStream << image_warning;
            outStream << '\n';
        }

        outStream << descSectionSymbols_end[lang] << "\n\n\n";
    }

    static void exportDescription(std::string &description, std::ostream &outStream, int lang) {
        int start = 0;
        int end = 0;

        bool firstBuffer = true;

        int section = 0;

        outStream << descSectionSymbols_start[lang] << "\n";

        while ((end = description.find(newLine_token, start)) != std::string::npos) {
            std::string buffer = description.substr(start, end - start);

            detokenize(buffer);

            if (firstBuffer) {
                buffer.erase(0, 1);
                firstBuffer = false;
            }

            if (section == 0) {
                int bufferLength = buffer.length();

                while (bufferLength > descLineMaxLength) {
                    int erasePos = descLineMaxLength;
                    while (buffer[--erasePos] != ' ')
                        ;

                    outStream << buffer.substr(0, erasePos) << '\n';
                    buffer.erase(0, erasePos);
                    if (buffer[0] == ' ')
                        buffer.erase(0, 1);

                    bufferLength -= erasePos;

                    descSectionSymbolLength = erasePos;
                }

                if (bufferLength > descSectionSymbolLength) {
                    descSectionSymbolLength = bufferLength;
                }
            } else if (section == 1) {
                buffer = std::regex_replace(buffer, std::regex("\\\\"), "");
            } else if (section == 2) {  // constraints
                buffer.insert(0, constraintsLineSymbol_start);
            }

            processSection(buffer, &section, outStream, lang);

            outStream << buffer;

            int pos = 0;

            processNewLine(description, &start, end, outStream, maxSectioNewLines[section]);

            buffer.clear();
        }

        outStream << descSectionSymbols_end[lang] << '\n';
    }
};

std::string cleanHTML(const std::string &html) {
    std::string result = html;

    if (result.find("<img") != std::string::npos) {
        problem_has_images = true;
    }

    result = std::regex_replace(result, std::regex("<sup>"), " to the power of : ");

    /// Remove HTML tags
    std::regex tagRegex("<[^>]*>");
    result = std::regex_replace(result, tagRegex, "");

    /// Decode HTML entities
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

    problem_id = stringExtractor::extractFromJson(problemDetail, "\"questionId\":");
    std::cout << "Problem Id : " << problem_id << std::endl;

    problem_title = stringExtractor::extractFromJson(problemDetail, "\"title\":");
    std::cout << "Problem title : " << problem_title << std::endl;

    std::string content = stringExtractor::extractFromJson(problemDetail, "\"content\":");

    problem_difficulty = stringExtractor::extractFromJson(problemDetail, "\"difficulty\":");
    std::cout << "Problem difficulty : " << problem_difficulty << std::endl;

    std::cout << "\n\n";
    std::cout << "=========PROBLEM DESCRIPTION=========" << "\n";

    std::string clean_html_description = cleanHTML(content);

    std::ofstream cleansedHtml_out("cleansedHtmlDesc");
    cleansedHtml_out << clean_html_description;
    // std::cout << cleanHTML(content) << "\n\n";

    stringExtractor::exportProblemHeader(std::cout, LANG_CPP);
    stringExtractor::exportDescription(clean_html_description, std::cout, LANG_CPP);

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