
#include "cmd.hpp"

COMMAND(status, 0, 1, cmd::option_container(), "")
{
    std::cout << "hello !" << std::endl;

    for (size_t i = 0; i < _args.size(); i++)
    {
        std::cout << _args[i].string << std::endl;
    }
}

int main(int _argc, char const* _argv[])
{
    // TODO :
    // - command line completion

    if (_argc > 1)
    {
        std::string command = _argv[1];
        int argc = _argc - 2;
        const char** argv = (argc > 0 ? &_argv[2] : nullptr);
        
        cmd::execute(command, argc, argv);
    }

    return 0;
}