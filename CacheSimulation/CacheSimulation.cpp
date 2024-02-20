/*
Thomas Martin
CDA 3101
Cache Analysis
*/

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

using namespace std;

// store options for cache configurations
class Cache_Options {
    size_t cache_size_bytes = 0;
    size_t line_size_bytes = 0;
    size_t total_line = 0;
    string cache_type;

public:
    Cache_Options(size_t cache_size, size_t line_size) {
        cache_size_bytes = cache_size;
        line_size_bytes = line_size;
        total_line = cache_size / line_size;
    }
    size_t get_size() {
        return cache_size_bytes;
    }
    size_t get_line_size() {
        return line_size_bytes;
    }
    size_t get_line_count() {
        return total_line;
    }
};

// initialize static lookup table
std::map<unsigned char, char> binaryToHex = std::map<unsigned char, char>{
        {0, '0'},
        {1, '1'},
        {2, '2'},
        {3, '3'},
        {4, '4'},
        {5, '5'},
        {6, '6'},
        {7, '7'},
        {8, '8'},
        {9, '9'},
        {10, 'A'},
        {11, 'B'},
        {12, 'C'},
        {13, 'D'},
        {14, 'E'},
        {15, 'F'},
};

// used for binary lookup of hex digits
std::map<unsigned char, string> hex_lookup = std::map<unsigned char, string>{
        {'0', "0000"},
        {'1', "0001"},
        {'2', "0010"},
        {'3', "0011"},
        {'4', "0100"},
        {'5', "0101"},
        {'6', "0110"},
        {'7', "0111"},
        {'8', "1000"},
        {'9', "1001"},
        {'a', "1010"},
        {'b', "1011"},
        {'c', "1100"},
        {'d', "1101"},
        {'e', "1110"},
        {'f', "1111"},
};

// convert given hex string to binary
string hex_to_binary(const string& str) {
    string result;
    for (auto& c : str) {
        result += hex_lookup[tolower(c)];
    }
    return result;
}

// base cache class
template<typename T>
struct cache {

    //
    unsigned int hits = 0;
    unsigned int misses = 0;
    unsigned counter = 0;
    string name;
    string replacement_mode;

    virtual T get_cache() = 0;
    float get_hit_rate() {
        return (get_cache().size() > 0) ? (float)hits / (float)(hits + misses) : 0.0f;
    }

    void print_results(size_t output_width) {
        cout << fixed << setprecision(2) << setfill('-');

        string header;
        if (name == "Direct Map") {
            header = "______________ " + name + " _______________";
        }
        else if (name == "Fully Associative") {
            header = "___________ " + name + " ___________";
        }
        else if (name.find("Set") != string::npos) {
            header = "_________ " + name + " _________";
        }

        cout << header << endl;
        cout << "Hits: " << setw(output_width - 6) << right << hits << endl;
        cout << "Misses: " << setw(output_width - 8) << right << misses << endl;
        cout << "Hit rate: " << setw(output_width - 11) << right << get_hit_rate() * 100 << "%" << endl;
        cout << endl;
    }
};

struct direct_map : cache<vector<pair<int, int>>> {

    // index -> set -> <tag, count>
    vector<pair<int, int>> cache;

    direct_map() {
        name = "Direct Map";
    }
    direct_map(Cache_Options& options) {
        name = "Direct Map";
        for (auto i = 0; i < options.get_line_count(); i++) {
            cache.push_back({ -1, 0 });
        }
    }

    vector<pair<int, int>> get_cache() {
        return cache;
    }
};


struct fully_associative : cache<vector<pair<int, int>>> {
    // index -> set -> <tag, count>
    vector<pair<int, int>> cache;

    int first_out_index = 0;

    fully_associative() {
        name = "Fully Associative";
    }

    fully_associative(Cache_Options& options, string replacement_mode = "LRU") {
        name = "Fully Associative";
        this->replacement_mode = replacement_mode;
        for (auto i = 0; i < options.get_line_count(); i++) {
            cache.push_back({ -1, 0 });
        }
    }

    vector<pair<int, int>> get_cache() {
        return cache;
    }

    void process_miss(int bin_tag_fully_associative) {

        // record miss
        ++misses;

        // used to find least recently used
        int least_recent = 0;

        // assume a replacement is needed
        bool replace_required = true;

        // loop over the face items
        for (auto i = 0; i < cache.size(); i++) {

            // if the spot is empty, no replacement is needed
            if (cache[i].first == -1 && replace_required) {
                cache[i].first = bin_tag_fully_associative;
                cache[i].second = ++counter;
                replace_required = false;
                break;
            }

            // update least_recent
            if (cache[i].second <= cache[least_recent].second) {
                least_recent = i;
            }
        }

        // handle necessary replacements
        if (replace_required) {
            if (replacement_mode == "LRU") {
                cache[least_recent].first = bin_tag_fully_associative;
                cache[least_recent].second = ++counter;
            }
            else if (replacement_mode == "FIFO") {
                cache[first_out_index].first = bin_tag_fully_associative;
                cache[first_out_index].second = ++counter;
                first_out_index++;
                if (first_out_index > cache.size()) {
                    first_out_index = 0;
                }
            }
        }
    }
};

struct set_associative : cache<vector<vector<pair<int, int>>>> {
    // index -> set -> <tag, count>
    vector<vector<pair<int, int>>> cache;
    int associativity = 2;
    vector<int> first_out_indices;

    set_associative() {
        name = "Set Associative " + to_string(associativity) + " Way";
        vector<int> v(associativity);
        first_out_indices = v;
    }
    set_associative(Cache_Options& options, int associativity = 2, string replacement_mode = "LRU") {

        this->associativity = associativity;
        name = "Set Associative " + to_string(associativity) + " Way";

        this->replacement_mode = replacement_mode;

        int set_number = options.get_line_count() / associativity;
        vector<int> v(associativity);
        first_out_indices = v;

        for (auto i = 0; i < set_number; i++) {
            vector<pair<int, int>> v;
            for (auto j = 0; j < associativity; j++) {
                v.push_back({ -1, 0 });
                first_out_indices.push_back(0);
            }
            cache.push_back(v);
        }
    }
    vector<vector<pair<int, int>>> get_cache() {
        return cache;
    }

    void process_miss(int bin_set, int bin_tag_set_associative) {

        // record miss
        ++misses;

        // find least recent to replace
        int least_recent = 0;
        for (auto i = 0; i < cache[bin_set].size(); i++) {

            // open spot found
            if (cache[bin_set][i].first == -1) {
                least_recent = i;
                break;
            }

            // update least_recent
            if (cache[bin_set][i].second <= cache[bin_set][least_recent].second) {
                least_recent = i;
            }
        }
        if (replacement_mode == "LRU") {

            // update the least recently used index with the new tag
            cache[bin_set][least_recent] = { bin_tag_set_associative, ++counter };
        }
        else if (replacement_mode == "FIFO") {
            cache[bin_set][first_out_indices[bin_set]] = { bin_tag_set_associative, ++counter };
            first_out_indices[bin_set]++;

            // wrap around to 0 if we're at the end of the list
            if (first_out_indices[bin_set] > cache[bin_set].size())
                first_out_indices[bin_set] = 0;
        }
    }
};

void trace_file(string filename) {

    // cache options
    Cache_Options cache_options(8192, 64);

    // create cache objects
    direct_map dm(cache_options);
    fully_associative fa(cache_options, "FIFO");
    set_associative sa(cache_options, 2, "FIFO");


    // create input file stream
    ifstream input_file(filename);
    string load_loc, mem_addr, num_bytes;

    unsigned long long int count = 0;

    // file opens successfully
    if (input_file.is_open()) {
        cout << "file found" << endl;
        string nextLine;

        while (getline(input_file, nextLine)) {

            // break line into string stream
            istringstream stream(nextLine);

            // break apart at spaces
            getline(stream, load_loc, ' ');
            getline(stream, mem_addr, ' ');
            getline(stream, num_bytes, ' ');

            // find the line number, offset, and set number
            unsigned int line = log(cache_options.get_line_count()) / log(2);
            unsigned int offset = log(cache_options.get_line_size()) / log(2);
            unsigned int set = line - 1;

            mem_addr = hex_to_binary(mem_addr.substr(2));

            // get offset, line, and tag;
            string mem_offset = mem_addr.substr(mem_addr.size() - offset);
            string mem_line = mem_addr.substr(mem_addr.size() - offset - line, line);
            string mem_tag = mem_addr.substr(0, mem_addr.size() - offset - line);
            string mem_set = mem_addr.substr(mem_addr.size() - offset - set, set);
            string mem_tag_set_associative = mem_addr.substr(0, mem_addr.size() - offset - set);
            string mem_tag_fully_associative = mem_addr.substr(0, mem_addr.size() - offset);


            // convert line and tag to binary
            auto bin_line = stoi(mem_line, 0, 2);
            auto bin_tag = stoi(mem_tag, 0, 2);

            // direct map
            if (bin_line < dm.cache.size()) {

                // tag is found, increase counter
                if (dm.cache[bin_line].first == bin_tag) {
                    dm.cache[bin_line].second++;
                    dm.hits++;
                }
                else {
                    dm.cache[bin_line].first = bin_tag;
                    dm.cache[bin_line].second = 0;
                    dm.misses++;
                }
            }

            // fully associative
            auto bin_tag_fully_associative = stoi(mem_tag_fully_associative, 0, 2);
            bool found = false;
            for (auto i = 0; i < fa.cache.size(); i++) {
                if (fa.cache[i].first == bin_tag_fully_associative) {
                    fa.cache[i].second = ++fa.counter;
                    fa.hits++;
                    found = true;
                    break;
                }
            }
            // process misses
            if (!found) {
                fa.process_miss(bin_tag_fully_associative);
            }

            // n way set
            auto bin_set = stoi(mem_set, 0, 2);
            auto bin_tag_set_associative = stoi(mem_tag_set_associative, 0, 2);

            bin_set = (bin_set / sa.associativity);
            if (bin_set < sa.cache.size()) {

                // look for item
                bool found_sa = false;
                for (auto i = 0; i < sa.cache[bin_set].size(); i++) {

                    // match found, record hit
                    if (bin_tag_set_associative == sa.cache[bin_set][i].first) {
                        sa.cache[bin_set][i].second = ++sa.counter;
                        sa.hits++;
                        found_sa = true;
                        break;
                    }
                }
                if (!found_sa) {
                    sa.process_miss(bin_set, bin_tag_set_associative);
                }
            }
            count++;
        }
        input_file.close();
    }
    else {
        cout << "Error: could not open " << endl;
    }

    // formatting output
    auto width = (filename.size() * 2 < 41) ? 41 : filename.size() + 4;
    cout << fixed << setfill(' ') << setw(width) << "\n";
    cout << "Trace File: " << setfill('-') << setw(width + 10) << right << " " + filename << endl;
    cout << fixed << setprecision(2) << setfill('-');

    // print all cache results
    dm.print_results(width);
    fa.print_results(width);
    sa.print_results(width);
}

int main() {
    // run all trace files
    cout << "\nCache Simulator" << endl;
    trace_file("gcc.trace");
    trace_file("read01.trace");
    trace_file("read03.trace");
    trace_file("write01.trace");
    trace_file("write02.trace");
    trace_file("swim.trace");
    return 0;
}
