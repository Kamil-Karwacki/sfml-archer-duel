#include "FileIO.h"

#include <iostream>

#include <fstream>
#include <regex>
namespace fs = std::filesystem;

void saveData(const fs::path &path, const std::string &playerName,
              uint32_t playerMoney, uint32_t upgradeCost,
              uint32_t slayedEnemies, float enemyInnacuracy)
{
    if (!fs::exists(path.parent_path()))
    {
        fs::create_directories(path.parent_path());
    }
    std::ofstream file(path);
    if (file)
    {
        file << "playerName:" << playerName << '\n';
        file << "playerMoney:" << playerMoney << '\n';
        file << "upgradeCost:" << upgradeCost << '\n';
        file << "slayedEnemies:" << slayedEnemies << '\n';
        file << "enemyInnacuracy:" << enemyInnacuracy << '\n';
    }
    else
    {
        std::cerr << "Failed to write to " << path << "\n";
    }
}

bool loadData(const fs::path &path, std::string &playerName,
              uint32_t &playerMoney, uint32_t &upgradeCost,
              uint32_t &slayedEnemies, float &enemyInnacuracy)
{
    if (!fs::exists(path))
    {
        std::cerr << "Save file does not exist: " << path << "\n";
        return false;
    }

    std::ifstream file(path);
    if (!file)
    {
        std::cerr << "Failed to open save file.\n";
        return false;
    }

    std::regex pattern(R"(^(\w+):(.+)$)");
    std::smatch matches;

    std::string line;
    while (std::getline(file, line))
    {
        if (std::regex_match(line, matches, pattern))
        {
            std::string key = matches[1].str();
            std::string value = matches[2].str();

            if (key == "playerName")
                playerName = value;
            else if (key == "playerMoney")
                playerMoney = std::stoul(value);
            else if (key == "upgradeCost")
                upgradeCost = std::stoul(value);
            else if (key == "slayedEnemies")
                slayedEnemies = std::stoul(value);
            else if (key == "enemyInnacuracy")
                enemyInnacuracy = std::stof(value);
        }
    }
    return true;
}

std::vector<std::string> listFilesInDirectory(const fs::path &dirPath)
{
    std::vector<std::string> filenames;
    if (!fs::exists(dirPath) || !fs::is_directory(dirPath))
    {
        std::cerr << "Directory does not exist: " << dirPath << "\n";
        return filenames;
    }

    for (const auto &entry : fs::directory_iterator(dirPath))
    {
        if (fs::is_regular_file(entry))
        {
            filenames.push_back(entry.path().filename().string());
        }
    }
    return filenames;
}
