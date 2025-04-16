
#ifndef ENGINE_H
#define ENGINE_H

#include <string>
#include <windows.h>
#include <fstream>

class Engine
{
public:
    Engine(const std::string &path, int thinktime, int id);
   ~Engine() { kill(); }

    void write_to_stdin(const std::string& message);
    void kill() { log.close(); write_to_stdin("stop\nquit\n"); }
    std::string read_stdout();
    std::string best_move();
    std::string name() const { return m_name; }

    int wins;

private:
    std::ofstream log;

    int    m_thinktime;
    int    m_id;
    HANDLE m_stdin;
    HANDLE m_stdout;

    std::string m_name;
};

#endif