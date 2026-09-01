#ifndef PIPELINE_H
#define PIPELINE_H

#include <string>
#include <vector>

class Pipeline
{
private:
    static std::vector<char> readFile(const std::string& filepath);

    void createGraphicsPipeline(const std::string& vertFilepath, const std::string& fragFilepath);

public:
    Pipeline(const std::string& vertFilepath, const std::string& fragFilepath);
};
#endif