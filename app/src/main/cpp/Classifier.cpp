#include "Classifier.hpp"
#include <fstream>
#include <cstdio>
#include <iostream>
#include "nlohmann/json.hpp"
#include <regex>
using namespace std;
using json = nlohmann::json;

// sort values based on <pos>
auto posComparator = [](const json& a, const json& b) {
    return a["pos"] < b["pos"];
};

// key_value matching encapsulation
void key_value_matching(json& label_json, json& value_json, json& res_dict) {
    int min_distance, distance, min_index;
    try {
        for (size_t i=0; i<label_json.size(); ++i) {
            min_distance = 65535;
            for (size_t j=0; j<value_json.size(); ++j) {
                if (value_json[j]["matched"].get<int>() == 1) {
                    continue;
                }
                distance = abs(label_json[i]["pos"].get<int>() - value_json[j]["pos"].get<int>());
                // cout << "distance: "  << distance << "| label-pos: " << label_json[i]["pos"].get<int>()  << "| value-pos: "<< value_json[j]["pos"].get<int>() << endl;
                if (distance < min_distance) {
                    min_distance = distance;
                    min_index = j;
                }
            }
            // cout << "min_distance: " << min_distance << " min_index:" << min_index << endl;
            label_json[i]["matched"] = 1;
            // 有可能错
            value_json[min_index]["matched"] = 1;
            res_dict["data"][label_json[i]["value"].get<string>()] = value_json[min_index]["value"].get<string>();
            // cout << "打印一下： " << label_json.dump() << endl;
            
        }
    } catch(exception& e) {
            cerr << "key-value matching 异常: " << e.what() << endl;
    }
}

// 分类
json Classifier::classify(string sentence) {
    json res_dict;

    map<string, vector<string>> final_dict = check_sentence(sentence);

    // for test print final_dict
    // for (auto const &i: final_dict) {
    //     cout << "{" << i.first << ": " << "[ ";
    //     for (auto const & j : i.second) {
    //         cout << j << ", " ;
    //     }
    //     cout << " ]" << "}" << endl;
    // }

    // default is routine which means not group
    // construct data structure when group
    res_dict["type"] = "routine";
    regex pattern("instructions_\\w+");
    for (auto const &i: final_dict) {
        if (i.second[0] == "label_group") {
            res_dict["type"] = "group";
            res_dict["entity"] = i.first;
        }
        if (regex_match(i.second[0], pattern)) {
            res_dict["action"] = i.second[0];
        }
    }

    json match_dict;
    // transform json for matching better
    for (auto const & i : final_dict) {
        string type = i.second[0];
        for (int j=1; j < i.second.size(); j++) {
            // 遍历多次出现的keyword
            // cout << "出现的keyword: " << i.second[j] << " | j: " << j << endl;
            json temp_j = {{"pos", stoi(i.second[j])}, {"value", i.first}, {"matched", 0}};
            match_dict[type].push_back(temp_j);
        }
    }
    
    for (auto it = match_dict.begin(); it != match_dict.end(); ++it) {
        sort(match_dict[it.key()].begin(), match_dict[it.key()].end(), posComparator);
        // std::cout << "Key: " << it.key() << ", Value: " << it.value() << std::endl;
    }
    // print for test
    // cout << match_dict.dump() << endl;

    // label_flag matching 
    if (match_dict.find("label_flag") != match_dict.end()) {
        key_value_matching(match_dict["label_flag"], match_dict["value_flag"], res_dict);
    }

    // label_num matching 
    if (match_dict.find("label_num") != match_dict.end()) {
        key_value_matching(match_dict["label_num"], match_dict["value_num"], res_dict);
    }

    // label_text matching
    if (match_dict.find("label_text") != match_dict.end()) {
        key_value_matching(match_dict["label_text"], match_dict["value_text"], res_dict);
    }

    // label_group matching
    if (match_dict.find("label_group") != match_dict.end()) {
        json merge_json = match_dict["value_text"];
        merge_json.insert(merge_json.end(), match_dict["value_num"].begin(),match_dict["value_num"].end());
        merge_json.insert(merge_json.end(), match_dict["value_flag"].begin(),match_dict["value_flag"].end());
        key_value_matching(match_dict["attr_group"], merge_json, res_dict);
    }
    
    // cout << "match_dict:" << endl <<  match_dict.dump() << endl;
    cout << "res_dict:" << endl << res_dict.dump() << endl;
    

    return res_dict;
    
};

// strNum1 是 strNum2 的前缀
bool isPrefix(string strNum1, string strNum2) {

    // 如果 num2 的长度小于 num1，则 num1 不可能是 num2 的前缀
    if (strNum1.length() > strNum2.length()) {
        return false;
    }

    // 检查 num1 是否是 num2 的前缀
    for (size_t i = 0; i < strNum1.length(); ++i) {
        if (strNum1[i] != strNum2[i]) {
            return false;
        }
    }

    return true;
}

bool compareByLength(const std::string& a, const std::string& b) {
    return a.length() < b.length();
}

// 文字预处理
map<string, vector<string>> Classifier::check_sentence(string sentence) {
    // 转小写
    for (char &c : sentence) {
        c = std::tolower(c);
    }
    map<string, vector<string>> final_dict;
    set<string> res = region_tree.SearchPattern(sentence);

    for (auto const i: res) {
        // for test
        // cout << "check_sentence:" << i << endl;
        string type = wdtype_dict[i];
        if (type != "value_num" && type != "value_flag") {

            final_dict[i].push_back(wdtype_dict[i]);
            size_t position = sentence.find(i);
            final_dict[i].push_back(to_string(position));

        }
    }
    // find flag position
    try {
        for (const string & key : value_flag_wds) {
            size_t pos = sentence.find(key);
            while(pos != string::npos) {
                // cout << "flag匹配到= " << key << ", 位置为= " << pos << endl;
                if (final_dict.find(key) == final_dict.end()) {
                    final_dict[key].push_back("value_flag");
                    final_dict[key].push_back(to_string(pos));
                } else if (final_dict[key][1] != to_string(pos)) {
                    final_dict[key].push_back(to_string(pos));
                }
                pos = sentence.find(key, pos + 1);
            }
        }
    }
    catch(const std::exception& e) {
        std::cerr << "find flag position: " << e.what() << '\n';
    }

    // find num position
    size_t found = sentence.find_first_of("0123456789");
    string currentNumStr = "";
    size_t lastfound = found, startfound;
    
    while (found != string::npos) {
        char digit = sentence[found];
        // cout << "匹配到= " << digit << ", 位置为= " << found << endl;
        if (lastfound == found) {
            // 第一次匹配
            currentNumStr += digit;
            startfound = found;
            // cout << "第一次匹配:" << currentNumStr << endl;
        } else if (lastfound+1 == found) {
            // 连续数字
            currentNumStr += digit;
            // cout << "连续数字:" << currentNumStr << endl;
        } else {
            // 下一串数字开始，保存上一次匹配到的完整数字字符串
            // cout << "保存:" << currentNumStr << endl;
            final_dict[currentNumStr].push_back("value_num");
            final_dict[currentNumStr].push_back(to_string(startfound));
            // 初始化 currentNumStr
            currentNumStr = "";
            currentNumStr += digit;
            startfound = found;
            // cout << "以保存上一part 下一趴:" << currentNumStr << endl;
        }
        lastfound = found;
        found = sentence.find_first_of("0123456789", found + 1);
    }
    
    if (currentNumStr!="" && final_dict.find(currentNumStr) == final_dict.end()) {
        // final_dict 中 key 不存在，则加入
        final_dict[currentNumStr].push_back("value_num");
        final_dict[currentNumStr].push_back(to_string(startfound));
    } else if (currentNumStr!="" && final_dict[currentNumStr][1] != to_string(startfound) ) {
        // key 存在，则判断位置是否相同，不同则加入
        // final_dict[currentNumStr].push_back("value_num");
        final_dict[currentNumStr].push_back(to_string(startfound));
    }

    // remove text duplicates
    map<string, vector<string>> check_dict;
    for (auto i :final_dict) {
        for (int j=1; j<i.second.size(); j++) {
            check_dict[i.second[j]].push_back(i.first);
        }
    }
    vector<string> wait_for_erase;
    for (auto i :check_dict) {
        if (i.second.size() > 1) {
            cout << "找到相同=" << i.second[0] << ", " << i.second[1] << endl;
            for (const string item : i.second) {
                wait_for_erase.push_back(item);
            }
        }
    }
    sort(wait_for_erase.begin(), wait_for_erase.end(), compareByLength);
    if (wait_for_erase.size() > 0) {
        wait_for_erase.pop_back();
    }
    for (string i : wait_for_erase) {
        cout << "遍历=" << i << endl;
        auto it = final_dict.find(i);
        final_dict.erase(it);
    }
    return final_dict;
}

// 合并原始数据
vector<string> Classifier::mergeVector() {
    vector<string> str;
    str.insert(str.end(), label_text_wds.begin(), label_text_wds.end());
    str.insert(str.end(), label_flag_wds.begin(), label_flag_wds.end());
    str.insert(str.end(), label_num_wds.begin(), label_num_wds.end());
    str.insert(str.end(), label_group_wds.begin(), label_group_wds.end());
    str.insert(str.end(), attr_group_wds.begin(), attr_group_wds.end());
    // str.insert(str.end(), value_num_wds.begin(), value_num_wds.end());
    str.insert(str.end(), value_text_wds.begin(), value_text_wds.end());
    str.insert(str.end(), value_flag_wds.begin(), value_flag_wds.end());
    str.insert(str.end(), value_group_attr_wds.begin(), value_group_attr_wds.end());

    str.insert(str.end(), instructions_add.begin(), instructions_add.end());
    str.insert(str.end(), instructions_edit.begin(), instructions_edit.end());
    str.insert(str.end(), instructions_complete.begin(), instructions_complete.end());
    str.insert(str.end(), instructions_delete.begin(), instructions_delete.end());
    str.insert(str.end(), instructions_start.begin(), instructions_start.end());
    str.insert(str.end(), instructions_end.begin(), instructions_end.end());

    return str;
}

// 读取数据
int Classifier::readDictFile(vector<string> &str, string path) {
//    ifstream ifs;
//    string line;
//    ifs.open(path);
////        ifs.imbue(locale("zh_CN.UTF-8"));
//    if (!ifs.is_open()) {
//        cerr << "无法打开文件" << endl;
//        return 1;
//    }
//    while(getline(ifs, line)){
//        str.push_back( line);
//    };
//    ifs.close();
//
//    return 0;
    FILE *file;
    file = fopen(path.c_str(), "r");
    if (file == NULL) {
        cout << "无法打开文件" << path << endl;
        return 1;
    }
    char line[256];
    while(fgets(line, sizeof(line), file) != NULL) {
        str.push_back(line);
    }
    fclose(file);
    return 0;

}

// for test 
void Classifier::printVector(vector<string> vec) {
    for (const string & strline : vec) {
        std::cout << strline << std::endl;
    }
}

// 构建actree
CAhoCorasick Classifier::build_actree(vector<string> vec) {
    CAhoCorasick cAhoCorasick(false);
    for (string strline : vec) {
        cAhoCorasick.AddWord(strline);
    }
    cAhoCorasick.RefreshRedirectState();
    region_tree.Destroy();
    return cAhoCorasick;
}

// 实体标签字典构建
map<string, string> Classifier::build_wdtype_dict() {
    map<string, string> wd_dict;
    for (const string& wd : region_wds) {
        // cout << wd << endl;
        wd_dict[wd] = "";
        if (find(label_text_wds.begin(), label_text_wds.end(), wd) != label_text_wds.end())
            wd_dict[wd] = "label_text";
        if (find(label_flag_wds.begin(), label_flag_wds.end(), wd) != label_flag_wds.end())
            wd_dict[wd] = "label_flag";
        if (find(label_num_wds.begin(), label_num_wds.end(), wd) != label_num_wds.end())
            wd_dict[wd] = "label_num";
        if (find(label_group_wds.begin(), label_group_wds.end(), wd) != label_group_wds.end())
            wd_dict[wd] = "label_group";
        if (find(attr_group_wds.begin(), attr_group_wds.end(), wd) != attr_group_wds.end())
            wd_dict[wd] = "attr_group";
        // if (find(value_num_wds.begin(), value_num_wds.end(), wd) != value_num_wds.end())
        //     wd_dict[wd] = "value_num";
        if (find(value_text_wds.begin(), value_text_wds.end(), wd) != value_text_wds.end())
            wd_dict[wd] = "value_text";
        if (find(value_flag_wds.begin(), value_flag_wds.end(), wd) != value_flag_wds.end())
            wd_dict[wd] = "value_flag";
        if (find(value_group_attr_wds.begin(), value_group_attr_wds.end(), wd) != value_group_attr_wds.end())
            wd_dict[wd] = "value_group_attr";

        if (find(instructions_add.begin(), instructions_add.end(), wd) != instructions_add.end())
            wd_dict[wd] = "instructions_add";
        if (find(instructions_edit.begin(), instructions_edit.end(), wd) != instructions_edit.end())
            wd_dict[wd] = "instructions_edit";
        if (find(instructions_complete.begin(), instructions_complete.end(), wd) != instructions_complete.end())
            wd_dict[wd] = "instructions_complete";
        if (find(instructions_delete.begin(), instructions_delete.end(), wd) != instructions_delete.end())
            wd_dict[wd] = "instructions_delete";
        if (find(instructions_start.begin(), instructions_start.end(), wd) != instructions_start.end())
            wd_dict[wd] = "instructions_start";
        if (find(instructions_end.begin(), instructions_end.end(), wd) != instructions_end.end())
            wd_dict[wd] = "instructions_end";
    }
    // for (auto const &pair: wd_dict) {
    //     std::cout << "{" << pair.first << ": " << pair.second << "}\n";
    // }
    return wd_dict;
}

void Classifier::loadData(string wd_name, vector<string> vecStr) {
    if (wd_name == "label_text")
        label_text_wds = vecStr;
    if (wd_name == "label_flag")
        label_flag_wds = vecStr;
    if (wd_name == "label_num")
        label_num_wds = vecStr;
    if (wd_name == "label_group")
        label_group_wds = vecStr;
    if (wd_name == "value_text")
        value_text_wds = vecStr;
    if (wd_name == "value_flag")
        value_flag_wds = vecStr;
    if (wd_name == "value_group_attr")
        value_group_attr_wds = vecStr;
    if (wd_name == "attr_group")
        attr_group_wds = vecStr;
}
