# Command line manager

## Instalation

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

// initialize and execute commands
cmd commander;
commander.add(status_cmd);
commander.execute(command, argc, argv);
```
