#include "parsing/Error.h"

Error Error::justMessage(std::string const& filepath, std::string const& message)
{
  return Error(filepath, Position::invalid(), Position::invalid(), Error::Type::Error, message);
}