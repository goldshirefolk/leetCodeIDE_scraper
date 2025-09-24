#pragma once
#include <string>
#include <vector>

namespace LeetcodeToolConfig {
//==GENERAL==========================================================================================================================================
inline const std::string image_warning = "\n!!! Problem has images, it is best you use the link !!!\n";

inline const std::string newLine_token = "\\n";

inline const std::vector<std::string> genericDescTokens = {"\\u200b", "\\t"};
inline const std::vector<std::string> descSectionTokens = {"Example", "Constraints"};

inline const std::string exampleSymbol_start = "(";
inline const std::string exampleSymbol_end = ")";

inline const std::string constraintsLineSymbol_start = "(*) ";

inline const std::vector<std::string> descSectionSymbols_start = {"/*"};
inline const std::vector<std::string> descSectionSymbols_end = {"*/"};
inline const std::vector<char> descSectionSymbols = {'=', '#', '-'};
inline const std::vector<int> maxSectioNewLines = {2, 2, 1};

inline const int descLineMaxLength = 96;
inline const int descSectionMax = 1;
//==QUERIES=========================================================================================================================================
inline const std::string generatedLinkStart = "https://leetcode.com/problems/";

inline const std::string getProblemDetailsquery = R"(
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

inline const std::string getProblemsListQuery = R"(
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

//==================================================================================================================================================

}  // namespace LeetcodeToolConfig