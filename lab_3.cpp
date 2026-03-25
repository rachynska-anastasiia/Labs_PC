#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <condition_variable>
#include <shared_mutex>
#include <vector>
#include <functional>
#include <random>
using namespace std;


/*
Пул потоків обслуговується 6-ма робочими потоками. Черга задач як така
відсутня. Задачі додаються відразу на виконання вільному робочому
потоку. Якщо всі робочі потоки зайняті – задача відкидається. Задача
займає випадковий час від 8 до 12 секунд. (Технічно черга потоків
відсутня, але в даній реалізації можлива реалізація 6-ти окремих черг на
одну задачу, що, по-факту, є відсутністю черги, адже черга це одна
активна задача).*/





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
        //cout << "Start task =" << this->id << " with duration = " << this->duration << endl;
        this_thread::sleep_for(chrono::seconds(this->duration));
        //cout << "Stop task =" << this->id << " with duration = " << this->duration << endl;
    }
};

class thread_pool {
private:
    bool is_paused = false;
    bool initialized = false;
    bool terminated = false;



    condition_variable pausing_cv;

    /*using read_write_lock = shared_mutex;
    using read_lock = shared_lock<read_write_lock>;
    using write_lock = unique_lock<read_write_lock>;
    read_write_lock*/
    mutex global_lock;

public:
    struct Worker {
        thread worker_thread;
        task  *current_task;
        bool working = false;
        condition_variable working_cv;
        mutex working_mutex;
        atomic<int> taked = 0;
    };
    vector<Worker*> workers_array;
    atomic<int> rejected = 0;

    thread_pool() = default;

    ~thread_pool() {
        terminate();
    }

    void initialize(const int worker_count) {
        unique_lock<mutex> lock(global_lock);
        if (!initialized && !terminated) {
            for (int i = 0; i < worker_count; i++) {
                Worker* worker_to_add = new Worker();
                this->workers_array.push_back(worker_to_add);
                worker_to_add->worker_thread = thread(&thread_pool::routine, this, worker_to_add);
            }
            initialized = true;
        }
    }

    void terminate() {
        {
            unique_lock<mutex> lock(global_lock);
            if (!this->working_unsafe()) {
                workers_array.clear();
                terminated = false;
                initialized = false;
                return;
            }
            terminated = true;

            for (int i = 0; i < this->workers_array.size(); i++) {
                this->workers_array[i]->working_cv.notify_one();
            }
        }

        for (int i = 0; i < this->workers_array.size(); i++) {
            workers_array[i]->worker_thread.join();
        }

        {
            unique_lock<mutex> lock(global_lock);
            workers_array.clear();
            terminated = false;
            initialized = false;
        }

    }

    void stop() {
        {
            unique_lock<mutex> lock(global_lock);
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
                    this_thread::sleep_for(chrono::milliseconds(50));
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

    void pause() {
        unique_lock<mutex> lock(global_lock);
        is_paused = true;
        cout << "Paused" << endl;
    }

    void resume() {
        {
            unique_lock<mutex> lock(global_lock);
            is_paused = false;
        }
        pausing_cv.notify_all();
        cout << "Resumed" << endl;

    }

    void routine(Worker* self) {
        while (true) {
            task new_task;
            {
                unique_lock<mutex> lock(self->working_mutex);
                while ((!self->working || is_paused) && !terminated) {
                    self->working_cv.wait(lock);
                }
                if (terminated && !self->working) return;
                new_task = *(self->current_task);
                cout << "Task " << new_task.id << " executed" << endl;
            }
            new_task();
            {
                cout << "Task " << new_task.id << " done" << endl;
                unique_lock<mutex> lock(self->working_mutex);
                self->working=false;
                self->current_task=nullptr;
            }
        }
    }

    bool working() {
        unique_lock<mutex> lock(global_lock);
        return this->working_unsafe();
    }

    bool working_unsafe() {
        return initialized && !terminated;
    }

    bool add_task(task* new_task) {
        unique_lock<mutex> lock(global_lock);
        if (!initialized || terminated || is_paused) return false;

        for (int i = 0; i < this->workers_array.size(); i++) {
            unique_lock<mutex> lock(this->workers_array[i]->working_mutex, try_to_lock);
            if (!this->workers_array[i]->working) {
                this->workers_array[i]->current_task = new_task;
                this->workers_array[i]->working_cv.notify_one();
                this->workers_array[i]->working = true;
                this->workers_array[i]->taked++;
                return true;
            }
        }
        rejected++;
        return false;

    }

};


void task_generation(thread_pool& thread_pool) {
    while (thread_pool.working()) {
        task new_task;
        bool result = thread_pool.add_task(&new_task);
        if (result) {
            cout << "Task " << new_task.id << " added" << endl;
            continue;
        }
        cout << "Task " << new_task.id << " cancelled" << endl;
        this_thread::sleep_for(chrono::seconds(2));
    }
}

int main() {
    srand(time(NULL));
    thread_pool pool;
    pool.initialize(6);

    thread generator_thread_1(task_generation, ref(pool));
    thread generator_thread_2(task_generation, ref(pool));

    this_thread::sleep_for(chrono::seconds(15));
    pool.terminate();

    generator_thread_1.join();
    generator_thread_2.join();

    cout << "work stopped" << endl;

    cout <<endl << "rejected = "<< pool.rejected << endl;
    for (int i = 0; i < 6; i++) {
        cout << "taked "<< i <<" = " << pool.workers_array[i]->taked << endl;
    }

    return 0;
}