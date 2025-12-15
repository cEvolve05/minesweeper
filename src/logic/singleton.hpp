#pragma once

class Singleton {
    Singleton() = default;
    ~Singleton() = default;

  public:
    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;

    static auto& instance() {
        static Singleton instance;
        return instance;
    }
};