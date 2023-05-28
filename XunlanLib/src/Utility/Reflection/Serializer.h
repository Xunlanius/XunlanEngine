#pragma once

#include "Reflection.h"

#include <cassert>
#include <vector>
#include <filesystem>
#include <fstream>

namespace Xunlan::Reflection
{
    class BinaryStream final
    {
    public:

        template<typename T>
        BinaryStream& operator<<(const T& value) { Input(value); return *this; }

        template<typename T>
        BinaryStream& operator>>(T& value) { Output(value); return *this; }

        void Save(const std::filesystem::path& filePath);
        void Load(const std::filesystem::path& filePath);

    private:

        template<typename T>
        void Input(const T& value, bool writeCategory = true);

        template<typename T>
        void Output(T& value, bool readCategory = true);

        template<typename T>
        T Read();
        void ReadBuffer(void* dst, size_t size);

        void WriteBuffer(const void* src, size_t size);
        void Reserve(size_t size);

    private:

        std::vector<byte> m_buffer;
        size_t m_pos = 0;
    };

    inline void BinaryStream::Save(const std::filesystem::path& filePath)
    {
        std::ofstream stream(filePath, std::ios::out | std::ios::binary);
        stream.write((char*)m_buffer.data(), m_buffer.size());
        stream.close();
    }

    inline void BinaryStream::Load(const std::filesystem::path& filePath)
    {
        assert(std::filesystem::exists(filePath));

        const size_t fileSize = std::filesystem::file_size(filePath);
        m_buffer.resize(fileSize);

        std::ifstream stream(filePath, std::ios::in | std::ios::binary);
        stream.read((char*)m_buffer.data(), fileSize);
        stream.close();
    }

    template<typename T>
    inline void BinaryStream::Input(const T& data, bool writeCategory)
    {
        static constexpr DataCategory category = DATA_CATEGORY<T>;

        if (writeCategory) WriteBuffer(&category, sizeof(DataCategory));

        if constexpr (category == DataCategory::CUSTOM)
        {
            auto func = [this, &data](const auto& field) { Input(field.GetMember(data)); };
            ForEachField<T>(func);
        }
        else if constexpr (category == DataCategory::NATIVE_ARRAY)
        {
            using value_type = std::remove_extent_t<T>;
            static_assert(!std::is_pointer_v<value_type>, "Value type cannot be pointer.");

            const uint32 len = (uint32)_countof(data);
            WriteBuffer(&len, sizeof(len));

            if constexpr (std::is_trivially_copyable_v<value_type>)
            {
                WriteBuffer(data, sizeof(data));
            }
            else
            {
                for (const auto& ele : data) Input(ele, false);
            }
        }
        else if constexpr (category == DataCategory::ARRAY || category == DataCategory::VECTOR || category == DataCategory::STRING)
        {
            static_assert(!std::is_pointer_v<typename T::value_type>, "Value type cannot be pointer.");

            const uint32 len = (uint32)data.size();
            WriteBuffer(&len, sizeof(len));

            if constexpr (std::is_trivially_copyable_v<typename T::value_type>)
            {
                WriteBuffer(data.data(), len * sizeof(typename T::value_type));
            }
            else
            {
                for (const auto& ele : data) Input(ele, false);
            }
        }
        else WriteBuffer(&data, sizeof(T));
    }

    template<typename T>
    inline void BinaryStream::Output(T& data, bool readCategory)
    {
        static constexpr DataCategory category = DATA_CATEGORY<T>;

        if (readCategory)
        {
            const DataCategory currType = Read<DataCategory>();
            assert(category == currType && "Type mismatch.");
        }

        if constexpr (category == DataCategory::CUSTOM)
        {
            auto func = [this, &data](auto& field) { Output(field.GetMember(data)); };
            ForEachField<T>(func);
        }
        else if constexpr (category == DataCategory::NATIVE_ARRAY)
        {
            using value_type = std::remove_extent_t<T>;
            const uint32 len = Read<uint32>();
            assert(len == _countof(data));

            if constexpr (std::is_trivially_copyable_v<value_type>)
            {
                ReadBuffer(data, sizeof(data));
            }
            else
            {
                for (auto& ele : data) Output(ele, false);
            }
        }
        else if constexpr (category == DataCategory::ARRAY || category == DataCategory::VECTOR || category == DataCategory::STRING)
        {
            const uint32 len = Read<uint32>();
            if constexpr (category != DataCategory::ARRAY) data.resize(len);
            assert(len == data.size());

            if constexpr (std::is_trivially_copyable_v<typename T::value_type>)
            {
                ReadBuffer(data.data(), len * sizeof(typename T::value_type));
            }
            else
            {
                for (auto& ele : data) Output(ele, false);
            }
        }
        else data = Read<T>();
    }

    template<typename T>
    inline T BinaryStream::Read()
    {
        assert(m_pos + sizeof(T) <= m_buffer.size());
        T result = *(T*)&m_buffer[m_pos];
        m_pos += sizeof(T);
        return result;
    }

    inline void BinaryStream::ReadBuffer(void* dst, size_t size)
    {
        assert(m_pos + size <= m_buffer.size());
        memcpy(dst, &m_buffer[m_pos], size);
        m_pos += size;
    }

    inline void BinaryStream::WriteBuffer(const void* src, size_t size)
    {
        size_t position = m_buffer.size();
        Reserve(size);
        memcpy(&m_buffer[position], src, size);
    }

    inline void BinaryStream::Reserve(size_t size)
    {
        size_t capacity = m_buffer.capacity();
        size_t requestSize = m_buffer.size() + size;

        if (requestSize > capacity)
        {
            size_t newCapacity = std::max(2 * capacity, requestSize);
            m_buffer.reserve(newCapacity);
        }

        m_buffer.resize(requestSize);
    }
}