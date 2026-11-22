#include <gamma/utils/arena.hpp>
#include <map>
#include <print>
#include <thread>
#include <vector>

std::map<std::string, std::string> before_main = {
    { "klucz_globalny_przekraczajacy_limit_SSO", "wartosc_zainicjalizowana_przed_main" }
};

int main()
{
    std::println("Przed main() zużyto {}B, zatwierdzono {}MB/{}GB", gamma::utils::arena_used(), gamma::utils::arena_committed_bytes() / (1024z * 1024z), gamma::utils::ARENA_CAPACITY / (1024z * 1024z * 1024z));

    std::vector<std::thread> threads;
    for (int t = 0; t < 4; ++t)
    {
        threads.emplace_back([t]()
            {
                std::map<std::string, std::vector<int>> local;
                for (int i = 0; i < 2000; ++i)
                {
                    std::string k = "watek_" + std::to_string(t) + "_klucz_bez_SSO_" + std::to_string(i);
                    local[k] = std::vector<int>(512, i);
                } });
    }

    for (auto &th : threads)
    {
        th.join();
    }

    std::println("Po pracy wątków: zużyto {:.2f}MB, zatwierdzono {}MB",
        static_cast<double>(gamma::utils::arena_used()) / (1024.0 * 1024.0),
        gamma::utils::arena_committed_bytes() / (1024 * 1024));

    return 0;
}
