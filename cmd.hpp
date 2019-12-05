#ifndef CMD__HPP
#define CMD__HPP

#include <iostream>
#include <string>
#include <unordered_map>
#include <utility>

#define COMMAND(name, min, max, options, help) \
void name ## _cmd_function(const std::vector<cmd::arg>& _args); \
cmd::data name ## _cmd(#name, &name ## _cmd_function, min, max, options, help); \
void name ## _cmd_function(const std::vector<cmd::arg>& _args)

#define INFINITE_ARGS -1

class cmd
{
    public:
    struct arg;
    struct data;
    typedef void (*function)(const std::vector<arg>&);

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
        std::vector<std::vector<std::string>> options;
        function callback = nullptr;
        int maxArgCount = -1;
        int minArgCount = 0;

        data() { }

        data(
            const std::string& _string,
            function _callback,
            int _minArgCount,
            int _maxArgCount,
            std::vector<std::vector<std::string>>&& _options,
            std::string&& _help) :
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
                throw std::exception("Not enough arguments !");
            }
            else if (maxArgCount > 0 && _args.size() > (unsigned)maxArgCount)
            {
                throw std::exception("Too much arguments !");
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

                                    throw std::exception(message.c_str());
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

                        throw std::exception(message.c_str());
                    }
                }
            }
        }
    };

    private:
    std::unordered_map<std::string, data> cmds;

    public:
    void add(const data& _cmd) { cmds[_cmd.string] = data(_cmd); }

    void execute(const std::string& _command, int _argc, char const* _argv[])
    {
        if (cmds.find(_command) != cmds.end())
        {
            try
            {
                const std::vector<arg>& args = parse_args(_argc, _argv);
                cmds[_command].check_args(args);

                if (cmds[_command].should_display_help(args))
                {
                    cmds[_command].display_help();
                }
                else
                {
                    cmds[_command].callback(args);
                }
            }
            catch (const std::exception& e)
            {
                std::cerr << e.what() << std::endl;
            }
        }
    }

    private:
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
};

#endif // !CMD__HPP