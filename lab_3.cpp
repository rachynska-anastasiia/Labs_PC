#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <condition_variable>
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

class Task {
public:
    string id;
    int duration;
    Task() {
        id = generate_id();
        duration = 8 + rand() % 5;
    }

    void operator()() {
        this_thread::sleep_for(chrono::seconds(this->duration));
    }
};

class ThreadPool {
private:
    atomic<bool> is_paused = false;
    atomic<bool> is_initialized = false;
    atomic<bool> is_terminated = false;
    mutex global_lock;

public:
    struct Worker {
        thread worker_thread;
        Task current_task;
        bool is_working = false;
        condition_variable working_cv;
        mutex worker_mutex;
        atomic<int> taked_stat = 0;
        long long waiting_time = 0;
    };
    vector<Worker*> workers_array;
    atomic<int> rejected_stat = 0;

    ThreadPool() = default;

    ~ThreadPool() {
        Terminate();
    }

    void Initialize(const int worker_count) {
        unique_lock<mutex> lock(global_lock);
        if (!is_initialized){
            for (int i = 0; i < worker_count; i++) {
                Worker* worker_to_add = new Worker();
                this->workers_array.push_back(worker_to_add);
                worker_to_add->worker_thread = thread(&ThreadPool::Routine, this, worker_to_add);
            }
            is_initialized = true;
        }
    }

    void Terminate() {
        {
            unique_lock<mutex> lock(global_lock);
            if (!this->Working()) return;
        }
        is_terminated = true;

        for (int i = 0; i < this->workers_array.size(); i++) {
            unique_lock<mutex> lock(this->workers_array[i]->worker_mutex);
            this->workers_array[i]->working_cv.notify_one();
        }

        for (int i = 0; i < this->workers_array.size(); i++)
            workers_array[i]->worker_thread.join();

        {
            unique_lock<mutex> lock(global_lock);
            ShowStatistics();
            for (int i = 0; i < this->workers_array.size(); i++)
                delete this->workers_array[i];
            workers_array.clear();
        }
        is_terminated = is_initialized = false;
    }

    void Stop() {
        {
            unique_lock<mutex> lock(global_lock);
            if (!this->Working()) return;
        }
        is_terminated = true;

        bool end = false;
        do{
            end = true;
            for (int i = 0; i < this->workers_array.size(); i++) {
                unique_lock<mutex> lock(this->workers_array[i]->worker_mutex);
                if (this->workers_array[i]->is_working) {
                    end = false;
                    this_thread::sleep_for(chrono::milliseconds(50));
                    break;
                }
            }
        } while (!end);

        for (int i = 0; i < this->workers_array.size(); i++) {
            unique_lock<mutex> lock(this->workers_array[i]->worker_mutex);
            this->workers_array[i]->working_cv.notify_one();
        }


        for (int i = 0; i < this->workers_array.size(); i++)
            workers_array[i]->worker_thread.join();

        unique_lock<mutex> lock(global_lock);
        for (int i = 0; i < this->workers_array.size(); i++)
            delete this->workers_array[i];
        workers_array.clear();
        is_terminated = is_initialized = false;
    }

    void Pause() {
        is_paused = true;
        cout << "Paused" << endl;
    }

    void Resume() {
        is_paused = false;
        for (int i = 0; i < this->workers_array.size(); i++) {
            unique_lock<mutex> lock(this->workers_array[i]->worker_mutex);
            this->workers_array[i]->working_cv.notify_one();
        }
        cout << "Resumed" << endl;
    }

    void Routine(Worker* concrete_worker) {
        while (!is_terminated) {
            Task new_task;
            {
                unique_lock<mutex> lock(concrete_worker->worker_mutex);
                auto begin = chrono::high_resolution_clock::now();
                while ((!concrete_worker->is_working || is_paused) && !is_terminated)
                    concrete_worker->working_cv.wait(lock);

                auto end = chrono::high_resolution_clock::now();
                concrete_worker->waiting_time += chrono::duration_cast<chrono::milliseconds>(end-begin).count();

                if (is_terminated && !concrete_worker->is_working) return;
                new_task = concrete_worker->current_task;
            }
            cout << "Task " << new_task.id << " executed" << endl;
            new_task();
            cout << "Task " << new_task.id << " done" << endl;
            {
                unique_lock<mutex> lock(concrete_worker->worker_mutex);
                concrete_worker->is_working=false;
            }
        }
    }

    bool Working() {
        return is_initialized && !is_terminated;
    }

    bool AddTask(Task new_task) {

        if (!is_initialized || is_terminated || is_paused) return false;
        unique_lock<mutex> gl_lock(global_lock);
        for (int i = 0; i < this->workers_array.size(); i++) {
            unique_lock<mutex> lock(this->workers_array[i]->worker_mutex);
            if (!this->workers_array[i]->is_working) {
                this->workers_array[i]->current_task = new_task;
                this->workers_array[i]->is_working = true;
                this->workers_array[i]->working_cv.notify_one();
                this->workers_array[i]->taked_stat++;
                return true;
            }

        }
        rejected_stat++;
        return false;
    }

    void ShowStatistics() {
        cout <<endl << "rejected = "<< this->rejected_stat << endl;
        long long general_waiting_time = 0;
        for (int i = 0; i < 6; i++) {
            cout << "Worker " << i << " statistics:" << endl;
            cout << "------taked "<< i <<" = " << this->workers_array[i]->taked_stat << endl;
            cout << "------waiting time" << i << " = " << this->workers_array[i]->waiting_time << endl;
            general_waiting_time += this->workers_array[i]->waiting_time;;
        }
        cout << "General waiting time = " << general_waiting_time << endl;
        cout << "Average waiting time = " << general_waiting_time / 6 << endl;
    }

};

void task_generation(ThreadPool& thread_pool) {
    while (thread_pool.Working()) {
        Task new_task;
        bool result = thread_pool.AddTask(new_task);
        if (result) cout << "Task " << new_task.id << " added" << endl;
        else cout << "Task " << new_task.id << " cancelled" << endl;
        this_thread::sleep_for(chrono::seconds(2));
    }
}

int main() {
    srand(time(NULL));
    ThreadPool pool;
    int time = 30;
    pool.Initialize(6);

    thread generator_thread_1(task_generation, ref(pool));
    thread generator_thread_2(task_generation, ref(pool));

    this_thread::sleep_for(chrono::seconds(time));
    pool.Pause();
    this_thread::sleep_for(chrono::seconds(10));
    pool.Resume();
    this_thread::sleep_for(chrono::seconds(time));
    pool.Terminate();

    generator_thread_1.join();
    generator_thread_2.join();

    cout << "executed for: " << time << endl;
    cout << "work stopped" << endl;

    return 0;
}

/*bool Working_unsafe() {
        return is_initialized && !is_terminated;
    }*/

//this_thread::sleep_for(chrono::seconds(1 + rand() % 10));