#include "config.h"
#include <curl/curl.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <jsoncpp/json/json.h>
#include <regex>
#include <string>
#include <vector>

using namespace LeetcodeToolConfig;

size_t WriteCallback(void *contents, size_t size, size_t nmemb, std::string *response) {
    size_t total_size = size * nmemb;
    response->append((char *)contents, total_size);
    return total_size;
}

std::string makeGraphQLRequest(const std::string &query, const std::string &variables = "{}") {
    CURL *curl = curl_easy_init();
    std::string response;

    if (!curl)
        return "";

    // Prepare the GraphQL request
    std::string post_data = "{\"query\":\"" + query + "\",\"variables\":" + variables + "}";

    // Escape query
    std::string escaped_query;
    for (char c : query) {
        if (c == '\n')
            escaped_query += "\\n";
        else if (c == '\"')
            escaped_query += "\\\"";
        else
            escaped_query += c;
    }

    post_data = "{\"query\":\"" + escaped_query + "\",\"variables\":" + variables + "}";

    struct curl_slist *headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, "User-Agent: Mozilla/5.0");

    curl_easy_setopt(curl, CURLOPT_URL, "https://leetcode.com/graphql");
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data.c_str());
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
    std::string query = getProblemsListQuery;

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
    std::string query = getProblemDetailsquery;

    std::string variables = R"({"titleSlug": ")" + titleSlug + "\"}";

    return makeGraphQLRequest(query, variables);
}

bool problem_has_images = false;

std::string problem_id;
std::string problem_title;
std::string problem_difficulty;

int desc_section_symbol_length = 32;

class stringExtractor {
private:
    static std::string extractToken(const std::string &string, const std::string &start_token, const std::string &end_token,
                                    int *newStartPos) {
        int start = string.find(start_token);

        if (start == std::string::npos) {
            // std::cout << "\n\nCould not token : (" << start_token << " | " << end_token
            //<< "), for : " << string << "\n\n";
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
        if (*section <= descSectionMax) {
            std::string token = descSectionTokens[*section];

            if (buffer.find(token) != std::string::npos) {
                std::string sectionSymbolBuffer;
                sectionSymbolBuffer.append(desc_section_symbol_length, descSectionSymbols[2]);

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
    } descSectionMaxNewLines;

    static void exportProblemHeader(std::ostream &outStream, int lang) {
        outStream << commentDelims[lang][COMMENT_DELIMS_START_INDX] << "\n";

        std::string start_buffer;

        start_buffer += problem_id + ". " + problem_title + " [" + problem_difficulty + "]";
        start_buffer = std::regex_replace(start_buffer, std::regex("\""), "");

        outStream << start_buffer << '\n';

        /// Show link
        start_buffer.clear();

        start_buffer = generatedLinkStart;
        start_buffer += problem_title;
        start_buffer = std::regex_replace(start_buffer, std::regex(" "), "-");
        start_buffer = std::regex_replace(start_buffer, std::regex("\""), "");
        std::transform(start_buffer.begin(), start_buffer.end(), start_buffer.begin(), ::tolower);
        start_buffer += '/';
        outStream << start_buffer << '\n';

        if (problem_has_images) {
            outStream << image_warning;
        }

        outStream << commentDelims[lang][COMMENT_DELIMS_END_INDX] << "\n\n\n";
    }

    static void exportDescription(std::string &description, std::ostream &outStream, int lang) {
        int start = 0;
        int end = 0;

        bool first_buffer = true;

        int section = 0;

        outStream << commentDelims[lang][COMMENT_DELIMS_START_INDX] << "\n";

        while ((end = description.find(newLine_token, start)) != std::string::npos) {
            std::string buffer = description.substr(start, end - start);

            detokenize(buffer);

            if (first_buffer) {
                buffer.erase(0, 1);
                first_buffer = false;
            }

            if (section == 0) {
                int buffer_length = buffer.length();

                while (buffer_length > descLineMaxLength) {
                    int erasePos = descLineMaxLength;
                    while (buffer[--erasePos] != ' ')
                        ;

                    outStream << buffer.substr(0, erasePos) << '\n';
                    buffer.erase(0, erasePos);
                    if (buffer[0] == ' ')
                        buffer.erase(0, 1);

                    buffer_length -= erasePos;

                    desc_section_symbol_length = erasePos;
                }

                if (buffer_length > desc_section_symbol_length) {
                    desc_section_symbol_length = buffer_length;
                }
            } else if (section == 1) {
                buffer = std::regex_replace(buffer, std::regex("\\\\"), "");
            } else if (section == 2) {  // constraints
                buffer.insert(0, constraintsLineSymbol_start);
            }

            processSection(buffer, &section, outStream, lang);

            outStream << buffer;

            processNewLine(description, &start, end, outStream, maxSectionNewLines[section]);

            buffer.clear();
        }

        outStream << commentDelims[lang][COMMENT_DELIMS_END_INDX] << "\n\n\n";
    }

    static void prepareCodeSnippet(std::string &description) {
        int start = description.find("code:", 0);
        int skipLength = sizeof("codep:p") + 2;
        description = description.substr(start + skipLength, description.size() - skipLength);

        int code_snippet_end = description.find("\"},{\"lang\"");
        description = description.substr(0, code_snippet_end);
    }

    static void exportCodeSnippet(std::string &description, std::ostream &outStream, languages chosen_language) {
        prepareCodeSnippet(description);

        // outStream << description << "\n\n";

        for (int i = 0; i < description.size(); i++) {
            if (i < description.size() - 1) {
                if (description[i] == '\\' && description[i + 1] == 'n') {
                    outStream << '\n';
                    i += 2;
                }

                if (i > description.size()) {
                    break;
                }

                if (chosen_language == LANG_PYTHON || chosen_language == LANG_PYTHON3) {
                    if (description[i] == '\\')
                        i++;
                }

                if (i > description.size()) {
                    break;
                }
            }
            outStream << description[i];
        }

        outStream << "\n\n\n";
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

    return result;
}

#define LANG_STRUCTURE_TYPE_C 0

languages getLanguageChar(std::istream &input_stream) {
    std::string lang;
    input_stream >> lang;

    for (char i = 0; i < LANGUAGE_COUNT; i++) {
        if (lang == languageTokens[i]) {
            return (languages)i;
        }
    }

    return LANG_INVALID;
}

namespace fs = std::filesystem;

fs::path createDir(const std::string &problem_name) {
    std::error_code ec;

    fs::path base = fs::current_path();
    fs::path current_new_dir = fs::path(problem_name);

    fs::path dir = base / current_new_dir;

    if (fs::create_directories(dir, ec)) {
        // std::cout << "Created: " << dir << '\n';
    } else if (ec) {
        std::cerr << "Error: " << ec.message() << '\n';
    } else {
        // std::cout << "Already exists: " << dir << '\n';
    }

    return dir;
}

std::ofstream createFileAndDir(std::string &problem_name, languages chosen_language) {

    fs::path file_path = createDir(problem_name);

    std::string code_file_name = problem_name;
    code_file_name.append(codeFileSufixes[chosen_language]);
    file_path /= code_file_name;
    std::ofstream code_file_path(file_path);

    if ((int)chosen_language < codeSnippetPrefixes.size()) {
        code_file_path << codeSnippetPrefixes[(int)chosen_language] << codeSnippetPrefixNewlines;
    }

    return code_file_path;
}

int main() {
    // std::cout << "=== Testing LeetCode GraphQL API ===\n"
    //<< std::endl;

    std::ifstream link_input("link");

    std::string link;
    link_input >> link;

    std::string problem_name = stringExtractor::nameFromLink(link);
    // std::cout << problem_name << std::endl;

    // // Test 1: Get problem list
    // //std::cout << "1. Fetching problem list..." << std::endl;
    // std::string problemList = getProblemList(0, 5);  // First 5 problems
    // //std::cout << "Problem list response size: " << problemList.size() << " bytes\n"
    //           << std::endl;

    // // Save raw response
    // std::ofstream listFile("problem_list.json");
    // listFile << problemList;
    // listFile.close();
    // //std::cout << "Problem list saved to problem_list.json\n"
    //           << std::endl;

    languages chosen_language = getLanguageChar(link_input);
    // std::cout << "\n\n\nChosen lang : " << languageTokens[chosen_language] << "\n\n\n";

    // std::cout << "Fetching problem details..." << std::endl;

    std::string problem_detail = getProblemDetail(problem_name);
    std::string problemDetail_copy = problem_detail;
    // std::cout << "Problem detail response size: " << problem_detail.size() << " bytes\n"
    //<< std::endl;

    std::ofstream rawDesc_out("rawDesc");
    rawDesc_out << problem_detail;

    std::ofstream code_file = createFileAndDir(problem_name, chosen_language);

    // std::cout << "\n\n\n\n\n";
    // std::cout << "=========EXTRACTED CONTENT=========" << "\n\n";

    problem_id = stringExtractor::extractFromJson(problem_detail, "\"questionId\":");
    // std::cout << "Problem Id : " << problem_id << std::endl;

    problem_title = stringExtractor::extractFromJson(problem_detail, "\"title\":");
    // std::cout << "Problem title : " << problem_title << std::endl;

    std::string content = stringExtractor::extractFromJson(problem_detail, "\"content\":");

    problem_difficulty = stringExtractor::extractFromJson(problem_detail, "\"difficulty\":");
    // std::cout << "Problem difficulty : " << problem_difficulty << std::endl;

    // std::cout << "\n\n";
    // std::cout << "=========PROBLEM DESCRIPTION=========" << "\n";

    std::string clean_html_description = cleanHTML(content);

    std::ofstream cleansedHtml_out("cleansedHtmlDesc");
    cleansedHtml_out << clean_html_description;
    // //std::cout << cleanHTML(content) << "\n\n";

    //============================================================================================================================================

    stringExtractor::exportProblemHeader(code_file, (int)chosen_language);

    //============================================================================================================================================
    std::string codeToken = "\"langSlug\":";
    codeToken.append("\"");
    codeToken.append(languageTokens[chosen_language]);
    codeToken.append("\"");

    // std::cout << "TOKEN : " << codeToken;

    std::string problem_code = stringExtractor::extractFromJson(problemDetail_copy, codeToken);
    // //std::cout << "\n\n"
    //           << problemDetail_copy << "\n\n";
    // std::cout << "\n\n\n";
    stringExtractor::exportCodeSnippet(problemDetail_copy, code_file, chosen_language);
    // std::cout << "\n\n\n";

    //============================================================================================================================================

    stringExtractor::exportDescription(clean_html_description, code_file, (int)chosen_language);

    //============================================================================================================================================

    /// Get code

    // //std::cout << problemDetail_copy;

    // // Save raw response
    // std::ofstream detailFile("two_sum_detail.json");
    // detailFile << problem_detail;
    // detailFile.close();
    // //std::cout << "Problem detail saved to two_sum_detail.json\n"
    //           << std::endl;

    // // Show preview of responses
    // //std::cout << "=== Problem List Preview ===" << std::endl;
    // //std::cout << problemList.substr(0, 500) << "...\n"
    //           << std::endl;

    // //std::cout
    //     << "=== Problem Detail Preview ===" << std::endl;
    // //std::cout << problem_detail << "..." << std::endl;

    return 0;
}