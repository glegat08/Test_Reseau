#pragma once

#include <thread>
#include <mutex>
#include <chrono>
#include <print>

inline std::mutex       vecMutex;
inline std::vector<int> vector;
inline std::atomic_bool hasAnyoneWon = false;

inline void timer(std::chrono::milliseconds(delay))
{
	std::this_thread::sleep_for(delay);
}

inline void consumerThread(std::stop_token st, uint32_t contestantId)
{
    using namespace std::chrono_literals;
    int sum = 0;

    while (!st.stop_requested())
    {
        int number;

        {
            std::scoped_lock lock(vecMutex);
            if (vector.empty())
            {
                timer(0ms);
                continue;
            }

            number = vector.back();
            vector.pop_back();
        }

        sum += number;
        if (sum >= 100)
        {
            hasAnyoneWon = true;
            break;
        }

        timer(0ms);
    }

    if (!st.stop_requested())
        std::println("I WON! :) (I am contestant {}, TID {}, total {})", contestantId, std::this_thread::get_id(), sum);
    else
        std::println("I lost :( (I am contestant {}, TID {}, total {})", contestantId, std::this_thread::get_id(), sum);
}

inline int Test()
{
    using namespace std::chrono_literals;

    srand(time(nullptr));

    std::jthread t1(consumerThread, 1);
    std::jthread t2(consumerThread, 2);

    while (!hasAnyoneWon)
    {
        {
            std::scoped_lock lock(vecMutex);
            vector.push_back(rand() % 10 + 1);
        }

        using namespace std::chrono_literals;
        timer(100ms);
    }

    return 0;
}

inline void Talking(std::stop_token st, uint32_t userId)
{
	
}