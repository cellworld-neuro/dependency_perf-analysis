#pragma once
#include <string>
#include <chrono>
#include <map>
#include <vector>

#define CONCAT_INNER(a, b) a ## b
#define CONCAT(a, b) CONCAT_INNER(a, b)
#define UNIQUE_NAME(base) CONCAT(base, __COUNTER__)

#ifdef NDEBUG
    #define PERF_SCOPE(T)
    #define PERF_START(T)
    #define PERF_STOP(T)

#else
    #define PERF_SCOPE(T) auto UNIQUE_NAME(scope_performance_analysis_) = performance::Scope_analysis(T)
    #define PERF_START(T) performance::Analysis::performance_analysis().start(T)
    #define PERF_STOP(T) performance::Analysis::performance_analysis().stop(T)
#endif
namespace performance{

    struct Analysis {
        static Analysis &performance_analysis();
        Analysis();
        ~Analysis();
        void start(const std::string &);
        void stop(const std::string &);
        struct Counter{
            explicit Counter(std::string, Counter *);
            int find_child(const std::string &);
            unsigned long get_name_size(int);
            Counter &add_child(const std::string &);
            void aggregate(Counter &);
            void show(int, int, float);
            void reset();
            std::string name;
            std::chrono::time_point<std::chrono::high_resolution_clock> check_point;
            long long int time;
            long call_count;
            Counter *parent;
            std::vector<Counter *> children;
            long thread_count{};
        };
        Counter root_counter;
        Counter *open_counter;
        friend struct Summary;
    };

    struct Summary{
        void aggregate(Analysis &);
        void show_report();
        ~Summary();
        std::chrono::time_point<std::chrono::high_resolution_clock> performance_analysis_start_time = std::chrono::high_resolution_clock::now();
        Analysis::Counter root_counter{"main", nullptr};
    };

    struct Scope_analysis{
        explicit Scope_analysis(const std::string &name) : name(name){
            PERF_START(name);
        }
        ~Scope_analysis(){
            PERF_STOP(name);
        }
        const std::string name;
    };
}
