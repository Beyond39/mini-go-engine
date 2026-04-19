#include "DataExtractor.h"

#include <iostream>
#include <filesystem>

int main()
{
    namespace fs = std::filesystem;

    // 这里建议你按自己项目根目录来写
    const std::string projectRoot = "E:/vscode-code/GoEngine";

    const std::string filelistDir = projectRoot + "/data/filelists";
    const std::string binDir = projectRoot + "/data/bin";

    const std::string trainListPath = filelistDir + "/train_10k.txt";
    const std::string valListPath   = filelistDir + "/val_600.txt";
    const std::string testListPath  = filelistDir + "/test_600.txt";

    const std::string trainBinPath = binDir + "/train_10k.bin";
    const std::string valBinPath   = binDir + "/val_600.bin";
    const std::string testBinPath  = binDir + "/test_600.bin";

    try {
        // 如果 bin 文件夹不存在，就自动创建
        if (!fs::exists(binDir)) {
            fs::create_directories(binDir);
            std::cout << "已创建目录: " << binDir << std::endl;
        }

        std::cout << "==============================" << std::endl;
        std::cout << "开始生成训练集 bin..." << std::endl;
        std::cout << "输入列表: " << trainListPath << std::endl;
        std::cout << "输出文件: " << trainBinPath << std::endl;
        DataExtractor::extractFromFileList(trainListPath, trainBinPath);

        std::cout << "\n==============================" << std::endl;
        std::cout << "开始生成验证集 bin..." << std::endl;
        std::cout << "输入列表: " << valListPath << std::endl;
        std::cout << "输出文件: " << valBinPath << std::endl;
        DataExtractor::extractFromFileList(valListPath, valBinPath);

        std::cout << "\n==============================" << std::endl;
        std::cout << "开始生成测试集 bin..." << std::endl;
        std::cout << "输入列表: " << testListPath << std::endl;
        std::cout << "输出文件: " << testBinPath << std::endl;
        DataExtractor::extractFromFileList(testListPath, testBinPath);

        std::cout << "\n==============================" << std::endl;
        std::cout << "全部 bin 文件生成完成！" << std::endl;
        std::cout << "train -> " << trainBinPath << std::endl;
        std::cout << "val   -> " << valBinPath << std::endl;
        std::cout << "test  -> " << testBinPath << std::endl;
        std::cout << "==============================" << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "发生异常: " << e.what() << std::endl;
        return 1;
    }
    catch (...) {
        std::cerr << "发生未知异常。" << std::endl;
        return 1;
    }

    return 0;
}