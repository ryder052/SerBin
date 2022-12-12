#include "serbin.h"

using namespace serbin;
using namespace std;

class Custom
{
    unique_ptr<tuple<float, double, long long>> data = make_unique<tuple<float, double, long long>>();

    friend SerBin<ios::out>& operator<<(SerBin<ios::out>& writer, const Custom& object)
    {
        writer << object.data;
        return writer;
    }

    friend SerBin<ios::in>& operator>>(SerBin<ios::in>& reader, Custom& object)
    {
        reader >> object.data;
        return reader;
    }

public:
    template<typename T>
    void set(T something)
    {
        get<T>(*data) = something;
    }
};

int main()
{
    string filename("test.txt");

    {
        SerBin<ios::out> writer(filename);

        vector<optional<int>> data0 = { {}, 456, 7890 };
        map<string, bool> data1 = { {"Aurora", true }, {"Borealis", false}, { "Club", true} };
        unordered_set<wstring> data2 = { {L"Dread"}, {L"Elemental"}, {L"Fang"} };
        Custom custom;
        custom.set(67.f);
        custom.set(0.125678);
        custom.set(800009LL);

        writer << data0;
        writer << data1;
        writer << data2;
        writer << custom;
    }

    {
        SerBin<ios::in> reader(filename);

        vector<optional<int>> data0;
        map<string, bool> data1;
        unordered_set<wstring> data2;
        Custom custom;

        reader >> data0;
        reader >> data1;
        reader >> data2;
        reader >> custom;
    }
}
