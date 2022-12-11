#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <fstream>
#include <ctime>
#include <sstream>
#include <cmath>

using namespace std;

struct frame
{
    bool accessed;
    bool dirty;
    int last_used;
    int age;
    int page;
    int factor;
    vector<int> reuse_list;
};

int frame_num;
vector<frame> frames;
unordered_map<int, int> record;
int page_fault = 0;
string filename;
int (*func)();
int access_num = 0;
bool enable_prepageing = false;
namespace SC
{
    int frame_pointer = 0;
}
namespace optimal
{
    bool is_OPT;
    int now_position;
    vector<int> future_list;
}
namespace my
{
    int max_factor;
}

int get_frame(int page)
{
    for (int i = 0; i < frame_num; ++i)
    {
        if (frames[i].page == page)
            return i;
    }
    return -1;
}

int FIFO()
{
    int frame_num = 0;
    int age = INT32_MAX;
    for (int i = 0; i < frames.size(); i++)
    {
        if (frames[i].age < age)
        {
            age = frames[i].age;
            frame_num = i;
        }
    }
    return frame_num;
}

int LRU()
{
    int frame_num = 0;
    int last_used = INT32_MAX;

    for (int i = 0; i < frames.size(); i++)
    {
        if (frames[i].last_used < last_used)
        {
            last_used = frames[i].last_used;
            frame_num = i;
        }
    }
    return frame_num;
}

int OPT()
{
    int frame_num = 0;

    int max_time = 0;
    int size = optimal::future_list.size();
    for (int j = 0; j < frames.size(); j++)
    {
        int name = frames[j].page;
        // cout<<name<<endl;

        for (int i = optimal::now_position + 1; i < size; i++)
        {
            if (name == optimal::future_list[i] || i == size - 1)
            {
                if (i - optimal::now_position > max_time)
                {
                    max_time = i - optimal::now_position;
                    frame_num = j;
                }
                break;
            }
        }
    }
    return frame_num;
}
int LFU()
{
    int frame_num = 0;
    int freq = INT32_MAX;
    for(int i=0;i<frames.size();++i){
        int count=0;
        for(auto iter=frames[i].reuse_list.begin();iter!=frames[i].reuse_list.end();++iter)
        {
            if(*iter>access_num-2*frames.size()){
                count++;
            }    
        }
        if(count<freq){
            freq=count;
            frame_num=i;
        }
    }
    return frame_num;
}
int Random()
{
    return rand() % frames.size();
}
int Second_Chance()
{
    while (1)
    {
        if (frames[SC::frame_pointer].accessed == 0)
        {
            return SC::frame_pointer;
        }
        else
        {
            frames[SC::frame_pointer].accessed = 0;
            SC::frame_pointer = (SC::frame_pointer + 1) % frames.size();
        }
    }
}

int aging()
{
    for (int k = 0; k < my::max_factor + 1; ++k)
    {
        if (k)
        {
            for (int i = 0; i < frames.size(); ++i)
                frames[i].factor++;
        }
        for (int i = 0; i < frames.size(); ++i)
        {
            if (frames[i].factor == my::max_factor)
                return i;
        }
    }

    return -1;
}

int getopt(int argc, char *argv[])
{
    for (int i = 0; i < argc; ++i)
    {
        cout << argv[i] << " ";
    }
    cout << endl;
    if (argc < 7)
        return -1;
    for (int i = 1; i < argc; i++)
    {
        if (argv[i][0] == '-')
        {
            if (argv[i][1] == 'f')
            {
                filename = argv[++i];
            }
            else if (argv[i][1] == 'n')
            {
                int n = atoi(argv[++i]);
                frame_num = n;
            }
            else if (argv[i][1] == 'p')
            {
                string str = string(argv[++i]);
                if (str == "FIFO")
                {
                    func = &FIFO;
                }
                else if (str == "LRU")
                {
                    func = &LRU;
                }
                else if (str == "LFU")
                {
                    func = &LFU;
                }
                else if (str == "OPT")
                {
                    func = &OPT;
                    optimal::is_OPT = 1;
                    // 读入
                    cout << "OPT" << endl;
                }
                else if (str == "RAND")
                {
                    func = &Random;
                }
                else if (str == "Second")
                {
                    func = &Second_Chance;
                }
                else if (str == "aging")
                {
                    func = &aging;
                }
                else
                {
                    std::cout << "Policy Match Fail" << endl;
                    return -1;
                }
            }
            else if (argv[i][1] == 'e')
            {
                enable_prepageing = true;
            }
        }
    }
    my::max_factor = log2(frame_num) + 1;
    // my::max_factor = 3;
    return 0;
}

void replace_frame(int page, int frame_num)
{
    frames[frame_num] = {false, false, access_num, access_num, page, my::max_factor - 2};
    frames[frame_num].reuse_list.clear();
    frames[frame_num].reuse_list.push_back(access_num);
}

int main(int argc, char *argv[])
{
    srand(time(NULL));
    if (getopt(argc, argv) < 0)
    {
        std::cout << "Usage: ./main -f filename -n frame_num -p Policy" << endl;
        return -1;
    }

    // filename = "traces/sort1.txt";
    // frame_num = 256;
    // func = &LRU;

    std::cout << "Filename: " << filename << endl;
    std::cout << "Frame number: " << frame_num << endl;

    frames = vector<frame>(frame_num, {0, 0, -1, -1, 0, my::max_factor});
    fstream infile;
    fstream log;
    log.open("log.txt", ios::out | ios::in | ios::trunc);
    if (optimal::is_OPT)
    {
        infile.open(filename);
        while (infile.good())
        {
            string s_newpage;
            infile >> s_newpage;
            stringstream ss;
            int newpage;
            ss << s_newpage;
            ss >> hex >> newpage;
            optimal::future_list.push_back(newpage);
        }
        infile.close();

        for (optimal::now_position = 0; optimal::now_position < optimal::future_list.size(); optimal::now_position++)
        {
            bool r = 0;
            if (record.find(optimal::future_list[optimal::now_position]) == record.end())
            {
                record[optimal::future_list[optimal::now_position]] = 1;
            }
            else
            {
                record[optimal::future_list[optimal::now_position]]++;
            }
            int frame_num = get_frame(optimal::future_list[optimal::now_position]);
            if (frame_num < 0)
            {
                r = 1;
                page_fault++;
                frame_num = func();
                replace_frame(optimal::future_list[optimal::now_position], frame_num);
            }
            else
            {
                frames[frame_num].accessed = true;
                frames[frame_num].last_used = access_num;
                frames[frame_num].reuse_list.push_back(access_num);
            }
            log << "page "
                << ": " << optimal::future_list[optimal::now_position] << " -> frame_num :" << frame_num;
            if (r)
                log << " replacement";
            log << endl;
            access_num++;
        }
        std::cout << access_num << " references to " << record.size() << " unique pages" << endl;
        cout << "Page fault: " << page_fault << "/" << access_num << " Ratio: " << 1.0 * page_fault / access_num << endl;
        return 0;
    }
    infile.open(filename);

    if (!infile)
    {
        std::cout << "File open failed" << endl;
        return -1;
    }

    while (infile.good())
    {
        // cout<<access_num<<endl;
        if (enable_prepageing)
        {
        }
        string s_newpage;
        infile >> s_newpage;
        if (s_newpage == "")
            break;
        stringstream ss;
        int newpage;
        ss << s_newpage;
        ss >> hex >> newpage;
        if (record.find(newpage) == record.end())
        {
            record[newpage] = 1;
        }
        else
        {
            record[newpage]++;
        }
        int frame_num = get_frame(newpage);
        bool r = 0;
        if (frame_num < 0)
        {
            r = 1;
            page_fault++;
            frame_num = func();
            // if (frames[frame_num].page == 0)
            // {
            //     page_fault--;
            //     r = 0;
            // }
            replace_frame(newpage, frame_num);
        }
        else
        {
            frames[frame_num].accessed = true;
            frames[frame_num].last_used = access_num;
            frames[frame_num].reuse_list.push_back(access_num);
            frames[frame_num].factor = 0;
        }
        access_num++;
        log << "page " << s_newpage << ": " << newpage << " -> frame_num :" << frame_num;
        if (r)
            log << " replacement";
        log << endl;
    }
    infile.close();
    std::cout << access_num << " references to " << record.size() << " unique pages" << endl;
    std::cout << "Page fault: " << page_fault << "/" << access_num << " Ratio: " << 1.0 * page_fault / access_num << endl;
}