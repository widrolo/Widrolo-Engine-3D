#pragma once
#include "CommonTypes.h"

// This file does not contain any AI generated documentation.

namespace WEngine
{
    /**
     * This class is used for manual reference counting.
     * @tparam T type of data
     */
    template<typename T>
    class Ref
    {
    public:
        /**
         * Constructor
         * @param data The data to be counted
         * @note The constructor does not increase the counter.
         */
        Ref(T& data) : m_data(data), m_count(0) {}

        /**
         *  Adds a reference
         */
        void Add() { m_count++; }

        /**
         * Removes a reference
         * @return If true, the references have depleted, nothing depends on it anymore
         */
        [[nodiscard]] bool Remove()
        {
            if (m_count == 0)
                return true;
            m_count--;
            return m_count == 0;
        }

        [[nodiscard]] T& Get() { return m_data; }
        [[nodiscard]] uint64 GetCount() const { return m_count; }

    private:
        uint64 m_count;
        T m_data;
    };
}
