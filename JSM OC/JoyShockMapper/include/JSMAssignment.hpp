#pragma once

#include "JoyShockMapper.h"
#include "CmdRegistry.h" // for JSMCommand
#include "JSMVariable.hpp"
#include "PlatformDefinitions.h"

#include <iostream>
#include <regex>

// This class handles any kind of assignment command by binding to a JSM variable
// of the parameterized type T. If T is not a base type, implement the following
// functions to reuse the defaultParser:
// static istream &operator >>(istream & input, T &foobar) { ... }
// static ostream &operator <<(ostream & output, T foobar) { ... }
template<typename T>
class JSMAssignment : public JSMCommand
{
protected:
	// The display name is usually the same as the name, but in some cases it might be different.
	// For example the two GYRO_SENS assignment commands will display MIN_GYRO_SENS and MAX_GYRO_SENS respectively.
	const string _displayName;

	// Reference to an existing variable. Make sure the variable's lifetime is longer than
	// the command objects
	JSMVariable<T>& _var;

	virtual bool parseData(string_view arguments, string_view label) override
	{
		smatch results;
		_ASSERT_EXPR(_parse, L"There is no function defined to parse this command.");
		const string argStr(arguments);
		if (arguments.empty())
		{
			displayCurrentValue();
		}
		else if (arguments.compare(0, 4, "HELP") == 0 && !_help.empty())
		{
			// Show help.
			COUT << _help << '\n';
		}
		else if (regex_match(argStr, results, regex(R"(\s*=\s*(.*))")))
		{
			string assignment(results.empty() ? arguments : results[1].str());
			if (assignment.rfind("DEFAULT", 0) == 0)
			{
				_var.reset();
			}
			else if (!_parse(this, assignment, label))
			{
				CERR << "Error assigning ";
				COUT_INFO << assignment;
				CERR << " to " << _displayName << '\n';
				CERR << "See ";
				COUT_INFO << "HELP";
				CERR << " and ";
				COUT_INFO << "README";
				CERR << " commands for further details.\n";
			}
		}
		else if (!_help.empty())
		{
			// Parsing has failed.
			CERR << "Error when processing the assignment. See the ";
			COUT_INFO << "README";
			CERR << " for details on valid assignment values\n";
		}
		return true; // Command is completely processed
	}

	static bool modeshiftParser(ButtonID modeshift, JSMSetting<T>* setting, JSMCommand::ParseDelegate* parser, JSMCommand* cmd, string_view argument, string_view label)
	{
		if (setting && argument.compare("NONE") == 0)
		{
			setting->markModeshiftForRemoval(modeshift);
			COUT << "Modeshift " << modeshift << "," << cmd->_name << " has been removed.\n";
			return true;
		}
		return (*parser)(cmd, argument, label);
	}

	// The default parser uses the overloaded >> operator to parse
	// any base type. Custom types can also be extracted if you define
	// a static parse operation for it.
	static bool defaultParser(JSMCommand* cmd, string_view data, string_view label)
	{
		auto inst = dynamic_cast<JSMAssignment<T>*>(cmd);

		stringstream ss(data.data());
		// Read the value
		T value(inst->readValue(ss));
		if (!ss.fail())
		{
			T oldVal = inst->_var;
			inst->_var.set(value);
			inst->_var.updateLabel(label);

			// The assignment won't trigger my listener displayNewValue if
			// the new value after filtering is the same as the old.
			if (oldVal == inst->_var.value())
			{
				// So I want to do it myself.
				inst->displayNewValue(inst->_var);
			}

			// Command succeeded if the value requested was the current one
			// or if the new value is different from the old.
			return value == oldVal || inst->_var.value() != oldVal; // Command processed successfully
		}
		// Couldn't read the value
		return false;
	}

	virtual void displayNewValue(const T& newValue)
	{
		// See Specialization for T=Mapping at the end of this file
		COUT << _displayName << " has been set to " << newValue << '\n';
	}

	virtual void displayCurrentValue()
	{
		COUT << _displayName << " = " << _var.value() << '\n';
	}

	virtual T readValue(stringstream& in)
	{
		// Default value reader
		T value = T();
		in >> value;
		return value;
	}

	virtual unique_ptr<JSMCommand> getModifiedCmd(char op, string_view chord) override
	{
		stringstream ss(chord.data());
		ButtonID btn;
		ss >> btn;
		if (btn > ButtonID::NONE)
		{
			stringstream name;
			name << chord << op << _displayName;
			if (op == ',')
			{
				auto settingVar = dynamic_cast<JSMSetting<T>*>(&_var);
				if (settingVar)
				{
					//Create Modeshift
					auto chordAssignment = make_unique<JSMAssignment<T>>(name.str(), *settingVar->atChord(btn));
					chordAssignment->setHelp(_help)->setParser(bind(&JSMAssignment<T>::modeshiftParser, btn, settingVar, &_parse, placeholders::_1, placeholders::_2, placeholders::_3))
						->setTaskOnDestruction(bind(&JSMSetting<T>::processModeshiftRemoval, settingVar, btn));
					return chordAssignment;
				}
				auto buttonVar = dynamic_cast<JSMButton*>(&_var);
				if (buttonVar && btn > ButtonID::NONE)
				{
					auto chordedVar = buttonVar->atChord(btn);
					// The reinterpret_cast is required for compilation, but settings will never run this code anyway.
					auto chordAssignment = make_unique<JSMAssignment<T>>(name.str(), reinterpret_cast<JSMVariable<T>&>(*chordedVar));
					chordAssignment->setHelp(_help)->setParser(_parse)->setTaskOnDestruction(bind(&JSMButton::processChordRemoval, buttonVar, btn, chordedVar));
					// BE ADVISED! If a custom parser was set using bind(), the very same bound vars will
					// be passed along.
					return chordAssignment;
				}
			}
			else if (op == '+')
			{
				auto buttonVar = dynamic_cast<JSMButton*>(&_var);
				if (buttonVar && btn > ButtonID::NONE)
				{
					auto simPressVar = buttonVar->atSimPress(btn);
					auto simAssignment = make_unique<JSMAssignment<Mapping>>(name.str(), *simPressVar);
					simAssignment->setHelp(_help)->setParser(_parse)->setTaskOnDestruction(bind(&JSMButton::processSimPressRemoval, buttonVar, btn, simPressVar));
					// BE ADVISED! If a custom parser was set using bind(), the very same bound vars will
					// be passed along.
					return simAssignment;
				}
			}
			else if (op == '*')
			{
				auto buttonVar = dynamic_cast<JSMButton*>(&_var);
				if (buttonVar && btn > ButtonID::NONE)
				{
					auto diagPressVar = buttonVar->atDiagPress(btn);
					auto diagAssignment = make_unique<JSMAssignment<Mapping>>(name.str(), *diagPressVar);
					diagAssignment->setHelp(_help)->setParser(_parse)->setTaskOnDestruction(bind(&JSMButton::processDiagPressRemoval, buttonVar, btn, diagPressVar));
					// BE ADVISED! If a custom parser was set using bind(), the very same bound vars will
					// be passed along.
					return diagAssignment;
				}
			}
		}
		return JSMCommand::getModifiedCmd(op, chord);
	}

	unsigned int _listenerId;

public:
	JSMAssignment(string_view name, string_view displayName, JSMVariable<T>& var, bool inNoListener = false)
	  : JSMCommand(name)
	  , _var(var)
	  , _displayName(displayName)
	  , _listenerId(0)
	{
		// Child Classes assign their own parser. Use bind to convert instance function call
		// into a static function call.
		using namespace placeholders;
		setParser(bind(&JSMAssignment::defaultParser, _1, _2, _3));
		if (!inNoListener)
		{
			_listenerId = _var.addOnChangeListener(bind(&JSMAssignment::displayNewValue, this, placeholders::_1));
		}
	}

	JSMAssignment(string_view name, JSMVariable<T>& var, bool inNoListener = false)
	  : JSMAssignment(name, name, var, inNoListener)
	{
	}

	JSMAssignment(JSMSetting<T>& var, bool inNoListener = false)
	  : JSMAssignment(magic_enum::enum_name(var._id).data(), var, inNoListener)
	{
	}

	JSMAssignment(JSMButton& var, bool inNoListener = false)
	  : JSMAssignment(magic_enum::enum_name(var._id).data(), var, inNoListener)
	{
	}

	virtual ~JSMAssignment()
	{
		if (_listenerId != 0)
		{
			_var.removeOnChangeListener(_listenerId);
		}
	}

	// This setter enables custom parsers to perform assignments
	inline T operator=(T newVal)
	{
		return (_var = newVal);
	}
};

// Specialization for Mapping
template<>
void JSMAssignment<Mapping>::displayNewValue(const Mapping& newValue)
{
	COUT << _name << " mapped to " << newValue.description() << '\n';
}
