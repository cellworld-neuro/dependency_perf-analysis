#include <performance.h>
#include <iostream>
#include <iomanip>
#include <mutex>
#define PADDING 2
using namespace std;

namespace performance {

    Analysis::Analysis() :
        root_counter("main", nullptr){
        root_counter.thread_count = 1;
        open_counter = &root_counter;
    }


    thread_local Analysis analysis;

    Summary summary;

    Analysis &Analysis::performance_analysis(){
        return analysis;
    }

    mutex mtx;
    Analysis::~Analysis() {
        stop("main");
        mtx.lock();
        summary.root_counter.aggregate(root_counter);
        mtx.unlock();
    }

    void Summary::show_report() {
        auto elapsed = std::chrono::high_resolution_clock::now() - performance_analysis_start_time;
        long long microseconds = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
        std::cout << std::endl;
        std::cout << "Total execution time: " << fixed << setprecision(2) << ((float) microseconds) / 1000000.0 << "s" << std::endl;
        std::cout << "Threads monitored: " << root_counter.thread_count << std::endl;
        std::cout << "Threads total running time: " << fixed << setprecision(2) << (float)root_counter.time / 1000000.0 << "s" << std::endl;
        std::cout << std::endl;
        std::cout << "Performance Analysis Counters " << std::endl;
        int name_size = root_counter.get_name_size(0) + 1;
        std::cout << std::string(name_size + 40, '-') << std::endl;
        std::cout << std::left << std::setw(name_size) << "counter";
        std::cout << std::right << std::setw(10) << "threads";
        std::cout << std::right << std::setw(10) << "calls";
        std::cout << std::right << std::setw(10) << "time(s)";
        std::cout << std::right << std::setw(10) << "time(%)";
        std::cout << endl;
        root_counter.show(0, name_size, 1);
        std::cout << std::string(name_size + 40, '-') << std::endl;
        std::cout << endl;
    }

    Summary::~Summary() {
        show_report();
    }

    void Summary::aggregate(Analysis &) {

    }

    void Analysis::start(const string &counter_name) {
        auto i = open_counter->find_child(counter_name);
        if (i==-1){
            open_counter = &open_counter->add_child(counter_name);
            open_counter->thread_count = 1;
        } else {
            open_counter = open_counter->children[i];
            open_counter->reset();
        }
    }

    void Analysis::stop(const string &counter_name) {
        if (open_counter->name == counter_name){
            auto elapsed = std::chrono::high_resolution_clock::now() - open_counter->check_point;
            long long microseconds = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
            open_counter->time += microseconds;
            open_counter->call_count ++;
            open_counter = open_counter->parent;
        } else {
            throw logic_error("Counter name " + counter_name + " does not match current open counter: " + open_counter ->name);
        }
    }

    Analysis::Counter::Counter(std::string name, Counter *parent):
        name (std::move(name)),
        time (0),
        call_count (0),
        parent(parent){
        reset();
    }

    int Analysis::Counter::find_child(const std::string &child_name) {
        for (size_t i=0; i < children.size(); i++){
            if (children[i]->name == child_name) return i;
        }
        return -1;
    }

    unsigned long Analysis::Counter::get_name_size(int padding) {
        unsigned long size = name.size() + padding;
        for (auto &c:children){
            auto child_size = c->get_name_size(padding + PADDING);
            if (child_size > size){
                size = child_size;
            }
        }
        return size;
    }

    void Analysis::Counter::show(int padding, int name_size, float r) {
        if (padding > PADDING) {
            std::cout << std::string(padding - PADDING, ' ');
            std::cout << std::string(PADDING, '-');
        } else {
            if (padding > 0) {
                std::cout << std::string(PADDING, '-');
            }
        }
        std::cout  <<  std::left << std::setw(name_size - padding) << name;
        std::cout <<  std::right << std::setw(10) << thread_count;
        std::cout <<  std::right << std::setw(10) << call_count;
        std::cout <<  std::right << std::setw(10) << fixed << setprecision(2) << ((float) time) / 1000000.0;
        std::cout  <<  std::right << std::setw(10) << fixed << setprecision(2) << (r * 100.0);
        std::cout << std::endl;
        long long int children_total = 0;
        for (auto &c:children) {
            children_total += c->time;
            c->show(padding + PADDING, name_size, r * ( (float)c->time / (float) time));
        }
        if (!children.empty()) {
            std::cout << std::string(padding, ' ');
            std::cout  <<  std::left << std::setw(name_size - padding + 20) << "Untracked:";
            std::cout <<  std::right << std::setw(10) << fixed << setprecision(2) << ((float)time - children_total) / 1000000.0;
            std::cout << std::right << std::setw(10) << fixed << setprecision(2) << ((float)(time - children_total) / (float) time * 100.0 * r);
            std::cout << std::endl;
        }
    }

    void Analysis::Counter::reset() {
        check_point = std::chrono::high_resolution_clock::now();
    }

    void Analysis::Counter::aggregate(Counter &new_counter) {
        thread_count += new_counter.thread_count;
        time += new_counter.time;
        call_count += new_counter.call_count;
        for (auto &c:new_counter.children){
            auto i = find_child(c->name);
            if(i==-1){
                auto &nc = children.emplace_back(c);
                nc->parent = this;
            } else {
                children[i]->aggregate(*c);
            }
        }
    }

    Analysis::Counter &Analysis::Counter::add_child(const string &name) {
        auto new_child = new Analysis::Counter(name, this);
        children.emplace_back(new_child);
        return *new_child;
    }
}