#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <fstream>
#include <ctime>
#include <sstream>

using namespace std;

struct frame
{
    bool R;
    bool M;
    int last_used;
    int age;
    int page;
};

int frame_num;
vector<frame> frames;
// unordered_map<string, int> page_table;
int page_fault = 0;
string filename;
int (*func)();
int access_num = 0;
bool enable_prepageing = false;

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
    int i = 0;
    for (; i < frames.size(); i++)
    {
        if (frames[i].last_used < last_used)
        {
            last_used = frames[i].last_used;
            frame_num = i;
        }
    }
    return frame_num;
}

int OPT() { return 0; }
int LFU() { return 0; }
int Random()
{
    return rand() % frame_num;
}
int Second_Chance() { return 0; }
int Clock() { return 0; }

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
                }
                else if (str == "RAND")
                {
                    func = &Random;
                }
                else if (str == "Second")
                {
                    func = &Second_Chance;
                }
                else if (str == "Clock")
                {
                    func = &Clock;
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
    return 0;
}

void replace_frame(int page, int frame_num)
{
    frames[frame_num].page = page;
    frames[frame_num].R = true;
    frames[frame_num].M = false;
    frames[frame_num].last_used = access_num;
    frames[frame_num].age = access_num;
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

    frames = vector<frame>(frame_num, {false, false, 0, 0, 0});
    fstream infile;
    infile.open(filename);
    fstream log;
    log.open("log.txt");

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

        int frame_num = get_frame(newpage);
        bool r = 0;
        if (frame_num < 0)
        {

            r = 1;
            page_fault++;
            frame_num = func();
            replace_frame(newpage, frame_num);
        }
        else
        {
            frames[frame_num].R = true;
            frames[frame_num].last_used = access_num;
        }
        access_num++;
        log << "page " << s_newpage << ": " << newpage << " -> frame_num :" << frame_num;
        if (r)
            log << " replacement";
        log << endl;
    }
    infile.close();
    std::cout << "Page fault: " << page_fault << "/" << access_num << " Ratio: " << 1.0 * page_fault / access_num << endl;
}