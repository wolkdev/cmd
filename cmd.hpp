#ifndef CMD__HPP
#define CMD__HPP

#include <stdexcept>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#define INFINITE_ARGS -1

#define COMMAND(name, min, max, options, help)                                      \
void name ## _cmd(const std::vector<cmd::arg>& _args);                              \
int name ## _cmd_index = cmd::add_command(cmd::data(                                \
    #name, &name ## _cmd, min, max, options, help));                                \
void name ## _cmd(const std::vector<cmd::arg>& _args)

namespace cmd
{
    struct arg;
    struct data;
    typedef void (*function)(const std::vector<arg>&);
    typedef std::vector<std::vector<std::string>> option_container;

    struct arg
    {
        struct option
        {
            std::string string;

            option(const std::string& _string) : string(_string) {}

            bool match(const std::string& _string) const
            {
                if (string.size() == 0 || _string.size() == 0)
                {
                    return false;
                }
                else if (string.size() == 1)
                {
                    return string[0] == _string[0];
                }
                else
                {
                    return string == _string;
                }
            }
        };

        std::string string;
        std::vector<option> options;

        bool have_option(const std::string& _string) const
        {
            for (size_t i = 0; i < options.size(); i++)
            {
                if (options[i].match(_string))
                {
                    return true;
                }
            }

            return false;
        }

        void clear()
        {
            string.clear();
            options.clear();
        }

        bool empty() const { return string == "" && options.empty(); }
    };

    struct data
    {
        std::string string;
        std::string help;
        option_container options;
        function callback = nullptr;
        int maxArgCount = -1;
        int minArgCount = 0;

        data() { }

        data(
            const std::string& _string,
            function _callback,
            int _minArgCount,
            int _maxArgCount,
            option_container&& _options,
            std::string&& _help)
            :
            string(_string),
            callback(_callback),
            minArgCount(_minArgCount),
            maxArgCount(_maxArgCount),
            options(std::move(_options)),
            help(_help)
        {
        }

        bool should_display_help(const std::vector<arg>& _args)
        {
            return help != "" && _args.size() == 1 && _args[0].string == ""
                && _args[0].options.size() == 1
                && _args[0].options[0].match("help");
        }

        void display_help() { std::cout << std::endl << help << std::endl; }

        void check_args(const std::vector<arg>& _args)
        {
            bool match;
            std::string matched;

            if (_args.size() < (unsigned)minArgCount)
            {
                throw std::runtime_error("Not enough arguments !");
            }
            else if (maxArgCount > 0 && _args.size() > (unsigned)maxArgCount)
            {
                throw std::runtime_error("Too much arguments !");
            }

            for (size_t i = 0; i < _args.size(); i++)
            {
                for (size_t j = 0; j < _args[i].options.size(); j++)
                {
                    match = false;

                    if (help != "" && _args[i].options[j].match("help"))
                    {
                        match = true;
                        matched = "help";
                    }

                    for (size_t k = 0; k < options.size(); k++)
                    {
                        for (size_t l = 0; l < options[k].size(); l++)
                        {
                            if (_args[i].options[j].match(options[k][l]))
                            {
                                if (match)
                                {
                                    std::string message = "Conflict error : \""
                                        + options[k][l] + "\" and \"" + matched
                                        + "\"";

                                    throw std::runtime_error(message.c_str());
                                }

                                match = true;
                                matched = options[k][l];
                            }
                        }
                    }

                    if (!match)
                    {
                        std::string message = "No matching options for \""
                            + _args[i].options[j].string + "\"";

                        throw std::runtime_error(message.c_str());
                    }
                }
            }
        }
    };

    static std::vector<data> commands;

    static int add_command(data&& _data)
    {
        commands.push_back(std::move(_data));
        return commands.size() - 1;
    }

    static bool is_option(std::string _str, int& _index)
    {
        if (_str.size() > 0)
        {
            if (_str[0] == '-')
            {
                if (_str.size() > 1)
                {
                    if (_str[1] == '-')
                    {
                        if (_str.size() > 2)
                        {
                            _index = 2;
                            return true;
                        }
                    }
                    else
                    {
                        _index = 1;
                        return true;
                    }
                }
            }
        }

        return false;
    }

    static std::vector<arg> parse_args(int _argc, char const* _argv[])
    {
        std::vector<arg> args;
        arg current;

        std::string str;
        int index;

        for (int i = 0; i < _argc; i++)
        {
            str = _argv[i];

            if (is_option(str, index))
            {
                current.options.push_back(arg::option(str.substr(index)));
            }
            else
            {
                current.string = str;
                args.push_back(current);

                current.clear();
            }
        }

        if (!current.empty())
        {
            args.push_back(current);
        }

        return args;
    }

    static bool find_command(const std::string& _command, unsigned int& _index)
    {
        for (unsigned int i = 0; i < commands.size(); i++)
        {
            if (commands[i].string == _command)
            {
                _index = i;
                return true;
            }
        }

        return false;
    }

    static void execute(const std::string& _command, int _argc, char const* _argv[])
    {
        unsigned int index;

        if (find_command(_command, index))
        {
            try
            {
                const std::vector<arg>& args = parse_args(_argc, _argv);
                commands[index].check_args(args);

                if (commands[index].should_display_help(args))
                {
                    commands[index].display_help();
                }
                else
                {
                    commands[index].callback(args);
                }
            }
            catch (const std::exception& e)
            {
                std::cerr << e.what() << std::endl;
            }
        }
        else
        {
            std::cerr << "unknown command : " << _command << std::endl;
        }
    }
};

#endif // !CMD__HPP