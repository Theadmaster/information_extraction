#include <jni.h>
#include <android/asset_manager_jni.h>
#include <android/asset_manager.h>
#include <sstream>
#include <fstream>
#include "Classifier.hpp"
#include "nlohmann/json.hpp"
#include <iostream>
#include <filesystem>
#include <unistd.h>

using json = nlohmann::json;
using namespace std;

Classifier classifier;

// 获取文件名前缀
std::string removeFileExtension(const std::string& filename) {
    size_t lastDotPosition = filename.find_last_of(".");
    if (lastDotPosition != std::string::npos) {
        return filename.substr(0, lastDotPosition);
    } else {
        // 如果没有找到点（.），则返回原始的文件名
        return filename;
    }
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_example_information_1extraction_1app_MainActivity_stringFromJNI(
        JNIEnv* env,
        jobject /* this */, jstring str) {

    std::string hello = "Hello from C++";
    const char* nativeString = env->GetStringUTFChars(str, NULL);
    std::string text = nativeString;
    cout << "收到的sentence是：" << text << endl;

    json res = classifier.classify(text);
    std::string res_string = res.dump();
    return env->NewStringUTF(res_string.c_str());
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_example_information_1extraction_1app_MainActivity_loadModel(
        JNIEnv* env,
        jobject,
        jobject assetManager){
    AAssetManager *mgr = AAssetManager_fromJava(env, assetManager);

    // 获取 assets 下的文件列表
    const char *subdirectory = "text";  // assets 子目录
    AAssetDir *assetDir = AAssetManager_openDir(mgr, subdirectory);

    // 存储文件
    std::map<std::string, vector<string>> fileContents;

    if (assetDir != nullptr) {
        const char *filename;
        std::stringstream contentStream;

        // 遍历文件列表
        while ((filename = AAssetDir_getNextFileName(assetDir)) != nullptr) {
            // 打开文件
            AAsset *asset = AAssetManager_open(mgr, (std::string(subdirectory) + "/" + filename).c_str(), AASSET_MODE_BUFFER);
            if (asset != nullptr) {
                const void *data = AAsset_getBuffer(asset);
                off_t size = AAsset_getLength(asset);

                // 将文件内容写入字符串
                std::string content(static_cast<const char *>(data), size);

                // 对文本的"\n"分割,并存储到map中
                string line;
                istringstream iss(content);
                while (getline(iss, line))
                    fileContents[removeFileExtension(filename)].push_back(line);

                // 关闭文件
                AAsset_close(asset);
            }
        }

        // 关闭文件夹
        AAssetDir_close(assetDir);

        // 遍历Map，按行处理每个文件的内容
        for (const auto &entry : fileContents) {
            const std::string &filename = entry.first;
            const vector<string> &content = entry.second;

            // 在这里处理每个文件的内容
            classifier.loadData(filename, content);

        }

    }

    classifier.region_wds = classifier.mergeVector();
    classifier.region_tree = classifier.build_actree(classifier.region_wds);
    classifier.wdtype_dict = classifier.build_wdtype_dict();
    string msg = "导入成功";
    return env->NewStringUTF(msg.c_str());

}

