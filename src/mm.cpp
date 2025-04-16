
#include "mm.h"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <chrono>
#include <thread>
#include <sstream>
#include <fstream>
#include <vector>

#include "bitboard.h"
#include "engine.h"
#include "position.h"

std::string time_()
{
    std::time_t now_c = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

    std::tm now_tm;
    localtime_s(&now_tm, &now_c);

    std::stringstream ss;
    ss << std::put_time(&now_tm, "%H:%M:%S");

    return ss.str();
}

void process_stdin(Status *status)
{
    std::string in;

    do
    {
        std::getline(std::cin, in);
        std::istringstream is(in);
        is >> in;

        if (in == "pause")
            *status = PAUSE;
        else if (in == "go")
            *status = GO;

    } while (in != "stop" && in != "quit");

    *status = QUIT;
}

void run_match(Match *match, Status *status) {
    match->run_games(status);
}

void Match::run_games(Status *status)
{    
    for (int i = 0; i < fens.size(); i++)
    {
        std::string fen = fens[i];

        printf("%s: Match %d: %s: %d %s: %d Draws: %d Games: %d/%d\n",
               time_().c_str(), m_id, e1.name().c_str(), e1.wins, e2.name().c_str(), e2.wins, draws, i, fens.size());

        pos.set(fen);

        Color e1_color = pos.side_to_move(), e2_color = !e1_color;

        std::string game_string = "position fen " + fen + " ";

        e1.write_to_stdin("ucinewgame\n" + game_string + "\n");
        e2.write_to_stdin("ucinewgame\n" + game_string + "\n");

        game_string += "moves ";
        log << game_string;

        while (*status != QUIT)
        {
            for (;*status == PAUSE; Sleep(100));

            Engine &engine = pos.side_to_move() == e1_color ? e1 : e2;

            std::string movestr = engine.best_move();
            Move        move    = pos.uci_to_move(movestr);

            if (move == Move::null())
            {
                log                            << std::endl
                    << uci_to_pgn(game_string) << std::endl
                    << pos.to_string()         << std::endl 
                    << engine.name() << ": " << movestr << " <- Invalid" << std::endl;

                failed = true;
                break;
            }

            game_string += movestr + " ";
            log << movestr << " ";

            e1.write_to_stdin(game_string + "\n");
            e2.write_to_stdin(game_string + "\n");

            pos.do_move(move);

            if (GameState g = pos.game_state(); g != ONGOING)
            {
                if (g == MATE)
                    engine.wins++;
                else
                    draws++;

                log << std::endl
                    << uci_to_pgn(game_string) << std::endl
                    << (g == MATE       ? "Checkmate"
                      : g == STALEMATE  ? "Stalemate"
                      : g == REPETITION ? "Repetition"
                      : g == FIFTY_MOVE ? "Fifty-move rule" : "?") << std::endl
                    << e1.name() << ": " << e1.wins << " " << e2.name() << ": " << e2.wins << " Draws: " << draws << "\n" << std::endl;

                break;
            }
        }

        if (*status == QUIT || failed) break;
    }

    e1.kill();
    e2.kill();

    std::cout << "Match " << m_id << (failed ? ": Engine error" : ": Done") << std::endl;
}

int main(int argc, char *argv[])
{
    Bitboards::init();
    Position::init();

    std::string name_1, name_2, fenpath = "lc01k.txt";
    int time    = DEFAULT_TIME;
    int threads = DEFAULT_THREADS;

    std::string tokens, token;

    for (int i = 1; i < argc; i++)
        tokens += std::string(argv[i]) + " ";

    std::istringstream args(tokens);

    if (!(args >> name_1 >> name_2))
    {
        std::cout << "Usage: MatchManager <engine1> <engine2> [options...]" << std::endl;
        return 1;
    }

    while (args >> token)
    {
        if (token.find("--time=") == 0)
        {
            time = std::stoi(token.substr(token.find('=') + 1));
        }
        else if (token.find("--threads=") == 0)
        {
            threads = std::stoi(token.substr(token.find('=') + 1));
        }
        else if (token.find("--fen=") == 0)
        {
            fenpath = token.substr(token.find('=') + 1);
        }
    }

    std::string path_1 = std::string("engines\\") + name_1 + ".exe";
    std::string path_2 = std::string("engines\\") + name_2 + ".exe";

    std::vector<Match*> matches;
    std::vector<std::thread> thread_pool;

    Status status = GO;
    std::thread t(process_stdin, &status);
    t.detach();

    for (int id = 0; id < threads; id++)
    {
        matches.push_back(new Match(path_1, path_2, time, id, fenpath));
        thread_pool.emplace_back(run_match, matches.back(), &status);
    }

    for (std::thread &thread : thread_pool)
        thread.join();

    int e1_wins = 0, e2_wins = 0, draws = 0;

    for (Match *m : matches)
    {
        e1_wins += m->e1.wins;
        e2_wins += m->e2.wins;
        draws += m->draws;

        delete m;
    }

    int total = e1_wins + e2_wins + draws, decisive = total - draws;
    
    double e1_winrate = decisive > 0 ? double(e1_wins) / decisive : 0.0;
    double e2_winrate = decisive > 0 ? double(e2_wins) / decisive : 0.0;

    printf("+-----------------+-------+----------+\n");
    printf("|     Outcome     |   #   | Win Rate |\n");
    printf("+-----------------+-------+----------+\n");

    printf("| %-16s|%6d |%8.2f%% |\n", name_1.c_str(), e1_wins, e1_winrate * 100);
    printf("| %-16s|%6d |%8.2f%% |\n", name_2.c_str(), e2_wins, e2_winrate * 100);
    printf("| Draws           |%6d |          |\n", draws);
    printf("| Total           |%6d |%6d ms |\n+-----------------+-------+----------+\n", total, time);
}
