#include <filesystem>
#include <string>
#include <vector>

namespace fs = std::filesystem;

void saveData(const fs::path &path, const std::string &playerName,
              uint32_t playerMoney, uint32_t upgradeCost,
              uint32_t slayedEnemies, float enemyInnacuracy);

bool loadData(const fs::path &path, std::string &playerName,
              uint32_t &playerMoney, uint32_t &upgradeCost,
              uint32_t &slayedEnemies, float &enemyInnacuracy);

std::vector<std::string> listFilesInDirectory(const fs::path &dirPath);
