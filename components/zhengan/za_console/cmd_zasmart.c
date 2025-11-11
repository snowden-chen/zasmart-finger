#include "za_common.h"
#include "za_command.h"

#include "esp_console.h"
#include "argtable3/argtable3.h"

//static const char *TAG = "cmd_zasmart";

static struct {
    struct arg_str *key;
    struct arg_end *end;
} test_args;

/*********/

/*
static int motor_ever_commands(int argc, char **argv)
{
    za_command_excute("MotorInr Y ever");
    
    return 0;
}

static int motor_stop_commands(int argc, char **argv)
{
    za_command_excute("MotorInr Y stop");
    
    return 0;
}
*/

/*
static void register_motor_ever_commands(void)
{
    const esp_console_cmd_t cmd = {
        .command = "motor-ever",
        .help = "-> do cmd : MotorInr Y ever. ",
        .hint = NULL,
        .func = &motor_ever_commands,
    };

    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );

    return;
}

static void register_motor_stop_commands(void)
{
    const esp_console_cmd_t cmd = {
        .command = "motor-stop",
        .help = "-> do cmd : MotorInr Y stop. ",
        .hint = NULL,
        .func = &motor_stop_commands,
    };

    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );

    return;
}
*/

static int zasmart_inputcmds(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **) &test_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, test_args.end, argv[0]);
        return 1;
    }

    char *zacmds = (ZA_CHAR *)(test_args.key->sval[0]);

    za_command_excute(zacmds);

    return 0;
}

static void register_zasmart_inputcmds(void)
{
    test_args.key = arg_str1(NULL, NULL, "<cmd>", "cmd of the string to be inputed by control zasmart. \"...\" ");
    test_args.end = arg_end(2);

    const esp_console_cmd_t input_cmd = {
        .command = "sudo",
        .help = "exp. -> sudo [ like: 1. sudo poweroff , 2. sudo \"poweroff\" ]",
        .hint = NULL,
        .func = &zasmart_inputcmds,
        .argtable = &test_args
    };

    ESP_ERROR_CHECK(esp_console_cmd_register(&input_cmd));

    return;
}

static int zasmart_inputcmds_list(int argc, char **argv)
{
    printf("\n__ commands list __ \n\n");

    printf("sudo poweroff - turn off your power\n");
    printf("sudo reboot - restart you system.\n");
    printf("sudo ifconfig - got information.\n");
    printf("sudo update - update forcibly.\n");
    printf("sudo upgrade - upgrade with version.\n");
    printf("sudo factory - go to factory mode, and you could update from this mode.\n");
    printf("sudo default - forget all config params, like wifi etc.\n");
    printf("sudo \"gamectrl y xxx\" - for users , commands when playing games on web, to see the doc.\n");

    printf("\n__ end __ \n\n");

    return 0;
}

static void register_zasmart_inputcmds_list(void)
{
    test_args.key = arg_str1(NULL, NULL, "<cmd>", "the more info of cmd, do not support now.");
    test_args.end = arg_end(2);

    const esp_console_cmd_t input_cmd = {
        .command = "list",
        .help = "list commands of zasmart.",
        .hint = NULL,
        .func = &zasmart_inputcmds_list,
        .argtable = &test_args
    };

    ESP_ERROR_CHECK(esp_console_cmd_register(&input_cmd));

    return;
}

static void register_zasmart_cmds(void)
{
    register_zasmart_inputcmds();
    register_zasmart_inputcmds_list();

    return;
}

/*
static void register_zasmart_cmds(void)
{
    register_motor_ever_commands();
    register_motor_stop_commands();

    return;
}
*/

void register_zasmart(void)
{
    //register_zasmart_test();
    register_zasmart_cmds();

    return;
}
