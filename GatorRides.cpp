#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <random>
#include <ctime>
#include <unordered_map>
#include <math.h>
#include <algorithm>
#include <iomanip>
#include <cmath>
using namespace std;
using namespace sf;

#define M_PI  3.1415926535897932

class Car
{
private:
    double latitude, longitude, distance;
    int carID;
public:
    Car() : latitude(0), longitude(0), distance(0), carID(0) {};
    Car(double lati, double longi, double dist, int id) : latitude(lati), longitude(longi), distance(dist), carID(id) {};
    double GetLatitude() { return latitude; };
    double GetLongitude() { return longitude; };
    double GetDistance() { return distance; };
    int GetCarID() { return carID; };
};

class Board
{
    int height, length, distance;
    double personX, personY, pCoorX, pCoorY, ratioX, ratioY;
    vector<Car> cars;
    bool checked, optionsCheck, distanceCheck, mapCheck, reset;
    string type;
public:
    Board();
    Board(int length, int height, string type);
    vector<Car*> MapCars(Vector2i& globalPosition);
    vector<Car*> TreeCars(Vector2i& globalPosition);
    void PrintBoard(RenderWindow& window);
    void PrintMenu(RenderWindow& window);
    void PrintCars(RenderWindow& window);
    void SetCars(vector<Car> cars);
    vector<Car> GetCars();
    double GetPersonX();
    double GetPersonY();
    void SetPersonX(double coorX);
    void SetPersonY(double coorY);
    void setDistance(int dist);
    int GetDistance();
    void SetRatioX(double ratio);
    double GetRatioX();
    void SetRatioY(double ratio);
    double GetRatioY();
    void SetBoard();
    void CheckLeftClick(Vector2i& globalPosition);
    bool GetChecked();
    void SetChecked(bool cond);
    string GetType();
    bool GetReset();
};


struct Node
{
    Car car;
    int height;
    Node* left = nullptr;
    Node* right = nullptr;
};

Node* newNode(Car car)
{
    Node* node = new Node();
    node->car = car;
    node->left = nullptr;
    node->right = nullptr;
    node->height = 1;

    return(node);
}

double haversine(double lati1, double longi1,
    double lati2, double longi2)
{
    /* find difference in latitudes
     * and longitudes and convert
     * to radians */
    double difLati = (lati2 - lati1) * M_PI / 180.0;
    double difLongi = (longi2 - longi1) *M_PI / 180.0;

    // convert latitudes to radians
    lati1 *= (M_PI / 180.0);
    lati2 *= (M_PI / 180.0);

    /* calculate distance in km
     * using the haversine formula */
    double a = pow(sin(difLati / 2), 2) +
        pow(sin(difLongi / 2), 2) *
        cos(lati1) * cos(lati2);
    double rad = 6371;
    double b = 2 * asin(sqrt(a));
    return rad * b;
}

// Height of given node
int Height(Node* node)
{
    if (node == nullptr) {
        return 0;
    }

    int l_height = 1 + Height(node->left);
    int r_height = 1 + Height(node->right);

    return max(l_height, r_height);
}

int getHeight(Node* node)
{
    if (node == nullptr) {
        return 0;
    }
    return node->height;
}

// Balance Factor
int checkBalance(Node* node) {
    if (node == nullptr)
    {
        return 0;
    }

    return getHeight(node->left) - getHeight(node->right);
}

// Rotations
Node* rotateLeft(Node* node)
{
    Node* newRoot = node->right;
    Node* leftNewRoot = newRoot->left;
    node->right = leftNewRoot;
    newRoot->left = node;

    // Update height
    node->height = max(Height(node->left), Height(node->right)) + 1;
    newRoot->height = max(Height(newRoot->left), Height(newRoot->right)) + 1;

    return newRoot;
}

Node* rotateRight(Node* node)
{
    Node* newRoot = node->left; // initialize new root
    Node* rightNewRoot = newRoot->right;
    node->left = rightNewRoot; // Make old pointer null
    newRoot->right = node;

    // Update height
    node->height = max(Height(node->left), Height(node->right)) + 1;
    newRoot->height = max(Height(newRoot->left), Height(newRoot->right)) + 1;

    return newRoot;
}

Node* rotateLeftRight(Node* node)
{
    node->left = rotateLeft(node->left);
    return rotateRight(node);
}

Node* rotateRightLeft(Node* node)
{
    node->right = rotateRight(node->right);
    return rotateLeft(node);
}

Node* insert(Node* node, Car car) {
    if (node == nullptr) {
        return newNode(car);
    }

    // Traverse tree
    if (car.GetDistance() < node->car.GetDistance())
        node->left = insert(node->left, car);

    else if (car.GetDistance() > node->car.GetDistance())
        node->right = insert(node->right, car);
    else
    {
        return node;
    }

    // Update height
    node->height = 1 + max(Height(node->left),
        Height(node->right));

    // Check balance
    int balance = checkBalance(node);


    // Left Left
    if (balance > 1 && car.GetDistance() < node->left->car.GetDistance())
    {
        return rotateRight(node);
    }

    // Right Right
    if (balance < -1 && car.GetDistance() > node->right->car.GetDistance())
    {
        return rotateLeft(node);
    }

    // Left Right
    if (balance > 1 && car.GetDistance() > node->left->car.GetDistance())
    {
        return rotateLeftRight(node);
    }

    // Right Left
    if (balance < -1 && car.GetDistance() < node->right->car.GetDistance())
    {
        return rotateRightLeft(node);
    }
    return node;
}

vector<Car> returnCars(Node* root, vector<Car>& cars) {

    if (root != nullptr)
    {
        returnCars(root->left, cars);
        cars.push_back(root->car);
        returnCars(root->right, cars);
    }
    return cars;
}


vector<Car> FindCars(double userLatitude, double userLongitude, double inputDistance) {
    ifstream inFS;
    string line, temp;
    double latitude, longitude, distance;
    int totalCars, carID;
    vector<Car> cars;

    // Initialize map data structure of cars and distances
    map<double, Car> carMap;

    // Initialize map of cars and distances
    map<double, Car>::iterator it;

    vector<Car> radiusCars;

    //double inputDistance = 3824;

    //userLatitude = 5.32;
    //userLongitude = 21.53;

    inFS.open(R"(C:\Users\douga\source\repos\DSAFinalProject\DSAFinalProject\2020_02_25.csv)");
    if (inFS.is_open()) {
        getline(inFS, line);

        int chk = 0, chk1 = 0;
        while (getline(inFS, line) && chk != 150 && chk1 != 4000) {
            chk1++;
            istringstream stream(line);

            // latitude
            getline(stream, temp, ',');
            latitude = stod(temp);
            //cout << latitude << " ";

            // longitude
            getline(stream, temp, ',');
            longitude = stod(temp);

            // total cars at location
            getline(stream, temp, ',');
            totalCars = stoi(temp);

            // carIDs
            // remove quotes
            stream >> quoted(temp);

            // Remove brackets '[]'
            temp.erase(remove(temp.begin(), temp.end(), '['), temp.end());
            temp.erase(remove(temp.begin(), temp.end(), ']'), temp.end());

            // clear buffer
            string buff;
            getline(stream, buff, ',');


            // Convert string car ID to int
            istringstream ss(temp);
            string tp;

            for (int i = 0; i < totalCars; i++) {

                getline(ss, tp, ',');
                carID = stoi(tp);
            }
            
            distance = haversine(latitude, longitude, userLatitude, userLongitude);
            // Create car object
            Car car(latitude, longitude, distance, carID);
            
            // Add car if it falls within the radius
            if (inputDistance > distance) {
                carMap.emplace(distance, car);
                chk++;
                
            }

        }

        inFS.close();
    }

    // Add cars from the map into the vector in sorted order
    for (it = carMap.begin(); it != carMap.end(); it++)
    {
        if (it->first < inputDistance)
        {
            radiusCars.push_back(it->second);
        }

    }

    // Print out cars within radius
    for (unsigned int i = 0; i < radiusCars.size(); i++) {
    }

    return radiusCars;
};

vector<Car> AVLTree(double x, double y, int maxDist) {
    ifstream inFS;
    Node* node = nullptr;

    string line, temp;
    double latitude, longitude;
    int totCars, searchCount = 0, foundCount = 0;
    vector<Car> cars;

    inFS.open(R"(C:\Users\douga\source\repos\DSAFinalProject\DSAFinalProject\2020_02_25.csv)");
    if (inFS.is_open()) {
        getline(inFS, line);

        int chk = 0;
        while (getline(inFS, line)) {
            istringstream stream(line);
            // latitude
            getline(stream, temp, ',');
            latitude = stod(temp);

            // longitude
            getline(stream, temp, ',');
            longitude = stod(temp);

            // total cars
            getline(stream, temp, ',');
            totCars = stoi(temp);

            // cars list
            stream >> quoted(temp); // remove quotes
            // Remove brackets '[]'
            temp.erase(remove(temp.begin(), temp.end(), '['), temp.end());
            temp.erase(remove(temp.begin(), temp.end(), ']'), temp.end());

            // clear buffer
            string buff;
            getline(stream, buff, ',');

            /* Calculate distance for this
             * location. If distance is within
             * radius (maxDist), convert string
             * of Car IDs to int, then create new
             * Car objects and insert in tree. */
            istringstream ss(temp);
            string tp;

            /* Use haversine formula to
             * calculate distance between two
             * coordinates in kilometers. */
            double d = haversine(x, y, latitude, longitude);

            for (int i = 0; i < totCars; i++) {
                getline(ss, tp, ',');
                int id = stoi(tp);

                if (d <= maxDist && foundCount < 150) {
                    Car car = Car(latitude, longitude, d, id);
                    node = insert(node, car);
                    foundCount++;
                }

            }
            /* After searching first 4000 rows,
             * or if 150 cars found, return cars. */
            ++searchCount;
            if (foundCount == 150 || searchCount == 4000) {
                inFS.close();
                return returnCars(node, cars);
            }
        }
    }

}

class TextureManager
{
    static unordered_map<string, sf::Texture> textures;
public:
    static void LoadTexture(string textureName);
    static sf::Texture& GetTexture(string textureName);
    static void Clear();
};

Board::Board()
{
    height = 0, length = 0, distance = 5;
    personX = 0, personY = 0, pCoorX = 0, pCoorY = 0;
    cars;
    checked = false, optionsCheck = false, distanceCheck = false, mapCheck = true, reset = false;
}

Board::Board(int length, int height, string type)
{
    this->length = length, this->height = height;
    personX = 0, personY = 0, pCoorX = 0, pCoorY = 0, distance = 5;
    cars;
    checked = false, optionsCheck = false, distanceCheck = false, reset = false;
    this->type = type;
}

void Board::SetBoard()
{
    distance = 5;
    checked = false, reset = false, reset = false; //optionsCheck = false, distanceCheck = false, reset = true;
}

void Board::PrintBoard(RenderWindow& window)
{
    Sprite mapSprite(TextureManager::GetTexture("Tel_Aviv_Map.png"));
    Sprite sideSprite(TextureManager::GetTexture("side.png"));
    Sprite resetSprite(TextureManager::GetTexture("tile_reset.png"));
    sideSprite.setPosition(760, 0);
    resetSprite.setPosition(920, 575);
    window.draw(mapSprite);
    window.draw(sideSprite);
    window.draw(resetSprite);

    if (checked) {
        Sprite personSprite(TextureManager::GetTexture("found_pin.png"));
        personSprite.setPosition(365, 315);
        window.draw(personSprite);
        PrintCars(window);
    }
}

void Board::PrintMenu(RenderWindow& window)
{
    Sprite mapSprite(TextureManager::GetTexture("Tel_Aviv_Map.png"));
    Sprite car_buttonSprite(TextureManager::GetTexture("car_button.jpg"));
    Sprite optionSprite(TextureManager::GetTexture("tile_options.png"));
    Sprite mapDSSprite(TextureManager::GetTexture("tile_map.png"));
    Sprite avlSprite(TextureManager::GetTexture("tile_avl.png"));
    Sprite distanceSprite(TextureManager::GetTexture("tile_distance.png"));
    Sprite tile_1Sprite(TextureManager::GetTexture("tile_1.png"));
    Sprite tile_2Sprite(TextureManager::GetTexture("tile_2.png"));
    Sprite tile_3Sprite(TextureManager::GetTexture("tile_3.png"));
    Sprite tile_4Sprite(TextureManager::GetTexture("tile_4.png"));
    Sprite tile_5Sprite(TextureManager::GetTexture("tile_5.png"));
    Sprite menuSprite(TextureManager::GetTexture("menu.jpg"));

    mapSprite.setPosition(0, 0);
    car_buttonSprite.setPosition(486, 416);
    optionSprite.setPosition(211, 301);
    avlSprite.setPosition(211, 381);
    mapDSSprite.setPosition(211, 461);
    distanceSprite.setPosition(788, 301);
    tile_1Sprite.setPosition(788, 346);
    tile_2Sprite.setPosition(788, 391);
    tile_3Sprite.setPosition(788, 436);
    tile_4Sprite.setPosition(788, 481);
    tile_5Sprite.setPosition(788, 526);

    window.draw(menuSprite);
    window.draw(car_buttonSprite);

    window.draw(optionSprite);
    if (optionsCheck) {
        window.draw(avlSprite);
        window.draw(mapDSSprite);
    }

    window.draw(distanceSprite);
    if (distanceCheck) {
        window.draw(tile_1Sprite);
        window.draw(tile_2Sprite);
        window.draw(tile_3Sprite);
        window.draw(tile_4Sprite);
        window.draw(tile_5Sprite);
    }
}



double Board::GetPersonX() { return personX; }
double Board::GetPersonY() { return personY; }
void Board::SetPersonX(double coorX) { personX = coorX; }
void Board::SetPersonY(double coorY) { personY = coorY; }
bool Board::GetChecked() { return checked; }
void Board::SetChecked(bool cond) { checked = cond; }
string Board::GetType() { return type; }
void Board::setDistance(int dist) { distance = dist; }
int Board::GetDistance() { return distance; }
void Board::SetCars(vector<Car> cars) { this->cars = cars; }
vector<Car> Board::GetCars() { return cars; }
void Board::SetRatioX(double ratio) { ratioX = ratio; }
double Board::GetRatioX() { return ratioX; }
void Board::SetRatioY(double ratio) { ratioY = ratio; }
double Board::GetRatioY() { return ratioY; }
bool Board::GetReset() { return reset; }

unordered_map<string, sf::Texture> TextureManager::textures;
void TextureManager::LoadTexture(string fileName)
{
    string path = "images/" + fileName;

    textures[fileName].loadFromFile(path);
}

sf::Texture& TextureManager::GetTexture(string textureName)
{
    if (textures.find(textureName) == textures.end())
    {
        LoadTexture(textureName);
    }
    return textures[textureName];
}

void TextureManager::Clear()
{
    textures.clear();
}

Image CropImage(Vector2u& globalPosition, int n)
{
    Image orig, crop;
    int leftBound, rightBound, upBound, downBound;
    leftBound = globalPosition.x - (n * 31) + 1;
    rightBound = globalPosition.x + (n * 31);
    upBound = globalPosition.y - (n * 31);
    downBound = globalPosition.y + (n * 31);
    orig.loadFromFile("images/Tel_Aviv_Map.png");
    crop.create(2 * n * 31, 2 * n * 31, Color::Black);

    int cropX = 0, cropY = 0;
    int testX = leftBound, testY = upBound;
    for (int y = 0; y < 691; y++, testY++)
    {
        for (int x = 0; x < 760; x++, testX++)
        {
            if (x >= leftBound && x <= rightBound && y >= upBound && y < downBound)
            {
                if (cropX == 2 * n * 31)
                    cropX = 0;

                    crop.setPixel(cropX, cropY, orig.getPixel(x, y));
                cropX++;
            }
        }
        if (y >= upBound && y < downBound && cropY <= downBound)
            cropY++;
    }
    return crop;
}

void resizeImage(const sf::Image& originalImage, sf::Image& resizedImage)
{
    const sf::Vector2u originalImageSize{ originalImage.getSize() };
    const sf::Vector2u resizedImageSize{ resizedImage.getSize() };
    for (unsigned int y{ 0u }; y < resizedImageSize.y; ++y)
    {
        for (unsigned int x{ 0u }; x < resizedImageSize.x; ++x)
        {
            unsigned int origX{ static_cast<unsigned int>(static_cast<double>(x) / resizedImageSize.x * originalImageSize.x) };
            unsigned int origY{ static_cast<unsigned int>(static_cast<double>(y) / resizedImageSize.y * originalImageSize.y) };

            resizedImage.setPixel(x, y, originalImage.getPixel(origX, origY));
        }
    }
}

void Board::CheckLeftClick(Vector2i& globalPosition)
{
    cout << globalPosition.x << "," << globalPosition.y << " ";

    if (!checked && type == "map")
    {
        Vector2u test = Vector2u(globalPosition.x, globalPosition.y);
        Image text = CropImage(test, distance);

        Image text2;
        text2.create(760, 691, Color::Black);
        resizeImage(text, text2);
        TextureManager::GetTexture("Tel_Aviv_Map.png").update(text2);
        TextureManager::GetTexture("Tel_Aviv_Map.png").setSmooth(false);
    }
    if (type == "menu") {
        if (globalPosition.x > 211 && globalPosition.x < 411 && globalPosition.y > 381 && globalPosition.y < 461) {
            mapCheck = false;
            cout << "false" << " ";
            optionsCheck = false;
        }
        if (globalPosition.x > 211 && globalPosition.x < 411 && globalPosition.y > 461 && globalPosition.y < 541) {
            mapCheck = true;
            cout << "true";
            optionsCheck = false;
        }
        if (globalPosition.x > 788 && globalPosition.x < 938 && globalPosition.y > 346 && globalPosition.y < 391 && distanceCheck) {
            distance = 1;
            distanceCheck = false;
            ratioX = (double)379 * 1.55;
            ratioY = (double)379 * 1.8258;
        }
        if (globalPosition.x > 788 && globalPosition.x < 938 && globalPosition.y > 391 && globalPosition.y < 436 && distanceCheck) {
            distance = 2;
            distanceCheck = false;
            ratioX = (double)189 * 1.55;
            ratioY = (double)189 * 1.8258;
        }
        if (globalPosition.x > 788 && globalPosition.x < 938 && globalPosition.y > 436 && globalPosition.y < 481 && distanceCheck) {
            distance = 3;
            distanceCheck = false;
            ratioX = (double)127 * 1.55;
            ratioY = (double)127 * 1.8258;
            cout << ratioX;
        }
        if (globalPosition.x > 788 && globalPosition.x < 938 && globalPosition.y > 481 && globalPosition.y < 526 && distanceCheck) {
            distance = 4;
            distanceCheck = false;
            ratioX = (double)94 * 1.55;
            ratioY = (double)94 * 1.8258;
        }
        if (globalPosition.x > 788 && globalPosition.x < 938 && globalPosition.y > 526 && globalPosition.y < 571 && distanceCheck) {
            distance = 5;
            distanceCheck = false;
            ratioX = (double)76 * 1.55;
            ratioY = (double)76 * 1.8258;
        }

        if (globalPosition.x > 211 && globalPosition.x < 411 && globalPosition.y > 301 && globalPosition.y < 381)
        {
            optionsCheck = !optionsCheck;
        }
        if (globalPosition.x > 788 && globalPosition.x < 938 && globalPosition.y > 301 && globalPosition.y < 346)
        {
            distanceCheck = !distanceCheck;
        }
        if (globalPosition.x > 486 && globalPosition.x < 726 && globalPosition.y > 416 && globalPosition.y < 656) {
            checked = true;
        }
    }
    if (type == "map")
    {
        if (!checked && globalPosition.x < 760)
        {
            checked = true;
            personX = globalPosition.x;
            personY = globalPosition.y;
            double seconds = 41.1875 + ((double)globalPosition.x / (double)48);
            pCoorX = 34 + (seconds / (double)60);
            seconds = 9.82332 - ((double)globalPosition.y / 56.6);
            pCoorY = 32 + (seconds / (double)60);
            cout << "[" << pCoorX << ", " << pCoorY << "]"; 
            if(mapCheck)
                cars = FindCars(pCoorY, pCoorX, distance);
            else
                cars = AVLTree(pCoorY, pCoorX, distance);
        }
        if (globalPosition.x > 920 && globalPosition.x < 1120 && globalPosition.y > 575 && globalPosition.y < 655) {
            reset = true;
            cout << "help me this sux";
        }
    }
}

void Board::PrintCars(RenderWindow& window)
{
    list<int> list;
    TextureManager::GetTexture("small_pin.png").setSmooth(true);
    Sprite foundSprite(TextureManager::GetTexture("small_pin.png"));
    
    for (Car car : cars)
    {
        int diffBuffer;
        if (pCoorY > car.GetLatitude())
            diffBuffer = -7;
        if (pCoorY < car.GetLatitude())
            diffBuffer = 27;

        
        double pixDiffX = (double)(car.GetLongitude() - pCoorX) * 60 * ratioX;
        double pixDiffY = (car.GetLatitude() - pCoorY) * 60 * ratioY; 
        foundSprite.setPosition(380 + pixDiffX - 15, 345 - pixDiffY - 30 + diffBuffer);
        window.draw(foundSprite);
    }
}

int main()
{

    RenderWindow window(sf::VideoMode(1200, 691), "GatorRides Menu");

    Board board = Board(691, 1200, "map");
    Board board2 = Board(691, 1200, "menu");
    Board* curr = &board2;
    sf::Event event;
    curr->PrintMenu(window);
    while (window.isOpen())
    {
        while (window.pollEvent(event)) {

            sf::Vector2i globalPosition = sf::Mouse::getPosition(window);
            if (event.type == sf::Event::Closed)
                window.close();

            if (board.GetReset())
            {
                cout << " look at me here";
                board.SetBoard();
                board2.SetBoard();
                Image resetMap;
                resetMap.loadFromFile("images/Tel_Aviv_Map.png");
                TextureManager::GetTexture("Tel_Aviv_Map.png").update(resetMap);
                curr = &board2;
            }

            if (event.type == Event::MouseButtonPressed)
                if (event.mouseButton.button == Mouse::Left)
                {
                    curr->CheckLeftClick(globalPosition);
                    if (board2.GetChecked() == true) {
                        curr = &board;
                        board.setDistance(board2.GetDistance());
                        board.SetRatioX(board2.GetRatioX());
                        board.SetRatioY(board2.GetRatioY());
                    }
                }
        }


        window.clear(Color::Black);
        if (curr->GetType() == "map") {
            board.PrintBoard(window);
        }
        else
            board2.PrintMenu(window);

        window.display();

    }
    return 0;
}