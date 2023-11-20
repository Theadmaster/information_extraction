#ifndef CLASSIFIER_H
#define CLASSIFIER_H
#include "AhoCorasick.hpp"
#include "nlohmann/json.hpp"
#include <fstream>

using json = nlohmann::json;
using namespace std;

class Classifier {
private:
    /* path */

    // label
    string label_group_path, label_text_path, label_flag_path, label_num_path;
    // attr
    string attr_group_path;
    // value
    string value_num_path, value_text_path, value_flag_path, value_group_attr_path;

    /* data */
    vector<string> label_text_wds;
    vector<string> label_flag_wds;
    vector<string> label_num_wds;
    vector<string> label_group_wds;

    vector<string> attr_group_wds;

//    vector<string> value_num_wds;
    vector<string> value_text_wds;
    vector<string> value_flag_wds;
    vector<string> value_group_attr_wds;

public:
    // all information words
    vector<string> region_wds;

    // dict -> <word : label>
    map<string, string> wdtype_dict;

    // actree for searching, including all information
    CAhoCorasick region_tree;

    // for matching instructions
    vector<string> instructions_add, instructions_edit, instructions_complete, instructions_delete, instructions_start, instructions_end;

    // json data
    json en2options;
    json cn2en;
    Classifier() {

//         string parent_path = __fs::filesystem::current_path().parent_path().string();
//        cout << parent_path << endl;

        /* path load */
        label_text_path =  "/cpp/data/label_text.txt";
        label_flag_path = "../data/label_flag.txt";
        label_num_path = "../data/label_num.txt";
        label_group_path = "../data/label_group.txt";
        attr_group_path = "../data/attr_group.txt";
        // value_num_path = "./data/value_num.txt";
        value_text_path = "../data/value_text.txt";
        value_flag_path =  "../data/value_flag.txt";
        value_group_attr_path = "../data/value_group_attr.txt";

        /* data load */
//        readDictFile(label_text_wds, label_text_path);
//        readDictFile(label_flag_wds, label_flag_path);
//        readDictFile(label_num_wds, label_num_path);
//        readDictFile(label_group_wds, label_group_path);
//        readDictFile(attr_group_wds, attr_group_path);
//        // readDictFile(value_num_wds, value_num_path);
//        readDictFile(value_text_wds, value_text_path);
//        readDictFile(value_flag_wds, value_flag_path);
//        readDictFile(value_group_attr_wds, value_group_attr_path);

        /* construct instructions */
        instructions_add = { "增加", "添加", "新增", "新添", "加上", "新加" };
        instructions_edit = { "修改", "更改", "编辑", "改掉" };
        instructions_complete = { "保存", "提交" };
        instructions_delete = { "删掉", "删除", "去除", "删去", "去掉" };
        instructions_start = {"手术开始", "手术启动", "开始手术", "启动手术"};
        instructions_end = {"手术结束", "手术完成"};

        // readJson();

        /* data combine */
//        region_wds = mergeVector();

        /* build actree */
//        region_tree = build_actree(region_wds);

        /* build dict */
//        wdtype_dict = build_wdtype_dict();

        

    }

    ~Classifier() {};

    // apply data
    void loadData(string wd_name, vector<string> vecStr);

    // information extraction
    json classify(string sentence);

    // sentence preprocessing
    map<string, vector<string>> check_sentence(string sentence);

    // merge Vector
    vector<string> mergeVector();

    // read file and push
    int readDictFile(vector<string> &str, string path);

    // for test
    void printVector(vector<string> vec);

    // build actree
    CAhoCorasick build_actree(vector<string> vec);

    // build wdtype_dict
    map<string, string> build_wdtype_dict();

    // read json
    void readJson() {
        ifstream f("./data/map.json");
        ifstream f1("./data/map_cn2en.json");
        this->en2options = json::parse(f);
        this->cn2en = json::parse(f1);
    }

    CAhoCorasick getTree() {
        return region_tree;
    }

};

#endif