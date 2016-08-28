#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <unordered_map>
#include <cassert>

class OrderBook{
public:
    void insert(uint32_t&& id, double&& price)
    {
        auto res = mPriceToCount.emplace(std::make_pair(price, 1));
        auto it = res.first;
        mIdToIterator[id] = it;
        if(!res.second){
            ++res.first->second;
        }
        getHighestPrice();
    }
    void erase(uint32_t&& id)
    {
        std::map<uint32_t,uint32_t>::iterator it = mIdToIterator[id];
        auto price = it->first;
        auto count = it->second;
        if(count > 1)
        {
            --it->second;
        }
        else
        {
            mPriceToCount.erase(it);
        }
        mIdToIterator.erase(id);
    }
    uint32_t getHighestPrice()
    {
        auto it = mPriceToCount.rbegin();
        return it->first;
    }
    bool empty()
    {
        return mPriceToCount.empty();
    }
private:
    std::map<uint32_t,uint32_t> mPriceToCount;
    std::unordered_map<uint32_t, std::map<uint32_t,uint32_t>::iterator> mIdToIterator;
};


char getOrderType(const std::string& line)
{
    char orderType;
    std::size_t found = line.find_first_of(" ");
    if (found!=std::string::npos)
    {
      orderType = line[found + 1];
    }
    return orderType;
}

double getPrice(const std::string& line)
{
    double price;
    std::size_t found = line.find_last_of(" ");
    if (found!=std::string::npos)
    {
        price = std::stod(line.substr(found + 1));
    }
    return price;
}

uint32_t getId(const std::string& line)
{
    uint32_t id;
    std::size_t start = line.find_first_of("IE");
    std::size_t end = line.find_last_of(" ");
    if (start!=std::string::npos & end!=std::string::npos)
    {
        start = start + 2;
        auto length = end - start;
        id = std::stoi(line.substr(start, length), nullptr);
    }
    return id;
}

uint32_t getTimeStamp(const std::string& line)
{
    uint32_t timeStamp;
    std::size_t found = line.find_first_of(" ");
    if (found!=std::string::npos)
    {
        std::string tString = line.substr(0, found);
        timeStamp = std::stoi(tString, nullptr);
    }
    return timeStamp;
}

void processOrder(const std::string& line, OrderBook& orderBook)
{
    char orderType = getOrderType(line);
    if(orderType == 'I')
    {
         orderBook.insert(getId(line),
                          getPrice(line));
    }
    else if (orderType == 'E')
    {
        orderBook.erase(getId(line));
    }
}

int main(int argc, char** argv)
{
    assert(argc == 2  && "Takes a single command line argument of the file name.");
    std::ifstream marketData;
    marketData.open(argv[1]);
    std::string line;
    OrderBook orderBook;
    uint32_t prev;
    uint32_t time;
    uint32_t elapsed = 0;
    double weightedHighestPriceSum = 0;
    if(std::getline(marketData, line))
    {
        prev = getTimeStamp(line);
        do
        {
            time = getTimeStamp(line);
            auto timeSegment = time - prev;
            if(!orderBook.empty())
            {
                elapsed += timeSegment;
                weightedHighestPriceSum += orderBook.getHighestPrice() * timeSegment;
            }
            processOrder(line, orderBook);
            prev = time;
        } while(std::getline(marketData, line));
    }
    if(elapsed > 0){
        std::cout << weightedHighestPriceSum / elapsed << std::endl;
    }
    else
    {
        std::cout << 0 << std::endl;
    }
    return 0;
}
