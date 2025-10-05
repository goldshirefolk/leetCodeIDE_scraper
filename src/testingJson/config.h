#pragma once
#include <string>
#include <vector>

#define LANGUAGE_COUNT 18

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
inline const std::vector<int> maxSectionNewLines = {2, 2, 1};

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

typedef enum {
    LANG_CPP,
    LANG_C,
    LANG_CSHARP,
    LANG_JAVA,
    LANG_PYTHON3,
    LANG_PYTHON,
    LANG_JAVASCRIPT,
    LANG_TYPESCRIPT,
    LANG_GO,
    LANG_KOTLIN,
    LANG_SWIFT,
    LANG_PHP,
    LANG_DART,
    LANG_SCALA,
    LANG_ELIXIR,
    LANG_ERLANG,
    LANG_RACKET,
    LANG_RUBY,
    LANG_RUST,
    LANG_INVALID
} languages;

inline const std::vector<std::string> languageTokens = {
    "cpp",         //  cpp
    "c",           //  c
    "csharp",      //  csharp
    "java",        //  java
    "python3",     //  python3
    "python",      //  python
    "javascript",  //  javascript
    "typescript",  //  typescript
    "golang",      //  go
    "kotlin",      //  kotlin
    "swift",       //  swift
    "php",         //  php
    "dart",        //  dart
    "scala",       //  scala
    "elixir",      //  elixir
    "erlang",      //  erlang
    "racket",      //  racket
    "rust",        //  rust
    "\n\nINVALID!!\n\n",
};

inline const std::vector<std::vector<languages>> languageStructureSimiliarities = {
    {LANG_JAVA, LANG_C, LANG_CPP, LANG_TYPESCRIPT, LANG_CSHARP, LANG_GO, LANG_KOTLIN, LANG_SWIFT, LANG_RUST, LANG_DART, LANG_SCALA},
    {}};

}  // namespace LeetcodeToolConfig