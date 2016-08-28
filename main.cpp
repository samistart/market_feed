#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <unordered_map>
#include <cassert>
#include <sstream>
#include <chrono>
#include <limits>
#include <cmath>

class OrderBook{
public:
    void insert(uint32_t id, double price)
    {
        auto res = mPriceToCount.emplace(std::make_pair(price, 1));
        auto it = res.first;
        mIdToIterator[id] = it;
        if(!res.second){
            ++res.first->second;
        }
    }
    void erase(uint32_t id)
    {
        auto it = mIdToIterator[id];
        auto& count = it->second;
        if(count > 1)
        {
            --count;
        }
        else
        {
            mPriceToCount.erase(it);
        }
        mIdToIterator.erase(id);
    }
    double getHighestPrice()
    {
        if(!mPriceToCount.empty()){
            return mPriceToCount.rbegin()->first;
        }
        else
        {
            return std::numeric_limits<double>::quiet_NaN();
        }
    }
private:
    std::map<double,uint32_t> mPriceToCount;
    std::unordered_map<uint32_t, decltype(mPriceToCount)::iterator> mIdToIterator;
};


char getOrderType(const std::string& line)
{
    std::size_t found = line.find_first_of(" ");
    return line[found + 1];
}

double getPrice(const std::string& line)
{
    std::size_t found = line.find_last_of(" ");
    return std::stod(line.substr(found + 1));
}

uint32_t getId(const std::string& line)
{
    std::size_t start = line.find_first_of("IE");
    std::size_t end = line.find_last_of(" ");
    start = start + 2;
    auto length = end - start;
    return std::stoi(line.substr(start, length), nullptr);
}

uint32_t getTimeStamp(const std::string& line)
{
    std::size_t found = line.find_first_of(" ");
    std::string tString = line.substr(0, found);
    return std::stoi(tString, nullptr);
}

void processOrder(const std::string& line, OrderBook& orderBook)
{
    char orderType = getOrderType(line);
    if(orderType == 'I')
    {
         orderBook.insert(getId(line), getPrice(line));
    }
    else if (orderType == 'E')
    {
        orderBook.erase(getId(line));
    }
}

int main(int argc, char** argv)
{
    using std::chrono::steady_clock;
    auto start = steady_clock::now();
    assert(argc == 2  && "Takes a single command line argument of the file name.");
    std::ifstream marketData(argv[1]);
    std::stringstream buffer;
    if ( marketData )
    {
        buffer << marketData.rdbuf();
        marketData.close();
    }
    std::string line;
    OrderBook orderBook;
    uint32_t prev;
    uint32_t t;
    uint32_t elapsed = 0;
    double weightedHighestPriceSum = 0;
    if(std::getline(buffer, line))
    {
        prev = getTimeStamp(line);
        do
        {
            t = getTimeStamp(line);
            auto timeSegment = t - prev;
            volatile double highestPrice = orderBook.getHighestPrice();
            if(!std::isnan(highestPrice))
            {
                elapsed += timeSegment;
                weightedHighestPriceSum += orderBook.getHighestPrice() * timeSegment;
            }
            processOrder(line, orderBook);
            prev = t;
        } while(std::getline(buffer, line));
    }
    if(elapsed > 0){
        std::cout << weightedHighestPriceSum / elapsed << "\n";
    }
    else
    {
        std::cout << "No orders found in market." << "\n";
    }
    auto end = steady_clock::now();
    double elapsedSeconds = ((end - start).count()) * steady_clock::period::num / static_cast<double>(steady_clock::period::den);
    std::cout << elapsedSeconds << "s taken." << "\n";
    return 0;
}
