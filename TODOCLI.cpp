#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <filesystem>
#include <windows.h>
#include <iomanip>
#include <cstring>

using namespace std;

namespace fs = filesystem;

struct Loaded_Task
{
    /* data */
    int id;
    char task[300];
    bool status;
};

string file_required = "List.dat"; // File to be used.

vector<Loaded_Task> task_cache; // I need to improve my pneumonics what the hell
Loaded_Task tc;

fs::path getExeDirectory()
{
    char buffer[MAX_PATH];
    GetModuleFileNameA(NULL, buffer, MAX_PATH);
    return fs::path(buffer).parent_path(); // Returns the folder containing the EXE
}

bool isNumber(string num)
{
    int len = num.size();

    for (int i = 0; i < len; i++)
    {
        if (!(num[i] >= '0' && num[i] <= '9'))
        {
            return false;
        }
    }

    return true;
}

void loadTask()
{

    task_cache.clear();

    fs::path fullPath = getExeDirectory() / file_required;

    if (!fs::exists(fullPath))
    {
        cout << "File not found at: " << fullPath << endl;
        return;
    }

    else
    {
        ifstream file(fullPath, ios::binary);

        if (file.is_open())
        {
            if (fs::file_size(fullPath) == 0)
            {
                file.close();
                return;
            }

            else
            {
                while (file.read((char *)&tc, sizeof(tc)))
                {
                    task_cache.push_back(tc);
                }
            }
        }

        file.close();
    }
}

bool addTask(string task)
{
    fs::path fullPath = getExeDirectory() / file_required;

    ofstream file(fullPath, ios::binary | ios::app);

    if (!file)
        return false;

    Loaded_Task tcn;

    tcn.status = false;
    tcn.id = task_cache.size() + 1;

    for (int i = 0; i < 300; i++)
        tcn.task[i] = 0;

    for (int i = 0; i < task.length() && i < 299; i++)
    {
        tcn.task[i] = task[i];
    }

    int lengthToTerm = task.length();

    if (lengthToTerm > 299)
        lengthToTerm = 299;

    tcn.task[lengthToTerm] = '\0';

    file.write((char *)&tcn, sizeof(tcn));
    file.close();

    task_cache.push_back(tcn);
    return true;
}

bool removeTask(int id)
{
    bool flag = false;

    fs::path exeDir = getExeDirectory();
    fs::path fullPath = exeDir / file_required;
    fs::path tempPath = exeDir / "temp.dat";

    if (!fs::exists(fullPath))
    {
        cout << "File not found at: " << fullPath << endl;
        return false;
    }

    ofstream tempfile(tempPath, ios::binary);

    int change_id = 0;

    for (int i = 0; i < task_cache.size(); i++)
    {
        Loaded_Task buffer_task = task_cache[i];

        if (buffer_task.id != id)
        {
            buffer_task.id -= change_id;

            tempfile.write((char *)&buffer_task, sizeof(buffer_task));
        }

        else
        {
            change_id = 1;
            flag = true;
        }
    }

    tempfile.close();

    fs::remove(fullPath);

    const char *name = file_required.c_str();

    fs::rename(tempPath, fullPath);

    loadTask();

    return flag;
}

bool markDone(int id)
{
    bool flag = false;

    fs::path exeDir = getExeDirectory();
    fs::path fullPath = exeDir / file_required;
    fs::path tempPath = exeDir / "temp.dat";

    if (!fs::exists(fullPath))
    {
        cout << "File not found at: " << fullPath << endl;
        return false;
    }

    ofstream tempfile(tempPath, ios::binary);

    for (int i = 0; i < task_cache.size(); i++)
    {
        Loaded_Task buffer_task = task_cache[i];

        if (buffer_task.id == id)
        {
            buffer_task.status = true;
            flag = true;
        }

        tempfile.write((char *)&buffer_task, sizeof(buffer_task));
    }

    tempfile.close();

    fs::remove(fullPath);

    const char *name = file_required.c_str();

    fs::rename(tempPath, fullPath);

    loadTask();

    return flag;
}

void display()
{

    if (task_cache.empty())
    {
        cout << "\nNo tasks found. Your list is empty!\n"
             << endl;
        return;
    }

    cout << "\n"
         << string(50, '-') << endl;
    cout << left << setw(5) << "ID" << setw(35) << "Task Description" << "Status" << endl;
    cout << string(50, '-') << endl;

    for (const auto &t : task_cache)
    {
        string statusText = t.status ? "[DONE]" : "[PENDING]";

        cout << left << setw(5) << t.id << setw(35) << t.task << statusText << endl;
    }

    cout << string(50, '-') << "\n"
         << endl;
}

void clearTasks()
{
    fs::path fullPath = getExeDirectory() / file_required;
    ofstream file(fullPath, ios::binary | ios::trunc); // 'trunc' wipes the file
    file.close();
    task_cache.clear();
    cout << "All tasks cleared." << endl;
}

int main(int argc, char *argv[])
{

    fs::path exeDir = getExeDirectory();
    fs::path fullPath = exeDir / file_required;

    if (!fs::exists(fullPath))
    {
        ofstream create_file(fullPath, ios::binary);
        create_file.close();
    }

    loadTask();

    if (argc < 2)
    {
        cout << "Usage: todo <command> [arguments]\nCommands: add, remove, upstat, displist" << endl;
        return 0;
    }

    string command = argv[1];

    if (command == "add")
    {
        if (argc < 3)
        {
            cout << "Error: No task description provided." << endl;
            return 1;
        }

        // Combine all remaining words into one task string
        string taskText = "";
        for (int i = 2; i < argc; i++)
        {
            taskText += argv[i];
            if (i < argc - 1)
                taskText += " "; // Add space between words
        }

        if (addTask(taskText))
            cout << "Added: " << taskText << endl;
    }
    else if (command == "displist")
    {
        display();
    }
    else if (command == "remove")
    {
        if (argc < 3)
            return 1;

        if (!isNumber(argv[2]))
        {
            cout << "Error: Please provide a valid numeric ID." << endl;
            return 1;
        }

        int id = stoi(argv[2]); // Convert string argument to int
        removeTask(id);
    }
    else if (command == "upstat")
    {
        if (argc < 3)
            return 1;

        if (!isNumber(argv[2]))
        {
            cout << "Error: Please provide a valid numeric ID." << endl;
            return 1;
        }

        int id = stoi(argv[2]);
        markDone(id);
    }
    else if (command == "clear")
    {
        clearTasks();
    }
    else
    {
        cout << "Unknown command: " << command << endl;
    }

    return 0;
}
