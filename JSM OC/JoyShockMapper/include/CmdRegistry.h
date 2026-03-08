#pragma once

#include "JoyShockMapper.h"

#include <functional>
#include <map>
#include <memory>
#include <string_view>

// This is a base class for any Command line operation. It binds a command name to a parser function
// Derivatives from this class have a default parser function and performs specific operations.
class JSMCommand
{
public:
	// A parser function has a pointer to the command being processed and
	// the data string to process. It returns whether the data was recognized or not.
	typedef function<bool(JSMCommand* cmd, string_view data, string_view label)> ParseDelegate;

	// Assignments can be given tasks to perform before destroying themselves.
	// This is used for chorded press, sim presses and modeshifts to remove
	// themselves from their host variable when assigned NONE
	typedef function<void(JSMCommand& me)> TaskOnDestruction;

protected:
	// Parse functor to be assigned by derived class or overwritten
	// Use setter to assign
	ParseDelegate _parse;

	// help string to display about the command. Cannot be changed after construction
	string _help;

	// Some task to perform when this object is destroyed
	TaskOnDestruction _taskOnDestruction;

public:
	// Name of the command. Cannot be changed after construction.
	// I don't mind leaving this public since it can't be changed.
	const string _name;

	// The constructor should only have mandatory arguments. optional arguments are assigned using setters.
	JSMCommand(string_view name);

	virtual ~JSMCommand();

	// A parser needs to be assigned for the command to be run without crashing.
	// It returns a pointer to itself for setter chaining.
	virtual JSMCommand* setParser(ParseDelegate parserFunction);

	// Assign a help description to the command.
	// It returns a pointer to itself for setter chaining.
	virtual JSMCommand* setHelp(string_view commandDescription);

	virtual unique_ptr<JSMCommand> getModifiedCmd(char op, string_view chord);

	// Get the help string for the command.
	inline string_view help() const
	{
		return _help;
	}

	// Set a task to perform when this command is destroyed
	inline JSMCommand* setTaskOnDestruction(const TaskOnDestruction& task)
	{
		_taskOnDestruction = task;
		return this;
	}

	// Request this command to parse the command arguments. Returns true if the command was processed.
	virtual bool parseData(string_view arguments, string_view label);
};

// The command registry holds all JSMCommands object and should not care what the derived type is.
// It's capable of recognizing a command and requesting it to process arguments. That's it.
// It uses regular expression to breakup a command string in its various components.
// Currently it refuses to accept different commands with the same name but there's an
// argument to be made to use the return value of JSMCommand::parseData() to attempt multiple
// commands until one returns true. This can enable multiple parsers for the same command.
class CmdRegistry
{
private:
	typedef multimap<string_view, unique_ptr<JSMCommand>> CmdMap;

	// multimap allows multiple entries with the same keys
	CmdMap _registry;

	static string_view strtrim(string_view str);

	static bool findCommandWithName(string_view name, const CmdMap::value_type& pair);

public:
	CmdRegistry();

	// Not string_view because the string is modified inside
	bool loadConfigFile(string fileName);

	// Add a command to the registry. The regisrty takes ownership of the memory of this pointer.
	// You can use _ASSERT() on the return value of this function to make sure the commands are
	// accepted.
	bool add(JSMCommand* newCommand);

	bool Remove(string_view name);

	bool hasCommand(string_view name) const;

	bool isCommandValid(string_view line) const;

	// Process a command entered by the user
	// intentionally dont't use const ref
	void processLine(const string& line);

	// Fill vector with registered command names
	void GetCommandList(vector<string_view>& outList) const;

	// Return help string for provided command
	string_view GetHelp(string_view command) const;
};

// Macro commands are simple function calls when recognized. But it could do different things
// by processing arguments given to it.
class JSMMacro : public JSMCommand
{
public:
	// A Macro function has it's command object passed as argument.
	typedef function<bool(JSMMacro* macro, string_view arguments)> MacroDelegate;

protected:
	// The macro function to call when the command is recognized.
	MacroDelegate _macro;

	// The default parser for the command processes no arguments.
	static bool DefaultParser(JSMCommand* cmd, string_view arguments, string_view label);

public:
	JSMMacro(string_view name);

	// Assign a Macro function to run. It returns a pointer to itself
	// to chain setters.
	JSMMacro* SetMacro(MacroDelegate macroFunction);
};
