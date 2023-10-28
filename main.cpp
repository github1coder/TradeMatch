#include <iostream>
#include <map>
#include <vector>
#include <sys/timeb.h>
using namespace std;

//订单的键和值
struct Order_Key {
    int price;
    unsigned long timestamp_id;

    Order_Key(unsigned long timestamp, int p) : timestamp_id(timestamp), price(p) {}
};
struct Order {
    char operate;  // B or S
    int price;
    int quantity;
    unsigned long timestamp_id; //采用时间戳当订单号

    Order(unsigned long timestamp, char s, int p, int q) : timestamp_id(timestamp), operate(s), price(p), quantity(q) {
        cout << "创建订单成功，订单号为" + to_string(timestamp_id) << endl;
    }
};

//订单排序
struct CmpByKey_asc {
    bool operator()(const Order_Key& k1, const Order_Key& k2) const {
        if (k1.price < k2.price) return true;
        else if (k1.price == k2.price && k1.timestamp_id < k2.timestamp_id) return true;
        return false;
    }
};
struct CmpByKey_desc {
    bool operator()(const Order_Key& k1, const Order_Key& k2) const {
        if (k1.price > k2.price) return true;
        else if (k1.price == k2.price && k1.timestamp_id < k2.timestamp_id) return true;
        return false;
    }
};


class OrderBook {
private:
    unordered_map<unsigned long, Order_Key> orders; //根据订单号定位订单的键，实现快速删除订单
    map<Order_Key, Order*, CmpByKey_desc> buy_orders; //用红黑树排序，实现订单的快速匹配，插入和删除
    map<Order_Key, Order*, CmpByKey_asc> sell_orders;

    void addOrder(Order* order) {
        Order_Key order_key = Order_Key(order->timestamp_id, order->price);
        orders.insert({order->timestamp_id, order_key});
        if (order->operate == 'B') buy_orders.insert({order_key, order});
        else if (order->operate == 'S') sell_orders.insert({order_key, order});
        cout << "添加订单到订单簿成功，订单号为" + to_string(order->timestamp_id) << endl;
    }

    //根据当前订单，得到匹配的订单，若没有，返回空
    Order* getMatchOrder(Order* order) {
        Order* match_order = nullptr;
        if (order->quantity > 0) {
            if (order->operate == 'B' && !sell_orders.empty()) {
                auto it = sell_orders.begin();
                if (order->price >= it->second->price) match_order = it->second;
            } else if (order->operate == 'S' && !buy_orders.empty()) {
                auto it = buy_orders.begin();
                if (order->price <= it->second->price) match_order = it->second;
            }
        }
        return match_order;
    }

    //处理两订单间的交易
    void handleMatchOrder(Order* order, Order* match_order) {
        int match_quantity = min(order->quantity, match_order->quantity);
        order->quantity -= match_quantity;
        match_order->quantity -= match_quantity;
        cout << "订单匹配成功，交易量为" + to_string(match_quantity) + ",交易的订单号为" + to_string(order->timestamp_id) + "和" + to_string(match_order->timestamp_id)<< endl;
    }

public:
    void removeOrder(unsigned long timestamp_id) {
        auto it = orders.find(timestamp_id);
        if (it != orders.end()) {
            Order_Key order_key = it->second;
            orders.erase(it);
            buy_orders.erase(order_key);
            sell_orders.erase(order_key);
            cout << "删除订单成功，订单号为" + to_string(timestamp_id) << endl;
        } else {
            cout << "删除订单失败，无此订单，订单号为" + to_string(timestamp_id) << endl;
        }
    }

    //根据当前订单，进行撮合交易，挂单等操作
    void matchOrder(Order* order) {
        Order* match_order = getMatchOrder(order);
        while (match_order) {
            handleMatchOrder(order, match_order);
            if (match_order->quantity == 0) {
                cout << "该订单交易已全部完成，订单号为" + to_string(match_order->timestamp_id) << endl;
                removeOrder(match_order->timestamp_id);
            }
            match_order = getMatchOrder(order);
        }
        if (order->quantity > 0) {
            addOrder(order);
        } else {
            cout << "该订单交易已全部完成，订单号为" + to_string(order->timestamp_id) << endl;
        }
    }
};

int main() {
    char operate;
    int price, quantity;
    unsigned long timestamp_id;
    timeb t;
    OrderBook* orderBook = new OrderBook();
    while(1) {
        cout << "请输入一个操作字符，代表要进行的操作（B代表买入订单，S代表卖出订单，D代表删除订单）" << endl;
        cin >> operate;
        if (operate == 'B' || operate == 'S') {
            cout << "依次请输入订单的价格和买卖数量" << endl;
            cin >> price >> quantity;
            ftime(&t);
            timestamp_id = t.time * 1000 + t.millitm;
            Order* order = new Order(timestamp_id, operate, price, quantity);
            orderBook->matchOrder(order);
        } else if (operate == 'D') {
            cout << "请输入要删除的订单号" << endl;
            cin >> timestamp_id;
            orderBook->removeOrder(timestamp_id);
        } else {
            cout << "您输入的操作字符错误，请重新输入" << endl;
        }
    }
    return 0;
}
