#include <iostream>
#include <thread>
#include <string>
#include <mutex>
#include <condition_variable>
#include <chrono>

std::mutex mtx; // Mutex for synchronization
std::condition_variable cv; // Condition variable for signaling
int terminationCount = 0; // Count of termination messages

void producer() {
    std::thread::id threadId = std::this_thread::get_id();

    try {
        double value = static_cast<double>(std::hash<std::thread::id>{}(threadId) % 10000) / 10.0;

        while (value > 0.0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Simulate some work

            std::unique_lock<std::mutex> lock(mtx);
            std::cout << value << " :sent " << threadId << std::endl;
            lock.unlock();

            value /= 10.0;
        }

        std::unique_lock<std::mutex> lock(mtx);
        std::cout << "finished> " << threadId << std::endl;
        lock.unlock();
        cv.notify_one();
    }
    catch (const std::exception& e) {
        std::cerr << "Producer " << threadId << " encountered an exception: " << e.what() << std::endl;
    }
}

void consumer() {
    try {
        while (true) {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [&] { return terminationCount >= 2; });

            if (terminationCount >= 2) {
                break;
            }
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Consumer encountered an exception: " << e.what() << std::endl;
    }
}

int main() {
    std::thread consumerThread(consumer);

    std::thread producerThread1(producer);
    std::thread producerThread2(producer);

    producerThread1.join();
    producerThread2.join();

    cv.notify_one(); // Notify the consumer thread to check the termination condition
    consumerThread.join();

    return 0;
}
