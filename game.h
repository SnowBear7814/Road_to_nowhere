#ifndef GAME_H
#define GAME_H

#include <vector>
#include <string>
#include <map>
#include <utility>
#include "json.hpp"

using json = nlohmann::json;

const int MAP_WIDTH = 18;
const int MAP_HEIGHT = 10;
const int BASE_DAMAGE = 5;
const int BASE_HP = 120;

enum CellType {
    EMPTY,
    ROAD,
    HERO_POS,
    ENEMY_POS,
    ITEM_POS
};

struct EnemyConfig {
    std::string name;
    char symbol;
    int base_hp;
    int hp_variation;
    int base_damage;
    int damage_variation;
    float spawn_chance;
};

struct ItemConfig {
    std::string name;
    std::string type;
    int damage_bonus;
    int hp_bonus;
    char symbol;
};

class Character {
protected:
    int hp;
    int max_hp;
    int damage;

public:
    Character(int h, int d) : max_hp(h), hp(h), damage(d) {}
    virtual ~Character() {}

    void takeDamage(int amount) { hp -= amount; if (hp < 0) hp = 0; }
    bool isAlive() const { return hp > 0; }
    int getHp() const { return hp; }
    int getMaxHp() const { return max_hp; }
    virtual int getDamage() const { return damage; }

    virtual char getSymbol() const = 0;
    virtual std::string getName() const = 0;
};

class Hero : public Character {
private:
    int bonus_damage;
    int bonus_hp;

public:
    Hero() : Character(BASE_HP, BASE_DAMAGE), bonus_damage(0), bonus_hp(0) {}

    char getSymbol() const override { return 'H'; }
    std::string getName() const override { return "Hero"; }

    int getDamage() const override { return Character::getDamage() + bonus_damage; }
    int getMaxHp() const { return Character::getMaxHp() + bonus_hp; }
    int getHp() const { return std::min(Character::getHp(), getMaxHp()); }

    void heal(int amount) {
        hp += amount;
        if (hp > getMaxHp()) hp = getMaxHp();
    }

    void addItemBonus(int dmg, int hp) {
        bonus_damage += dmg;
        bonus_hp += hp;
        this->max_hp = BASE_HP + bonus_hp;
    }
};

class Enemy : public Character {
private:
    std::string enemy_name;
    char enemy_symbol;
    std::string state; 

public:
    Enemy(const std::string& name, char symbol, int h, int d)
        : Character(h, d), enemy_name(name), enemy_symbol(symbol), state("tired") {
    }

    char getSymbol() const override { return enemy_symbol; }
    std::string getName() const override { return enemy_name; }

    void setState(const std::string& newState) { state = newState; }
    std::string getState() const { return state; }
};

class Game {
private:
    Hero hero;
    std::vector<std::unique_ptr<Enemy>> enemies;
    std::vector<ItemConfig> items_on_map;
    int day;

    std::vector<EnemyConfig> enemies_config;
    std::vector<ItemConfig> items_config;

    bool show_intro;
    void displayIntro() const;

    CellType map[MAP_HEIGHT][MAP_WIDTH];
    int hero_x, hero_y;
    int road_index;

    std::map<std::pair<int, int>, Enemy*> enemy_positions;
    std::map<std::pair<int, int>, ItemConfig*> item_positions;

    const std::vector<std::pair<int, int>> road_path = {
        {2, 1}, {3, 1}, {4, 1}, {5, 1}, {6, 1}, {7, 1}, {8, 1},
        {8, 2}, {8, 3}, {8, 4},
        {7, 4}, {6, 4}, {5, 4}, {4, 4}, {3, 4},
        {3, 5}, {3, 6}, {3, 7},
        {4, 7}, {5, 7}, {6, 7}, {7, 7}, {8, 7}, {9, 7}, {10, 7},
        {10, 6}, {10, 5}, {10, 4},
        {11, 4}, {12, 4}, {13, 4}, {14, 4}, {15, 4},
        {15, 3}, {15, 2}, {15, 1},
        {14, 1}, {13, 1}, {12, 1}, {11, 1}, {10, 1}, {9, 1}, {8, 1}
    };

    void initializeMap();
    void loadConfigs();
    void spawnEnemies();
    void spawnItems();
    void battle(Enemy& enemy);
    void pickItem(const ItemConfig& item);
    void drawMap() const;
    void drawStats() const;
    void clearScreen() const;

public:
    Game();
    void run();
};

#endif // GAME_H
