#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <condition_variable>
#include <shared_mutex>
using namespace std;


/*
Пул потоків обслуговується 6-ма робочими потоками. Черга задач як така
відсутня. Задачі додаються відразу на виконання вільному робочому
потоку. Якщо всі робочі потоки зайняті – задача відкидається. Задача
займає випадковий час від 8 до 12 секунд. (Технічно черга потоків
відсутня, але в даній реалізації можлива реалізація 6-ти окремих черг на
одну задачу, що, по-факту, є відсутністю черги, адже черга це одна
активна задача).*/
//#include "task_queue.h"
#include <vector>
#include <functional>

string generate_id() {
    auto time_now = chrono::system_clock::now();
    return  to_string(time_now.time_since_epoch().count()%int(1e7));
}

class task {
public:
    string id;
    int duration;
    task() {
        id = generate_id();
        duration = 8 + rand() % 5;
    }

    void operator()() {
        cout << "Start task =" << this->id << " with duration = " << this->duration << endl;
        this_thread::sleep_for(chrono::seconds(this->duration));
        cout << "Stop task =" << this->id << " with duration = " << this->duration << endl;
    }
};

class thread_pool {
public:
    bool initialized = false;
    bool terminated = false;

    struct Worker {
        thread worker_thread;
        task  current_task;
        bool working = false;
        condition_variable working_cv;
        mutex working_mutex;
    };
    vector<Worker*> workers_array;

    using read_write_lock = shared_mutex;
    using read_lock = shared_lock<read_write_lock>;
    using write_lock = unique_lock<read_write_lock>;
    read_write_lock global_lock;

    thread_pool() = default;

    ~thread_pool() {
        terminate();
    }

    void initialize(const int worker_count) {
        if (!initialized) {
            for (int i = 0; i < worker_count; i++) {
                Worker* worker_to_add = new Worker();
                worker_to_add->worker_thread = thread(&thread_pool::routine, this, worker_to_add);
                this->workers_array.push_back(worker_to_add);
            }
            initialized = true;
        }
    }

    void terminate() {
        {
            write_lock lock(global_lock);
            if (!this->working_unsafe()) return;
            terminated = true;
        }

        for (int i = 0; i < this->workers_array.size(); i++) {
            this->workers_array[i]->working_cv.notify_one();
        }

        for (int i = 0; i < this->workers_array.size(); i++) {
            workers_array[i]->worker_thread.join();
        }
        workers_array.clear();
    }

    void stop() {
        {
            write_lock lock(global_lock);
            if (!this->working_unsafe()) return;
            terminated = true;
        }

        bool end = false;
        do{
            end = true;
            for (int i = 0; i < this->workers_array.size(); i++) {
                unique_lock<mutex> lock(this->workers_array[i]->working_mutex);
                if (this->workers_array[i]->working) {
                    end = false;
                    break;
                }
            }
        } while (!end);

        for (int i = 0; i < this->workers_array.size(); i++) {
            this->workers_array[i]->working_cv.notify_one();
        }

        for (int i = 0; i < this->workers_array.size(); i++) {
            workers_array[i]->worker_thread.join();
        }
        workers_array.clear();
    }

    void routine(Worker* self) {
        while (true) {
            task new_task;
            {
                unique_lock<mutex> lock(self->working_mutex);
                while (!self->working && !terminated) {
                    self->working_cv.wait(lock);
                }
                if (terminated && !self->working) return;
                new_task = self->current_task;
            }
            new_task();
            {
                unique_lock<mutex> lock(self->working_mutex);
                self->working=false;
            }
        }
    }

    bool working() {
        write_lock lock(global_lock);
        return this->working_unsafe();
    }

    bool working_unsafe() {
        return initialized && !terminated;
    }

    bool add_task(task new_task) {
        for (int i = 0; i < this->workers_array.size(); i++) {
            unique_lock<mutex> lock(this->workers_array[i]->working_mutex, defer_lock);
            if (!this->workers_array[i]->working) {
                this->workers_array[i]->current_task = new_task;
                this->workers_array[i]->working_cv.notify_one();
                this->workers_array[i]->working = true;
                return true;
            }
        }
        return false;
    }

};


void task_generation(thread_pool& thread_pool) {
    while (!thread_pool.terminated) {
        task new_task;
        bool result = thread_pool.add_task(new_task);
        if (result) {
            cout << "Task " << new_task.id << " added" << endl;
        }
        new_task.operator()();

    }
}

int main() {
    cout << generate_id() << endl;
    thread_pool pool;
    pool.initialize(6);

    thread generator_thread_1(task_generation, ref(pool));
    thread generator_thread_2(task_generation, ref(pool));

    generator_thread_1.join();
    generator_thread_2.join();

    this_thread::sleep_for(chrono::seconds(15));
    pool.terminate();

    return 0;
}

/*
void operator()() {
    id = generate_id();
    duration = 8 + rand() % 5;
    cout << "Start task =" << this->id << " with duration = " << this->duration << endl;
    this_thread::sleep_for(chrono::seconds(this->duration));
    cout << "Stop task =" << this->id << " with duration = " << this->duration << endl;
}*/

/*class start_generation {
    public:
    thread_pool concrete_pool;
    vector<task> generator_threads;
    void start_generation(thread_pool pool) {
        this->concrete_pool = pool;

    }
};*/