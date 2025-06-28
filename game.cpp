#include "game.h"
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <thread>
#include <chrono>

void Game::displayIntro() const {
    clearScreen();
    std::cout << "========================================\n";
    std::cout << "             Road to nowhere\n";
    std::cout << "========================================\n\n";

    std::cout << "You are a hero walking a looped path.\n";
    std::cout << "The world around you is initially empty,\n";
    std::cout << "but gradually fills up thanks to your movement...\n\n";

    std::cout << "Every day on the road there appear:\n";
    std::cout << "- Monsters thirsty for your blood\n";
    std::cout << "- Lost items of forgotten travelers\n";
    std::cout << "- Mysterious structures on the roadside\n\n";

    std::cout << "Your goal is to hold out as long as possible.\n";
    std::cout << "in this endless cycle!\n\n";

    std::cout << "========================================\n";
    std::cout << "Press any key to start...\n";
    std::cin.get();
}

Game::Game() : hero(), day(1), road_index(0), show_intro(true) {
    std::srand(std::time(nullptr));
    loadConfigs();
    initializeMap();
}

void Game::loadConfigs() {
    try {
        std::ifstream enemies_file("config/enemies.json");
        json enemies_json;
        enemies_file >> enemies_json;

        for (const auto& enemy : enemies_json["enemies"]) {
            EnemyConfig config;
            config.name = enemy["name"];
            config.symbol = enemy["symbol"].get<std::string>()[0];
            config.base_hp = enemy["base_hp"];
            config.hp_variation = enemy["hp_variation"];
            config.base_damage = enemy["base_damage"];
            config.damage_variation = enemy["damage_variation"];
            config.spawn_chance = enemy["spawn_chance"];
            enemies_config.push_back(config);
        }

        std::ifstream items_file("config/items.json");
        json items_json;
        items_file >> items_json;

        for (const auto& item : items_json["items"]) {
            ItemConfig config;
            config.name = item["name"];
            config.type = item["type"];
            config.damage_bonus = item["damage_bonus"];
            config.hp_bonus = item["hp_bonus"];
            config.symbol = item["symbol"].get<std::string>()[0];
            items_config.push_back(config);
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error loading configs: " << e.what() << std::endl;
        exit(1);
    }
}

void Game::initializeMap() {
    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            map[y][x] = EMPTY;
        }
    }

    for (const auto& point : road_path) {
        if (point.first < MAP_WIDTH && point.second < MAP_HEIGHT) {
            map[point.second][point.first] = ROAD;
        }
    }

    hero_x = road_path[0].first;
    hero_y = road_path[0].second;
}

void Game::spawnEnemies() {
    enemies.clear();
    enemy_positions.clear();

    for (const auto& config : enemies_config) {
        if ((float)std::rand() / RAND_MAX < config.spawn_chance) {
            int hp = config.base_hp + (std::rand() % config.hp_variation);
            int dmg = config.base_damage + (std::rand() % config.damage_variation);

            int pos = std::rand() % road_path.size();
            int x = road_path[pos].first;
            int y = road_path[pos].second;

            if (x != hero_x || y != hero_y) {
                enemies.emplace_back(config.name, config.symbol, hp, dmg);
                enemy_positions[{x, y}] = &enemies.back();
                map[y][x] = ENEMY_POS;
            }
        }
    }
}

void Game::spawnItems() {
    items_on_map.clear();
    item_positions.clear();

    if (day % 3 == 0 && !items_config.empty()) {
        int item_index = std::rand() % items_config.size();
        int pos = std::rand() % road_path.size();
        int x = road_path[pos].first;
        int y = road_path[pos].second;

        if (map[y][x] == ROAD) {
            items_on_map.push_back(items_config[item_index]);
            item_positions[{x, y}] = &items_on_map.back();
            map[y][x] = ITEM_POS;
        }
    }
}

void Game::battle(Enemy& enemy) {
    std::cout << "\nBattle starts! " << hero.getName() << " vs " << enemy.getName() << "\n";

    while (hero.isAlive() && enemy.isAlive()) {
        enemy.takeDamage(hero.getDamage());
        std::cout << hero.getName() << " hits " << enemy.getName()
            << " for " << hero.getDamage() << " damage. "
            << enemy.getHp() << "/" << enemy.getMaxHp() << " HP left.\n";

        if (!enemy.isAlive()) break;

        hero.takeDamage(enemy.getDamage());
        std::cout << enemy.getName() << " hits " << hero.getName()
            << " for " << enemy.getDamage() << " damage. "
            << hero.getHp() << "/" << hero.getMaxHp() << " HP left.\n";

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    if (hero.isAlive()) {
        std::cout << hero.getName() << " defeated " << enemy.getName() << "!\n";
        hero.heal(5 + (std::rand() % 6));
        std::cout << hero.getName() << " healed for " << 5 + (std::rand() % 6) << " HP.\n";
    }
    else {
        std::cout << hero.getName() << " was defeated by " << enemy.getName() << "...\n";
    }

    std::this_thread::sleep_for(std::chrono::seconds(1));
}

void Game::pickItem(const ItemConfig& item) {
    std::cout << "\nFound item: " << item.name << "! ";

    if (item.type == "weapon") {
        std::cout << "Damage +" << item.damage_bonus;
    }
    else if (item.type == "armor") {
        std::cout << "HP +" << item.hp_bonus;
    }
    else {
        std::cout << "Damage +" << item.damage_bonus << ", HP +" << item.hp_bonus;
    }

    hero.addItemBonus(item.damage_bonus, item.hp_bonus);
    std::cout << "\n";
    std::this_thread::sleep_for(std::chrono::seconds(1));
}

void Game::drawMap() const {
    std::cout << "\nDay " << day << "\n";
    std::cout << "+";
    for (int x = 0; x < MAP_WIDTH; x++) std::cout << "-";
    std::cout << "+\n";

    for (int y = 0; y < MAP_HEIGHT; y++) {
        std::cout << "|";
        for (int x = 0; x < MAP_WIDTH; x++) {
            if (x == hero_x && y == hero_y) {
                std::cout << hero.getSymbol();
            }
            else {
                switch (map[y][x]) {
                case ENEMY_POS: {
                    auto it = enemy_positions.find({ x, y });
                    if (it != enemy_positions.end()) {
                        std::cout << it->second->getSymbol();
                    }
                    else {
                        std::cout << ".";
                    }
                    break;
                }
                case ITEM_POS: {
                    auto it = item_positions.find({ x, y });
                    if (it != item_positions.end()) {
                        std::cout << it->second->symbol;
                    }
                    else {
                        std::cout << ".";
                    }
                    break;
                }
                case ROAD:
                    std::cout << ".";
                    break;
                default:
                    std::cout << " ";
                }
            }
        }
        std::cout << "|\n";
    }

    std::cout << "+";
    for (int x = 0; x < MAP_WIDTH; x++) std::cout << "-";
    std::cout << "+\n";
}

void Game::drawStats() const {
    std::cout << hero.getName() << ": HP " << hero.getHp() << "/" << hero.getMaxHp()
        << " DMG " << hero.getDamage() << "\n";

    if (!enemies.empty()) {
        std::cout << "Enemies on road: " << enemies.size() << "\n";
    }
}

void Game::clearScreen() const {
    system("cls");
}

void Game::run() {
    if (show_intro) {
        displayIntro();
        show_intro = false;
    }
    while (hero.isAlive()) {
        clearScreen();
        drawStats();
        drawMap();

        road_index = (road_index + 1) % road_path.size();
        hero_x = road_path[road_index].first;
        hero_y = road_path[road_index].second;

        if (road_index == 0) {
            day++;
            spawnEnemies();
            spawnItems();
        }

        if (map[hero_y][hero_x] == ENEMY_POS) {
            auto enemy_it = enemy_positions.find({ hero_x, hero_y });
            if (enemy_it != enemy_positions.end()) {
                battle(*(enemy_it->second));

                for (auto it = enemies.begin(); it != enemies.end(); ++it) {
                    if (&(*it) == enemy_it->second) {
                        enemies.erase(it);
                        break;
                    }
                }
                enemy_positions.erase(enemy_it);
                map[hero_y][hero_x] = ROAD;

                if (!hero.isAlive()) break;
            }
        }

        if (map[hero_y][hero_x] == ITEM_POS) {
            auto item_it = item_positions.find({ hero_x, hero_y });
            if (item_it != item_positions.end()) {
                pickItem(*(item_it->second));

                for (auto it = items_on_map.begin(); it != items_on_map.end(); ++it) {
                    if (&(*it) == item_it->second) {
                        items_on_map.erase(it);
                        break;
                    }
                }
                item_positions.erase(item_it);
                map[hero_y][hero_x] = ROAD;
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(300));
    }

    std::cout << "\nGame Over! You survived " << day - 1 << " days.\n";
}