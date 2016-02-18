//
// Created by andrea on 15/09/15.
//
#include <string>
#include <future>
#include <algorithm>
#include <vector>
#include <cstring>
#include <iostream>
#include <signal.h>
#include <unistd.h>
#include "catch.hpp"

extern "C" {
#include <lib/inmemory_logger.h>
#include "tsar.h"

}

using namespace std;

SCENARIO("multiple tsar_init create multiple tsar_t*", "[tsar]") {
    GIVEN("a tsar_init") {
        auto tsar=tsar_init();
        REQUIRE(tsar!= NULL);
        WHEN("a new tsar_init is executed") {
            auto tsar2 = tsar_init();
            THEN("the two pointers are different") {
                REQUIRE(tsar!= tsar2);
            }
            tsar_destroy(tsar2);
        }
        tsar_destroy(tsar);
    }
    GIVEN("a fresh startup"){
        WHEN("multiple tsar_init are executed multithreaded"){
            vector<future<tsar_t*>> f_instances (100);
            generate(begin(f_instances), end(f_instances),
                []{return async(launch::async, tsar_init);}
            );
            THEN("multiple structures are created distinc"){
                vector<tsar_t*> instances(f_instances.size());
                REQUIRE_NOTHROW(
                    transform(begin(f_instances), end(f_instances), begin(instances),
                        [](auto& f){return f.get();});                       //auto& because future cant be copyed
                );
                sort(begin(instances), end(instances));

                REQUIRE(adjacent_find(begin(instances), end(instances))==end(instances));

                //TODO: for now just pointer equality is used.
                for(auto i: instances)
                    tsar_destroy(i);
            }

        }
    }
}



SCENARIO("inmemory_logger is threadsafe", "[inmem_log]"){
    GIVEN("multiple threads") {
        auto gt_inmem_evt=[](inmem_event &e1, inmem_event &e2) { return e1.seq >= e2.seq; };

        WHEN("multiple threads write concurrently in the log in the limit of INMEM_BUFFER_SIZE") {
            auto num_writers = 6;
            auto num_logs = long(INMEM_BUFFER_SIZE / (num_writers * 1.5));
            REQUIRE(num_writers*num_logs <= INMEM_BUFFER_SIZE);
            auto log_writer = [num_logs] {
                for (auto i = 0; i < num_logs; ++i)
                    inmem_log("Test inmem_log", i);
            };
            vector<future<void>> f_inst(num_writers);

            //mark init
            CAPTURE(inmem_pos);
            unsigned long start_position = inmem_pos;

            //launch #num_writers to write (each) #num_logs
            generate(begin(f_inst), end(f_inst), [log_writer] { return async(launch::async, log_writer); });
            REQUIRE_NOTHROW(for_each(begin(f_inst), end(f_inst), [](auto &f) { f.wait(); }));

            //mark end
            inmem_log("poison_pill_end", -42);

            //dumps the events and reorder them (poison pill first as first item in the vector)
            vector<inmem_event> events_dump(&inmem_buffer[0], &inmem_buffer[INMEM_BUFFER_SIZE]);
            rotate(begin(events_dump), begin(events_dump) + start_position, end(events_dump));

            THEN("no writes are lost") {
                INFO("INMEM_BUFFER_SIZE := " << INMEM_BUFFER_SIZE);
                //i could use inmem_pos, but this is more fun
                auto pill_it = find_if(begin(events_dump), end(events_dump),
                                       [](auto &e) { return strcmp(e.msg, "poison_pill_end") == 0; });
                REQUIRE((num_writers * num_logs) == (pill_it - begin(events_dump)));
                REQUIRE(all_of(begin(events_dump), pill_it, [](auto& e){return strcmp(e.msg, "Test inmem_log")==0;}));
            }
            AND_THEN("events are in sequential total order") {
                events_dump.resize(num_logs * num_writers);
                //find the events wo do not respect total order
                REQUIRE(adjacent_find(begin(events_dump), end(events_dump), gt_inmem_evt) == end(events_dump));
            }
        }
        AND_WHEN("the writes are over the INMEM_BUFFER_SIZE"){
            auto num_writers = 6;
            auto num_logs = long(INMEM_BUFFER_SIZE / (num_writers * 0.5));
            REQUIRE(num_writers*num_logs > INMEM_BUFFER_SIZE);
            auto log_writer = [num_logs] {
                for (auto i = 0; i < num_logs; ++i) {
                    inmem_log("Test inmem_log", i);
                    //adding a yield help prevent starvation of other thread. obviously, this depends on the scheduler, which is out of reach
                    this_thread::yield();
                }
            };

            vector<future<void>> f_inst(num_writers);

            generate(begin(f_inst), end(f_inst), [log_writer]{ return async(launch::async, log_writer); });
            for_each(begin(f_inst), end(f_inst), [](auto &f) { f.wait(); });

            unsigned long end_position_plus_1 = inmem_pos;

            //dumps the events and reorder them
            vector<inmem_event> events_dump(&inmem_buffer[0], &inmem_buffer[INMEM_BUFFER_SIZE]);

            THEN("remaining events are in sequential total order, most of the times") {
                //find the events wo do not respect total order

                auto counter=-1;        //as end(events_dump) is returned as an outlier, counter as to start from -1
                auto outlier=begin(events_dump);
                while(outlier!=end(events_dump)) {
                    outlier = adjacent_find(outlier+1, end(events_dump), gt_inmem_evt);
                    ++counter;
                }
                REQUIRE(counter<=1);
            }
        }
    }
}
