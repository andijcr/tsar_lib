//
// Created by andrea on 18/02/16.
//

#include <vector>
#include <algorithm>
#include <future>
#include <csignal>

extern "C" {
#include <lib/inmemory_logger.h>
#include <unistd.h>

}
using namespace std;

int main(){

    auto num_writers = 6;
    auto num_logs = long(INMEM_BUFFER_SIZE / (num_writers * 0.5));
    auto log_writer = [num_logs] {
        for (auto i = 0; i < num_logs; ++i) {
            inmem_log("Test inmem_log", i);
            this_thread::yield();
        }
    };

    vector<future<void>> f_inst(num_writers);

    generate(begin(f_inst), end(f_inst), [log_writer]{ return async(launch::async, log_writer); });
    for_each(begin(f_inst), end(f_inst), [](auto &f) { f.wait(); });

//dumps the events and reorder them
    vector<inmem_event> events_dump(&inmem_buffer[0], &inmem_buffer[INMEM_BUFFER_SIZE]);

    auto gt_inmem_evt=[](inmem_event &e1, inmem_event &e2) { return e1.seq >= e2.seq; };


    auto counter=-1;        //as end(events_dump) is returned as an outlier, counter as to start from -1
    auto outlier=begin(events_dump);
    while(outlier!=end(events_dump)) {
        outlier = adjacent_find(outlier+1, end(events_dump), gt_inmem_evt);
        ++counter;
    }

    if(counter >1) {
        kill(getpid(), SIGABRT);
    }

}