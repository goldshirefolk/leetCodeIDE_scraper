#pragma once
#include <string>
#include <vector>

#define LANGUAGE_COUNT 18

#define COMMENT_DELIMS_START_INDX 0
#define COMMENT_DELIMS_END_INDX 1

#define IDE_CONFIG_VAL_VSCODE 0
#define IDE_CONFIG_VAL_NVIM 1

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

inline const std::vector<std::string> codeFileSufixes = {
    ".cpp",
    ".c",
    ".cs",
    ".java",
    ".py",
    ".py",
    ".js",
    ".ts",
    ".go",
    ".kt",
    ".swift",
    ".php",
    ".dart",
    ".scala",
    ".ex",
    ".erl",
    ".rkt",
    ".rs",
    "",
};

inline const std::vector<std::vector<std::string>> commentDelims = {
    {"/*", "*/"},          // cpp
    {"/*", "*/"},          // c
    {"/*", "*/"},          // csharp
    {"/*", "*/"},          // java
    {"\"\"\"", "\"\"\""},  // python3
    {"\"\"\"", "\"\"\""},  // python
    {"/*", "*/"},          // javascript
    {"/*", "*/"},          // typescript
    {"/*", "*/"},          // golang
    {"/*", "*/"},          // kotlin
    {"/*", "*/"},          // swift
    {"/*", "*/"},          // php
    {"/*", "*/"},          // dart
    {"/*", "*/"},          // scala
    {"", ""},              // elixir (no block comments)
    {"", ""},              // erlang (no block comments)
    {"#|", "|#"},          // racket
    {"/*", "*/"},          // rust
    {"", ""}               // INVALID!!
};

inline const std::string codeSnippetPrefixNewlines = "\n\n\n";

inline const std::vector<std::string> codeSnippetPrefixes = {
    "#include<iostream>\n#include<vector>\n#include<algorithm>\n#include<list>\n\nusing namespace std;",
    "#include <stdio.h>\n#include <stdlib.h>\n#include <string.h>\n#include <math.h>\n#include <stdbool.h>\n#include <stdint.h>",
    "using System;\nusing System.Collections.Generic;\nusing System.Linq;\nusing System.Text;",
    "import java.util.*;\nimport java.io.*;",
    "from typing import List, Optional, Tuple, Dict, Set\nimport sys\nimport math\nimport collections\nimport heapq\nimport bisect\nimport itertools",
    "import sys\nimport math\nimport collections\nimport heapq\nimport bisect\nimport itertools",
    "'use strict';",
    "export {};",
    "package main\n\nimport (\n    \"bufio\"\n    \"fmt\"\n    \"os\"\n    \"strconv\"\n    \"strings\"\n    \"math\"\n    \"sort\"\n    \"container/heap\"\n)",
    "import java.util.*\nimport kotlin.math.*",
    "import Foundation",
    "<?php\ndeclare(strict_types=1);",
    "import 'dart:math';\nimport 'dart:collection';\nimport 'dart:io';",
    "import scala.collection.mutable\nimport scala.collection.immutable\nimport scala.math._",
    "import Enum\nimport MapSet\nimport Bitwise\nrequire Integer",
    "-module(solution).",
    "#lang racket",
    "use std::cmp::{min, max, Ordering};\nuse std::collections::{HashMap, HashSet, BTreeMap, BTreeSet, VecDeque, BinaryHeap};\nuse std::io::{self, Read};",
    "",
};

inline const std::vector<std::string> ideLaunchCommands = {
    "code",  // vscode
    "nvim"   // nvim
};
}  // namespace LeetcodeToolConfig