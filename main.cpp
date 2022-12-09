#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <fstream>
#include <ctime>

using namespace std;

struct frame
{
    bool R;
    bool M;
    int last_used;
    int age;
    string page;
};

int frame_num;
vector<frame> frames;
// unordered_map<string, int> page_table;
int page_fault = 0;
string filename;
int (*func)();
int access_num = 0;
bool enable_prepageing = false;

int get_frame(string page)
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

int OPT() {}
int LFU() {}
int Random()
{
    return rand() % frame_num;
}
int Second_Chance() {}
int Clock() {}

int getopt(int argc, char *argv[])
{
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
                    cout << "Policy Match Fail" << endl;
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

void replace_frame(string page, int frame_num)
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
        cout << "Usage: ./main -f filename -n frame_num -p Policy" << endl;
        return -1;
    }

    cout << "Filename: " << filename << endl;
    cout << "Frame number: " << frame_num << endl;

    frames = vector<frame>(frame_num, {false, false, 0, 0, ""});
    std::fstream infile;
    infile.open(filename);
    if (!infile)
    {
        cout << "File open failed" << endl;
        return -1;
    }

    while (infile.good())
    {
        // cout<<access_num<<endl;
        if (enable_prepageing)
        {
        }
        string newpage;
        infile >> newpage;
        int frame_num = get_frame(newpage);
        if (frame_num < 0)
        {
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
    }
    infile.close();
    cout << "Page fault: " << page_fault << "/" << access_num<< " Ratio: "<< 1.0*page_fault/access_num << endl;
}