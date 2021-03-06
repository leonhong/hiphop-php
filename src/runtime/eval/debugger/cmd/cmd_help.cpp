/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#include <runtime/eval/debugger/cmd/cmd_help.h>

using namespace std;

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

void CmdHelp::HelpAll(DebuggerClient *client) {
  client->helpCmds(
    "Session Commands", "",
    "[m]achine",    "connects to an HPHPi server",
    "[t]hread",     "switches between different threads",
    "[q]uit",       "quits debugger",

    "Program Flow Control", "",
    "[b]reak",      "sets/clears/displays breakpoints",
    "[e]xception",  "catches/clears exceptions",
    "[r]un",        "starts over a program",
    "<Ctrl-C>",     "breaks program execution",
    "[c]ontinue *", "continues program execution",
    "[s]tep     *", "steps into a function call or an expression",
    "[n]ext     *", "steps over a function call or a line",
    "[o]ut      *", "steps out a function call",
    "[j]ump",       "jumps to specified line of code for execution",

    "Display Commands", "",
    "[p]rint",      "prints a variable's value",
    "[w]here",      "displays stacktrace",
    "[u]p",         "goes up by frame(s)",
    "[d]own",       "goes down by frame(s)",
    "[f]rame",      "goes to a frame",
    "[v]ariable",   "lists all local variables",
    "[g]lobal",     "lists all global variables",
    "[k]onstant",   "lists all constants",

    "Evaluation Commands", "",
    "=",            "prints right-hand-side's value",
    "${name}=",     "assigns a value to left-hand-side",
    "[<?]php",      "starts input of a block of PHP code",
    "?>",           "ends and evaluates a block a PHP code",
    "[a]bort",      "aborts input of a block of PHP code",
    "[z]end",       "evaluates the last snippet in Zend PHP",

    "Documentation and Source Code", "",
    "[i]nfo",       "displays documentations and other information",
    "[l]ist     *", "displays source codes",
    "[h]elp",       "displays this help",
    "?",            "displays this help",

    "Shell and Extended Commands", "",
    "! {cmd}",      "executes a shell command",
    "& {cmd}",      "records and replays macros",
    "x {cmd}",      "extended commands",
    "y {cmd}",      "user extended commands",

    NULL
  );

  client->helpBody("* These commands are replayable by just hitting return.");
}

void CmdHelp::HelpStarted(DebuggerClient *client) {
  client->helpTitle("Getting Started with Debugger");

  client->helpBody(
    "1. Quick Overview\n"
    "\n"
    "(1) from A to Z\n"
    "\n"
    "All built-in debugger commands are un-ambiguous with their first "
    "letters. Therefore, a single letter is sufficient to issue the "
    "command.\n"
    "\n"
    "(2) tab, tab, tab\n"
    "\n"
    "Use TAB to auto-complete.\n"
    "\n"
    "(3) help\n"
    "\n"
    "Use \"help\" to read more about command details.\n"
    "\n"
    "(4) info and list\n"
    "\n"
    "Use \"info\" and \"list\" commands to read more about source code."
  );

  client->helpBody(
    "2. Debugging local script\n"
    "\n"
    "The command to run a script normally looks like this,\n"
    "\n"
    "  ./hphpi -f myscript.php\n"
    "\n"
    "Simply add \"-m debug\" to run the script in debugger,\n"
    "\n"
    "  ./hphpi -m debug -f myscript.php\n"
    "\n"
    "Once started, set breakpoints like this,\n"
    "\n"
    "  hphpd> break myscript.php:10\n"
    "  hphpd> break foo()\n"
    "\n"
    "Then let it run, until it hits the breakpoints,\n"
    "\n"
    "  hphpd> run\n"
    "\n"
    "At breakpoints, examine variables or evaluate expressions,\n"
    "\n"
    "  hphpd> variable\n"
    "  hphpd> print $a\n"
    "  hphpd> =$a\n"
    "  hphpd> <?php print $a; ?>\n"
    "  hphpd> <?php\n"
    "   ..... print $a;\n"
    "   ..... ?>\n"
    "\n"
    "Optionally, modify variables like this,\n"
    "\n"
    "  hphpd> $a = 10\n"
    "  hphpd> <?php $a = 10; ?>\n"
    "  hphpd> <?php\n"
    "   ..... $a = 10;\n"
    "   ..... ?>\n"
    "\n"
    "Then let it continue, until it hits more breakpoints,\n"
    "\n"
    "  hphpd> continue\n"
    "\n"
    "Finally, quit debugger,\n"
    "\n"
    "  hphpd> quit"
  );

  client->helpBody(
    "3. Debugging sandbox\n"
    "\n"
    "Connect to an HPHPi server from command line,\n"
    "\n"
    "  ./hphpi -m debug --debug-host mymachine.com\n"
    "\n"
    "Or, connect from within debugger,\n"
    "\n"
    "  hphpd> machine connect mymachine.com\n"
    "\n"
    "This will try to attach to a default sandbox on that machine. "
    "\"Attaching\" means it will only debug web requests hitting that "
    "sandbox. To switch to a different sandbox,\n"
    "\n"
    "  mymachine> machine list\n"
    "  mymachine> machine attach 2\n"
    "\n"
    "In remote debugging mode, a breakpoint can be specific about an URL,\n"
    "\n"
    "  mymachine> break myscript.php:10@index.php\n"
    "  mymachine> break foo()@index.php\n"
    "\n"
    "You may connect to more than one machine and breakpoints will be "
    "shared by all of them."
  );

  client->helpBody(
    "4. Understanding dummy sandbox\n"
    "\n"
    "When a web request hits a breakpoint, debugger will run in a "
    "\"Web Request\" thread. Use \"thread\" command to display this "
    "information,\n"
    "\n"
    "  mymachine> thread\n"
    "\n"
    "What will debugger use when there is no web request thread that's "
    "active, but we need to set a breakpoint? We created so-called "
    "\"dummy sandbox\", purely for taking debugger commands when there is "
    "no active web request. When there is no active request, hit Ctrl-C to "
    "break debugger, and use \"thread\" to display dummy sandbox thread's "
    "information.\n"
    "\n"
    "  Ctrl-C\n"
    "  mymachine> thread\n"
    "\n"
    "In dummy sandbox, a PHP file can be pre-loaded, so that we can "
    "\"info\" functions and classes and execute certain code. This file is "
    "specified on server side by\n"
    "\n"
    "  Eval.Debugger.StartupDocument = scripts/startup.php\n"
    "\n"
    "Dummy sandbox will always use currently attached sandbox's PHP files. "
    "When files are modified, simply reload them by\n"
    "\n"
    "  mymachine> continue\n"
    "  Ctrl-C"
  );

  client->helpBody(
    "5. Colors and Configuration\n"
    "\n"
    "By default, it will use emacs colors for dark background. To change "
    "them, run debugger at least once, then look for ~/.hphpd.hdf file. "
    "Replace \"Code\" node with,\n"
    "\n"
    "  Color {\n"
    "    Code : Color.Palette.vim\n"
    "  }\n"
    "\n"
    "Or, specify your own colors in different places of the configuration "
    "file."
  );
}

void CmdHelp::list(DebuggerClient *client) {
  if (client->argCount() == 0) {
    client->addCompletion(DebuggerClient::GetCommands());
    client->addCompletion("tutorial");
    client->addCompletion("start");
  } else if (client->arg(1, "tutorial")) {
    client->addCompletion("on");
    client->addCompletion("off");
    client->addCompletion("auto");
  }
}

bool CmdHelp::help(DebuggerClient *client) {
  client->helpTitle("Help Command");
  client->helpCmds(
    "[h]elp [s]tart", "displays material for getting started",
    "[h]elp [t]utorial on|off|auto", "changing tutorial modes",
    NULL
  );
  client->helpBody(
    "Please read \"Getting Started\" material with '[h]elp [s]tart' for "
    "first time use to get yourself familiar with basics.\n"
    "\n"
    "Tutorial mode displays extra information when something didn't work "
    "as you expected. \"auto\" mode will display the same information just "
    "once. \"on\" mode will display it as long as you run into the same "
    "situation. \"off\" mode completely turns off all tutorial texts.\n"
    "\n"
    "To get detailed information of a command, type '{cmd} [h]elp' or '{cmd} "
    "?' or 'help {cmd}' or '? {cmd}'."
  );
  return true;
}

bool CmdHelp::onClient(DebuggerClient *client) {
  if (DebuggerCommand::onClient(client)) return true;

  if (client->argCount() == 0) {
    HelpAll(client);
    return true;
  }

  if (client->arg(1, "start")) {
    HelpStarted(client);
    return true;
  }

  if (client->arg(1, "tutorial")) {
    if (!processTutorial(client)) {
      return help(client);
    }
  }

  client->swapHelp();
  if (client->process()) {
    return true;
  }

  return help(client);
}

bool CmdHelp::processTutorial(DebuggerClient *client) {
  string mode = client->argValue(2);
  if (mode == "off") {
    client->setTutorial(-1);
  } else if (mode == "on") {
    client->setTutorial(1);
  } else if (mode == "auto") {
    client->setTutorial(0);
  } else {
    return false;
  }
  return true;
}

///////////////////////////////////////////////////////////////////////////////
}}
