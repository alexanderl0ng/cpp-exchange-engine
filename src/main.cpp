#include <inttypes.h> // IWYU pragma: keep
#include <vector>
#include <iostream>
#include <random>
#include <chrono>

// TODO add raylib srcs to third_party, statically link in
#include <raylib.h>

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

typedef std::int32_t i32;
typedef std::int64_t i64;
typedef std::uint32_t u32;
typedef std::uint64_t u64;

enum Side
{
    Buy,
    Sell
};

struct Order
{
    u32 orderId;
    u32 quantity;
    u32 price;
    Side side;
};

struct Trade
{
    u64 orderId;
    u32 quantity;
    u32 price;
};

u32 uniform_rng(std::mt19937 &rng, u32 min, u32 max)
{
    // std::random_device dev;
    // std::uniform_int_distribution<std::mt19937::result_type> uni_dist(min, max);
    std::uniform_int_distribution<int> uni_dist(min, max);

    return uni_dist(rng);
}

u32 normal_rng(std::mt19937 &rng, u32 mean, double stddev)
{
    std::normal_distribution<double> nor_dist(mean, stddev);
    // std::mt19937 rng(dev());
    // std::normal_distribution<std::mt19937::result_type> nor_dist(mean, stddev);

    double value = nor_dist(rng);
    return (u32) std::round(std::max(value, 0.0) * 100);
}

Order create_order(u32 cur_price, u32 orderId)
{
    // generate random trades

    std::random_device dev;
    std::mt19937 rng(dev());

    Order order;
    order.quantity = uniform_rng(rng, 1, 10) * 10;
    order.price = normal_rng(rng, cur_price, cur_price * 0.005);
    order.orderId = orderId;

    u32 side = uniform_rng(rng, 1, 2);
    if (side == 1)
    {
        order.side = Buy;
    }
    else
    {
        order.side = Sell;
    }

    return order;
}

std::vector<Order> generate_orders(u32 cur_price, u32 orderId)
{
    const double orders_per_second = 0.5;

    static auto prev_time = std::chrono::system_clock::now();
    static double accumulated_pending_orders = 0.0;
    auto current_time = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_time = current_time - prev_time;
    accumulated_pending_orders += elapsed_time.count() * orders_per_second;
    prev_time = current_time;

    double desired_orders = std::floor(accumulated_pending_orders);
    accumulated_pending_orders -= desired_orders;

    // TODO: sample from a distribution to introduce randomness in the number of orders (must be above 0)

    std::vector<Order> new_orders;
    for (int i = 0; i < desired_orders; i++)
    {
        Order order = create_order(cur_price, orderId);
        new_orders.push_back(order);
        orderId += 1;
    }

    return new_orders;
}

struct Market
{
    std::vector<Order> bid_orders;
    std::vector<Order> ask_orders;
};

void print_market(std::vector<Order> bid_orders, std::vector<Order> ask_orders)
{
    std::cout << "Bids" << "                          " << "Asks" << '\n';
    std::cout << "Price, Quantity" << "               " << "Price, Quantity" << '\n';

    int size = (bid_orders.size() >= ask_orders.size()) ? bid_orders.size() : ask_orders.size();

    for (int i = 0; i < size; i++)
    {
        bool bid_valid = (i < bid_orders.size());
        bool ask_valid = (i < ask_orders.size());

        if (bid_valid)
        {
            std::cout << bid_orders[i].price << ", " << bid_orders[i].quantity;
        }
        else
        {
            std::cout << "                ";
        }
        std::cout << "                    ";
        if (ask_valid)
        {
            std::cout << ask_orders[i].price << ", " << ask_orders[i].quantity;
        }
        std::cout << std::endl;
    }
}

int main()
{
    Market market = {};
    u32 cur_price = 100;
    u32 orderId = 0;

    int width = 500;
    int height = 500;

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(width, height, "Matching Engine");
    SetTargetFPS(120);

    enum ButtonState
    {
        OPEN_BUTTON,
        CLOSE_BUTTON
    };

    ButtonState market_state = OPEN_BUTTON;

    Rectangle market_button_rect = { 24, 24, 120, 36 };

    while (!WindowShouldClose())
    {
        std::vector<Order> inc_orders = generate_orders(cur_price, orderId);
        auto& bid_orders = market.bid_orders;
        auto& ask_orders = market.ask_orders;

        for (int i = 0; i < inc_orders.size(); i++)
        {
            if (inc_orders[i].side == Buy)
            {
                bid_orders.push_back(inc_orders[i]);
                std::sort(bid_orders.begin(), bid_orders.end(), [](Order a, Order b) {
                    if (a.price == b.price)
                        return a.orderId < b.orderId;
                    return a.price > b.price; // descending order
                });
            }
            else
            {
                ask_orders.push_back(inc_orders[i]);
                std::sort(ask_orders.begin(), ask_orders.end(), [](Order a, Order b) {
                    if (a.price == b.price)
                        return a.orderId < b.orderId;
                    return a.price < b.price; // ascending order
                });
            }
        }

        switch(market_state)
        {
            case OPEN_BUTTON:
                if (GuiButton(market_button_rect, " Open Market"))
                {
                    market_state = CLOSE_BUTTON;
                }
                break;

            case CLOSE_BUTTON:
                if (GuiButton(market_button_rect, "Close Market"))
                {
                    market_state = OPEN_BUTTON;
                }
                print_market(bid_orders, ask_orders);
                break;
        }

        //------------------------------------- DRAW -------------------------------------

        BeginDrawing();
        ClearBackground(RAYWHITE);

        width = GetRenderWidth();
        height = GetRenderHeight();

        EndDrawing();
    }

    CloseWindow();

    return 0;
}

