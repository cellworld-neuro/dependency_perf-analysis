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

    void Summary::write_report() {
        ofstream report;
        report.open("perf_report.html");
        report <<
        "<!DOCTYPE html>\n"
        "<html>\n"
        "<style>\n"
        "\n"
        "  .tree{\n"
        "    --spacing : 1.5rem;\n"
        "    --radius  : 10px;\n"
        "    \n"
        "  }\n"
        "\n"
        "  .tree li{\n"
        "    display      : block;\n"
        "    position     : relative;\n"
        "    padding-left : calc(2 * var(--spacing) - var(--radius) - 2px);\n"
        "  }\n"
        "\n"
        "  .tree ul{\n"
        "    margin-left  : calc(var(--radius) - var(--spacing));\n"
        "    padding-left : 0;\n"
        "  }\n"
        "\n"
        "  .tree ul li{\n"
        "    border-left : 2px solid #ddd;\n"
        "  }\n"
        "\n"
        "  .tree ul li:last-child{\n"
        "    border-color : transparent;\n"
        "  }\n"
        "\n"
        "  .tree ul li::before{\n"
        "    content      : '';\n"
        "    display      : block;\n"
        "    position     : absolute;\n"
        "    top          : calc(var(--spacing) / -2);\n"
        "    left         : -2px;\n"
        "    width        : calc(var(--spacing) + 2px);\n"
        "    height       : calc(var(--spacing) + 1px);\n"
        "    border       : solid #ddd;\n"
        "    border-width : 0 0 2px 2px;\n"
        "  }\n"
        "\n"
        "  .tree summary{\n"
        "    display : block;\n"
        "    cursor  : pointer;\n"
        "  }\n"
        "\n"
        "  .tree summary::marker,\n"
        "  .tree summary::-webkit-details-marker{\n"
        "    display : none;\n"
        "  }\n"
        "\n"
        "  .tree summary:focus{\n"
        "    outline : none;\n"
        "  }\n"
        "\n"
        "  .tree summary:focus-visible{\n"
        "    outline : 1px dotted #000;\n"
        "  }\n"
        "\n"
        "  .tree li::after,\n"
        "  .tree summary::before{\n"
        "    content       : '';\n"
        "    display       : block;\n"
        "    position      : absolute;\n"
        "    top           : calc(var(--spacing) / 2 - var(--radius));\n"
        "    left          : calc(var(--spacing) - var(--radius) - 1px);\n"
        "    width         : calc(2 * var(--radius));\n"
        "    height        : calc(2 * var(--radius));\n"
        "    border-radius : 50%;\n"
        "    background    : #ddd;\n"
        "  }\n"
        "\n"
        "  .tree summary::before{\n"
        "    z-index    : 1;\n"
        "    background : #696 url('data:image/svg+xml;base64,PD94bWwgdmVyc2lvbj0iMS4wIj8+CjxzdmcgeG1sbnM9Imh0dHA6Ly93d3cudzMub3JnLzIwMDAvc3ZnIiB3aWR0aD0iNDAiIGhlaWdodD0iMjAiPgogIDxnIGZpbGw9IiNmZmYiPgogICAgPHBhdGggZD0ibTUgOWg0di00aDJ2NGg0djJoLTR2NGgtMnYtNGgtNHoiLz4KICAgIDxwYXRoIGQ9Im0yNSA5aDEwdjJoLTEweiIvPgogIDwvZz4KPC9zdmc+Cg==') 0 0;\n"
        "  }\n"
        "\n"
        "  .tree details[open] > summary::before{\n"
        "    background-position : calc(-2 * var(--radius)) 0;\n"
        "  }\n"
        "\n"
        "</style>\n"
        "<body class=\"tree\">\n";
        root_counter.write(report, 100,1);
        report <<
        "</body>\n"
        "</html>";
        report.close();
    }


    Summary::~Summary() {
        show_report();
        write_report();
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

    void Analysis::Counter::write(std::ofstream &report, int name_size, float r) {
        report << "<li><details><summary>";
        report << name;
        report << std::right << std::setw(10) << thread_count;
        report << std::right << std::setw(10) << call_count;
        report << std::right << std::setw(10) << fixed << setprecision(2) << ((float) time) / 1000000.0;
        report << std::right << std::setw(10) << fixed << "<meter id=\"disk_d\" value=\"" << r << "\">" << setprecision(2) << (r * 100.0) << "%</meter>";
        report << "</summary>";
        long long int children_total = 0;
        if (!children.empty()) {
            report << "<ul>";
        }
        for (auto &c:children) {
            children_total += c->time;
            c->write(report, name_size, r * ( (float)c->time / (float) time));
        }
        if (!children.empty()) {
            report << "<li><summary>";
            report << "Untracked";
            report << std::right << std::setw(10) << fixed << setprecision(2) << ((float)time - children_total) / 1000000.0;
            report << std::right << std::setw(10) << fixed << "<meter id=\"disk_d\" value=\"" << ((float)(time - children_total) / (float) time * r) << "\">" << setprecision(2) << ((float)(time - children_total) / (float) time * 100.0 * r) << "%</meter>";
            report << "</summary>";
            report << "</li>";
            report << "</ul>";
        }
        report << "</details>";
        report << "</li>";
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