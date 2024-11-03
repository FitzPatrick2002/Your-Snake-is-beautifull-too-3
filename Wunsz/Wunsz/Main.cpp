#include <SFML\Graphics.hpp>
#include <SFML\Audio.hpp>
#include <list>
#include <chrono>
#include <ctime>
#include <iostream>
#include <algorithm>
#include <map>
#include <vector>
#include <Windows.h>

float window_w = 768;
float window_h = 768;

float tilesNum_x = 8;
float tilesNum_y = 8;

float spriteScaling_x = float(window_w) / float(tilesNum_x);
float spriteScaling_y = float(window_h) / float(tilesNum_y);

struct TextureManager {
private:
    std::map<std::string, sf::Texture*> manager;

public:
    TextureManager() {

    }

    ~TextureManager() {

    }

    bool addFromFile(std::string name, std::string file) {
        if (manager.find(name) != manager.end()) // Such name for teture already exist
            return false;

        manager[name];
        manager[name] = new sf::Texture;
        manager[name]->loadFromFile(file);
    }

    sf::Texture* getTexture(std::string name) {
        if (manager.find(name) == manager.end()) // Such name for teture already exist
            return nullptr;

        return manager[name];
    }

};

struct Segment {
    sf::Sprite sprite;
    sf::RectangleShape hitBox;

    float x, y;
    float width;
    float height;

    Segment(sf::Texture& t, float xx = 0, float yy = 0) {
        sprite.setTexture(t);

        width = float(window_w) / float(tilesNum_x);
        height = float(window_h) / float(tilesNum_y);

        float scale_x =  width * 1.0 / t.getSize().x;
        float scale_y =  height * 1.0 / t.getSize().y;

        sprite.setScale(scale_x, scale_y);
        sprite.setOrigin( t.getSize().x * 0.5f,  t.getSize().y * 0.5f);

        hitBox.setSize(sf::Vector2f(width * 0.9, height * 0.9));
        hitBox.setFillColor(sf::Color::Red);
        hitBox.setOrigin(0.5f * width, 0.5f * height);

        x = xx;
        y = yy;

        set_x(xx + 0.5f * width);
        set_y(yy + 0.5f * height);

    }

    void set_x(float xx) {
        x = xx;
        sprite.setPosition(xx, y); // Same y
        hitBox.setPosition(xx + 0.05 * width, y + 0.05  *height);
    }

    void set_y(float yy) {
        y = yy;
        sprite.setPosition(x, yy);
        hitBox.setPosition(x + 0.05 * width, yy + 0.05 * height);
    }

    void setPosition(float xx, float yy) {
        set_x(xx);
        set_y(yy);
    }

};

struct TilesBoard {
private:
    sf::RenderWindow* w;
    TextureManager* tManager;

    int widthNum, heightNum;

    std::vector<sf::Sprite> tiles; // Vector stores all tiles in that way: [r1, r1, r1 ... r2, r2, ...] By rows
    // Access [i, j] (x, y) -> [width*j + i

public:

    TilesBoard(TextureManager* tManag, sf::RenderWindow *win, int ww, int hh) {
        tManager = tManag;
        w = win;

        widthNum = ww;
        heightNum = hh;

        tiles.resize(widthNum * heightNum);

        sf::Texture* t = tManag->getTexture("tile");

        sf::Sprite s;
        s.setTexture(*t);

        // Resize sprite so that it matches single cell of the board
        float spriteWidth = float(window_w) / float(widthNum);
        float spriteHeight = float(window_h) / float(heightNum);
        
        float scale_x = spriteWidth * 1.0 / t->getSize().x;
        float scale_y = spriteHeight * 1.0 / t->getSize().y;

        s.setScale(scale_x, scale_y);
       // s.setOrigin(spriteWidth / 2.0f, spriteHeight / 2.0f);

        for (int i = 0; i < heightNum; i++) {
            for (int j = 0; j < widthNum; j++) {
                tiles[i * widthNum + j] = s;

                tiles[i * widthNum + j].setPosition(spriteWidth * (float(j) /*+ 0.5f*/), spriteHeight * (float(i) /* + 0.5f */ ));
            }
        }
    }

    ~TilesBoard() {

    }

    void drawTiles() {
        for (auto& t : tiles) {
            w->draw(t);
        }
    }
};

class Snake {
public:
    std::list<Segment> body;
    float vx, vy;
    float v_magnitude;
    sf::RenderWindow* w;

    TextureManager* t_manager;

public:

    Snake(TextureManager* textureManager, sf::RenderWindow* ww) {
        t_manager = textureManager;

        sf::Texture* tBody = t_manager->getTexture("body");
        sf::Texture* tHead = t_manager->getTexture("head");

        Segment s = Segment(*tHead, 0, 0);
        body.push_back(s);
        vx = 0;
        vy = 1;
        v_magnitude = window_h / tilesNum_y; // Only for going N / S, for E / W -> window_w / tilesNum_x
        w = ww;
    }

    ~Snake() {

    }

    void changeHeadDirection(int i = 0) {
        sf::Sprite& headSprite = body.begin()->sprite;

        switch (i) {
        case 0: //W
            headSprite.setRotation(-180.0f);

            break;
        case 1: //A
            headSprite.setRotation(-270.0f);
            
            break;
        case 2: //S
            headSprite.setRotation(-0.0f);
            
            break;
        case 3: //D
            headSprite.setRotation(-90.0f);
            
            break;
        default:

            break;
        }
    }

    void changeBodySegmentDirection(Segment &s, int next_x, int next_y) {

        int current_x = s.sprite.getPosition().x;
        int current_y = s.sprite.getPosition().y;

        if (next_x > current_x)
            s.sprite.setRotation(270.0f);
        else if (next_x < current_x)
            s.sprite.setRotation(90.0f);
        else if (next_y < current_y)
            s.sprite.setRotation(180.0f);
        else if (next_y > current_y)
            s.sprite.setRotation(0.0f);

    }

    void applySteering(int i = 0) {
        changeHeadDirection(i);

        switch (i) {
        case 0: //W
            vx = 0;
            vy = -1;

            v_magnitude = window_h / tilesNum_y;
            break;
        case 1: //A
            vx = -1;
            vy = 0;

            v_magnitude = window_w / tilesNum_x;
            break;
        case 2: //S
            vx = 0;
            vy = 1;

            v_magnitude = window_h / tilesNum_y;
            break;
        case 3: //D
            vx = 1;
            vy = 0;

            v_magnitude = window_w / tilesNum_x;
            break;
        default:

            break;
        }

        
    }

    void moveHead(double delta_t) {
        
        Segment* h = &body.front();

        float xx = h->x;
        float yy = h->y;

        xx += delta_t * vx * v_magnitude;
        yy += delta_t * vy * v_magnitude;
        

        h->setPosition(xx, yy);
    }

    void moveBody(double delta_t) {
        if (body.size() > 1) {
            float new_x;
            float new_y;

            for (auto rit = body.rbegin(); rit != std::prev(body.rend()); ++rit) {
                auto following = std::next(rit);

                new_x = following->sprite.getPosition().x;
                new_y = following->sprite.getPosition().y;

                changeBodySegmentDirection(*rit, new_x, new_y);

                rit->setPosition(new_x, new_y);
            }
        }
    }

    void advanceSnake(double delta_t) {
        moveBody(delta_t);
        moveHead(delta_t);
    }

    bool checkCollisions() {
        // We only have to check if head collides with anything
        sf::RectangleShape& headHitBox = body.begin()->hitBox;
        for (auto itr = std::next(body.begin()); itr != body.end(); itr++) {
            sf::RectangleShape& segmentHitBox = itr->hitBox;
            bool collision = headHitBox.getGlobalBounds().intersects(segmentHitBox.getGlobalBounds());

            if (collision == true) {
                std::cout << "Aaaaaand, you've failed!\n";
                return true;
            }
            
        }
        return false;
    }

    void expandSnake() {

        // In the first step put it on the position of the last element. It will teleport to it's right position after first calling of moveBody().
        float xx = body.back().x;
        float yy = body.back().y;

        sf::Texture* t = t_manager->getTexture("body");
        Segment s(*t, -100, -100); // Just spawn it outside the map and rotate later.
        s.sprite.setRotation(body.back().sprite.getRotation());
        body.push_back(s);
    }

    void drawSnake() {
        for (auto& seg : body) {
            w->draw(seg.sprite);
            //w->draw(seg.hitBox);
        }
    }

};

struct FoodSpawner {
    Segment *food;
    Snake* snakePtr;
    int mapWidth;
    int mapHeight;
    sf::RenderWindow* w;

    FoodSpawner(sf::RenderWindow *ww, Snake* hP = nullptr, int w = 0, int h = 0) : snakePtr(hP), mapWidth(w), mapHeight(h), w(ww){

    }

    ~FoodSpawner() {

    }

    void spawnFood() {
        std::vector<sf::Vector2f> coords;
        coords.reserve(mapHeight * mapWidth);

        int tileWidth = window_w / tilesNum_x;
        int tileHeight = window_h / tilesNum_y;
        
        // Get all possible positions on the map
        for (int i = 0; i < mapHeight; i++) {
            for (int j = 0; j < mapWidth; j++)
                coords.emplace_back(sf::Vector2f((float(i) + 0.5f) * tileWidth, (float(j) + 0.5f) * tileHeight));
        }
        
        // Positions taken by the snake's body and head are to be deleted
        std::vector<sf::Vector2f> toDelete(snakePtr->body.size());
        int counter = 0;
        for (auto itr = snakePtr->body.begin(); itr != snakePtr->body.end(); itr++) {
            toDelete[counter] = sf::Vector2f(itr->x, itr->y);
            counter++;
        }

        //Extract coords taken by snake
        coords.erase(std::remove_if(coords.begin(), coords.end(), [&](sf::Vector2f xy) {
            return (std::find(toDelete.begin(), toDelete.end(), xy) != toDelete.end()); // This return is flawed
            }), coords.end());

        //Choose random coords;
        coords.shrink_to_fit();
        int len = coords.size();
        int pick = std::rand() % len;
        sf::Vector2f newPos = coords[pick];

        food->setPosition(newPos.x, newPos.y);
    }

    bool checkCollision() {
        Segment* h = &snakePtr->body.front();

        bool collision = food->hitBox.getGlobalBounds().intersects(h->hitBox.getGlobalBounds());

        if (collision == true) {
            spawnFood();
            snakePtr->expandSnake();
        }

        return collision;
    }

    void drawFood() {
        w->draw(food->sprite);
    }

};

struct Score {
    int count = 0;
    
    sf::Text txt;
    sf::RenderWindow* w;

    Score(sf::Font& f, sf::RenderWindow* ww = nullptr, std::string baseText = "Score: ") : w(ww) {
        txt.setFont(f);
        txt.setString(baseText);

        txt.setPosition(window_w / tilesNum_x, window_h / tilesNum_y);
        txt.setCharacterSize(30.0f);
    }

    ~Score() {

    }

    void resetScore() {
        count = 0;
    }

    void updateScore() {
        count++;
        
        std::string message = "Score: " + std::to_string(count);
        txt.setString(message);
    }

    void drawScore() {
        w->draw(txt);
    }

};

void loadTexturesFromFiles(TextureManager& textureManager) {
    textureManager.addFromFile("head", "Graphics/WunszHead.png");
    textureManager.addFromFile("body", "Graphics/WunszBody.png");
    textureManager.addFromFile("food", "Graphics/OC_1.png");
    textureManager.addFromFile("tile", "Graphics/Tile.png");
    textureManager.addFromFile("endScreen", "Graphics/Wunsz_you_died.png");
}

int main()
{
    std::srand(time(0));

    sf::RenderWindow window(sf::VideoMode(window_w, window_h), "Wunsz!");

    // Textures setup
    TextureManager textureManager;
    loadTexturesFromFiles(textureManager);

    // Rescale the textures so that they fit the square board -> each separtately TO DO;

    sf::Music music;
    music.openFromFile("Music/Wunsz_music.wav");

    music.play();
    music.setLoop(true);

    int lives = 3;

    while(lives > 0 ){

        Snake wunsz(&textureManager, &window);

        TilesBoard tiles(&textureManager, &window, tilesNum_x, tilesNum_y);

        // Food spawner
        FoodSpawner foodSpawner(&window, &wunsz, tilesNum_x, tilesNum_y);
        foodSpawner.food = new Segment(*textureManager.getTexture("food"), 0, 0);

        sf::Font arialFont;
        arialFont.loadFromFile("Fonts/arial.ttf");

        // Score
        Score scr(arialFont, &window, "Score: 0");

        // Clock setup
        std::chrono::duration<double> delta_t;
        std::chrono::high_resolution_clock::time_point t1;
        std::chrono::high_resolution_clock::time_point t2;

        t1 = std::chrono::high_resolution_clock::now();
        t2 = std::chrono::high_resolution_clock::now();
        bool resetTimer = false;

        while (window.isOpen())
        {
            sf::Event event; // Listen for events eg. closing window, clicking, etc.
            while (window.pollEvent(event))
            {
                if (event.type == sf::Event::Closed)
                    window.close();
            }

            if (resetTimer == true) {
                t1 = std::chrono::high_resolution_clock::now();
                resetTimer = false;
            }

            // Get input from user and handle it
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
                wunsz.applySteering(0);
            }
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
                wunsz.applySteering(1);
            }
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
                wunsz.applySteering(2);
            }
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
                wunsz.applySteering(3);
            }

            t2 = std::chrono::high_resolution_clock::now();
            delta_t = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
            //std::cout << "Time elapsed: " << delta_t.count() << "\n";

            if (delta_t.count() >= 1.0) {
                wunsz.advanceSnake(delta_t.count());
                
                if (wunsz.checkCollisions() == true) {
                    break;
                }

                if (foodSpawner.checkCollision() == true)
                    scr.updateScore();

                resetTimer = true;
            }

            window.clear();

            tiles.drawTiles();
            wunsz.drawSnake();
            foodSpawner.drawFood();
            scr.drawScore();

            window.display();

        }

        // End screen
        // You've lost fool!

        sf::Sprite endScreen;
        endScreen.setTexture(*textureManager.getTexture("endScreen"));

        sf::Text endText("YOU DIED FOOL!", arialFont, 100.0f);

        sf::RectangleShape rect(sf::Vector2f(window_w, window_h));

        float a = float(window_w);
        float b = float(window_h);

        float scale_x = a * 1.0 / endScreen.getTexture()->getSize().x;
        float scale_y = b * 1.0 / endScreen.getTexture()->getSize().y;

        endScreen.setScale(scale_x, scale_y);

        window.clear();
        
        window.draw(endScreen);
        window.draw(endText);

        window.display();
        Sleep(5000.0f);

        lives--;
    }

    return 0;
}