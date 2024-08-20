#include "KeyRecomander.h"

using std::size_t;
using std::vector;
using std::cout;

// vector<string> KeyRecomander::generateCandidates(vector<string> splited_words) {
//     set<int> resultSet;
//     for (const auto &word : splited_words) {
//         auto it = _Index.find(word);
//         if (it != _Index.end()) {
//             if (resultSet.empty()) {
//                 resultSet = it->second;
//             } else {
//                 set<int> tempSet;
//                 std::set_union(resultSet.begin(), resultSet.end(), it->second.begin(), it->second.end(), std::inserter(tempSet, tempSet.begin()));
//                 resultSet = tempSet;
//             }
//         }
//     }

//     vector<string> recommendations;
//     for (int idx : resultSet) {
//         if (idx >= 0 && idx < _Dict.size()) {
//             recommendations.push_back(_Dict[idx].first);
//         }
//     }
//     return recommendations;
// }

// vector<string> KeyRecomander::sort(vector<string> recommendations) {
//     // 这里可以实现一个排序逻辑，比如按频率排序
//     std::sort(recommendations.begin(), recommendations.end());
//     return recommendations;
// }


vector<string> KeyRecomander::split_query(string query) {
    cout << "splitting query: \n";

    vector<string> keywords;
    string current;

    for (size_t i = 0; i < query.size(); ++i) {
        char c = query[i];
        if ((unsigned char)c >= 0x80) {  // 非ASCII字符（可能是中文字符）
            // 处理中文字符（假定UTF-8编码，中文字符占用3个字节）
            string chinese_char;
            chinese_char += c;
            if (i + 1 < query.size()) chinese_char += query[i + 1];
            if (i + 2 < query.size()) chinese_char += query[i + 2];
            keywords.push_back(chinese_char);
            i += 2;  // 跳过下两个字节（中文字符已经处理）
        } else if (isalnum(c)) {
            // 英文字符或数字
            keywords.push_back(string(1, c));
        } else {
            // 跳过标点符号或空白字符
            continue;
        }
    }

    return keywords;
}