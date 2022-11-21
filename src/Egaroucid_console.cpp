﻿/*
	Egaroucid Project

	@file Egaroucid_console.cpp
		Main file for Console application
	@date 2021-2022
	@author Takuto Yamana (a.k.a. Nyanyan)
	@license GPL-3.0 license
*/

#include <iostream>
#include "engine/engine_all.hpp"
#include "console/console_all.hpp"

void init_console(Options options){
    thread_pool.resize(std::max(0, options.n_threads - 1));
    bit_init();
    mobility_init();
    hash_resize(DEFAULT_HASH_LEVEL, options.hash_level, options.show_log);
    stability_init();
    if (!evaluate_init(options.eval_file, options.show_log))
        std::exit(0);
    if (!options.nobook)
        book_init(options.book_file, options.show_log);
    if (options.show_log)
        std::cerr << "initialized" << std::endl;
}

int main(int argc, char* argv[]){
    std::vector<Commandline_option> commandline_options = get_commandline_options(argc, argv);
    Options options = get_options(commandline_options);
    print_special_commandline_options(commandline_options, &options);
    init_console(options);
    execute_special_commandline_tasks(commandline_options, &options);
    Board_info board;
    State state;
    board.reset();
    while (true)
        check_command(&board, &state, &options);
    return 0;
}
