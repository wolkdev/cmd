# Command line manager

## Installation

Just drag and include the __cmd.hpp__ to your c++ project !

## Usage

```c++
// declare your cmd function
// 1 - name
// 2 - min arguments
// 3 - max arguments
// 4 - valid options
// 5 - help text
COMMAND(add, 1, 1, { { "local", "global" }, { "header-only" } }, "")
{
    // we know that we have exactly 1 argument

    if (_args[0].have_option("local"))
    {
        // do something different
    }
}

// simply execute commands
cmd::execute(command, argc, argv);
```
