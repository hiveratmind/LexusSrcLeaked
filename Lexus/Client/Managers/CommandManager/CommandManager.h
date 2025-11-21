#pragma once
#include "Commands/CommandBase.h"

#include "Commands/BindCommand.h"
#include "Commands/ConfigCommand.h"
#include "Commands/EjectCommand.h"
#include "Commands/UnbindCommand.h"
#include "Commands/HelpCommand.h"
#include "Commands/TeleportCommand.h"
#include "Commands/ToggleCommand.h"
#include "Commands/BindListCommand.h"
#include "Commands/PanicCommand.h"
#include "Commands/VClipCommand.h"
#include "Commands/ModulesCommand.h"
#include "Commands/TpPlayerCommand.h"

class CommandManager {
public:
	static inline std::vector<CommandBase*> commandList;
	static inline char prefix = '.';

	static void init();
	static void shutdown();
	static void execute(const std::string& message);
};