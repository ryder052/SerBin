#pragma once
#include <fstream>
#include <concepts>

#include <memory>
#include <tuple>
#include <optional>

#include <array>
#include <vector>
#include <list>
#include <deque>

#include <map>
#include <unordered_map>

#include <set>
#include <unordered_set>

namespace serbin
{
    // Big opt-in optimization, mostly for contiguously allocating containers of Ts.
    template<typename T>
    constexpr bool serializeAsPOD = std::is_fundamental_v<T>;

    //////////////////////////////////////////////////////////////////////////////////
    // Reader / Writer class
    //////////////////////////////////////////////////////////////////////////////////
    template<decltype(std::ios::in) mode>
    class SerBin
    {
        constexpr int getFinalMode()
        {
            if constexpr (mode == std::ios::out)
                return mode | std::ios::binary | std::ios::trunc;
            else
                return mode | std::ios::binary;
        }

    public:
        SerBin(const std::string& filename)
        {
            stream.open(filename, getFinalMode());
        }

        ~SerBin()
        {
            stream.close();
        }

        std::fstream stream;
    };

    // Fundamental types and opt-in PODs
    template<typename T, typename = std::enable_if_t<serializeAsPOD<T>>>
    inline SerBin<std::ios::out>& operator<<(SerBin<std::ios::out>& writer, const T& object)
    {
        writer.stream.write((const char*)(&object), sizeof(T));
        return writer;
    }

    template<typename T, typename = std::enable_if_t<serializeAsPOD<T>>>
    inline SerBin<std::ios::in>& operator>>(SerBin<std::ios::in>& reader, T& object)
    {
        reader.stream.read((char*)(&object), sizeof(T));
        return reader;
    }

    // Smart pointers
    template<typename T>
    inline SerBin<std::ios::out>& operator<<(SerBin<std::ios::out>& writer, const std::unique_ptr<T>& object)
    {
        writer << bool(object);
        writer << *object;
        return writer;
    }

    template<typename T>
    inline SerBin<std::ios::in>& operator>>(SerBin<std::ios::in>& reader, std::unique_ptr<T>& object)
    {
        bool hasValue;
        reader >> hasValue;

        if (hasValue)
        {
            object = std::make_unique<T>();
            reader >> *object;
        }

        return reader;
    }

    template<typename T>
    inline SerBin<std::ios::out>& operator<<(SerBin<std::ios::out>& writer, const std::shared_ptr<T>& object)
    {
        writer << bool(object);
        writer << *object;
        return writer;
    }

    template<typename T>
    inline SerBin<std::ios::in>& operator>>(SerBin<std::ios::in>& reader, std::shared_ptr<T>& object)
    {
        bool hasValue;
        reader >> hasValue;

        if (hasValue)
        {
            object = std::make_shared<T>();
            reader >> *object;
        }

        return reader;
    }

    // std::string, std::wstring etc
    template<typename T>
    inline SerBin<std::ios::out>& operator<<(SerBin<std::ios::out>& writer, const std::basic_string<T>& object)
    {
        writer << object.size();

        if (object.size() > 0)
            writer.stream.write((const char*)(object.data()), object.size() * sizeof(T));

        return writer;
    }

    template<typename T>
    inline SerBin<std::ios::in>& operator>>(SerBin<std::ios::in>& reader, std::basic_string<T>& object)
    {
        decltype(object.size()) s;
        reader >> s;

        if (s > 0)
        {
            object.resize(s);
            reader.stream.read((char*)(object.data()), object.size() * sizeof(T));
        }

        return reader;
    }

    // std::vector
    template<typename T>
    inline SerBin<std::ios::out>& operator<<(SerBin<std::ios::out>& writer, const std::vector<T>& object)
    {
        writer << object.size();
        if constexpr (serializeAsPOD<T>)
        {
            if (object.size() > 0)
                writer.stream.write((const char*)(&object[0]), sizeof(T) * object.size());
        }
        else
        {
            for (auto&& value : object)
                writer << value;
        }

        return writer;
    }

    template<typename T>
    inline SerBin<std::ios::in>& operator>>(SerBin<std::ios::in>& reader, std::vector<T>& object)
    {
        decltype(object.size()) s;
        reader >> s;

        if (s == 0)
            return reader;

        object.resize(s);

        if constexpr (serializeAsPOD<T>)
        {
            reader.stream.read((char*)(&object[0]), sizeof(T) * s);
        }
        else
        {
            for (auto&& value : object)
                reader >> value;
        }

        return reader;
    }

    // std::array
    template<typename T, size_t N>
    inline SerBin<std::ios::out>& operator<<(SerBin<std::ios::out>& writer, const std::array<T, N>& object)
    {
        if constexpr (N > 0)
        {
            if constexpr (serializeAsPOD<T>)
            {
                writer.stream.write((const char*)(&object[0]), sizeof(T) * N);
            }
            else
            {
                for (auto&& value : object)
                    writer << value;
            }
        }

        return writer;
    }

    template<typename T, size_t N>
    inline SerBin<std::ios::in>& operator>>(SerBin<std::ios::in>& reader, std::array<T, N>& object)
    {
        if constexpr (N > 0)
        {
            if constexpr (serializeAsPOD<T>)
            {
                reader.stream.read((char*)(&object[0]), sizeof(T) * N);
            }
            else
            {
                for (auto&& value : object)
                    reader >> value;
            }
        }

        return reader;
    }

    // std::list
    template<typename T>
    inline SerBin<std::ios::out>& operator<<(SerBin<std::ios::out>& writer, const std::list<T>& object)
    {
        writer << object.size();

        for (auto&& value : object)
            writer << value;

        return writer;
    }

    template<typename T>
    inline SerBin<std::ios::in>& operator>>(SerBin<std::ios::in>& reader, std::list<T>& object)
    {
        decltype(object.size()) s;
        reader >> s;

        for (decltype(s) i = 0; i < s; ++i)
        {
            object.push_back(T());
            reader >> object.back();
        }

        return reader;
    }

    // std::deque
    template<typename T>
    inline SerBin<std::ios::out>& operator<<(SerBin<std::ios::out>& writer, const std::deque<T>& object)
    {
        writer << object.size();

        for (auto&& value : object)
            writer << value;

        return writer;
    }

    template<typename T>
    inline SerBin<std::ios::in>& operator>>(SerBin<std::ios::in>& reader, std::deque<T>& object)
    {
        decltype(object.size()) s;
        reader >> s;

        for (decltype(s) i = 0; i < s; ++i)
        {
            object.push_back(T());
            reader >> object.back();
        }

        return reader;
    }

    // std::set
    template<typename T>
    inline SerBin<std::ios::out>& operator<<(SerBin<std::ios::out>& writer, const std::set<T>& object)
    {
        writer << object.size();

        for (auto&& value : object)
            writer << value;

        return writer;
    }

    template<typename T>
    inline SerBin<std::ios::in>& operator>>(SerBin<std::ios::in>& reader, std::set<T>& object)
    {
        decltype(object.size()) s;
        reader >> s;

        for (decltype(s) i = 0; i < s; ++i)
        {
            T value;
            reader >> value;
            object.insert(std::move(value));
        }

        return reader;
    }

    // std::unordered_set
    template<typename T>
    inline SerBin<std::ios::out>& operator<<(SerBin<std::ios::out>& writer, const std::unordered_set<T>& object)
    {
        writer << object.size();

        for (auto&& value : object)
            writer << value;

        return writer;
    }

    template<typename T>
    inline SerBin<std::ios::in>& operator>>(SerBin<std::ios::in>& reader, std::unordered_set<T>& object)
    {
        decltype(object.size()) s;
        reader >> s;

        for (decltype(s) i = 0; i < s; ++i)
        {
            T value;
            reader >> value;
            object.insert(std::move(value));
        }

        return reader;
    }

    // std::pair
    template<typename T0, typename T1>
    inline SerBin<std::ios::out>& operator<<(SerBin<std::ios::out>& writer, const std::pair<T0, T1>& object)
    {
        writer << object.first << object.second;
        return writer;
    }

    template<typename T0, typename T1>
    inline SerBin<std::ios::in>& operator>>(SerBin<std::ios::in>& reader, std::pair<T0, T1>& object)
    {
        reader >> object.first >> object.second;
        return reader;
    }

    // std::map
    template<typename K, typename V>
    inline SerBin<std::ios::out>& operator<<(SerBin<std::ios::out>& writer, const std::map<K, V>& object)
    {
        writer << object.size();

        for (auto&& kv : object)
            writer << kv;

        return writer;
    }

    template<typename K, typename V>
    inline SerBin<std::ios::in>& operator>>(SerBin<std::ios::in>& reader, std::map<K, V>& object)
    {
        decltype(object.size()) s;
        reader >> s;

        for (decltype(s) i = 0; i < s; ++i)
        {
            std::pair<K, V> kv;
            reader >> kv;
            object.emplace(std::move(kv));
        }

        return reader;
    }

    // std::unordered_map
    template<typename K, typename V>
    inline SerBin<std::ios::out>& operator<<(SerBin<std::ios::out>& writer, const std::unordered_map<K, V>& object)
    {
        writer << object.size();

        for (auto&& kv : object)
            writer << kv;

        return writer;
    }

    template<typename K, typename V>
    inline SerBin<std::ios::in>& operator>>(SerBin<std::ios::in>& reader, std::unordered_map<K, V>& object)
    {
        decltype(object.size()) s;
        reader >> s;

        for (decltype(s) i = 0; i < s; ++i)
        {
            std::pair<K, V> kv;
            reader >> kv;
            object.emplace(std::move(kv));
        }

        return reader;
    }

    // std::tuple
    template<int id = 0, typename... Ts>
    inline SerBin<std::ios::out>& operator<<(SerBin<std::ios::out>& writer, const std::tuple<Ts...>& object)
    {
        if constexpr (id < std::tuple_size_v<std::tuple<Ts...>>)
        {
            writer << std::get<id>(object);
            operator<<<id + 1>(writer, object);
        }

        return writer;
    }

    template<int id = 0, typename... Ts>
    inline SerBin<std::ios::in>& operator>>(SerBin<std::ios::in>& reader, std::tuple<Ts...>& object)
    {
        if constexpr (id < std::tuple_size_v<std::tuple<Ts...>>)
        {
            reader >> std::get<id>(object);
            operator>><id + 1>(reader, object);
        }

        return reader;
    }

    // std::optional
    template<typename T>
    inline SerBin<std::ios::out>& operator<<(SerBin<std::ios::out>& writer, const std::optional<T>& object)
    {
        bool hasValue = object;
        writer << hasValue;

        if (hasValue)
            writer << *object;

        return writer;
    }

    template<typename T>
    inline SerBin<std::ios::in>& operator>>(SerBin<std::ios::in>& reader, std::optional<T>& object)
    {
        bool hasValue;
        reader >> hasValue;

        if (hasValue)
        {
            object = T();
            reader >> *object;
        }

        return reader;
    }
}